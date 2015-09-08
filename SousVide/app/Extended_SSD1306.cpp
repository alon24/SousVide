/*
 * Extended_SSD1306.cpp
 *
 *  Created on: Aug 17, 2015
 *      Author: ilan
 */
#include <Extended_SSD1306.h>
#include <SmingCore/SmingCore.h>

//Timers
Timer textBlinkTimer;

void doBlink() {

}

// initializer for I2C - we only indicate the reset pin!
Extended_SSD1306::Extended_SSD1306(int8_t reset) :
Adafruit_SSD1306(reset) {
	textBlinkTimer.initializeMs(150, doBlink).start();
}

int16_t Extended_SSD1306::getCursorX() {
	return cursor_x;
}

int16_t Extended_SSD1306::getCursorY() {
	return cursor_y;
}

textRect* Extended_SSD1306::print(const String &s){
	textRect *t = new textRect();
	t->x = cursor_x;
	t->y = cursor_y;
	t->h = 8;
	Adafruit_SSD1306::print(s);
	t->w = cursor_x - t->x;
//	Serial.printf("Extended_SSD1306::print %s %s,%s\n", s.c_str(), String(t->x).c_str(), String(t->y).c_str()  );
	return t;
}

void Extended_SSD1306::printToLoc(const String &s, textRect &t){
	setCursor(t.x, t.y);
//	uint32_t free = system_get_free_heap_size();
//	int ff = (int)free;
//	Serial.printf("free printToLoc= %i", ff );
	textRect* newt = this->print(s);
	t.x = newt->x;
	t.y = newt->y;
	t.h = newt->h;
	t.w = newt->w;

	delete(newt);
//	free = system_get_free_heap_size();
//	ff = (int)free;
//	Serial.printf(", 2= %i\n", ff );
}

void Extended_SSD1306::writeover(textRect &orig, const String &s) {
	this->fillRect( orig.x, orig.y, orig.w, orig.h, BLACK);
	this->printToLoc(s, orig);
}

textRect* Extended_SSD1306::getCurrentRect() {
	textRect *t = new textRect();
	t->x = cursor_x;
	t->y = cursor_y;
}

void Extended_SSD1306::blink(textRect loc, bool showMarker) {

}


