/*
 * ILI9341_Driver.h
 *
 *  Created on: Dec 6, 2015
 *      Author: iklein
 */

#ifndef INCLUDE_DRIVERS_PCD8544_DRIVER_H_
#define INCLUDE_DRIVERS_PCD8544_DRIVER_H_

#include <Libraries/Adafruit_PCD8544/Adafruit_PCD8544.h>
//#include <SmingCore/SmingCore.h>
#include "Base_Display_Driver.h"

class PCD8544_Driver: public Base_Display_Driver, public Adafruit_PCD8544 {
public:
	// Software SPI with explicit CS pin.
	PCD8544_Driver(int8_t SCLK, int8_t DIN, int8_t DC, int8_t CS, int8_t RST);
	// Software SPI with CS tied to ground.  Saves a pin but other pins can't be shared with other hardware.
	PCD8544_Driver(int8_t SCLK, int8_t DIN, int8_t DC, int8_t RST);
	// Hardware SPI based on hardware controlled SCK (SCLK) and MOSI (DIN) pins. CS is still controlled by any IO pin.
	// NOTE: MISO and SS will be set as an input and output respectively, so be careful sharing those pins!
	PCD8544_Driver(int8_t DC, int8_t CS, int8_t RST);

	void init();
	int16_t getCursorX();
	int16_t getCursorY();
	textRect* print(const String &s);
	void printToLoc(const String &s, textRect &t, int textColor);
	void writeover(textRect &orig, const String &s, bool inverse = false);
	textRect* getCurrentRect();
	int getMaxLines();
	int getBlack();
	int getWhite();

	void clearDisplay(void);
	void fillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color);
	void display();
	void setCursor(int16_t x, int16_t y);
	void drawFastVLine(int16_t x, int16_t y, int16_t h, uint16_t color);
	void drawFastHLine(int16_t x, int16_t y, int16_t w, uint16_t color);
	void drawPixel(int16_t x, int16_t y, uint16_t color);
	size_t println(void);
	void setTextSize(uint8_t s);
	void setTextColor(uint16_t c);
};

#endif /* INCLUDE_DRIVERS_PCD8544_DRIVER_H_ */
