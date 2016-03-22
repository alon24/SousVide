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
//void  handleCommands(String commands);
void  updateInitWebSockets(WebSocket &client);
void  setRelayState(boolean state);
void updateInfoOnStart();

InfoScreens* infos;

//* SSD1306 - I2C
Base_Display_Driver* display;
//SousVideController *sousController;

//OperationMode operationMode = Manual;

//Timers
Timer procTimer;
Timer currentWorkTimeTimer;
Timer initTimer;
//Timer heartBeat;
Timer updateWebSocketTimer;
//time_t lastActionTime = 0;
long currentWorkTime = 0;

int totalActiveSockets = 0;

String name;
String ip;

HttpServer server;
FTPServer ftp;

BssList networks;
String network, password;
Timer connectionTimer;

void handleSousInfoUpdates(String param, String value) {
//	debugf("*********** handleSousInfoUpdates %s=%s", param.c_str(), value.c_str());
	infos->updateParamValue(param.c_str(), value.c_str());
}

SousvideCommand sousCommand(RELAY_PIN, DS_TEMP_PIN, handleSousInfoUpdates);

////Web Sockets ///////

void wsConnected(WebSocket& socket)
{
//	debugf("******* wsConnected");
	totalActiveSockets++;
	// Notify everybody about new connection
//	WebSocketsList &clients = server.getActiveWebSockets();
//	for (int i = 0; i < clients.count(); i++) {
//		updateInitWebSockets(clients[i]);
//	}
	updateInitWebSockets(socket);
}

void wsMessageReceived(WebSocket& socket, const String& message)
{
//	debugf("******* Incomming ws message:%s",message.c_str());
//	handleCommands(message);
}

void updateInitAllClients() {
	WebSocketsList &clients = server.getActiveWebSockets();
	debugf("server has %i websocket clients", clients.size());
	for (int i = 0; i < clients.count(); i++) {
		updateInitWebSockets(clients[i]);
//		clients[i].sendString("updatePID:2,2,2");
	}
}

void updateInitWebSockets(WebSocket &client) {
//	DynamicJsonBuffer jsonBuffer;
//	JsonObject& root = jsonBuffer.createObject();
//	JsonObject& update = jsonBuffer.createObject();
//	root["PID"] = update;
//	update["P"] = String(sousCommand.sousController->Kp, 1);
//	update["I"] = String(sousCommand.sousController->Ki, 1);
//	update["D"] = String(sousCommand.sousController->Kd, 1);
//
//	update["SetPoint"] = String(sousCommand.sousController->Setpoint, 1);
//
//	String st = "";
//	root.prettyPrintTo(st);
//	client.sendString(st);

	char *buf = new char[200];
	sprintf(buf, "updatePID:%s,%s,%s;updateSetPoint:%s;relayState:%s;highlow:%s;currentTemp:%s",
			String(sousCommand.sousController->Kp, 1).c_str(),
			String(sousCommand.sousController->Ki, 1).c_str(),
			String(sousCommand.sousController->Kd, 1).c_str(),
			String(sousCommand.sousController->Setpoint, 1).c_str(),
////			ActiveConfig.NetworkSSID.c_str(),
////			ActiveConfig.NetworkPassword.c_str(),
			(sousCommand.relayState == true ? "true" : "false"),
			ActiveConfig.highlow == true ? "true" : "false",
			String(sousCommand.currentTemp,2).c_str());
////			,ActiveConfig.operationMode == Manual ?0:1)

	String ret = String(buf);
//	debugf("%s", ret.c_str());
	client.sendString(ret);
//	Serial.println("message sent");
	delete buf;
	//This is a simple test for just sending text!!!
//	client.sendString("updatePID:2,2,2");
}

void updateWebSockets(String cmd) {
	WebSocketsList &clients = server.getActiveWebSockets();
	for (int i = 0; i < clients.count(); i++) {
		clients[i].sendString(cmd);
	}
}

void updateWebSocketsFromTimer() {
	updateInitAllClients();
}

//should return the command and make the cmd string hold data onl
String getCommandAndData(String &cmd) {
	int cmdEnd = cmd.indexOf(":");
//	debugf("in getCommand:: orig=%s, deli=%i", cmd.c_str(), cmdEnd );
	String retCmd = cmd.substring(0, cmdEnd);

	String data = cmd.substring(cmdEnd+1);
	cmd = data;

//	debugf("in getCommand::retcmd=%s, data=%s", retCmd.c_str(), cmd.c_str());

	return retCmd;
}

