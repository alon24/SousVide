#include <user_config.h>
#include <SmingCore/SmingCore.h>
#include <Libraries/Adafruit_SSD1306/Adafruit_SSD1306.h>
#include <menues.h>
#include <ButtonActions.cpp>
#include <mqttHelper.h>
#include <utils.h>

//encoder code from http://bildr.org/2012/08/rotary-encoder-arduino/

//* For I2C mode:
// Default I2C pins 0 and 2. Pin 4 - optional reset
Adafruit_SSD1306 display(4);

//Pins used
#define relayPin 2
#define sclPin 5
#define sdaPin 4
#define encoderPin1 13
#define encoderPin2 12
#define encoderSwitchPin 14 //push button switch

//Timers
Timer procTimer;
Timer buttonTimer;
Timer displayTimer;
Timer timeTimer;
Timer initTimer;
Timer blinkTimer;

// Put you SSID and Password here
#define WIFI_SSID ""
#define WIFI_PWD ""

void onMessageReceived(String topic, String message); // Forward declaration for our callback
String name;
String ip;

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

// Publish our message
void publishMessage(String topic, String payload)
{
	Serial.println("Let's publish message now! t=" + topic + ", p=" + payload);
	mqtt->publishMessage("dood/" + topic, payload); // or publishWithQoS
}

void publishInit()
{
	String payload = "{\"mac\":\"esp-" + WifiAccessPoint.getMAC() + "\"}";
	publishMessage("init", payload);
}

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
	if (topic.compareTo("dood") == 0){
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

time_t lastActionTime =0;
String currentTime = "00:00:00";

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
	settings->createItem("t3");
	settings->createItem("t4");
	settings->createItem("t5");
	settings->createItem("t6");
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

  int MSB = digitalRead(encoderPin1); //MSB = most significant bit
  int LSB = digitalRead(encoderPin2); //LSB = least significant bit

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
//	String t = String(millis()/1000);
//	display.print(t);
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


// Will be called when WiFi station was connected to AP
void connectOk()
{
	Serial.println("I'm CONNECTED");

//	if (!mqtt)
//	{
//		// Run MQTT client
//		mqtt->start();
//		mqtt = new mqttHelper("test.mosquitto.org", 1883);
//	}
//
//	// Start publishing loop
//	procTimer.initializeMs(20 * 1000, publishMessage).start(); // every 20 seconds
}

// Will be called when WiFi station timeout was reached
void connectFail()
{
	Serial.println("I'm NOT CONNECTED. Need help :(");

	// .. some you code for device configuration ..
}

void init()
{
	Wire.pins(sclPin, sdaPin);
	Serial.begin(SERIAL_BAUD_RATE); // 74880

	setupMenu();

//	delay(3000);
	say("======= SousVide ==========");
	Serial.println();

	display.begin(SSD1306_SWITCHCAPVCC);
	display.clearDisplay();
	display.display();

//	printStatus();

	pinMode(encoderPin1, INPUT);
	pinMode(encoderPin2, INPUT);
	digitalWrite(encoderPin1, HIGH); //turn pullup resistor on
	digitalWrite(encoderPin2, HIGH); //turn pullup resistor on

//	pinMode(relayPin, OUTPUT);
//	digitalWrite(relayPin, LOW);
//	blinkTimer.initializeMs(1000, blink).start();

	attachInterrupt(encoderPin1, updateEncoder, CHANGE);
	attachInterrupt(encoderPin2, updateEncoder, CHANGE);

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

//	mqtt = new mqttHelper("test.mosquitto.org", 1883);

	WifiStation.config(WIFI_SSID, WIFI_PWD);
	WifiStation.enable(false);
	WifiAccessPoint.enable(false);

	// Run our method when station was connected to AP (or not connected)
//	WifiStation.waitConnection(connectOk, 20, connectFail); // We recommend 20+ seconds for connection timeout at start
}



//#include <user_config.h>
//#include <SmingCore/SmingCore.h>
//
//#define LED_PIN 2 // GPIO2
//
//Timer procTimer;
//bool state = true;
//int freeMemory;
//void blink()
//{
//    digitalWrite(LED_PIN, state);
//    state = !state;
//
//}
//
//void init()
//{
//    pinMode(LED_PIN, OUTPUT);
//    procTimer.initializeMs(1000, blink).start();
//    Serial.println("flashmem: " + String(flashmem_get_size_bytes()));
//    freeMemory = system_get_free_heap_size();
//    Serial.println(freeMemory);
//}
