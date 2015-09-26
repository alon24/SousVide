#include <user_config.h>
#include <SmingCore/SmingCore.h>
//#include <Libraries/Adafruit_SSD1306/Adafruit_SSD1306.h>
#include <Extended_SSD1306.h>
#include <menues.h>
#include <InfoScreens.h>
#include <ButtonActions.cpp>
#include <mqttHelper.h>
#include <utils.h>
#include <AppSettings.h>
#include <configuration.h>

#include <Libraries/OneWire/OneWire.h>

//#include <PID_v1.h>
//#include <PID_AutoTune_v0.h>
#include <MyPID.h>

//Forward declerations
//mqtt
//void onMessageReceived(String topic, String message); // Forward declaration for our callback

//wifi
//void process();
void IRAM_ATTR connectOk();
void IRAM_ATTR connectFail();
void IRAM_ATTR handleCommands(String commands);
void IRAM_ATTR updateWebSockets();
void IRAM_ATTR setRelayState(boolean state);

//encoder code from http://bildr.org/2012/08/rotary-encoder-arduino/

//* SSD1306 - I2C
//Adafruit_SSD1306 display(4);
Extended_SSD1306 display(4);

MySousVideController *sousController;

//Pins used
#define sclPin 0
#define sdaPin 2
//#define dsTempPin 4
//#define relayPin 5
#define encoderA 13
#define encoderB 12
#define encoderSwitchPin 14 //push button switch

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
Timer timeTimer;
Timer initTimer;
Timer blinkTimer;

//textRect lastTimeRect;

time_t lastActionTime =0;
String currentTime = "00:00:00";

float currentTemp = 0;

int totalActiveSockets = 0;

String name;
String ip;

int currentInfoScreenIndex = 0;

HttpServer server;
FTPServer ftp;

BssList networks;
String network, password;
Timer connectionTimer;

bool relayState = false;

////Web Sockets ///////

void IRAM_ATTR wsConnected(WebSocket& socket)
{
	totalActiveSockets++;

	// Notify everybody about new connection
	WebSocketsList &clients = server.getActiveWebSockets();
	for (int i = 0; i < clients.count(); i++) {
		clients[i].sendString("New friend arrived! Total: " + String(totalActiveSockets));
	}
	//TODO: ilan need to do a global set so need to set a registry
//	updateWebSockets();
}

void wsMessageReceived(WebSocket& socket, const String& message)
{
	Serial.printf("WebSocket message received:\r\n%s\r\n", message.c_str());
//	String response = "Echo: " + message;
//	socket.sendString(response);
	handleCommands(message);
}

void IRAM_ATTR updateWebSockets(String to, String info) {

}

