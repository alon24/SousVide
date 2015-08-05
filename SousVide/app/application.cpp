#include <user_config.h>
#include <SmingCore/SmingCore.h>
#include <Libraries/Adafruit_SSD1306/Adafruit_SSD1306.h>
#include <menues.h>
#include <ButtonActions.cpp>
#include <mqttHelper.h>
#include <utils.h>
#include <AppSettings.h>

#include <Libraries/OneWire/OneWire.h>

//Forward declerations
//mqtt
//void onMessageReceived(String topic, String message); // Forward declaration for our callback

//wifi
//void process();
void connectOk();
void connectFail();

void handleCommands(String commands);
void updateWebUI();

//encoder code from http://bildr.org/2012/08/rotary-encoder-arduino/

//* SSD1306 - I2C
Adafruit_SSD1306 display(4);

//Pins used
#define sclPin 0
#define sdaPin 2
#define dsTempPin 4
#define relayPin 5
#define encoderA 13
#define encoderB 12
#define encoderSwitchPin 14 //push button switch

//// If you want, you can define WiFi settings globally in Eclipse Environment Variables
//#ifndef WIFI_SSID
//	#define WIFI_SSID "PleaseEnterSSID" // Put you SSID and Password here
//	#define WIFI_PWD "PleaseEnterPass"
//#endif

//Timers
Timer procTimer;
Timer buttonTimer;
Timer displayTimer;
Timer timeTimer;
Timer initTimer;
Timer blinkTimer;

time_t lastActionTime =0;
String currentTime = "00:00:00";

int totalActiveSockets = 0;

String name;
String ip;

int currentScreenIndex = 0;

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
		clients[i].sendString("New friend arrived! Total: " + String(totalActiveSockets));
	}
	updateWebUI();
}

void wsMessageReceived(WebSocket& socket, const String& message)
{
	Serial.printf("WebSocket message received:\r\n%s\r\n", message.c_str());
//	String response = "Echo: " + message;
//	socket.sendString(response);
	handleCommands(message);
}

void updateWebUI() {
	WebSocketsList &clients = server.getActiveWebSockets();
	for (int i = 0; i < clients.count(); i++) {
		Serial.println("updateWebUI::relayState:" + String(relayState == true ? "true" : "false"));
		clients[i].sendString("relayState:" + String(relayState == true ? "true" : "false"));
		clients[i].sendString("updatetime:" + currentTime);
	}
}


void handleCommands(String commands) {
	Serial.println("now handling " + commands);

	if (commands.startsWith("toggleRelay")) {
		String state = commands.substring(12);
		Serial.println("handleCommands::toggeled relay " + String(state));
		relayState = state.equals("true") ? true : false;
		Serial.println("handleCommand:: (state.equals(true)=" + String(state.equals("true")));
		digitalWrite(relayPin, (state.equals("true") ? HIGH : LOW));

		updateWebUI();
	}
}

void wsBinaryReceived(WebSocket& socket, uint8_t* data, size_t size)
{
	Serial.printf("wsBinaryReceived::Websocket binary data recieved, size: %d\r\n", size);
}

void wsDisconnected(WebSocket& socket)
{
	totalActiveSockets--;

	// Notify everybody about lost connection
	WebSocketsList &clients = server.getActiveWebSockets();
	for (int i = 0; i < clients.count(); i++)
		clients[i].sendString("We lost our friend :( Total: " + String(totalActiveSockets));
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
		        	digitalWrite(relayPin, oper.compareTo("1") ? HIGH : LOW);
		        if (in.compareTo("out1")) {
		        }
//		        break;
		    }

//		    std::cout << s.substr(start, end);


//		String val = getValue(message,':',0);
//
//		Serial.println("tok: " + val);
//		Vector<int> splits;
//		splitString(message, ',', splits);
//		for (int i = 0; i < splits.size(); i++)
//		{
//			Serial.print(":\r\n\t"); // Prettify alignment for printing
//			const char* ss = ((const char*)splits.elementAt(i));
//			String *str = new String(ss);
//			Serial.println("tok: " + *str);
//		}
//		message.splitString();
//		if (messag) {
//
//		}

//		DynamicJsonBuffer jsonBuffer;
//
//		char* jsonString = strcpy((char*)malloc(message.length()+1), message.c_str());
//		Serial.println("111");
////		char * jsonString = message.c_str();
//		JsonObject& root = jsonBuffer.parseObject(jsonString);
//		Serial.println("222");
//		JsonObject& payload = root["payload"];
//		Serial.println("333");
//		String com = String((const char*)payload["command"]);
//		Serial.println("444");
//		Serial.println(" commmmm = " + com);
////		JsonObject jo = JsonBuffer.pa
//
//		free(jsonString);
	}
}

