#include "../include/configuration.h"

#include <SmingCore.h>

SousvideConfig ActiveConfig;

SousvideConfig loadConfig()
{
	DynamicJsonBuffer jsonBuffer;
//	StaticJsonBuffer<ConfigJsonBufferSize> jsonBuffer;
	SousvideConfig cfg;
	if (fileExist(SOUSVIDE_CONFIG_FILE))
	{
		int size = fileGetSize(SOUSVIDE_CONFIG_FILE);
//		char* jsonString = new char[size + 1];
		String jsonString = fileGetContent(SOUSVIDE_CONFIG_FILE);
		JsonObject& root = jsonBuffer.parseObject(jsonString);

		JsonObject& network = root["network"];
		cfg.NetworkSSID = String((const char*)network["StaSSID"]);
		cfg.NetworkPassword = String((const char*)network["StaPassword"]);

		JsonObject& sousvide = root["Sousvide"];
		cfg.Needed_temp = sousvide["T"];
		cfg.Kp = sousvide["kp"];
		cfg.Ki = sousvide["ki"];
		cfg.Kd = sousvide["kd"];
		cfg.enabled = sousvide["enabled"];


		JsonObject& trigger = root["operations"];
		cfg.operationMode = (OperationMode)(int)trigger["type"];

//		delete[] jsonString;
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
	DynamicJsonBuffer jsonBuffer;
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
	sousvide["enabled"] = cfg.enabled;

	JsonObject& operations = jsonBuffer.createObject();
	root["operations"] = operations;
	operations["type"] = (int)cfg.operationMode;
	operations["HighLow"] = int(cfg.highlow);
	String st = "";
	root.prettyPrintTo(st);
//	char buf[ConfigFileBufferSize];
//	root.prettyPrintTo(buf, sizeof(buf));
	fileSetContent(SOUSVIDE_CONFIG_FILE, st);
}