void IRAM_ATTR updateWebSockets(String cmd) {
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

void IRAM_ATTR handleCommands(String commands) {
	Serial.println("now handling " + commands);

	//iterate over commands
	Vector<String> commandToken;
	int numToken = splitString(commands, ';' , commandToken);

	for (int i = 0; i < numToken; ++i) {
		String command = commandToken.get(i);
		String parsedCmd = getCommandAndData(command);

		//command is now the just the data (stripped of the command itself)
//		debugf("handleCommands::original cmd=%s, parsed=%s, data=%s", command.c_str(), parsedCmd.c_str(), command.c_str());

		if(parsedCmd.equals("query")) {

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
	}
}

void hadleQuery(String items) {
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
	}
}

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

InfoScreens* infos = new InfoScreens("SousVide");

enum DisplayMode
{
	Display_Info=1,
	Display_Menu=2
};
DisplayMode currentDisplayMode = Display_Info;

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

void IRAM_ATTR initMenu()
{
	MenuPage *settings = menu.createPage("Settings");
	MenuItem *i1 = settings->createItem("Set Time");
	MenuItem *i2 = settings->createItem("Sous Work");

	MenuPage* sous = menu.createPage("Sous Options");
	sous->createItem("SetPidStartTime");
	sous->createItem("SetPidOffTime");
	sous->createItem("off");
	sous->createItem("Tune P");
	sous->createItem("Tune I");
	sous->createItem("Tune D");
	sous->createItem("Run");
	sous->createItem("Tune SP");

	MenuPage* time = new MenuPage("Time");
	MenuItem* hh = new MenuItem("HH");
	MenuItem* mm = new MenuItem("MM");
	time->addChild(hh);
	time->addChild(mm);
	menu.addChild(time);

	i1->setLinker(time);
	i2->setLinker(sous);
	menu.setCurrentItem(i1);
	menu.setMaxPerPage(5);
	menu.setRoot(settings);
	menu.getParams()->highlightMode = HighlightMode::Inverse;
}

/**
 * setup infoscreens moved by the rotary
 */
void IRAM_ATTR initInfoScreens() {
	InfoScreenPage* p1 = infos->createScreen("M", "M");
	InfoPageLine* el = p1->createLine("h", "SousVide");
	paramStruct* t = el->addParam("time", currentTime);
	t->t.x = getXOnScreenForString(currentTime, 1);

	InfoPageLine* el2 = p1->createLine("2", "temp: ");
	el2->addParam("temp", String(currentTemp, 3));

	InfoPageLine* el3 = p1->createLine("3", "ap: ");
	el3->addParam("ap", "0.0.0.0");

	InfoPageLine* el4 = p1->createLine("4", "sta:");
	el4->addParam("station", "0.0.0.0");

	//Sousvide info page params
	InfoScreenPage* p2 = infos->createScreen("s1", "s1");
	InfoPageLine* ep2 = p2->createLine("h", "Params");
	paramStruct* t1 = ep2->addParam("time", currentTime);
	t1->t.x = getXOnScreenForString(currentTime, 1);

	p2->createLine("2", "Needed_t: ")->addParam("Needed_t", String(sousController->Setpoint, 1));
	p2->createLine("3", "Kp: ")->addParam("Kp", String(sousController->Kp, 2));
	p2->createLine("4", "Ki: ")->addParam("Ki", String(sousController->Ki, 2));
	p2->createLine("5", "Kd: ")->addParam("Kd", String(sousController->Kd, 2));
}

void IRAM_ATTR moveToMenuMode()
{
	if (currentDisplayMode == Display_Info)
	{
		infos->setCanUpdateDisplay(false);
	    currentDisplayMode = Display_Menu;
		sayln("Display - move to menu mode");
//		menu.setCurrentItem(1,1);
	    menu.moveToRoot();
	    refreshScreen();
	}

//    lastActionTime = SystemClock.now().toUnixTime();
//	shouldDimScreen = false;
}

Timer refreshScreenTimer;

void handleEncoderInterrupt() {
	if(lastValue != encoderValue/4 && (encoderValue %4 == 0)) {
		if (currentDisplayMode == Display_Menu) {
//			debugf("Display_Menu");
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
//			moveToMenuMode();

			refreshScreenTimer.initializeMs(10, refreshScreen).startOnce();
//			refreshScreen();
		}
		else {
//			debugf("handleEncoderInterrupt::!Display_Menu");
			if (encoderValue % 16 != 0 ) {
				return;
			}

			int i = encoderValue/16;
			if (lastValue > i) {
				infos->moveLeft(display);
			}
			else if (lastValue < i)
			{
				infos->moveRight(display);
			}
		}
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

void IRAM_ATTR handleClick() {

	switch(currentDisplayMode)
	{
		case Display_Menu:
			menu.movetolinked();
//			currentDisplayMode = Display_Info;
			refreshScreen();
			break;
		case Display_Info:
			moveToMenuMode();
//			displayMode = Display_Menu;
			refreshScreen();
//			checkDisplayState();
			break;
	}

}

void handleDoubleClick() {
	if (currentDisplayMode == Display_Menu) {
		menu.moveUpLevel();
		refreshScreen();
	}
}

void moveToRegMode()
{
	if (currentDisplayMode == Display_Menu)
	{
		infos->setCanUpdateDisplay(true);
	    currentDisplayMode = Display_Info;
		sayln("Display - move to regular mode");
//		menu.setCurrentItem(1,1);
		refreshScreen();
	}

//    lastActionTime = SystemClock.now().toUnixTime();
//	shouldDimScreen = false;
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

int getCenterXForString(MenuParams *params, String t, int textSize) {
	int width = t.length() * (textSize == 1 ? 6 : 6);
	int x = (128 - width)/2 + (params->boxed ? 1 : 0);
	return x;
}

void printxy() {
	int x = display.getCursorX();
	int y = display.getCursorY();
	Serial.print("x,y=" +String(x) + ","+ String(y));
}
void IRAM_ATTR showMenuScreen() {
//	sayln(lastValue);

	display.dim(false);
	display.setTextSize(1);
	display.setTextColor(WHITE);
	String t = "SousVide by alon24";
	display.setCursor(getCenterXForString(menu.getParams(), t, 1), 1);
	display.clearDisplay();
	display.println(t);
//	display.get
//	display.get
//	Serial.println("xC=" + String(display.cursor_x) + ", yC=" +String(display.cursor_y) );
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

void IRAM_ATTR showInfoScreen() {
	menu.moveto(menu.getRoot());
	display.clearDisplay();
	display.setCursor(0,0);
//	infos->showCurrent(display);
	infos->show(1, display);
	//TODO: why?
//	infos->print(currentInfoScreenIndex, display);
}

void IRAM_ATTR refreshScreen()
{
	debugf("Refresh Screen called");

//	showMenuScreen();
	switch (currentDisplayMode)
	{
		case Display_Menu:
			showMenuScreen();
			break;
		case Display_Info:
			showInfoScreen();
			break;
	}
}

void refreshTimeForUi()
{
	if (currentDisplayMode == Display_Menu)
	{
		return;
	}

//	int x = getXOnScreenForString(currentTime, 1);
//	if(lastTimeRect.x = -1) {
////		display.fillRect( x, 1, currentTime.length() * 6, 8, BLACK);
//	}
//	else {
//		display.fillRect( lastTimeRect.x, lastTimeRect.y, lastTimeRect.w, lastTimeRect.h, BLACK);
//	}
//
////	Serial.printf("computed rect time =x %i, y %i, w %i\n", x, 1, currentTime.length() * 6);
//	display.setTextSize(1);
//	display.setTextColor(WHITE);
//	display.setCursor(x, 1 );
////	display.setCursor(15, 10);
////	String t = String(millis()/1000);
////		display.print(t);
//	lastTimeRect = display.print(currentTime);
//	Serial.printf("current time =x %i, y %i, w %i\n", lastTimeRect.x, lastTimeRect.y, lastTimeRect.w);
////	display.print(currentTime);
////	display.get;
////	Serial.println("xC=" + String(display.cursor_x) + ", yC=" +String(display.cursor_y) );
//	display.display();

	//need to update the time param on screen if shown


	//TODO:ilan
	infos->updateParamValue("time", currentTime, display);
//	InfoScreenPage* iScreen = infos->get(currentInfoScreenIndex);
//	iScreen->updateParam("time", currentTime, display);
	display.display();

//	Vector<InfoScreenLine*>* lines = iScreen->getAllLinesParamsForId("time");
//
//	//update all params with the "time" id
//	if (lines->size() > 0) {
//		String t = currentTime;
//		for (int i = 0; i < lines->size(); ++i) {
//			lines->elementAt(i)->updateDataForId(display, "time", currentTime);
//		}
////		iPage->itemAt(0)->updateData(display, 0, t);
////		iPage->print(display);
////		//update lasttimerect with alue
////		lastTimeRect = iPage->itemAt(0)->getParam(0)->t;
//	}

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

	//TODO:ilan
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
void IRAM_ATTR connectOk()
{
	infos->updateParamValue("station", WifiStation.getIP().toString(), display);
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
void IRAM_ATTR connectFail()
{
	Serial.println("I'm NOT CONNECTED. Need help :(");
	infos->updateParamValue("station", "Unknown", display);
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
////	server.addPath("/ajax/doCommand", onDoCommand);
	server.setDefaultHandler(onFile);

	// Web Sockets configuration
	server.enableWebSockets(true);
	server.setWebSocketConnectionHandler(wsConnected);
	server.setWebSocketMessageHandler(wsMessageReceived);
	server.setWebSocketBinaryHandler(wsBinaryReceived);
	server.setWebSocketDisconnectionHandler(wsDisconnected);

}

void IRAM_ATTR startFTP()
{
	if (!fileExist("index.html"))
		fileSetContent("index.html", "<h3>Please connect to FTP and upload files from folder 'web/build' (details in code)</h3>");

	// Start FTP server
	ftp.listen(21);
	ftp.addUser("me", "123"); // FTP account
}

// Will be called when system initialization was completed
void IRAM_ATTR startServers()
{
	infos->updateParamValue("ap", WifiAccessPoint.getIP().toString(), display);
	Serial.println("Starting servers");
////	startFTP();
	startWebServer();
}

void IRAM_ATTR networkScanCompleted(bool succeeded, BssList list)
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

void IRAM_ATTR setRelayState(boolean state) {
	if (state != relayState) {
		relayState = state;
		digitalWrite(relayPin, (relayState ? HIGH : LOW));
	}
}

//Temp
OneWire ds(dsTempPin);
Timer readTempTimer;

void IRAM_ATTR checkTempTriggerRelay(float temp) {
	//move on thsi only when Sousvide, do not interfere with manual mode
	if (operationMode == Sousvide)
	{
		int trigger = (int)28;
		if((int)temp >= trigger && !relayState ) {
			Serial.println("temp is >= " + String(trigger) + ", starting relay");
			handleCommands("toggleRelay:true");
		} else if((int)temp < trigger && relayState){
			Serial.println("temp is <  " + String(trigger) + ",stopping relay");
			handleCommands("toggleRelay:false");
		}
	}
}

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
	debugf("  Temperature = %f Celsius ", celsius);
	currentTemp = celsius;

	if(currentDisplayMode == Display_Info) {
		infos->updateParamValue("temp", String(currentTemp, 2), display);
		display.display();
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

void IRAM_ATTR ShowInfo() {
    Serial.printf("\r\nSDK: v%s\r\n", system_get_sdk_version());
    Serial.printf("Free Heap: %d\r\n", system_get_free_heap_size());
    Serial.printf("CPU Frequency: %d MHz\r\n", system_get_cpu_freq());
    Serial.printf("System Chip ID: %x\r\n", system_get_chip_id());
    Serial.printf("SPI Flash ID: %x\r\n", spi_flash_get_id());
    //Serial.printf("SPI Flash Size: %d\r\n", (1 << ((spi_flash_get_id() >> 16) & 0xff)));
}

void IRAM_ATTR serialCallBack(Stream& stream, char arrivedChar,
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

void init()
{
	Serial.begin(SERIAL_BAUD_RATE); // 115200
	Serial.systemDebugOutput(true); // Debug output to serial

	// mount spiffs
		int slot = rboot_get_current_rom();
	#ifndef DISABLE_SPIFFS
		if (slot == 0) {
	#ifdef RBOOT_SPIFFS_0
			debugf("trying to mount spiffs at %x, length %d", RBOOT_SPIFFS_0 + 0x40200000, SPIFF_SIZE);
			spiffs_mount_manual(RBOOT_SPIFFS_0 + 0x40200000, SPIFF_SIZE);
	#else
			debugf("trying to mount spiffs at %x, length %d", 0x40300000, SPIFF_SIZE);
			spiffs_mount_manual(0x40300000, SPIFF_SIZE);
	#endif
		} else {
	#ifdef RBOOT_SPIFFS_1
			debugf("trying to mount spiffs at %x, length %d", RBOOT_SPIFFS_1 + 0x40200000, SPIFF_SIZE);
			spiffs_mount_manual(RBOOT_SPIFFS_1 + 0x40200000, SPIFF_SIZE);
	#else
			debugf("trying to mount spiffs at %x, length %d", 0x40500000, SPIFF_SIZE);
			spiffs_mount_manual(0x40500000, SPIFF_SIZE);
	#endif
		}
	#else
		debugf("spiffs disabled");
	#endif

	Wire.pins(sclPin, sdaPin);

	ActiveConfig = loadConfig();

	sousController = new MySousVideController();

	debugf("======= SousVide ==========");
	Serial.println();

	display.begin(SSD1306_SWITCHCAPVCC);
	display.clearDisplay();

	display.setTextSize(1);
	display.setTextColor(WHITE);

	initMenu();
	initInfoScreens();

	pinMode(encoderA, INPUT_PULLUP);
	pinMode(encoderB, INPUT_PULLUP);
	digitalWrite(encoderA, HIGH); //turn pullup resistor on
	digitalWrite(encoderB, HIGH); //turn pullup resistor on

	pinMode(relayPin, OUTPUT);
	digitalWrite(relayPin, LOW);
	attachInterrupt(encoderA, updateEncoder, CHANGE);
	attachInterrupt(encoderB, updateEncoder, CHANGE);

	pinMode(encoderSwitchPin, INPUT);
	digitalWrite(encoderSwitchPin, HIGH); //turn pullup resistor on

	DateTime date = SystemClock.now();
	lastActionTime = date.Milliseconds;
//	sayln(lastActionTime);

	buttonTimer.initializeMs(80, checkRotaryBtn).start();

	showInfoScreen();

	//ilan
	timeTimer.initializeMs(1000, updateTimeTimerAction).start();

	AppSettings.load();

	ds.begin(); // It's required for one-wire initialization!
	readTempTimer.initializeMs(1000, readTempData).startOnce();

	WifiStation.enable(true);
//	if (AppSettings.exist())
//	{
//		WifiStation.config(AppSettings.ssid, AppSettings.password);
//		if (!AppSettings.dhcp && !AppSettings.ip.isNull())
//			WifiStation.setIP(AppSettings.ip, AppSettings.netmask, AppSettings.gateway);
//	}

//	debugf("net=%s, pass=%s", WIFI_SSID, WIFI_PWD);

	WifiStation.config(WIFI_SSID, WIFI_PWD);
	WifiStation.startScan(networkScanCompleted);


	// Start AP for configuration
	WifiAccessPoint.enable(true);
	WifiAccessPoint.config("ESP Configuration", "", AUTH_OPEN);

	// Run WEB server on system ready
	System.onReady(startServers);

	WifiStation.waitConnection(connectOk, 30, connectFail); // We recommend 20+ seconds for connection timeout at start

	Serial.printf("\r\nCurrently running rom %d.\r\n", slot);
	Serial.println("Type 'help' and press enter for instructions.");
	Serial.println();

	Serial.setCallback(serialCallBack);
}