// Will be called when WiFi station was connected to AP
//void connectOk()
//{
//	debugf("Connect to AP successful");
//	Serial.println("I'm CONNECTED");
//
//	// disable softAP mode
//	WifiAccessPoint.enable(false);
//
////	// start webserver
////	startWebServer();
//
//	setName("esp" + WifiAccessPoint.getMAC() );
//	// Run MQTT client
//	mqtt.connect("esp-" + WifiAccessPoint.getMAC());
//	mqtt.subscribe("dood/#");
//
//	display.setTextSize(1);
//	display.setTextColor(WHITE);
//	display.setCursor(0, 9);
//	display.println(WifiStation.getIP().toString());
//	display.setCursor(0, 18);
//	display.println("Connecting to mqtt");
//	display.display();
//
//	// Start publishing loop
//	initTimer.initializeMs(15 * 1000, publishInit).startOnce();
//}

// Will be called when WiFi station timeout was reached
//void connectFail()
//{
//	Serial.println("I'm NOT CONNECTED. Need help :(");
//	debugf("Connect to AP failed");
//
//	// config softAP mode
//	WifiAccessPoint.config(SOFTAP_SSID, "", AUTH_OPEN);
//	WifiAccessPoint.enable(true);
//
//	// start webserver
//	startWebServer();
//}

///// Display //////////////
#define say(a) ( Serial.print(a) )
#define sayln(a) (Serial.println(a))
void IRAM_ATTR refreshScreen();

volatile int lastEncoded = 0;
volatile long encoderValue = 0;

long lastencoderValue = 0;
int lastValue = -20;

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

InfoPages* infos = new InfoPages("SousVide");

enum DisplayMode
{
	Display_Regular=1,
	Display_Menu=2
};
DisplayMode displayMode = Display_Regular;

////button modes
//boolean clicked = false;
//boolean doubleClicked = false;
//boolean hold = false;
//boolean longHold = false;

//should check screen action
boolean shouldDimScreen = false;

void buttonUseEvent(ButtonUseEvent used);

ButtonActions bAct(encoderSwitchPin, buttonUseEvent);

Menu menu("SousVide");

void setupMenu()
{
//	MenuPage *pidSettings = new MenuPage("PID Tuning");
//	pidSettings->createItem("Tune SP");
//	pidSettings->createItem("Tune P");
//	pidSettings->createItem("Tune I");
//	pidSettings->createItem("Tune D");

	MenuPage *settings = new MenuPage("Settings");
	MenuItem *i1 =  new MenuItem("Set Time");
	MenuItem *i2 = new MenuItem("Set Needed Temp");
	settings->addChild(i1);
	settings->addChild(i2);
//	settings->createItem("t3");
//	settings->createItem("t4");
//	settings->createItem("t5");
//	settings->createItem("t6");
	menu.addChild(settings);

	MenuPage* time = new MenuPage("Time");
	MenuItem* hh = new MenuItem("HH");
	MenuItem* mm = new MenuItem("MM");
	time->addChild(hh);
	time->addChild(mm);
	menu.addChild(time);
//
	MenuPage* opt = new MenuPage("Options");
	MenuItem* o1 = new MenuItem("opt1");
	MenuItem* o2 = new MenuItem("opt2");
	opt->addChild(o1);
	opt->addChild(o2);
	menu.addChild(opt);

//	BaseMenuElement e = MenuItem("Asd");
	i1->setLinker(time);
	i2->setLinker(opt);
	menu.setCurrentItem(i1);
	menu.setMaxPerPage(5);
	menu.setRoot(settings);
	menu.getParams()->highlightMode = HighlightMode::Inverse;
}

void setupInfos() {
	InfoPage* p1 = infos->createPage("1", "D1");
	p1->createInfo("1", "text1");
}

void moveToMenuMode()
{
	if (displayMode == Display_Regular)
	{
		sayln("Display - move to menu mode");
//		menu.setCurrentItem(1,1);
	    displayMode = Display_Menu;
	    menu.moveToRoot();
	    refreshScreen();
	}

//    lastActionTime = SystemClock.now().toUnixTime();
//	shouldDimScreen = false;
}
void handleEncoderInterrupt() {
	if(lastValue != encoderValue/4 && (encoderValue %4 == 0)) {
		if (lastValue > encoderValue/4) {
			menu.moveUp();
		}
		else
		{
			menu.moveDown();
		}
	    lastValue = encoderValue/4;
//	    say("encode used");
	    //move to menu mode
	    moveToMenuMode();
	    refreshScreen();
	  }
}

