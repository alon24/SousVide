#include "SmingCore.h"
#include <drivers/SSD1306_driver.h>

#include <InfoScreens.h>
#include <mqttHelper.h>
#include <utils.h>
#include <configuration.h>
#include <Libraries/OneWire/OneWire.h>

//#include <PID_v1.h>
//#include <PID_AutoTune_v0.h>
#include <MyPID.h>

// download urls, set appropriately
#define ROM_0_URL  "http://192.168.7.5:80/rom0.bin"
#define ROM_1_URL  "http://192.168.7.5:80/rom1.bin"
#define SPIFFS_URL "http://192.168.7.5:80/spiff_rom.bin"

rBootHttpUpdate* otaUpdater = 0;

void OtaUpdate_CallBack(bool result) {

	Serial.println("In callback...");
	if(result == true) {
		// success
		uint8 slot;
		slot = rboot_get_current_rom();
		if (slot == 0) slot = 1; else slot = 0;
		// set to boot new rom and then reboot
		Serial.printf("Firmware updated, rebooting to rom %d...\r\n", slot);
		rboot_set_current_rom(slot);
		System.restart();
	} else {
		// fail
		Serial.println("Firmware update failed!");
	}
}

void OtaUpdate() {

	uint8 slot;
	rboot_config bootconf;

	Serial.println("Updating...");

	// need a clean object, otherwise if run before and failed will not run again
	if (otaUpdater) delete otaUpdater;
	otaUpdater = new rBootHttpUpdate();

	// select rom slot to flash
	bootconf = rboot_get_config();
	slot = bootconf.current_rom;
	if (slot == 0) slot = 1; else slot = 0;

#ifndef RBOOT_TWO_ROMS
	// flash rom to position indicated in the rBoot config rom table
	otaUpdater->addItem(bootconf.roms[slot], ROM_0_URL);
#else
	// flash appropriate rom
	if (slot == 0) {
		otaUpdater->addItem(bootconf.roms[slot], ROM_0_URL);
	} else {
		otaUpdater->addItem(bootconf.roms[slot], ROM_1_URL);
	}
#endif

#ifndef DISABLE_SPIFFS
	// use user supplied values (defaults for 4mb flash in makefile)
	if (slot == 0) {
		otaUpdater->addItem(RBOOT_SPIFFS_0, SPIFFS_URL);
	} else {
		otaUpdater->addItem(RBOOT_SPIFFS_1, SPIFFS_URL);
	}
#endif

	// request switch and reboot on success
	//otaUpdater->switchToRom(slot);
	// and/or set a callback (called on failure or success without switching requested)
	otaUpdater->setCallback(OtaUpdate_CallBack);

	// start update
	otaUpdater->start();
}

void Switch() {
	uint8 before, after;
	before = rboot_get_current_rom();
	if (before == 0) after = 1; else after = 0;
	Serial.printf("Swapping from rom %d to rom %d.\r\n", before, after);
	rboot_set_current_rom(after);
	Serial.println("Restarting...\r\n");
	System.restart();
}

void initSpiff()
{
	// mount spiffs
	int slot = rboot_get_current_rom();
	#ifndef DISABLE_SPIFFS
		if (slot == 0)
		{
	#ifdef RBOOT_SPIFFS_0
			debugf("trying to mount spiffs at %x, length %d", RBOOT_SPIFFS_0 , SPIFF_SIZE);
			spiffs_mount_manual(RBOOT_SPIFFS_0, SPIFF_SIZE);
	#else
			debugf("trying to mount spiffs at %x, length %d", 0x100000, SPIFF_SIZE);
			spiffs_mount_manual(0x100000, SPIFF_SIZE);
	#endif
		}
		else
		{
	#ifdef RBOOT_SPIFFS_1
			debugf("trying to mount spiffs at %x, length %d", RBOOT_SPIFFS_1 , SPIFF_SIZE);
			spiffs_mount_manual(RBOOT_SPIFFS_1, SPIFF_SIZE);
	#else
			debugf("trying to mount spiffs at %x, length %d", SPIFF_SIZE);
			spiffs_mount_manual(0x300000, SPIFF_SIZE);
	#endif
		}
	#else
		debugf("spiffs disabled");
	#endif
	//WifiAccessPoint.enable(false);

	Serial.printf("\r\nCurrently running rom %d.\r\n", slot);
	Serial.println("Type 'help' and press enter for instructions.");
	Serial.println();
}

//Forward declerations
//mqtt
//void onMessageReceived(String topic, String message); // Forward declaration for our callback

//wifi
//void process();
void  connectOk();
void  connectFail();
void  handleCommands(String commands);
void  updateInitWebSockets(WebSocket client);
void  setRelayState(boolean state);

/* Pinout:
 * MISO GPIO12 - not needed
 *
 * MOSI GPIO13 CLK GPIO14 CS GPIO15 DC GPIO5 RST GPIO4 */

