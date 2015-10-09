/*
 * ExtendedDisplay.h
 *
 *  Created on: Sep 30, 2015
 *      Author: ilan
 */

#ifndef INCLUDE_BASEDISPLAY_H_
#define INCLUDE_BASEDISPLAY_H_

#include <SmingCore/SmingCore.h>

struct textRect {
	int x = -1;
	int y =-1;
	int h = -1;
	int w=-1;
};

class BaseDisplay {
public:
	virtual void init();
	virtual int16_t getCursorX();
	virtual int16_t getCursorY();

	virtual textRect* print(const String &s);
	virtual void printToLoc(const String &s, textRect &t);
	virtual void writeover(textRect &orig, const String &s);

	virtual textRect* getCurrentRect();

	virtual void blink(textRect loc, bool showMarker);
};

#endif /* INCLUDE_BASEDISPLAY_H_ */
