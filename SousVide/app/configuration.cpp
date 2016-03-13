#include "../include/configuration.h"

#include <SmingCore.h>

SousvideConfig ActiveConfig;

SousvideConfig loadConfig()
{
	StaticJsonBuffer<ConfigJsonBufferSize> jsonBuffer;
	SousvideConfig cfg;
	if (fileExist(SOUSVIDE_CONFIG_FILE))
	{
		int size = fileGetSize(SOUSVIDE_CONFIG_FILE);
		char* jsonString = new char[size + 1];
		fileGetContent(SOUSVIDE_CONFIG_FILE, jsonString, size + 1);
		JsonObject& root = jsonBuffer.parseObject(jsonString);

		JsonObject& network = root["network"];
		cfg.NetworkSSID = String((const char*)network["StaSSID"]);
		cfg.NetworkPassword = String((const char*)network["StaPassword"]);

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
		//Factory defaults if no config file present
		cfg.NetworkSSID = WIFI_SSID;
		cfg.NetworkPassword = WIFI_PWD;
		cfg.Needed_temp =54;
		cfg.Kp =9;
		cfg.Ki =9;
		cfg.Kd =9;
	}
	return cfg;
}

void saveConfig(SousvideConfig& cfg)
{
	StaticJsonBuffer<ConfigJsonBufferSize> jsonBuffer;
	JsonObject& root = jsonBuffer.createObject();

	JsonObject& network = jsonBuffer.createObject();
	root["network"] = network;
	network["ssid"] = cfg.NetworkSSID.c_str();
	network["StaPassword"] = cfg.NetworkPassword.c_str();

	JsonObject& sousvide = jsonBuffer.createObject();
	root["Sousvide"] = sousvide;
	sousvide["T"] = cfg.Needed_temp;
	sousvide["kp"] = cfg.Kp;
	sousvide["ki"] = cfg.Ki;
	sousvide["kd"] = cfg.Kd;

	JsonObject& trigger = jsonBuffer.createObject();
	root["trigger"] = trigger;
	trigger["type"] = (int)cfg.operationMode;

	char buf[ConfigFileBufferSize];
	root.prettyPrintTo(buf, sizeof(buf));
	fileSetContent(SOUSVIDE_CONFIG_FILE, buf);
}


