#include "../include/configuration.h"

#include <SmingCore.h>

SousvideConfig ActiveConfig;

SousvideConfig loadConfig()
{
	DynamicJsonBuffer jsonBuffer;
	SousvideConfig cfg;
	if (fileExist(SOUSVIDE_CONFIG_FILE))
	{
		debugf("loading file exists");
		int size = fileGetSize(SOUSVIDE_CONFIG_FILE);
//		char* jsonString = new char[size + 1];
		String jsonString = fileGetContent(SOUSVIDE_CONFIG_FILE);

		debugf("loaded file = %s",jsonString.c_str() );

		JsonObject& root = jsonBuffer.parseObject(jsonString);

		JsonObject& network = root["network"];
		cfg.NetworkSSID = String((const char*)network["StaSSID"]);
		cfg.NetworkPassword = String((const char*)network["StaPassword"]);

		JsonObject& sousvide = root["Sousvide"];
		cfg.Needed_temp = sousvide["T"];
		cfg.Kp = sousvide["kp"];
		cfg.Ki = sousvide["ki"];
		cfg.Kd = sousvide["kd"];

		JsonObject& operations = root["operations"];
		cfg.operationMode = operations["type"] == "0" ? Manual : Sousvide;
		cfg.highlow = operations["highlow"] == "0" ? false : true;
	}
	else
	{
		//Factory defaults if no config file present
		cfg.NetworkSSID = WIFI_SSID;
		cfg.NetworkPassword = WIFI_PWD;
//		cfg.Needed_temp =54;
//		cfg.Kp =9;
//		cfg.Ki =9;
//		cfg.Kd =9;
		cfg.highlow = 0;
		cfg.operationMode = Manual;
	}
	return cfg;
}

void saveConfig(SousvideConfig& cfg)
{
	DynamicJsonBuffer jsonBuffer;
	JsonObject& root = jsonBuffer.createObject();

	JsonObject& network = jsonBuffer.createObject();
	root["network"] = network;
//	network["ssid"] = cfg.NetworkSSID.c_str();
//	network["StaPassword"] = cfg.NetworkPassword.c_str();

	JsonObject& sousvide = jsonBuffer.createObject();
	root["Sousvide"] = sousvide;
	sousvide["T"] = cfg.Needed_temp;
	sousvide["kp"] = cfg.Kp;
	sousvide["ki"] = cfg.Ki;
	sousvide["kd"] = cfg.Kd;
//	sousvide["enabled"] = cfg.enabled;

	JsonObject& operations = jsonBuffer.createObject();
	root["operations"] = operations;
	operations["type"] = cfg.operationMode == Manual ? "0" : "1";
	operations["highLow"] = String(cfg.highlow);

	String st = "";
	root.prettyPrintTo(st);
//	char buf[ConfigFileBufferSize];
//	root.prettyPrintTo(buf, sizeof(buf));
	debugf("saved file = %s",st.c_str() );
	fileSetContent(SOUSVIDE_CONFIG_FILE, st);
}