InfoScreens* infos;

//* SSD1306 - I2C
Base_Display_Driver* display;

//BaseDisplay* display;

MySousVideController *sousController;

//// If you want, you can define WiFi settings globally in Eclipse Environment Variables
//#ifndef WIFI_SSID
//	#define WIFI_SSID "PleaseEnterSSID" // Put you SSID and Password here
//	#define WIFI_PWD "PleaseEnterPass"
//#endif

//enum class OperationMode
//{
//	Manual = 0, Sousvide = 1,
//};

OperationMode operationMode = Manual;

//Timers
Timer procTimer;
Timer buttonTimer;
Timer displayTimer;
Timer keepAliveTimer;
Timer initTimer;
//Timer blinkTimer;
Timer heartBeat;

//textRect lastTimeRect;

time_t lastActionTime = 0;
String currentTime = "00:00:00";

float currentTemp = 0;

int totalActiveSockets = 0;

String name;
String ip;

//int currentInfoScreenIndex = 0;

HttpServer server;
FTPServer ftp;

BssList networks;
String network, password;
Timer connectionTimer;

bool relayState = false;

////Web Sockets ///////

void wsConnected(WebSocket& socket)
{
	totalActiveSockets++;

	// Notify everybody about new connection
	WebSocketsList &clients = server.getActiveWebSockets();
	for (int i = 0; i < clients.count(); i++) {
		updateInitWebSockets(clients[i]);
//		clients[i].sendString("New friend arrived! Total: " + String(totalActiveSockets));
	}
}

void wsMessageReceived(WebSocket& socket, const String& message)
{
//	Serial.printf("WebSocket message received:\r\n%s\r\n", message.c_str());
//	String response = "Echo: " + message;
//	socket.sendString(response);
	handleCommands(message);
}

void updateInitWebSockets(WebSocket client) {
	char buf[1000];
	sprintf(buf, "updatePID:[%s,%s,%s];updateSetPoint:%s;updateWIFI:%s,%s;relayState:%s",
			String(sousController->Kp, 1).c_str(),
			String(sousController->Ki, 1).c_str(),
			String(sousController->Kd, 1).c_str(),
			String(sousController->Setpoint, 1).c_str(),
			ActiveConfig.NetworkSSID.c_str(), ActiveConfig.NetworkPassword.c_str(),
			(relayState == true ? "true" : "false"));

	client.sendString(String(buf));
}

void updateWebSockets(String cmd) {
	WebSocketsList &clients = server.getActiveWebSockets();
	for (int i = 0; i < clients.count(); i++) {
		clients[i].sendString(cmd);
//		Serial.println("updateWebUI::relayState:" + String(relayState == true ? "true" : "false"));
//		clients[i].sendString("relayState:" + String(relayState == true ? "true" : "false"));
//		clients[i].sendString("updatetime:" + currentTime);
//		clients[i].sendString("temp:" + String(currentTemp));
	}
}

//should return the command and make the cmd string hold data only
String getCommandAndData(String &cmd) {
	int cmdEnd = cmd.indexOf(":");
//	debugf("in getCommand:: orig=%s, deli=%i", cmd.c_str(), cmdEnd );
	String retCmd = cmd.substring(0, cmdEnd);

	String data = cmd.substring(cmdEnd+1);
	cmd = data;

//	debugf("in getCommand::retcmd=%s, data=%s", retCmd.c_str(), cmd.c_str());

	return retCmd;
}


