/*
 * mqttHelper.h
 *
 *  Created on: Jun 12, 2015
 *      Author: ilan
 */

#ifndef APP_MQTTHELPER_H_
#define APP_MQTTHELPER_H_

#include <SmingCore/SmingCore.h>



class ReconnctingMqttClient2: public MqttClient{
      using MqttClient::MqttClient; // Inherit Base's constructors.

    void onError(err_t err)  {
        close();
        connect(mqttName());
        subscribe(commandTopic());
        return;
    }

    String mqttName(){
        String name;
        int id = system_get_chip_id();
        name = "SmartController-";
        name  += id;
        return name;
    }


    String commandTopic(){
        String topic;
        int id = system_get_chip_id();
        topic = "/smart_plug_work/SmartPlug-";
        topic  = topic + id;
        return topic;
    }

};
//// MQTT client
//// For quickly check you can use: http://www.hivemq.com/demos/websocket-client/ (Connection= test.mosquitto.org:8080)
//ReconnctingMqttClient2 mqtt("dmarkey.com", 8000, onMessageReceived);

class mqttHelper
{
	// MQTT client
	ReconnctingMqttClient2 *mqtt;

public:
	mqttHelper();
	mqttHelper(String broker, int port);
	virtual ~mqttHelper();
	boolean initWithParams(String broker, int port);
	void start();
	void publishMessage(String topic, String payload);
	void publishInit();
//	void connectOk();
//	void connectFail();
//	void onMessageReceived(String topic, String message); // Forward declaration for our callback
private:
};
#endif /* APP_MQTTHELPER_H_ */
