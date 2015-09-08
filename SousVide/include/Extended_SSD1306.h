/*
 * Extended_SSD1306.h
 *
 *  Created on: Aug 17, 2015
 *      Author: ilan
 */

#ifndef INCLUDE_EXTENDED_SSD1306_H_
#define INCLUDE_EXTENDED_SSD1306_H_

#include <Libraries/Adafruit_SSD1306/Adafruit_SSD1306.h>

struct textRect {
	int x = -1;
	int y =-1;
	int h = -1;
	int w=-1;
};

class Extended_SSD1306 : public Adafruit_SSD1306 {
public:
	Extended_SSD1306(int8_t RST);

	int16_t getCursorX();
	int16_t getCursorY();

	textRect* print(const String &s);
	void printToLoc(const String &s, textRect &t);
	void writeover(textRect &orig, const String &s);

	textRect* getCurrentRect();

	void blink(textRect loc, bool showMarker);

};
#endif /* INCLUDE_EXTENDED_SSD1306_H_ */
