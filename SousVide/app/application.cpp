#include "SmingCore.h"
#include <libraries/InfoScreens/SSD1306_driver.h>

#include <libraries/InfoScreens/InfoScreens.h>
#include <mqttHelper.h>
#include <utils.h>
#include <configuration.h>

//#include <PID_v1.h>
//#include <PID_AutoTune_v0.h>
#include "../include/pid/SousVideController.h"
#include <SousvideCommand.h>

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

InfoScreens* infos;

//* SSD1306 - I2C
Base_Display_Driver* display;
//SousVideController *sousController;

//OperationMode operationMode = Manual;

//Timers
Timer procTimer;
Timer keepAliveTimer;
Timer initTimer;
Timer heartBeat;
time_t lastActionTime = 0;
String currentTime = "00:00:00";

//float currentTemp = 0;

int totalActiveSockets = 0;

String name;
String ip;

HttpServer server;
FTPServer ftp;

BssList networks;
String network, password;
Timer connectionTimer;

//bool relayState = false;


void handleSousInfoUpdates(String param, String value) {

}

SousvideCommand sousCommand(RELAY_PIN, DS_TEMP_PIN, handleSousInfoUpdates);

////Web Sockets ///////

void wsConnected(WebSocket& socket)
{
	totalActiveSockets++;
	// Notify everybody about new connection
	WebSocketsList &clients = server.getActiveWebSockets();
	for (int i = 0; i < clients.count(); i++) {
		updateInitWebSockets(clients[i]);
	}
}

void wsMessageReceived(WebSocket& socket, const String& message)
{
	handleCommands(message);
}

void updateInitWebSockets(WebSocket client) {
	char buf[1000];
	sprintf(buf, "updatePID:[%s,%s,%s];updateSetPoint:%s;updateWIFI:%s,%s;relayState:%s",
			String(sousCommand.sousController->Kp, 1).c_str(),
			String(sousCommand.sousController->Ki, 1).c_str(),
			String(sousCommand.sousController->Kd, 1).c_str(),
			String(sousCommand.sousController->Setpoint, 1).c_str(),
			ActiveConfig.NetworkSSID.c_str(), ActiveConfig.NetworkPassword.c_str(),
			(sousCommand.relayState == true ? "true" : "false"));

	client.sendString(String(buf));
}