void handleCommands(String commands) {
//	debugf("handleCommands::now handling %s", commands.c_str());

	//iterate over commands
	Vector<String> commandToken;
	int numToken = splitString(commands, ';' , commandToken);

	for (int i = 0; i < numToken; ++i) {
		String command = commandToken.get(i);
		String parsedCmd = getCommandAndData(command);

		debugf("handleCommands::command=%s, data=%s", parsedCmd.c_str(), command.c_str());
		//command is now the just the data (stripped of the command itself)
//		debugf("handleCommands::original cmd=%s, parsed=%s, data=%s", command.c_str(), parsedCmd.c_str(), command.c_str());

		if(parsedCmd.equals("query")) {

		}
		else if (parsedCmd.equals("toggleSousvideOperation")) {
			if (command.equals("false")) {
				operationMode = Manual;
				updateWebSockets("relayState:false");
			} else {
				operationMode = Sousvide;
			}

			debugf("sous operation = %s", (operationMode == Sousvide ? "Sousvide" : " Manual"));
		}
		else if (parsedCmd.equals("toggleRelay")) {
//			String state = commands.substring(12);
//			setRelayState(state.equals("true"));

			setRelayState(command.equals("true"));

	//		Serial.println("handleCommands::toggeled relay " + String(state));
	//		relayState = state.equals("true") ? true : false;
	//		digitalWrite(relayPin, (state.equals("true") ? HIGH : LOW));

			Serial.println("handleCommand:: state.equals(true)==" + String(command.equals("true")));
			updateWebSockets("relayState:" + String(relayState == true ? "true" : "false"));
		}
		else if(parsedCmd.equals("change-val-SetPoint")) {
			double temp = atof(command.c_str());
			sousController->Setpoint = temp;
			infos->updateParamValue("Setpoint", command);
		}
		else if(parsedCmd.equals("change-val-p")) {
			double p = atof(command.c_str());
			sousController->Kp = p;
			infos->updateParamValue("Kp", command);
		}
		else if(parsedCmd.equals("change-val-i")) {
			double i = atof(command.c_str());
			sousController->Ki = i;
			infos->updateParamValue("Ki", command);
		}
		else if(parsedCmd.equals("change-val-d")) {
			double d = atof(command.c_str());
			sousController->Kd = d;
			infos->updateParamValue("Kd", command);
		}
		else if(parsedCmd.equals("saveSettings")) {
			ActiveConfig.Kp = sousController->Kp;
			ActiveConfig.Ki = sousController->Ki;
			ActiveConfig.Kd = sousController->Kd;
			ActiveConfig.Needed_temp = sousController->Setpoint;
			saveConfig(ActiveConfig);
			//TODO: save changes to file
		}
		else if(parsedCmd.equals("wifi")) {
			Vector<String> commandToken;
			int numToken = splitString(command, ',' , commandToken);
			ActiveConfig.NetworkSSID = commandToken.elementAt(0);
			ActiveConfig.NetworkPassword = commandToken.elementAt(1);
		}
		else if(parsedCmd.equals("connect")) {
			Vector<String> commandToken;
			int numToken = splitString(command, ',' , commandToken);
			ActiveConfig.NetworkSSID = commandToken.elementAt(0);
			ActiveConfig.NetworkPassword = commandToken.elementAt(1);
			saveConfig(ActiveConfig);
			debugf("Connecting to %s, %s", ActiveConfig.NetworkSSID.c_str(), ActiveConfig.NetworkPassword.c_str());
			WifiStation.enable(false);
			WifiStation.config(ActiveConfig.NetworkSSID, ActiveConfig.NetworkPassword);
			WifiStation.enable(true);
			infos->updateParamValue("ssid", "try:" + ActiveConfig.NetworkSSID);
			infos->updateParamValue("station", "reconnecting");
			WifiStation.waitConnection(connectOk, 25, connectFail); // We recommend 20+ seconds for connection timeout at start
		}
		else if(parsedCmd.equals("reboot")) {
			debugf("Rebooting");
			System.restart();
		}
	}
}

void handleQuery(String items) {
	Vector<String> commandToken;
	int numToken = splitString(items, ';' , commandToken);

	for (int i = 0; i < numToken; ++i) {

	}
}

void wsBinaryReceived(WebSocket& socket, uint8_t* data, size_t size)
{
	Serial.printf("wsBinaryReceived::Websocket binary data recieved, size: %d\r\n", size);
}

void wsDisconnected(WebSocket& socket)
{
	totalActiveSockets--;
}

////Web Sockets ///////

void setName(String newName)
{
	name = newName;
}

String getName()
{
	return name;
}

// For quickly check you can use: http://www.hivemq.com/demos/websocket-client/ (Connection= test.mosquitto.org:8080)
// MQTT client
//MqttClient mqtt("test.mosquitto.org", 1883, onMessageReceived);

mqttHelper *mqtt;

//// Publish our message
//void publishMessage(String topic, String payload)
//{
//	Serial.println("Let's publish message now! t=" + topic + ", p=" + payload);
//	mqtt->publishMessage("dood/" + topic, payload); // or publishWithQoS
//}
//
//void publishInit()
//{
//	String payload = "{\"mac\":\"esp-" + WifiAccessPoint.getMAC() + "\"}";
//	publishMessage("init", payload);
//}

String getValue(String data, char separator, int index)
{
	int found = 0;
	int strIndex[] = {
			0, -1  };
	int maxIndex = data.length()-1;
	for(int i=0; i<=maxIndex && found<=index; i++){
		if(data.charAt(i)==separator || i==maxIndex){
			found++;
			strIndex[0] = strIndex[1]+1;
			strIndex[1] = (i == maxIndex) ? i+1 : i;
		}
	}
	return found>index ? data.substring(strIndex[0], strIndex[1]) : "";
}