void IRAM_ATTR updateEncoder(){

  int MSB = digitalRead(encoderA); //MSB = most significant bit
  int LSB = digitalRead(encoderB); //LSB = least significant bit

  int encoded = (MSB << 1) |LSB; //converting the 2 pin value to single number
  int sum  = (lastEncoded << 2) | encoded; //adding it to the previous encoded value

  if(sum == 0b1101 || sum == 0b0100 || sum == 0b0010 || sum == 0b1011) encoderValue ++;
  if(sum == 0b1110 || sum == 0b0111 || sum == 0b0001 || sum == 0b1000) encoderValue --;

  lastEncoded = encoded; //store this value for next time

  handleEncoderInterrupt();
}

void IRAM_ATTR checkRotaryBtn()
{
	bAct.actOnButton();
}

void handleClick() {

	switch(displayMode)
	{
		case Display_Menu:
			menu.movetolinked();
			displayMode = Display_Menu;
			refreshScreen();
			break;
		case Display_Regular:
			displayMode = Display_Menu;
			refreshScreen();
//			checkDisplayState();
			break;
	}

}

void handleDoubleClick() {
	menu.moveUpLevel();
	refreshScreen();
}

void moveToRegMode()
{
	if (displayMode == Display_Menu)
	{
		sayln("Display - move to regular mode");
//		menu.setCurrentItem(1,1);
	    displayMode = Display_Regular;
	}

//    lastActionTime = SystemClock.now().toUnixTime();
//	shouldDimScreen = false;
	refreshScreen();
}

void handleLongClick() {
	sayln("long");
//	moveToRegMode();
//	m_dispMode = DIS_SHOW_DATA;
//	checkDisplayState();
}

void handleHoldClick() {
	moveToRegMode();
}

void buttonUseEvent(ButtonUseEvent used)
{
	Serial.println("button action used = " + String(used.action));
	switch (used.action) {
		case BTN_CLICK:
			handleClick();
			break;
		case BTN_DOUBLE_CLICK:
			handleDoubleClick();
			break;
		case BTN_HOLD_CLICK:
			handleLongClick();
			break;
		case BTN_LONG_CLICK:
			handleHoldClick();
			break;
	}
}
////////////////////// Display //////////////////
void checkDisplayState()
{
//	if (m_dispMode == DIS_SHOW_DATA) {
//		return;
//	}
//sayln(String(timeForScreenAction == true));
	if (shouldDimScreen == false) {
		DateTime d = SystemClock.now();
		time_t temp = d.toUnixTime();
//		Serial.print("last=");
//		Serial.print(String(lastActionTime));
//		Serial.print(", now=");
//		Serial.println(String(temp));

//		if (temp > lastActionTime + 10) {
//			sayln("its been more 10 secs - dimming screen");
////			display.clearDisplay();
//			display.dim(true);
////			display.display();
//			shouldDimScreen = true;
//		}
	}
}

void highlightRow(int i) {
	int hight = 8;
	int start = 10 + hight;
	int y = (start + (i*hight));
	display.fillRect(2, y, 121, hight, WHITE);
}

void IRAM_ATTR showMenuScreen() {
	sayln(lastValue);

	display.setTextSize(1);
	display.setTextColor(WHITE);
	String t = "SousVide by alon24";
	display.setCursor(getCenterXForString(menu.getParams(), t, 1), 1);
	display.clearDisplay();
	display.println(t);
//	display.setCursor(2,10);

	Vector<BaseMenuElement*> v = menu.getDisplayedItems();
	BaseMenuElement* cur = menu.getCurrent();
	BaseMenuElement* p = cur->getParent();
	t = p->getId();
	display.setCursor(getCenterXForString(menu.getParams(), t, 1), 10);
	display.println(t);
	display.setTextSize(1);

	uint16_t regColor = WHITE;
	uint16_t highlightColor = (menu.getParams()->highlightMode == HighlightMode::pointer) ? WHITE : BLACK;
	int hight = 8;
	int start = 10 + hight;

	for (int i = 0; i < v.size(); ++i) {
		BaseMenuElement* item = v.elementAt(i);
		String txt;
		if(cur == item) {
			display.setTextColor(highlightColor);
			if (menu.getParams()->highlightMode == HighlightMode::pointer) {
				txt = String(">");
			}
			else {
				int y = (start + (i*hight));
				display.setCursor(4, y);
//				txt= String(" ");
				highlightRow(i);
			}
		} else {
			display.setTextColor(regColor);
			if (menu.getParams()->highlightMode == HighlightMode::pointer) {
				txt = String(" ");
			}
			else
			{
				int y = (start + (i*hight));
				display.setCursor(4, y);
			}
		}

		txt += item->getId();
		display.println(txt);
	}
	if(menu.getParams()->boxed == true) {
		display.drawRect(0,0,128,64, WHITE);
	}

	display.display();
}

