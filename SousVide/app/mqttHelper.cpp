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

////void onMessageReceived(String topic, String message); // Forward declaration for our callback
//mqttHelper::mqttHelper(String broker, int port, MqttStringSubscriptionCallback callback)
////mqttHelper::mqttHelper(String broker, int port)
//{
//	mqtt = new ReconnctingMqttClient2(broker, port, callback);
//}


mqttHelper::~mqttHelper()
{
}

//// Callback for messages, arrived from MQTT server
//void mqttHelper::onMessageReceived(String topic, String message)
//{
//	Serial.print(topic);
//	Serial.print(":\r\n\t"); // Prettify alignment for printing
//	Serial.println(message);
//	//msg to all client
//	if (topic.compareTo(MQTT_MAIN_TOPIC) == 0){
//		    int start = 0;
//		    int end = 0;
//		    while (end <= message.length())
//		    {
//		    	end = message.indexOf(",", start);
//		    	if (end == -1 ) {
//		    		if (end < message.length()) {
//		    			end = message.length();
//		    		}
//		    	}
//
//		    	String s = message.substring(start, end);
//		        start = end + 1;
////		        end = message.indexOf(",", start);
////		        Serial.println("tok: " + s);
//		        String in = getValue(s, ',' , 0);
//		        String oper = getValue(s, ':' , 1);
//		        	Serial.println("tok: " + s + ", in=" + in + ",oper=" + oper);
//		        	digitalWrite(relayPin, oper.compareTo("1") ? HIGH : LOW);
//		        if (in.compareTo("out1")) {
//		        }
////		        break;
//		    }
//
////		    std::cout << s.substr(start, end);
//
//
////		String val = getValue(message,':',0);
////
////		Serial.println("tok: " + val);
////		Vector<int> splits;
////		splitString(message, ',', splits);
////		for (int i = 0; i < splits.size(); i++)
////		{
////			Serial.print(":\r\n\t"); // Prettify alignment for printing
////			const char* ss = ((const char*)splits.elementAt(i));
////			String *str = new String(ss);
////			Serial.println("tok: " + *str);
////		}
////		message.splitString();
////		if (messag) {
////
////		}
//
////		DynamicJsonBuffer jsonBuffer;
////
////		char* jsonString = strcpy((char*)malloc(message.length()+1), message.c_str());
////		Serial.println("111");
//////		char * jsonString = message.c_str();
////		JsonObject& root = jsonBuffer.parseObject(jsonString);
////		Serial.println("222");
////		JsonObject& payload = root["payload"];
////		Serial.println("333");
////		String com = String((const char*)payload["command"]);
////		Serial.println("444");
////		Serial.println(" commmmm = " + com);
//////		JsonObject jo = JsonBuffer.pa
////
////		free(jsonString);
//}

//boolean mqttHelper::initWithParams(String broker, int port) {
//	//since we cannot replace server, we cannot create a new instance of mqtt so check
//	if (mqtt) {
//		return false;
//	}
//
//	mqtt = new ReconnctingMqttClient2(broker, port, NULL);
//}

void mqttHelper::start() {

	String name;
	int id = system_get_chip_id();
	name = "IHA-";
	name  += id;

	mqtt->connect(name);
	String top = String(MQTT_MAIN_TOPIC) + "/#";
//	Serial.println("mqttHelper::start::Connecting to " + top);
	mqtt->subscribe(top);
}

// Publish our message
void mqttHelper::publishMessage(String topic, String payload)
{
	Serial.println("Let's publish message now! t=" + topic + ", p=" + payload);
	String top = String(MQTT_MAIN_TOPIC) + "/" + topic;
	mqtt->publish(top, payload); // or publishWithQoS
}

void mqttHelper::publishInit()
{
	String payload = "{\"mac\":\"esp-" + WifiAccessPoint.getMAC() + "\"}";
	publishMessage("init", payload);
}