// Callback for messages, arrived from MQTT server
void onMessageReceived(String topic, String message)
{
	Serial.print(topic);
	Serial.print(":\r\n\t"); // Prettify alignment for printing
	Serial.println(message);
	//msg to all client
	if (topic.compareTo(MQTT_MAIN_TOPIC) == 0){
		    int start = 0;
		    int end = 0;
		    while (end <= message.length())
		    {
		    	end = message.indexOf(",", start);
		    	if (end == -1 ) {
		    		if (end < message.length()) {
		    			end = message.length();
		    		}
		    	}

		    	String s = message.substring(start, end);
		        start = end + 1;
//		        end = message.indexOf(",", start);
//		        Serial.println("tok: " + s);
		        String in = getValue(s, ',' , 0);
		        String oper = getValue(s, ':' , 1);
		        	Serial.println("tok: " + s + ", in=" + in + ",oper=" + oper);
		        	setRelayState(oper.compareTo("1"));
//		        	digitalWrite(relayPin, oper.compareTo("1") ? HIGH : LOW);
		        if (in.compareTo("out1")) {
		        }
//		        break;
		    }
	}
}

///// Display //////////////
#define say(a) ( Serial.print(a) )
#define sayln(a) (Serial.println(a))
void  refreshScreen();

volatile int lastEncoded = 0;
volatile long encoderValue = 0;

long lastencoderValue = 0;
int lastValue =-1000;

int lastMSB = 0;
int lastLSB = 0;

struct displayMode
{
	enum DisplayMode
	{
		Regular=1,
		Menu=2
	};
};

enum DisplayMode
{
	Display_Info=1,
	Display_Menu=2
};
DisplayMode currentDisplayMode = Display_Info;

//should check screen action
boolean shouldDimScreen = false;

/**
 * setup infoscreens moved by the rotary
 */
void  initInfoScreens() {
	// Add a new Page
	InfoPage* p1 = infos->createPage("Main");
	//Add a line item
	p1->createLine("Network")->addParam("time", currentTime)->t.x = getXOnScreenForString(currentTime, 1);
	InfoLine *apLine = p1->createLine("ap ");
	apLine->addParam("apState", "on: ", true, 4);
	apLine->addParam("apIp", "Not Connected");

	InfoLine* staLine = p1->createLine("ssid: ");
	staLine->addParam("staState", "off:");
	p1->createLine("sta:")->addParam("stationIp", "");

	InfoPage* p2 = infos->createPage("Sous");
	InfoLine* line = p2->createLine("SousVide");
	line->addParam("state", "off", true, 3);
	line->addParam("time", currentTime)->t.x = getXOnScreenForString(currentTime, 1);
	p2->createLine("Current Temp:")->addParam("temp", String(currentTemp, 2));
	p2->createLine("Setpoint Temp:")->addParam("Setpoint", String(sousController->Setpoint, 2));
	p2->createLine("Kp:")->addParam("kp", String(sousController->Kp, 1), true, 5);
	p2->createLine("Ki:")->addParam("kp", String(sousController->Ki, 1), true, 5);
	p2->createLine("Kd:")->addParam("kp", String(sousController->Kd, 1), true, 5);

	//add a list of static Values
	paramDataValues* ofOnVals = new paramDataValues();
	ofOnVals->addValue(new String("off:"));
	ofOnVals->addValue(new String("on: "));

	//assign the values to the id=station parameter
	infos->setEditModeValues("state", ofOnVals);
	infos->setEditModeValues("apState", ofOnVals);
	infos->setEditModeValues("staState", ofOnVals);

}

void refreshTimeForUi()
{
	if (currentDisplayMode == Display_Menu)
	{
		return;
	}

	infos->updateParamValue("time", currentTime);
//	display.display();

	updateWebSockets("updatetime:" + currentTime);
}

//void IRAM_ATTR updateTimeTimerAction()
void updateTimeTimerAction()
{
//	unsigned long start = millis();
	currentTime = SystemClock.now().toShortTimeString(true);
//	debugf("%s", currentTime.c_str());
//	debugf("mem %d",system_get_free_heap_size());
//	Serial.println(currentTime);

	refreshTimeForUi();
//	unsigned long end = millis();
//	debugf("Time took %lu",  (long)(end- start));
}

bool state = true;

void blink()
{
	digitalWrite(relayPin, state);
	state = !state;
}

void doInit() {
	mqtt->publishInit();
}

// Will be called when WiFi station was connected to AP
void connectOk()
{
	infos->updateParamValue("station", WifiStation.getIP().toString());
	infos->updateParamValue("ssid", WifiStation.getSSID());
//	Serial.println("I'm CONNECTED");
//
//	if (!mqtt)
//	{
//		// Run MQTT client
//		mqtt = new mqttHelper("test.mosquitto.org", 1883, onMessageReceived);
//		mqtt->start();
//	}
//
//	// Start publishing loop
//	procTimer.initializeMs(20 * 1000, doInit).startOnce(); // every 20 seconds
}



// Will be called when WiFi station timeout was reached
void connectFail()
{
	Serial.println("I'm NOT CONNECTED. Need help :(");
	infos->updateParamValue("station", "Disconnected");
	infos->updateParamValue("ssid", "no:" + ActiveConfig.NetworkSSID);
	WifiStation.enable(false);
	// .. some you code for device configuration ..
}

