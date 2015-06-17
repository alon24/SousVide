/*
 * mqttHelper.h
 *
 *  Created on: Jun 12, 2015
 *      Author: ilan
 */

#ifndef APP_MQTTHELPER_H_
#define APP_MQTTHELPER_H_

#include <SmingCore/SmingCore.h>

class mqttHelper
{
	// MQTT client
	MqttClient *mqtt;

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
