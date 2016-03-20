#ifndef INCLUDE_CONFIGURATION_H_
#define INCLUDE_CONFIGURATION_H_

#include <SmingCore.h>
#include <basicStructures.h>

// If you want, you can define WiFi settings globally in Eclipse Environment Variables
#ifndef WIFI_SSID
	#define WIFI_SSID "PleaseEnterSSID" // Put you SSID and Password here
	#define WIFI_PWD "PleaseEnterPass"
#endif

const uint8_t ConfigJsonBufferSize = 200; // Application configuration JsonBuffer size ,increase it if you have large config
const uint16_t ConfigFileBufferSize = 2048; // Application configuration FileBuffer size ,increase it if you have large config

//// Pin for communication with DHT sensor
////#define DHT_PIN 1 // UART0 TX pin
//#define DHT_PIN 12
//
//// Pin for trigger control output
////#define CONTROL_PIN 3 // UART0 RX pin
//#define CONTROL_PIN 15


//Pins used

////SPI LCD Pins
//#define mosiPin 13
//#define clkPin 14
//#define csPin 15
//#define dcPin 5

//SSD13006 Pins
#define sclPin 13
#define sdaPin 14

// Pin for communication with Temp sensor
#define DS_TEMP_PIN 12

// Pin for trigger control output
#define RELAY_PIN 4

////Rotary Pins
#define encoderSwitchPin 0 //push button switch
//#define encoderCLK 4
//#define encoderDT 12

#define lightPin = 14;

const char SOUSVIDE_CONFIG_FILE[] = ".sousvide.conf"; // leading point for security reasons :)

//enum OperationMode
//{
//	Manual = 0,
//	Sousvide = 1,
//};

struct SousvideConfig
{
	SousvideConfig()
	{
		Needed_temp = 36;
		Kp = 1;
		Ki = 1;
		Kd = 1;
		operationMode = Manual;
		enabled = false;
		highlow = 0;
	}

	String NetworkSSID;
	String NetworkPassword;

	float Needed_temp;
	float Kp;
	float Ki;
	float Kd;
	bool enabled;
	OperationMode operationMode;

	// 0 == operation on low, 1== operation on HIGH
	bool highlow;
};
//
SousvideConfig loadConfig();
void saveConfig(SousvideConfig& cfg);
//extern void startWebClock();

extern SousvideConfig ActiveConfig;

#endif /* INCLUDE_CONFIGURATION_H_ */
