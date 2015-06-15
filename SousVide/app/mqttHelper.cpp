/*
 * mqttHelper.cpp
 *
 *  Created on: Jun 12, 2015
 *      Author: ilan
 */

#include "mqttHelper.h"

//Default broker
mqttHelper::mqttHelper()
{
//	mqtt = MqttClient("test.mosquitto.org", 1883, NULL);
}

mqttHelper::mqttHelper(String broker, int port)
{
//	mqtt(broker, port, onMessageReceived);
}

mqttHelper::~mqttHelper()
{
	// TODO Auto-generated destructor stub
}

// Publish our message
void mqttHelper::publishMessage(String topic, String payload)
{
	Serial.println("Let's publish message now! t=" + topic + ", p=" + payload);
//	mqtt.publish("dood/" + topic, payload); // or publishWithQoS
}

void mqttHelper::publishInit()
{
	String payload = "{\"mac\":\"esp-" + WifiAccessPoint.getMAC() + "\"}";
	publishMessage("init", payload);
}

// Callback for messages, arrived from MQTT server
void mqttHelper::onMessageReceived(String topic, String message)
{
	Serial.print(topic);
	Serial.print(":\r\n\t"); // Prettify alignment for printing
	Serial.println(message);
}

// Will be called when WiFi station was connected to AP
void mqttHelper::connectOk()
{
//	Serial.println("I'm CONNECTED");
//
//	// Run MQTT client
//	mqtt.connect("esp8266");
//	mqtt.subscribe("main/status/#");
//
//	// Start publishing loop
//	procTimer.initializeMs(20 * 1000, publishMessage).start(); // every 20 seconds
}

// Will be called when WiFi station timeout was reached
void mqttHelper::connectFail()
{
	Serial.println("I'm NOT CONNECTED. Need help :(");

	// .. some you code for device configuration ..
}