void onIndex(HttpRequest &request, HttpResponse &response)
{
	Serial.println("inside onindex");
	debugf("inside onindex");
	if (!fileExist("index.html"))
	{
		Serial.println("!fileExist(index)");
		debugf("!fileExist(index)");
		fileSetContent("index.html",
				"<h3>Please connect to FTP and upload files from folder 'web/build' (details in code)</h3>");
	}
	else
	{
		TemplateFileStream *tmpl = new TemplateFileStream("index.html");
		auto &vars = tmpl->variables();
		response.sendTemplate(tmpl); // will be automatically deleted
	}
}
//
//void onIpConfig(HttpRequest &request, HttpResponse &response)
//{
//	if (request.getRequestMethod() == RequestMethod::POST)
//	{
//		AppSettings.dhcp = request.getPostParameter("dhcp") == "1";
//		AppSettings.ip = request.getPostParameter("ip");
//		AppSettings.netmask = request.getPostParameter("netmask");
//		AppSettings.gateway = request.getPostParameter("gateway");
//		debugf("Updating IP settings: %d", AppSettings.ip.isNull());
//		AppSettings.save();
//	}
//
//	TemplateFileStream *tmpl = new TemplateFileStream("settings.html");
//	auto &vars = tmpl->variables();
//
//	bool dhcp = WifiStation.isEnabledDHCP();
//	vars["dhcpon"] = dhcp ? "checked='checked'" : "";
//	vars["dhcpoff"] = !dhcp ? "checked='checked'" : "";
//
//	if (!WifiStation.getIP().isNull())
//	{
//		vars["ip"] = WifiStation.getIP().toString();
//		vars["netmask"] = WifiStation.getNetworkMask().toString();
//		vars["gateway"] = WifiStation.getNetworkGateway().toString();
//	}
//	else
//	{
//		vars["ip"] = "192.168.1.77";
//		vars["netmask"] = "255.255.255.0";
//		vars["gateway"] = "192.168.1.1";
//	}
//
//	response.sendTemplate(tmpl); // will be automatically deleted
//}

void onFile(HttpRequest &request, HttpResponse &response)
{
	String file = request.getPath();
	if (file[0] == '/')
		file = file.substring(1);

	if (file[0] == '.')
		response.forbidden();
	else
	{
		response.setCache(86400, true); // It's important to use cache for better performance.
		response.sendFile(file);
	}
}

void onAjaxNetworkList(HttpRequest &request, HttpResponse &response)
{
//	JsonObjectStream* stream = new JsonObjectStream();
//	JsonObject& json = stream->getRoot();
//
//	json["status"] = (bool)true;
//
//	bool connected = WifiStation.isConnected();
//	json["connected"] = connected;
//	if (connected)
//	{
//		// Copy full string to JSON buffer memory
//		json.addCopy("network", WifiStation.getSSID());
//	}
//
//	JsonArray& netlist = json.createNestedArray("available");
//	for (int i = 0; i < networks.count(); i++)
//	{
//		if (networks[i].hidden) continue;
//		JsonObject &item = netlist.createNestedObject();
//		item.add("id", (int)networks[i].getHashId());
//		// Copy full string to JSON buffer memory
//		item.addCopy("title", networks[i].ssid);
//		item.add("signal", networks[i].rssi);
//		item.add("encryption", networks[i].getAuthorizationMethodName());
//	}
//
//	response.setAllowCrossDomainOrigin("*");
//	response.sendJsonObject(stream);
}
//
//bool flipRelay(){
//	relayState = !relayState;
//	digitalWrite(relayPin, relayState ? HIGH : LOW);
//	return relayState;
//}

void onDoCommand(HttpRequest &request, HttpResponse &response)
{
	Serial.println("onDoCommand");
	JsonObjectStream* stream = new JsonObjectStream();
	JsonObject& json = stream->getRoot();


//	if (response.)
//	json["relayState"] = flipRelay();

	response.setAllowCrossDomainOrigin("*");
	response.sendJsonObject(stream);
}

//void makeConnection()
//{
//	WifiStation.enable(true);
//	WifiStation.config(network, password);
//
//	AppSettings.ssid = network;
//	AppSettings.password = password;
//	AppSettings.save();
//
//	network = ""; // task completed
//}

