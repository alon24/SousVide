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

#endif /* INCLUDE_UTILS_H_ */
