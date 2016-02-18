/*
 * PCD8544_Driver.cpp
 *
 *  Created on: Aug 17, 2015
 *      Author: ilan
 */
#include <drivers/PCD8544_Driver.h>

// Software SPI with explicit CS pin.
PCD8544_Driver::PCD8544_Driver(int8_t SCLK, int8_t DIN, int8_t DC, int8_t CS, int8_t RST) : Base_Display_Driver(), Adafruit_PCD8544(SCLK, DIN, DC, CS, RST){

}

// Software SPI with CS tied to ground.  Saves a pin but other pins can't be shared with other hardware.
PCD8544_Driver::PCD8544_Driver(int8_t SCLK, int8_t DIN, int8_t DC, int8_t RST) : Base_Display_Driver(), Adafruit_PCD8544(SCLK, DIN, DC, RST){

}
// Hardware SPI based on hardware controlled SCK (SCLK) and MOSI (DIN) pins. CS is still controlled by any IO pin.
// NOTE: MISO and SS will be set as an input and output respectively, so be careful sharing those pins!
PCD8544_Driver::PCD8544_Driver(int8_t DC, int8_t CS, int8_t RST) : Adafruit_PCD8544(DC, CS, RST), Base_Display_Driver(){

}

void PCD8544_Driver::init() {
}

int16_t PCD8544_Driver::getCursorX() {
	return cursor_x;
}

int16_t PCD8544_Driver::getCursorY() {
	return cursor_y;
}

textRect* PCD8544_Driver::print(const String &s){
	textRect *t = new textRect();
	t->x = cursor_x;
	t->y = cursor_y;
	t->h = 8;
	Adafruit_PCD8544::print(s);
	t->w = cursor_x - t->x;
//	Serial.printf("PCD8544_Driver::print %s %s,%s\n", s.c_str(), String(t->x).c_str(), String(t->y).c_str()  );
	return t;
}

void PCD8544_Driver::printToLoc(const String &s, textRect &t, int color){
	setCursor(t.x, t.y);
//	uint32_t free = system_get_free_heap_size();
//	int ff = (int)free;
//	Serial.printf("free printToLoc= %i", ff );

	int tempTextColor = textcolor;
	setTextColor(color);

	textRect* newt = this->print(s);
	t.x = newt->x;
	t.y = newt->y;
	t.h = newt->h;
	t.w = newt->w;

	delete(newt);

	setTextColor(tempTextColor);
//	free = system_get_free_heap_size();
//	ff = (int)free;
//	Serial.printf(", 2= %i\n", ff );
}

void PCD8544_Driver::writeover(textRect &orig, const String &s, bool inverse) {
	int textColor = WHITE;
	int bkColor = BLACK;
	if (inverse) {
		textColor = BLACK;
		bkColor = WHITE;
	}

	this->fillRect( orig.x, orig.y, orig.w, orig.h, bkColor);
	this->printToLoc(s, orig, textColor);
}

textRect* PCD8544_Driver::getCurrentRect() {
	textRect *t = new textRect();
	t->x = cursor_x;
	t->y = cursor_y;
}

int PCD8544_Driver::getMaxLines() {
	return 6;
}

int PCD8544_Driver::getBlack() {
	return BLACK;
}

int PCD8544_Driver::getWhite() {
	return WHITE;
}

void PCD8544_Driver::clearDisplay(void){
	Adafruit_PCD8544::fillScreen(WHITE);
}

void PCD8544_Driver::fillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color){
	Adafruit_PCD8544::fillRect(x, y, w, h, color);
}

void PCD8544_Driver::display(){
//	Adafruit_PCD8544::display();
}

void PCD8544_Driver::setCursor(int16_t x, int16_t y){
	Adafruit_PCD8544::setCursor(x, y);
}

void PCD8544_Driver::drawFastVLine(int16_t x, int16_t y, int16_t h, uint16_t color){
	Adafruit_PCD8544::drawFastVLine(x, y, h, color);
}

void PCD8544_Driver::drawFastHLine(int16_t x, int16_t y, int16_t w, uint16_t color){
	Adafruit_PCD8544::drawFastHLine(x, y, w, color);
}

void PCD8544_Driver::drawPixel(int16_t x, int16_t y, uint16_t color){
	Adafruit_PCD8544::drawPixel(x, y, color);
}

size_t PCD8544_Driver::println(void){
	return Adafruit_PCD8544::println();
}

void PCD8544_Driver::setTextSize(uint8_t s){
	Adafruit_PCD8544::setTextSize(s);
}

void PCD8544_Driver::setTextColor(uint16_t c){
	Adafruit_PCD8544::setTextColor(c);
}

