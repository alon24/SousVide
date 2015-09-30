/*
 * ExtendedDisplay.h
 *
 *  Created on: Sep 30, 2015
 *      Author: ilan
 */

#ifndef INCLUDE_EXTENDEDDISPLAY_H_
#define INCLUDE_EXTENDEDDISPLAY_H_

struct textRect {
	int x = -1;
	int y =-1;
	int h = -1;
	int w=-1;
};

class ExtendedDisplay {
public:
	int16_t getCursorX();
	int16_t getCursorY();

	textRect* print(const String &s);
	void printToLoc(const String &s, textRect &t);
	void writeover(textRect &orig, const String &s);

	textRect* getCurrentRect();

	void blink(textRect loc, bool showMarker);
};

#endif /* INCLUDE_EXTENDEDDISPLAY_H_ */