void processAppCommands(Command inputCommand, CommandOutput* commandOutput)
{
    commandOutput->printf("App : %s\r\n",inputCommand.getCmdString().c_str());

    JsonObject & jsonCommand = inputCommand.getRoot();
    if  (jsonCommand.success())
    {
    	if(jsonCommand.containsKey("reboot")) {
    		debugf("jsonCommand=%s", jsonCommand["reboot"][0]);
			debugf("Rebooting");
			System.restart();
		}

    	if(jsonCommand.containsKey("saveSettings")) {
			ActiveConfig.Kp = sousCommand.sousController->Kp;
			ActiveConfig.Ki = sousCommand.sousController->Ki;
			ActiveConfig.Kd = sousCommand.sousController->Kd;
			ActiveConfig.Needed_temp = sousCommand.sousController->Setpoint;
			ActiveConfig.enabled = sousCommand.enabled;
			saveConfig(ActiveConfig);
			//TODO: save changes to file
		}

    	if(jsonCommand.containsKey("wifi")) {
//			ActiveConfig.NetworkSSID = commandToken[2];
//			ActiveConfig.NetworkPassword = commandToken[3];
		}
    }
    else
    {
        Serial.printf("App debug : %s\r\n",inputCommand.getCmdString().c_str() );

        Vector<String> commandToken;
        String cmd = inputCommand.getCmdString();
        int numToken = splitString(cmd, ' ' , commandToken);
        if(commandToken[1].equals("reset")) {
        	if (currentWorkTimeTimer.isStarted()) {
				currentWorkTimeTimer.stop();
			}

        	currentWorkTime = 0;
		}
		else if(commandToken[1].equals("play")) {
			if (!currentWorkTimeTimer.isStarted()) {
				currentWorkTimeTimer.start();
			}
		}
		else if(commandToken[1].equals("stop")) {
			if (currentWorkTimeTimer.isStarted()) {
				currentWorkTimeTimer.stop();
			}
		}
		else if(commandToken[1].equals("toggleEnable")) {
			//enable work
			if (currentWorkTimeTimer.isStarted()) {
				currentWorkTimeTimer.stop();
			}
		}
		else if(commandToken[1].equals("toggleDisable")) {
			//disable work
			if (currentWorkTimeTimer.isStarted()) {
				currentWorkTimeTimer.stop();
			}
		}
		else if(commandToken[1].equals("saveConfig")) {
			debugf("SaveConfig called");
			ActiveConfig.Kp = sousCommand.sousController->Kp;
			ActiveConfig.Ki = sousCommand.sousController->Ki;
			ActiveConfig.Kd = sousCommand.sousController->Kd;
			ActiveConfig.Needed_temp = sousCommand.sousController->Setpoint;
			ActiveConfig.enabled = sousCommand.enabled;
			ActiveConfig.highlow = sousCommand.highLow;
			ActiveConfig.operationMode = sousCommand.operationMode;
			saveConfig(ActiveConfig);
		}
		else if(commandToken[1].equals("loadConfig")) {
			debugf("loadConfig called");
			loadConfig();
			updateInfoOnStart();

			//TODO: check with this only for sending a simple message
			updateInitAllClients();
		}
    }
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

bool menuEventLister(paramStruct* data, ViewMode v, InfoScreenMenuAction actionType, String newValue) {
	if (actionType == InfoScreenMenuAction::InfoNextValue) {
		if (data->id == "Kp") {
			float currentKi = sousCommand.sousController->Kp;
			if (currentKi < 10) {
				currentKi = currentKi+0.1;
			}
			else {
				currentKi = 0;
			}
			debugf("current kp = %d", currentKi);
			sousCommand.sousController->Kp = currentKi;
			infos->updateParamValue(data->id, String(currentKi, 2));
			return true;
		}
		else if (data->id == "Ki") {
			float currentKi = sousCommand.sousController->Ki;
			if (currentKi < 10) {
				currentKi = currentKi+0.1;
			}
			else {
				currentKi = 0;
			}
			sousCommand.sousController->Ki = currentKi;
			infos->updateParamValue(data->id, String(currentKi, 2));
			return true;
		}
		else if (data->id == "Kd") {
			float currentKi = sousCommand.sousController->Kd;
			if (currentKi < 10) {
				currentKi = currentKi+0.1;
			}
			else {
				currentKi = 0;
			}
			sousCommand.sousController->Kd = currentKi;
			infos->updateParamValue(data->id, String(currentKi, 2));
			return true;
		}
	}


	return false;
}

/**
 * setup infoscreens moved by the rotary
 */
void initInfoScreens() {
	InfoPage* p2 = infos->createPage("Sous   ");
	InfoLine* line = p2->createLine("Cook");
	line->addParam("state", "off", true, 3);
	line->addParam("workTime", "00:00:00")->t.x = getXOnScreenForString("00:00:00", 1);
	p2->createLine("Current Temp:")->addParam("temp", "init");
	p2->createLine("Setpoint Temp:")->addParam("target", "init");
	p2->createLine("Kp:")->addParam("Kp", "init", true, 5);
	p2->createLine("Ki:")->addParam("Ki", "init", true, 5);
	p2->createLine("Kd:")->addParam("Kd", "init", true, 5);

	// Add a new Page
	InfoPage* p1 = infos->createPage("Main");
	//Add a line item
	p1->createLine("Network")->addParam("workTime", "00:00:00")->t.x = getXOnScreenForString("00:00:00", 1);
	InfoLine *apLine = p1->createLine("ap ");
	apLine->addParam("apState", "on: ", true, 4);
	apLine->addParam("apIp", "Not Connected");

	InfoLine* staLine = p1->createLine("ssid: ");
	staLine->addParam("staState", "off:");
	InfoLine* lsta = p1->createLine("sta:");
	lsta->addParam("stationIP", "unknown");
//	p1->createLine("sta:")->addParam("stationIp", "");


	//add a list of static Values
	paramDataValues* ofOnVals = new paramDataValues();
	ofOnVals->addValue(new String("off:"));
	ofOnVals->addValue(new String("on: "));

	//assign the values to the id=station parameter
	infos->setEditModeValues("state", ofOnVals);
	infos->setEditModeValues("apState", ofOnVals);
	infos->setEditModeValues("staState", ofOnVals);

	paramDataValues* stateVals = new paramDataValues();
	stateVals->addValue(new String("off"));
	stateVals->addValue(new String("on "));
	infos->setEditModeValues("state", stateVals);

	infos->setOnMenuEventDelegate(menuEventLister);
}

void refreshTimeForUi()
{
	infos->updateParamValue("workTime", getElapsedime(currentWorkTime));
//	updateWebSockets("updatetime:" + currentTime);
}

void updateWorkTimeTimerAction()
{
//	unsigned long start = millis();
	currentWorkTime++;
//	debugf("%s", currentTime.c_str());
//	debugf("updateTimeTimerAction - mem %d",system_get_free_heap_size());
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
//	infos->updateParamValue("station", WifiStation.getIP().toString());
//	infos->updateParamValue("ssid", WifiStation.getSSID());
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
//	infos->updateParamValue("station", "Disconnected");
//	infos->updateParamValue("ssid", "no:" + ActiveConfig.NetworkSSID);
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
	// Web Sockets configuration
	server.enableWebSockets(true);
	server.commandProcessing(true,"command");


	server.setWebSocketConnectionHandler(wsConnected);
	server.setWebSocketMessageHandler(wsMessageReceived);
	server.setWebSocketBinaryHandler(wsBinaryReceived);
	server.setWebSocketDisconnectionHandler(wsDisconnected);

}

//void startFTP()
//{
//	if (!fileExist("index.html"))
//		fileSetContent("index.html", "<h3>Please connect to FTP and upload files from folder 'web/build' (details in code)</h3>");
//
//	// Start FTP server
//	ftp.listen(21);
//	ftp.addUser("me", "123"); // FTP account
//}

// Will be called when system initialization was completed
void startServers()
{
//	infos->updateParamValue("ap", WifiAccessPoint.getIP().toString());
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

void initFromConfig() {
//	sousCommand.sousController->Setpoint = ActiveConfig.Needed_temp;
//	sousCommand.sousController->Kp = ActiveConfig.Kp;
//	sousCommand.sousController->Ki = ActiveConfig.Ki;
//	sousCommand.sousController->Kd = ActiveConfig.Kd;
}

const String WS_HEARTBEAT = "--heartbeat--";

//void sendHeartBeat() {
////	debugf("sendHeartBeat");
//	updateWebSockets(WS_HEARTBEAT);
//}

void STADisconnect(String ssid, uint8_t ssid_len, uint8_t bssid[6], uint8_t reason)
{
//	debugf("DISCONNECT - SSID: %s, REASON: %d\n", ssid.c_str(), reason);
	WifiStation.enable(false);
	if (!WifiAccessPoint.isEnabled())
	{
		debugf("Starting OWN AP");
		WifiStation.disconnect();
		WifiAccessPoint.enable(true);
//		WifiStation.connect();
	}
}
void STAConnected(String ssid, uint8_t ssid_len, uint8_t bssid[6], uint8_t reason) {

}

void onConnect() {
	infos->updateParamValue("stationIP", WifiStation.getIP().toString());
}

void updateInfoOnStart() {
	infos->updateParamValue("target", String(ActiveConfig.Needed_temp));
	infos->updateParamValue("Kp", String(ActiveConfig.Kp));
	infos->updateParamValue("Ki", String(ActiveConfig.Ki));
	infos->updateParamValue("Kd", String(ActiveConfig.Kd));
}

void init()
{
	Serial.begin(115200); // 74880
	Serial.systemDebugOutput(true); // Debug output to serial

	initSpiff();

	//Change CPU freq. to 160MHZ
	System.setCpuFrequency(eCF_160MHz);
	wifi_set_sleep_type(NONE_SLEEP_T);

	//setup i2c pins
	Wire.pins(sclPin, sdaPin);

	display = new SSD1306_Driver(4);
	display->init();
//
	infos = new InfoScreens(display);
	initInfoScreens();
	infos->initMFButton(encoderSwitchPin);
	infos->show();

	updateWebSocketTimer.initializeMs(80, updateWebSocketsFromTimer);

	ActiveConfig = loadConfig();
	sousCommand.initCommand(ActiveConfig.Needed_temp, ActiveConfig.Kp, ActiveConfig.Ki, ActiveConfig.Kd);

	updateInfoOnStart();

//	sousCommand.initCommand(0,0,0,0);
	sousCommand.startwork();
//	commandHandler.registerCommand(CommandDelegate("fromClient","Application commands","Application",processAppCommands));
	commandHandler.registerCommand(CommandDelegate("app","Application commands","Application",processAppCommands));

//////	sousController = new SousVideController();
////	initFromConfig();
////
//	debugf("======= SousVide ==========");
////	Serial.println();
////

//	DateTime date = SystemClock.now();
//	lastActionTime = date.Milliseconds;

	//ilan
//	keepAliveTimer.initializeMs(1000, updateTimeTimerAction).start();
//
//	AppSettings.load();
//
////	ds.begin(); // It's required for one-wire initialization!
////	readTempTimer.initializeMs(1000, readTempData).startOnce();
//
////	ReadTemp.Init(dsTempPin);  			// select PIN It's required for one-wire initialization!
////	ReadTemp.StartMeasure(); // first measure start,result after 1.2 seconds * number of sensors
////
////	readTempTimer.initializeMs(10000, readData).start();   // every 10 seconds
//
	WifiStation.enable(false);
//	WifiStation.config(ActiveConfig.NetworkSSID, ActiveConfig.NetworkPassword);
//	infos->updateParamValue("ssid", ActiveConfig.NetworkSSID);
////	if (AppSettings.exist())
////	{
////		WifiStation.config(AppSettings.ssid, AppSettings.password);
//////		if (!AppSettings.dhcp && !AppSettings.ip.isNull())
//////			WifiStation.setIP(AppSettings.ip, AppSettings.netmask, AppSettings.gateway);
////	}
//
////	debugf("net=%s, pass=%s", WIFI_SSID, WIFI_PWD);
//

	bool staOn = true;
	WifiStation.enable(staOn);
	if(staOn) {
		WifiStation.config(WIFI_SSID, WIFI_PWD);
		WifiStation.connect();
	}
//
////	WifiStation.startScan(networkScanCompleted);
//
//
	// Start AP for configuration
	WifiAccessPoint.enable(true);

	WifiAccessPoint.config("Sousvide Config", "", AUTH_OPEN);
	infos->updateParamValue("apIp", "192.168.4.1");

	// Attach Wifi events handlers
	WifiEvents.onStationDisconnect(STADisconnect);
	WifiEvents.onStationConnect(STAConnected);

	WifiStation.waitConnection(onConnect);
//	WifiEvents.onStationGotIP(STAGotIP);
//
//	// Run WEB server on system ready
////	System.onReady(startServers);
	startServers();
//	WifiStation.waitConnection(connectOk, 20, connectFail); // We recommend 20+ seconds for connection timeout at start
//	heartBeat.initializeMs(4000, sendHeartBeat).start();
}