//void onAjaxConnect(HttpRequest &request, HttpResponse &response)
//{
//	JsonObjectStream* stream = new JsonObjectStream();
//	JsonObject& json = stream->getRoot();
//
//	String curNet = request.getPostParameter("network");
//	String curPass = request.getPostParameter("password");
//
//	bool updating = curNet.length() > 0 && (WifiStation.getSSID() != curNet || WifiStation.getPassword() != curPass);
//	bool connectingNow = WifiStation.getConnectionStatus() == eSCS_Connecting || network.length() > 0;
//
//	if (updating && connectingNow)
//	{
//		debugf("wrong action: %s %s, (updating: %d, connectingNow: %d)", network.c_str(), password.c_str(), updating, connectingNow);
//		json["status"] = (bool)false;
//		json["connected"] = (bool)false;
//	}
//	else
//	{
//		json["status"] = (bool)true;
//		if (updating)
//		{
//			network = curNet;
//			password = curPass;
//			debugf("CONNECT TO: %s %s", network.c_str(), password.c_str());
//			json["connected"] = false;
//			connectionTimer.initializeMs(1200, makeConnection).startOnce();
//		}
//		else
//		{
//			json["connected"] = WifiStation.isConnected();
//			debugf("Network already selected. Current status: %s", WifiStation.getConnectionStatusName());
//		}
//	}
//
//	if (!updating && !connectingNow && WifiStation.isConnectionFailed())
//		json["error"] = WifiStation.getConnectionStatusName();
//
//	response.setAllowCrossDomainOrigin("*");
//	response.sendJsonObject(stream);
//}

void startWebServer()
{
	server.listen(80);
	server.addPath("/", onIndex);
//	server.addPath("/ipconfig", onIpConfig);
	server.addPath("/ajax/get-networks", onAjaxNetworkList);
//	server.addPath("/ajax/connect", onAjaxConnect);
////	server.addPath("/ajax/doCommand", onDoCommand);
	server.setDefaultHandler(onFile);

	// Web Sockets configuration
	server.enableWebSockets(true);
	server.setWebSocketConnectionHandler(wsConnected);
	server.setWebSocketMessageHandler(wsMessageReceived);
	server.setWebSocketBinaryHandler(wsBinaryReceived);
	server.setWebSocketDisconnectionHandler(wsDisconnected);

}

void startFTP()
{
	if (!fileExist("index.html"))
		fileSetContent("index.html", "<h3>Please connect to FTP and upload files from folder 'web/build' (details in code)</h3>");

	// Start FTP server
	ftp.listen(21);
	ftp.addUser("me", "123"); // FTP account
}

// Will be called when system initialization was completed
void startServers()
{
	infos->updateParamValue("ap", WifiAccessPoint.getIP().toString());
	Serial.println("Starting servers");
////	startFTP();
	startWebServer();
}

void networkScanCompleted(bool succeeded, BssList list)
{
	if (succeeded)
	{
		for (int i = 0; i < list.count(); i++)
			if (!list[i].hidden && list[i].ssid.length() > 0) {
				networks.add(list[i]);
				debugf("Network found %s", list[i].ssid.c_str());
			}
	}
	networks.sort([](const BssInfo& a, const BssInfo& b){ return b.rssi - a.rssi; } );
}

void setRelayState(boolean state) {
	if (state != relayState) {
		relayState = state;
//		digitalWrite(relayPin, (relayState ? HIGH : LOW));
		digitalWrite(relayPin, (relayState ? LOW : HIGH));
	}
}

//Temp
OneWire ds(dsTempPin);
Timer readTempTimer;

void IRAM_ATTR checkTempTriggerRelay(float temp) {
	//move on this only when Sousvide, do not interfere with manual mode
	if (operationMode == Sousvide)
	{
		int trigger = (int)sousController->Setpoint;
		if((int)temp < trigger && !relayState ) {
			debugf("current temp is below %i, so starting relay", temp);
//			Serial.println("temp is " + String(trigger) + ", starting relay");
			handleCommands("toggleRelay:true");
		} else if((int)temp >= trigger && relayState){
			debugf("current temp is %i, so stopping relay", temp);
//			Serial.println("temp is <  " + String(trigger) + ",stopping relay");
			handleCommands("toggleRelay:false");
		}
	}
}

//http://forum.arduino.cc/index.php?topic=72654.0
void readTempData();
Timer oneTempReadTimer;
byte addr[8];
byte data[12];
byte type_s;

void readAfterWait() {
	byte present = 0;

	// we might do a ds.depower() here, but the reset will take care of it.

	present = ds.reset();
	ds.select(addr);
	ds.write(0xBE);         // Read Scratchpad

	for ( int i = 0; i < 9; i++)
	{
		// we need 9 bytes
		data[i] = ds.read();
//		Serial.print(data[i], HEX);
//		Serial.print(" ");
	}

	// Convert the data to actual temperature
	// because the result is a 16 bit signed integer, it should
	// be stored to an "int16_t" type, which is always 16 bits
	// even when compiled on a 32 bit processor.
	int16_t raw = (data[1] << 8) | data[0];
	if (type_s)
	{
		raw = raw << 3; // 9 bit resolution default
		if (data[7] == 0x10)
		{
		  // "count remain" gives full 12 bit resolution
		  raw = (raw & 0xFFF0) + 12 - data[6];
		}
	} else {
		byte cfg = (data[4] & 0x60);
		// at lower res, the low bits are undefined, so let's zero them
		if (cfg == 0x00) raw = raw & ~7;  // 9 bit resolution, 93.75 ms
		else if (cfg == 0x20) raw = raw & ~3; // 10 bit res, 187.5 ms
		else if (cfg == 0x40) raw = raw & ~1; // 11 bit res, 375 ms
		//// default is 12 bit resolution, 750 ms conversion time
	}

	float celsius = (float)raw / 16.0;
//	fahrenheit = celsius * 1.8 + 32.0;

	//debugf("  Temperature = %f Celsius ", celsius);
	currentTemp = celsius;

	if(currentDisplayMode == Display_Info) {
		infos->updateParamValue("temp", String(currentTemp, 2));
//		display.display();
	}

	checkTempTriggerRelay(celsius);

//	unsigned long end = millis();
//	debugf("Temp took %lu",  (long)(end- start));

	updateWebSockets("temp:" + String(currentTemp));
	readTempTimer.initializeMs(1000, readTempData).startOnce();
}