void writeToScreen(int index) {
	if (index == 0) {
		display.setCursor(1, 10);
		display.print("sc1 ");
	}
	else if (index == 1) {
		display.setCursor(1, 15);
		display.print("sc2 ");
	}
}

void IRAM_ATTR showRegScreen() {
	menu.moveto(menu.getRoot());
	display.clearDisplay();
	display.setTextSize(1);
	display.setTextColor(WHITE);
	display.setCursor(1, 1);
	String t= "SousVide";
	display.println(t);
	t = currentTime;
	display.setCursor(getXOnScreenForString(t, 1), 1 );
	display.print(t);

	writeToScreen(currentScreenIndex);

	display.display();
}

void IRAM_ATTR refreshScreen()
{
//	showMenuScreen();
	switch (displayMode)
	{
	case Display_Menu:
		showMenuScreen();
		break;
	case Display_Regular:
		showRegScreen();
		break;
	}
}

void refreshTimeForUi()
{
	if (displayMode == Display_Menu)
	{
		return;
	}
	int x = getXOnScreenForString(currentTime, 1);
	display.fillRect( x, 1, currentTime.length() * 6, 8, BLACK);
	display.setTextSize(1);
	display.setTextColor(WHITE);
	display.setCursor(x, 1 );
//	display.setCursor(15, 10);
//	String t = String(millis()/1000);
//		display.print(t);
	display.print(currentTime);
	display.display();
	updateWebUI();
}

void IRAM_ATTR updateTimeTimerAction()
{
	currentTime = SystemClock.now().toShortTimeString(true);

//	Serial.println("time=" + currentTime);
	refreshTimeForUi();
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
	Serial.println("I'm CONNECTED");

	if (!mqtt)
	{
		// Run MQTT client
		mqtt = new mqttHelper("test.mosquitto.org", 1883, onMessageReceived);
		mqtt->start();
	}

	// Start publishing loop
	procTimer.initializeMs(20 * 1000, doInit).startOnce(); // every 20 seconds
}



