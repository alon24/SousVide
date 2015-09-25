#include "../include/configuration.h"

#include <SmingCore/SmingCore.h>

SousvideConfig ActiveConfig;

SousvideConfig loadConfig()
{
	DynamicJsonBuffer jsonBuffer;
	SousvideConfig cfg;
	if (fileExist(SOUSVIDE_CONFIG_FILE))
	{
		int size = fileGetSize(SOUSVIDE_CONFIG_FILE);
		char* jsonString = new char[size + 1];
		fileGetContent(SOUSVIDE_CONFIG_FILE, jsonString, size + 1);
		JsonObject& root = jsonBuffer.parseObject(jsonString);

		JsonObject& network = root["network"];
		cfg.NetworkSSID = String((const char*)network["ssid"]);
		cfg.NetworkPassword = String((const char*)network["password"]);

		JsonObject& sousvide = root["Sousvide"];
		cfg.Needed_temp = sousvide["T"];
		cfg.Kp = sousvide["kp"];
		cfg.Ki = sousvide["ki"];
		cfg.Kd = sousvide["kd"];

		JsonObject& trigger = root["trigger"];
		cfg.operationMode = (OperationMode)(int)trigger["type"];

		delete[] jsonString;
	}
	else
	{
		cfg.NetworkSSID = WIFI_SSID;
		cfg.NetworkPassword = WIFI_PWD;
	}
	return cfg;
}

void saveConfig(SousvideConfig& cfg)
{
	ActiveConfig = cfg;

	DynamicJsonBuffer jsonBuffer;
	JsonObject& root = jsonBuffer.createObject();

	JsonObject& network = jsonBuffer.createObject();
	root["network"] = network;
	network["ssid"] = cfg.NetworkSSID.c_str();
	network["password"] = cfg.NetworkPassword.c_str();

	JsonObject& sousvide = jsonBuffer.createObject();
	root["Sousvide"] = sousvide;
	sousvide["T"] = cfg.Needed_temp;
	sousvide["kp"] = cfg.Kp;
	sousvide["ki"] = cfg.Ki;
	sousvide["kd"] = cfg.Kd;

	JsonObject& trigger = jsonBuffer.createObject();
	root["trigger"] = trigger;
	trigger["type"] = (int)cfg.operationMode;

	char buf[3048];
	root.prettyPrintTo(buf, sizeof(buf));
	fileSetContent(SOUSVIDE_CONFIG_FILE, buf);
}