void readTempData()
{
//	unsigned long start = millis();
	byte i;
	byte present = 0;
//	byte type_s;
//	byte data[12];
//	byte addr[8];
	float celsius, fahrenheit;

	ds.reset_search();
	if (!ds.search(addr))
	{
		Serial.println("No addresses found.");
		Serial.println();
		ds.reset_search();
		delay(250);
		readTempTimer.initializeMs(1000, readTempData).startOnce();
		return;
	}

//	Serial.print("Thermometer ROM =");
//	for( i = 0; i < 8; i++)
//	{
//		Serial.write(' ');
//		Serial.print(addr[i], HEX);
//	}

	if (OneWire::crc8(addr, 7) != addr[7])
	{
	  Serial.println("CRC is not valid!");
	  return;
	}
//	Serial.println();

	// the first ROM byte indicates which chip
	switch (addr[0]) {
	case 0x10:
//	  Serial.println("  Chip = DS18S20");  // or old DS1820
	  type_s = 1;
	  break;
	case 0x28:
//	  Serial.println("  Chip = DS18B20");
	  type_s = 0;
	  break;
	case 0x22:
//	  Serial.println("  Chip = DS1822");
	  type_s = 0;
	  break;
	default:
		debugf("Device is not a DS18x20 family device.");
//	  Serial.println("Device is not a DS18x20 family device.");
	  return;
	}

	ds.reset();
	ds.select(addr);
	ds.write(0x44, 1);        // start conversion, with parasite power on at the end


	oneTempReadTimer.initializeMs(1000, readAfterWait).startOnce();
}

//ThingSpeak
HttpClient thingSpeak;
void onDataSent(HttpClient& client, bool successful)
{
	if (successful)
		Serial.println("Success sent");
	else
		Serial.println("Failed");

	String response = client.getResponseString();
	Serial.println("Server response: '" + response + "'");
	if (response.length() > 0)
	{
		int intVal = response.toInt();

		if (intVal == 0)
			Serial.println("Sensor value wasn't accepted. May be we need to wait a little?");
	}
}

void sendData(int field, String data)
{
	if (thingSpeak.isProcessing())
		return; // We need to wait while request processing was completed
	thingSpeak.downloadString("https://api.thingspeak.com/update?api_key=NSOR6ZIK3WLWY4PIY&field" + String(1) + "=" + String(currentTemp), onDataSent);
//	thingSpeak.downloadString("http://api.thingspeak.com/update?key=7XXUJWCWYTMXKN3L&field1=" + String(sensorValue), onDataSent);
}

void ShowInfo() {
    Serial.printf("\r\nSDK: v%s\r\n", system_get_sdk_version());
    Serial.printf("Free Heap: %d\r\n", system_get_free_heap_size());
    Serial.printf("CPU Frequency: %d MHz\r\n", system_get_cpu_freq());
    Serial.printf("System Chip ID: %x\r\n", system_get_chip_id());
    Serial.printf("SPI Flash ID: %x\r\n", spi_flash_get_id());
    //Serial.printf("SPI Flash Size: %d\r\n", (1 << ((spi_flash_get_id() >> 16) & 0xff)));
}

