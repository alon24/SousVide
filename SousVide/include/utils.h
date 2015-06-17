/*
 * utils.h
 *
 *  Created on: Jun 18, 2015
 *      Author: ilan
 */

#ifndef INCLUDE_UTILS_H_
#define INCLUDE_UTILS_H_

#include <menues.h>

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

int getCenterXForString(MenuParams *params, String t, int textSize) {
	int width = t.length() * (textSize == 1 ? 6 : 6);
	int x = (128 - width)/2 + (params->boxed ? 1 : 0);
	return x;
}

#endif /* INCLUDE_UTILS_H_ */