void updateWebSockets(String cmd) {
	WebSocketsList &clients = server.getActiveWebSockets();
	for (int i = 0; i < clients.count(); i++) {
		clients[i].sendString(cmd);
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
////	debugf("handleCommands::now handling %s", commands.c_str());
//
//	//iterate over commands
//	Vector<String> commandToken;
//	int numToken = splitString(commands, ';' , commandToken);
//
//	for (int i = 0; i < numToken; ++i) {
//		String command = commandToken.get(i);
//		String parsedCmd = getCommandAndData(command);
//
//		debugf("handleCommands::command=%s, data=%s", parsedCmd.c_str(), command.c_str());
//		//command is now the just the data (stripped of the command itself)
////		debugf("handleCommands::original cmd=%s, parsed=%s, data=%s", command.c_str(), parsedCmd.c_str(), command.c_str());
//
//		if(parsedCmd.equals("query")) {
//
//		}
//		else if (parsedCmd.equals("toggleSousvideOperation")) {
//			if (command.equals("false")) {
//				operationMode = Manual;
//				updateWebSockets("relayState:false");
//			} else {
//				operationMode = Sousvide;
//			}
//
//			debugf("sous operation = %s", (operationMode == Sousvide ? "Sousvide" : " Manual"));
//		}
//		else if (parsedCmd.equals("toggleRelay")) {
////			String state = commands.substring(12);
////			setRelayState(state.equals("true"));
//
//			setRelayState(command.equals("true"));
//
//	//		Serial.println("handleCommands::toggeled relay " + String(state));
//	//		relayState = state.equals("true") ? true : false;
//	//		digitalWrite(relayPin, (state.equals("true") ? HIGH : LOW));
//
//			Serial.println("handleCommand:: state.equals(true)==" + String(command.equals("true")));
//			updateWebSockets("relayState:" + String(relayState == true ? "true" : "false"));
//		}
//		else if(parsedCmd.equals("change-val-SetPoint")) {
//			double temp = atof(command.c_str());
//			sousController->Setpoint = temp;
//			infos->updateParamValue("Setpoint", command);
//		}
//		else if(parsedCmd.equals("change-val-p")) {
//			double p = atof(command.c_str());
//			sousController->Kp = p;
//			infos->updateParamValue("Kp", command);
//		}
//		else if(parsedCmd.equals("change-val-i")) {
//			double i = atof(command.c_str());
//			sousController->Ki = i;
//			infos->updateParamValue("Ki", command);
//		}
//		else if(parsedCmd.equals("change-val-d")) {
//			double d = atof(command.c_str());
//			sousController->Kd = d;
//			infos->updateParamValue("Kd", command);
//		}
//		else if(parsedCmd.equals("saveSettings")) {
//			ActiveConfig.Kp = sousController->Kp;
//			ActiveConfig.Ki = sousController->Ki;
//			ActiveConfig.Kd = sousController->Kd;
//			ActiveConfig.Needed_temp = sousController->Setpoint;
//			saveConfig(ActiveConfig);
//			//TODO: save changes to file
//		}
//		else if(parsedCmd.equals("wifi")) {
//			Vector<String> commandToken;
//			int numToken = splitString(command, ',' , commandToken);
//			ActiveConfig.NetworkSSID = commandToken.elementAt(0);
//			ActiveConfig.NetworkPassword = commandToken.elementAt(1);
//		}
//		else if(parsedCmd.equals("connect")) {
//			Vector<String> commandToken;
//			int numToken = splitString(command, ',' , commandToken);
//			ActiveConfig.NetworkSSID = commandToken.elementAt(0);
//			ActiveConfig.NetworkPassword = commandToken.elementAt(1);
//			saveConfig(ActiveConfig);
//			debugf("Connecting to %s, %s", ActiveConfig.NetworkSSID.c_str(), ActiveConfig.NetworkPassword.c_str());
//			WifiStation.enable(false);
//			WifiStation.config(ActiveConfig.NetworkSSID, ActiveConfig.NetworkPassword);
//			WifiStation.enable(true);
//			infos->updateParamValue("ssid", "try:" + ActiveConfig.NetworkSSID);
//			infos->updateParamValue("station", "reconnecting");
//			WifiStation.waitConnection(connectOk, 25, connectFail); // We recommend 20+ seconds for connection timeout at start
//		}
//		else if(parsedCmd.equals("reboot")) {
//			debugf("Rebooting");
//			System.restart();
//		}
//	}
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

//should check screen action
boolean shouldDimScreen = false;

/**
 * setup infoscreens moved by the rotary
 */
void initInfoScreens() {
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
	p2->createLine("Current Temp:")->addParam("temp", "init");
	p2->createLine("Setpoint Temp:")->addParam("Setpoint", "init");
	p2->createLine("Kp:")->addParam("kp", "init", true, 5);
	p2->createLine("Ki:")->addParam("kp", "init", true, 5);
	p2->createLine("Kd:")->addParam("kp", "init", true, 5);

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
	infos->updateParamValue("time", currentTime);
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

//void blink()
//{
//	digitalWrite(relayPin, state);
//	state = !state;
//}

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

//void setRelayState(boolean state) {
//	if (state != relayState) {
//		relayState = state;
////		digitalWrite(relayPin, (relayState ? HIGH : LOW));
//		digitalWrite(relayPin, (relayState ? LOW : HIGH));
//	}
//}

////Temp
//DS18S20 ReadTemp;
//Timer readTempTimer;

//void IRAM_ATTR checkTempTriggerRelay(float temp) {
//	//move on this only when Sousvide, do not interfere with manual mode
//	if (operationMode == Sousvide)
//	{
//		int trigger = (int)sousController->Setpoint;
//		if((int)temp < trigger && !relayState ) {
//			debugf("current temp is below %i, so starting relay", temp);
////			Serial.println("temp is " + String(trigger) + ", starting relay");
//			handleCommands("toggleRelay:true");
//		} else if((int)temp >= trigger && relayState){
//			debugf("current temp is %i, so stopping relay", temp);
////			Serial.println("temp is <  " + String(trigger) + ",stopping relay");
//			handleCommands("toggleRelay:false");
//		}
//	}
//}

//void readData()
//{
//	uint8_t a;
//	uint64_t info;
//
//	if (!ReadTemp.MeasureStatus())  // the last measurement completed
//	{
//      if (ReadTemp.GetSensorsCount()) {  // is minimum 1 sensor detected ?
//	    Serial.println(" Reading temperature");
//	    //only read first
//	    int a=0;
//	    float celsius=0;
//	      Serial.print(" T");
//	      Serial.print(a+1);
//	      Serial.print(" = ");
//
//	      if (ReadTemp.IsValidTemperature(a))   // temperature read correctly ?
//	        {
//	    	  Serial.print(ReadTemp.GetCelsius(a));
//	    	  Serial.print(" Celsius, (");
//	    	  currentTemp = celsius;
//	    	  celsius = ReadTemp.GetCelsius(a);
//	    	  updateWebSockets("temp:" + String(celsius));
//	    	  infos->updateParamValue("temp", String(currentTemp, 2));
//	    	  checkTempTriggerRelay(celsius);
//	        }
//	      else
//	    	  Serial.println("Temperature not valid");
//
//	      ReadTemp.StartMeasure();  // next measure, result after 1.2 seconds * number of sensors
//      }
//	}
//	else
//		Serial.println("No valid Measure so far! wait please");
//}

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
//	thingSpeak.downloadString("https://api.thingspeak.com/update?api_key=NSOR6ZIK3WLWY4PIY&field" + String(1) + "=" + String(currentTemp), onDataSent);
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

//void initFromConfig() {
//	sousController->Setpoint = ActiveConfig.Needed_temp;
//	sousController->Kp = ActiveConfig.Kp;
//	sousController->Ki = ActiveConfig.Ki;
//	sousController->Kd = ActiveConfig.Kd;
//}

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
	System.setCpuFrequency(eCF_160MHz);

	//setup i2c pins
	Wire.pins(sclPin, sdaPin);

	ActiveConfig = loadConfig();

	sousCommand.initCommand(ActiveConfig.Needed_temp, ActiveConfig.Kp, ActiveConfig.Ki, ActiveConfig.Kd);
//	sousController = new SousVideController();
//	initFromConfig();

	debugf("======= SousVide ==========");
	Serial.println();

	display = new SSD1306_Driver(4);
	display->init();

	initInfoScreens();
	infos = new InfoScreens(display);
	initInfoScreens();
	infos->initMFButton(encoderSwitchPin);

//	pinMode(relayPin, OUTPUT);
////	setRelayState(false);
//	digitalWrite(relayPin, HIGH);

	DateTime date = SystemClock.now();
	lastActionTime = date.Milliseconds;

	//ilan
	keepAliveTimer.initializeMs(1000, updateTimeTimerAction).start();

//	AppSettings.load();

//	ds.begin(); // It's required for one-wire initialization!
//	readTempTimer.initializeMs(1000, readTempData).startOnce();

//	ReadTemp.Init(dsTempPin);  			// select PIN It's required for one-wire initialization!
//	ReadTemp.StartMeasure(); // first measure start,result after 1.2 seconds * number of sensors
//
//	readTempTimer.initializeMs(10000, readData).start();   // every 10 seconds

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
	heartBeat.initializeMs(4000, sendHeartBeat).start();
}