// Will be called when WiFi station timeout was reached
void connectFail()
{
	Serial.println("I'm NOT CONNECTED. Need help :(");

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

void onIpConfig(HttpRequest &request, HttpResponse &response)
{
	if (request.getRequestMethod() == RequestMethod::POST)
	{
		AppSettings.dhcp = request.getPostParameter("dhcp") == "1";
		AppSettings.ip = request.getPostParameter("ip");
		AppSettings.netmask = request.getPostParameter("netmask");
		AppSettings.gateway = request.getPostParameter("gateway");
		debugf("Updating IP settings: %d", AppSettings.ip.isNull());
		AppSettings.save();
	}

	TemplateFileStream *tmpl = new TemplateFileStream("settings.html");
	auto &vars = tmpl->variables();

	bool dhcp = WifiStation.isEnabledDHCP();
	vars["dhcpon"] = dhcp ? "checked='checked'" : "";
	vars["dhcpoff"] = !dhcp ? "checked='checked'" : "";

	if (!WifiStation.getIP().isNull())
	{
		vars["ip"] = WifiStation.getIP().toString();
		vars["netmask"] = WifiStation.getNetworkMask().toString();
		vars["gateway"] = WifiStation.getNetworkGateway().toString();
	}
	else
	{
		vars["ip"] = "192.168.1.77";
		vars["netmask"] = "255.255.255.0";
		vars["gateway"] = "192.168.1.1";
	}

	response.sendTemplate(tmpl); // will be automatically deleted
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
	JsonObjectStream* stream = new JsonObjectStream();
	JsonObject& json = stream->getRoot();

	json["status"] = (bool)true;

	bool connected = WifiStation.isConnected();
	json["connected"] = connected;
	if (connected)
	{
		// Copy full string to JSON buffer memory
		json.addCopy("network", WifiStation.getSSID());
	}

	JsonArray& netlist = json.createNestedArray("available");
	for (int i = 0; i < networks.count(); i++)
	{
		if (networks[i].hidden) continue;
		JsonObject &item = netlist.createNestedObject();
		item.add("id", (int)networks[i].getHashId());
		// Copy full string to JSON buffer memory
		item.addCopy("title", networks[i].ssid);
		item.add("signal", networks[i].rssi);
		item.add("encryption", networks[i].getAuthorizationMethodName());
	}

	response.setAllowCrossDomainOrigin("*");
	response.sendJsonObject(stream);
}

bool flipRelay(){
	relayState = !relayState;
	digitalWrite(relayPin, relayState ? HIGH : LOW);
	return relayState;
}

void onDoCommand(HttpRequest &request, HttpResponse &response)
{
	Serial.println("onDoCommand");
	JsonObjectStream* stream = new JsonObjectStream();
	JsonObject& json = stream->getRoot();


//	if (response.)
	json["relayState"] = flipRelay();

	response.setAllowCrossDomainOrigin("*");
	response.sendJsonObject(stream);
}

void makeConnection()
{
	WifiStation.enable(true);
	WifiStation.config(network, password);

	AppSettings.ssid = network;
	AppSettings.password = password;
	AppSettings.save();

	network = ""; // task completed
}

void onAjaxConnect(HttpRequest &request, HttpResponse &response)
{
	JsonObjectStream* stream = new JsonObjectStream();
	JsonObject& json = stream->getRoot();

	String curNet = request.getPostParameter("network");
	String curPass = request.getPostParameter("password");

	bool updating = curNet.length() > 0 && (WifiStation.getSSID() != curNet || WifiStation.getPassword() != curPass);
	bool connectingNow = WifiStation.getConnectionStatus() == eSCS_Connecting || network.length() > 0;

	if (updating && connectingNow)
	{
		debugf("wrong action: %s %s, (updating: %d, connectingNow: %d)", network.c_str(), password.c_str(), updating, connectingNow);
		json["status"] = (bool)false;
		json["connected"] = (bool)false;
	}
	else
	{
		json["status"] = (bool)true;
		if (updating)
		{
			network = curNet;
			password = curPass;
			debugf("CONNECT TO: %s %s", network.c_str(), password.c_str());
			json["connected"] = false;
			connectionTimer.initializeMs(1200, makeConnection).startOnce();
		}
		else
		{
			json["connected"] = WifiStation.isConnected();
			debugf("Network already selected. Current status: %s", WifiStation.getConnectionStatusName());
		}
	}

	if (!updating && !connectingNow && WifiStation.isConnectionFailed())
		json["error"] = WifiStation.getConnectionStatusName();

	response.setAllowCrossDomainOrigin("*");
	response.sendJsonObject(stream);
}

void startWebServer()
{
	server.listen(80);
	server.addPath("/", onIndex);
	server.addPath("/ipconfig", onIpConfig);
	server.addPath("/ajax/get-networks", onAjaxNetworkList);
	server.addPath("/ajax/connect", onAjaxConnect);
//	server.addPath("/ajax/doCommand", onDoCommand);
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
	Serial.println("Starting servers");
//	startFTP();
	startWebServer();
}

void networkScanCompleted(bool succeeded, BssList list)
{
	if (succeeded)
	{
		for (int i = 0; i < list.count(); i++)
			if (!list[i].hidden && list[i].ssid.length() > 0)
				networks.add(list[i]);
	}
	networks.sort([](const BssInfo& a, const BssInfo& b){ return b.rssi - a.rssi; } );
}

//Temp
OneWire ds(dsTempPin);
Timer readTempTimer;

void checkTempTriggerRelay(float temp) {
	int trigger = (int)28;
	if((int)temp >= trigger && !relayState ) {
		Serial.println("temp is >= " + String(trigger) + ", starting relay");
		handleCommands("toggleRelay:true");
	} else if((int)temp < trigger && relayState){
		Serial.println("temp is <  " + String(trigger) + ",stopping relay");
		handleCommands("toggleRelay:false");
	}
}

void readData()
{

	byte i;
	byte present = 0;
	byte type_s;
	byte data[12];
	byte addr[8];
	float celsius, fahrenheit;

	ds.reset_search();
	if (!ds.search(addr))
	{
		Serial.println("No addresses found.");
		Serial.println();
		ds.reset_search();
		delay(250);
		return;
	}

	Serial.print("Thermometer ROM =");
	for( i = 0; i < 8; i++)
	{
		Serial.write(' ');
		Serial.print(addr[i], HEX);
	}

	if (OneWire::crc8(addr, 7) != addr[7])
	{
	  Serial.println("CRC is not valid!");
	  return;
	}
	Serial.println();

	// the first ROM byte indicates which chip
	switch (addr[0]) {
	case 0x10:
	  Serial.println("  Chip = DS18S20");  // or old DS1820
	  type_s = 1;
	  break;
	case 0x28:
	  Serial.println("  Chip = DS18B20");
	  type_s = 0;
	  break;
	case 0x22:
	  Serial.println("  Chip = DS1822");
	  type_s = 0;
	  break;
	default:
	  Serial.println("Device is not a DS18x20 family device.");
	  return;
	}

	ds.reset();
	ds.select(addr);
	ds.write(0x44, 1);        // start conversion, with parasite power on at the end

	delay(1000);     // maybe 750ms is enough, maybe not
	// we might do a ds.depower() here, but the reset will take care of it.

	present = ds.reset();
	ds.select(addr);
	ds.write(0xBE);         // Read Scratchpad

	Serial.print("  Data = ");
	Serial.print(present, HEX);
	Serial.print(" ");
	for ( i = 0; i < 9; i++)
	{
		// we need 9 bytes
		data[i] = ds.read();
		Serial.print(data[i], HEX);
		Serial.print(" ");
	}
	Serial.print(" CRC=");
	Serial.print(OneWire::crc8(data, 8), HEX);
	Serial.println();

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

	celsius = (float)raw / 16.0;
	fahrenheit = celsius * 1.8 + 32.0;
	Serial.print("  Temperature = ");
	Serial.print(celsius);
	Serial.print(" Celsius, ");
	Serial.println(" Fahrenheit");
	Serial.println();

	checkTempTriggerRelay(celsius);
}

void init()
{
	Serial.begin(SERIAL_BAUD_RATE); // 74880
	Serial.systemDebugOutput(true); // Debug output to serial
	Wire.pins(sclPin, sdaPin);

	debugf("======= SousVide ==========");

	setupMenu();
	setupInfos();

//	delay(3000);
//	say("======= SousVide ==========");
	Serial.println();

	display.begin(SSD1306_SWITCHCAPVCC);
	display.clearDisplay();
	display.display();

	pinMode(encoderA, INPUT);
	pinMode(encoderB, INPUT);
	digitalWrite(encoderA, HIGH); //turn pullup resistor on
	digitalWrite(encoderB, HIGH); //turn pullup resistor on

	pinMode(relayPin, OUTPUT);
	digitalWrite(relayPin, LOW);
//	blinkTimer.initializeMs(1000, blink).start();

	attachInterrupt(encoderA, updateEncoder, CHANGE);
	attachInterrupt(encoderB, updateEncoder, CHANGE);

	pinMode(encoderSwitchPin, INPUT);
	digitalWrite(encoderSwitchPin, HIGH); //turn pullup resistor on

	DateTime date = SystemClock.now();
	lastActionTime = date.Milliseconds;
	sayln(lastActionTime);

//	procTimer.initializeMs(15, handleEncoderInterrupt).start();
	buttonTimer.initializeMs(15, checkRotaryBtn).start();

	refreshScreen();

//	displayTimer.initializeMs(15, handleScreen).start();
	timeTimer.initializeMs(1000, updateTimeTimerAction).start();

	AppSettings.load();

	ds.begin(); // It's required for one-wire initialization!

	readTempTimer.initializeMs(3000, readData).start();

//	WifiStation.enable(true);
//	if (AppSettings.exist())
//	{
//		WifiStation.config(AppSettings.ssid, AppSettings.password);
//		if (!AppSettings.dhcp && !AppSettings.ip.isNull())
//			WifiStation.setIP(AppSettings.ip, AppSettings.netmask, AppSettings.gateway);
//	}

	WifiStation.config(WIFI_SSID, WIFI_PWD);
	WifiStation.startScan(networkScanCompleted);

//
	// Start AP for configuration
	WifiAccessPoint.enable(true);
	WifiAccessPoint.config("ESP Configuration", "", AUTH_OPEN);

	// Run WEB server on system ready
	System.onReady(startServers);

	WifiStation.waitConnection(connectOk, 20, connectFail); // We recommend 20+ seconds for connection timeout at start
}