void serialCallBack(Stream& stream, char arrivedChar,
		unsigned short availableCharsCount)
{
	if (arrivedChar == '\n')
	{
		char str[availableCharsCount];
		for (int i = 0; i < availableCharsCount; i++)
		{
			str[i] = stream.read();
			if (str[i] == '\r' || str[i] == '\n')
			{
				str[i] = '\0';
			}
		}

		if (!strcmp(str, "connect"))
		{
			// connect to wifi
			WifiStation.config(WIFI_SSID, WIFI_PWD);
			WifiStation.enable(true);
		}
		else if (!strcmp(str, "ip"))
		{
			Serial.printf("ip: %s mac: %s\r\n",
					WifiStation.getIP().toString().c_str(),
					WifiStation.getMAC().c_str());
		}
		else if (!strcmp(str, "ota"))
		{
//			OtaUpdate();
		}
		else if (!strcmp(str, "switch"))
		{
//			Switch();
		}
		else if (!strcmp(str, "restart"))
		{
			System.restart();
		}
		else if (!strcmp(str, "ls"))
		{
			Vector<String> files = fileList();
			Serial.printf("filecount %d\r\n", files.count());
			for (unsigned int i = 0; i < files.count(); i++)
			{
				Serial.println(files[i]);
			}
		}
		else if (!strcmp(str, "cat"))
		{
			Vector<String> files = fileList();
			if (files.count() > 0)
			{
				Serial.printf("dumping file %s:\r\n", files[0].c_str());
				Serial.println(fileGetContent(files[0]));
			}
			else
			{
				Serial.println("Empty spiffs!");
			}
		}
		else if (!strcmp(str, "info"))
		{
			ShowInfo();
		}
		else if (!strcmp(str, "help"))
		{
			Serial.println();
			Serial.println("available commands:");
			Serial.println("  help - display this message");
			Serial.println("  ip - show current ip address");
			Serial.println("  connect - connect to wifi");
			Serial.println("  restart - restart the esp8266");
			Serial.println("  switch - switch to the other rom and reboot");
			Serial.println("  ota - perform ota update, switch rom and reboot");
			Serial.println("  info - show esp8266 info");
			#ifndef DISABLE_SPIFFS
						Serial.println("  ls - list files in spiffs");
						Serial.println("  cat - show first file in spiffs");
			#endif
			Serial.println();
		}
		else
		{
			Serial.println("unknown command");
		}
	}

}

void initFromConfig() {
	sousController->Setpoint = ActiveConfig.Needed_temp;
	sousController->Kp = ActiveConfig.Kp;
	sousController->Ki = ActiveConfig.Ki;
	sousController->Kd = ActiveConfig.Kd;

}

const String WS_HEARTBEAT = "--heartbeat--";

void sendHeartBeat() {
	 updateWebSockets(WS_HEARTBEAT);
}

void init()
{
	Serial.begin(SERIAL_BAUD_RATE); // 115200
	Serial.systemDebugOutput(true); // Debug output to serial

	initSpiff();

//	//Change CPU freq. to 160MHZ
//	System.setCpuFrequency(eCF_160MHz);

	Wire.pins(sclPin, sdaPin);

	ActiveConfig = loadConfig();

	sousController = new MySousVideController();
	initFromConfig();


	debugf("======= SousVide ==========");
	Serial.println();

	display = new SSD1306_Driver(4);
	display->init();

	initInfoScreens();
	infos = new InfoScreens(display);
	initInfoScreens();
	infos->initMFButton(encoderSwitchPin);


	pinMode(encoderCLK, INPUT_PULLUP);
	pinMode(encoderDT, INPUT_PULLUP);
	digitalWrite(encoderCLK, HIGH); //turn pullup resistor on
	digitalWrite(encoderDT, HIGH); //turn pullup resistor on

	pinMode(relayPin, OUTPUT);
//	setRelayState(false);
	digitalWrite(relayPin, HIGH);

	pinMode(encoderSwitchPin, INPUT);
	digitalWrite(encoderSwitchPin, HIGH); //turn pullup resistor on

	DateTime date = SystemClock.now();
	lastActionTime = date.Milliseconds;

	//ilan
	keepAliveTimer.initializeMs(1000, updateTimeTimerAction).start();

//	AppSettings.load();

	ds.begin(); // It's required for one-wire initialization!
	readTempTimer.initializeMs(1000, readTempData).startOnce();

	WifiStation.enable(true);
	WifiStation.config(ActiveConfig.NetworkSSID, ActiveConfig.NetworkPassword);
	infos->updateParamValue("ssid", ActiveConfig.NetworkSSID);
//	if (AppSettings.exist())
//	{
//		WifiStation.config(AppSettings.ssid, AppSettings.password);
////		if (!AppSettings.dhcp && !AppSettings.ip.isNull())
////			WifiStation.setIP(AppSettings.ip, AppSettings.netmask, AppSettings.gateway);
//	}

//	debugf("net=%s, pass=%s", WIFI_SSID, WIFI_PWD);

//	WifiStation.config(WIFI_SSID, WIFI_PWD);

//	WifiStation.startScan(networkScanCompleted);


	// Start AP for configuration
	WifiAccessPoint.enable(true);
	WifiAccessPoint.config("Sousvide Config", "", AUTH_OPEN);

	// Run WEB server on system ready
//	System.onReady(startServers);
	startServers();
	WifiStation.waitConnection(connectOk, 20, connectFail); // We recommend 20+ seconds for connection timeout at start

	Serial.setCallback(serialCallBack);

	heartBeat.initializeMs(4000, sendHeartBeat).start();
}
