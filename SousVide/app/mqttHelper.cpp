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
}

//void onMessageReceived(String topic, String message); // Forward declaration for our callback

mqttHelper::mqttHelper(String broker, int port)
{
	mqtt = new ReconnctingMqttClient2(broker, port, NULL);
}


mqttHelper::~mqttHelper()
{
}

////// Callback for messages, arrived from MQTT server
//void onMessageReceived(String topic, String message)
//{
//	Serial.print(topic);
//	Serial.print(":\r\n\t"); // Prettify alignment for printing
//	Serial.println(message);
//}

boolean mqttHelper::initWithParams(String broker, int port) {
	//since we cannot replace server, we cannot create a new instance of mqtt so check
	if (mqtt) {
		return false;
	}

	mqtt = new ReconnctingMqttClient2(broker, port, NULL);
}

void mqttHelper::start() {
	mqtt->connect("esp8266");
	mqtt->subscribe("Dood/#");
}

// Publish our message
void mqttHelper::publishMessage(String topic, String payload)
{
	Serial.println("Let's publish message now! t=" + topic + ", p=" + payload);
	mqtt->publish("Dood/" + topic, payload); // or publishWithQoS
}

void mqttHelper::publishInit()
{
	String payload = "{\"mac\":\"esp-" + WifiAccessPoint.getMAC() + "\"}";
	publishMessage("init", payload);
}
