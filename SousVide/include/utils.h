/*
 * utils.h
 *
 *  Created on: Jun 18, 2015
 *      Author: ilan
 */

#ifndef INCLUDE_UTILS_H_
#define INCLUDE_UTILS_H_

//#include <Libraries/Adafruit_SSD1306/Adafruit_SSD1306.h>

//#include <menues.h>

// If you want, you can define WiFi settings globally in Eclipse Environment Variables
#ifndef WIFI_SSID
	#define WIFI_SSID "PleaseEnterSSID" // Put you SSID and Password here
	#define WIFI_PWD "PleaseEnterPass"
#endif

byte degree[8] = // define the degree symbol
{
 B00110,
 B01001,
 B01001,
 B00110,
 B00000,
 B00000,
 B00000,
 B00000
};

int getXOnScreenForString(String t, int textSize) {
	int width = t.length() * (textSize == 1 ? 6 : 6);
	int x = 123 - width -1;
	return x;
}

//String getValue(String data, char separator, int index)
//{
//	int found = 0;
//	int strIndex[] = {
//			0, -1  };
//	int maxIndex = data.length()-1;
//	for(int i=0; i<=maxIndex && found<=index; i++){
//		if(data.charAt(i)==separator || i==maxIndex){
//			found++;
//			strIndex[0] = strIndex[1]+1;
//			strIndex[1] = (i == maxIndex) ? i+1 : i;
//		}
//	}
//	return found>index ? data.substring(strIndex[0], strIndex[1]) : "";
//}

// download urls, set appropriately
#define ROM_0_URL  "http://192.168.7.5:80/rom0.bin"
#define ROM_1_URL  "http://192.168.7.5:80/rom1.bin"
#define SPIFFS_URL "http://192.168.7.5:80/spiff_rom.bin"

rBootHttpUpdate* otaUpdater = 0;

void OtaUpdate_CallBack(bool result) {

	Serial.println("In callback...");
	if(result == true) {
		// success
		uint8 slot;
		slot = rboot_get_current_rom();
		if (slot == 0) slot = 1; else slot = 0;
		// set to boot new rom and then reboot
		Serial.printf("Firmware updated, rebooting to rom %d...\r\n", slot);
		rboot_set_current_rom(slot);
		System.restart();
	} else {
		// fail
		Serial.println("Firmware update failed!");
	}
}

void OtaUpdate() {

	uint8 slot;
	rboot_config bootconf;

	Serial.println("Updating...");

	// need a clean object, otherwise if run before and failed will not run again
	if (otaUpdater) delete otaUpdater;
	otaUpdater = new rBootHttpUpdate();

	// select rom slot to flash
	bootconf = rboot_get_config();
	slot = bootconf.current_rom;
	if (slot == 0) slot = 1; else slot = 0;

#ifndef RBOOT_TWO_ROMS
	// flash rom to position indicated in the rBoot config rom table
	otaUpdater->addItem(bootconf.roms[slot], ROM_0_URL);
#else
	// flash appropriate rom
	if (slot == 0) {
		otaUpdater->addItem(bootconf.roms[slot], ROM_0_URL);
	} else {
		otaUpdater->addItem(bootconf.roms[slot], ROM_1_URL);
	}
#endif

#ifndef DISABLE_SPIFFS
	// use user supplied values (defaults for 4mb flash in makefile)
	if (slot == 0) {
		otaUpdater->addItem(RBOOT_SPIFFS_0, SPIFFS_URL);
	} else {
		otaUpdater->addItem(RBOOT_SPIFFS_1, SPIFFS_URL);
	}
#endif

	// request switch and reboot on success
	//otaUpdater->switchToRom(slot);
	// and/or set a callback (called on failure or success without switching requested)
	otaUpdater->setCallback(OtaUpdate_CallBack);

	// start update
	otaUpdater->start();
}

void Switch() {
	uint8 before, after;
	before = rboot_get_current_rom();
	if (before == 0) after = 1; else after = 0;
	Serial.printf("Swapping from rom %d to rom %d.\r\n", before, after);
	rboot_set_current_rom(after);
	Serial.println("Restarting...\r\n");
	System.restart();
}

void initSpiff()
{
	// mount spiffs
	int slot = rboot_get_current_rom();
	#ifndef DISABLE_SPIFFS
		if (slot == 0)
		{
	#ifdef RBOOT_SPIFFS_0
			debugf("trying to mount spiffs at %x, length %d", RBOOT_SPIFFS_0 , 0);
			spiffs_mount_manual(RBOOT_SPIFFS_0, 0);
	#else
			debugf("trying to mount spiffs at %x, length %d", 0x100000, 0);
			spiffs_mount_manual(0x100000, 0);
	#endif
		}
		else
		{
	#ifdef RBOOT_SPIFFS_1
			debugf("trying to mount spiffs at %x, length %d", RBOOT_SPIFFS_1 , 0);
			spiffs_mount_manual(RBOOT_SPIFFS_1, 0);
	#else
			debugf("trying to mount spiffs at %x, length %d", 0);
			spiffs_mount_manual(0x300000, 0);
	#endif
		}
	#else
		debugf("spiffs disabled");
	#endif
	//WifiAccessPoint.enable(false);

	Serial.printf("\r\nCurrently running rom %d.\r\n", slot);
	Serial.println("Type 'help' and press enter for instructions.");
	Serial.println();
}



#endif /* INCLUDE_UTILS_H_ */
