#ifndef INCLUDE_SSD1306_extended_H_
#define INCLUDE_SSD1306_extended_H_


#include <BaseDisplay.h>
#include <Libraries/Adafruit_SSD1306/Adafruit_SSD1306.h>

//Timers
Timer textBlinkTimer;

void doBlink() {

}

class SSD1306_ExtendedDisplay: public Adafruit_SSD1306, public BaseDisplay
{
public:

	 ~SSD1306_ExtendedDisplay() {
	 };

	 void init() {
		 Adafruit_SSD1306(4);
//		 textBlinkTimer.initializeMs(150, doBlink).start();
	 };

	 int16_t getCursorX() {
	 	return cursor_x;
	 }

	 int16_t getCursorY() {
	 	return cursor_y;
	 }

	 textRect* print(const String &s){
	 	textRect *t = new textRect();
	 	t->x = cursor_x;
	 	t->y = cursor_y;
	 	t->h = 8;
	 	Adafruit_SSD1306::print(s);
	 	t->w = cursor_x - t->x;
	 //	Serial.printf("SSD1306_ExtendedDisplay::print %s %s,%s\n", s.c_str(), String(t->x).c_str(), String(t->y).c_str()  );
	 	return t;
	 }

	 void printToLoc(const String &s, textRect &t){
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

	 void Ewriteover(textRect &orig, const String &s) {
	 	this->fillRect( orig.x, orig.y, orig.w, orig.h, BLACK);
	 	this->printToLoc(s, orig);
	 }

	 textRect* getCurrentRect() {
	 	textRect *t = new textRect();
	 	t->x = cursor_x;
	 	t->y = cursor_y;
	 }

	 void blink(textRect loc, bool showMarker) {

	 }
};

#endif /* INCLUDE_SSD1306_extended_H_ */
