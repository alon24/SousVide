/*
 * Rotary.h
 *
 *  Created on: Dec 5, 2015
 *      Author: iklein
 */

#ifndef INCLUDE_UTILS_ROTARY_H_
#define INCLUDE_UTILS_ROTARY_H_

#include "SmingCore.h"
#include "utils/MultiFunctionButton.h"

enum class RotaryAction {
	PREV = 0,
	NEXT = 1,
	NONE = 2
};

typedef Delegate<void()> RotaryButtonActionDelegate;
typedef Delegate<void(RotaryAction)> RotaryWheelActionDelegate;

class Rotary {

private:
	MultiFunctionButton* btn = NULL;
	int encoderCLK =12;
	int encoderDT =13;
	volatile int lastEncoded = 0;
	volatile long encoderValue = 0;
	long lastencoderValue = 0;
	int lastValue =-1000;
	ButtonActionDelegate delegatedActionEvent = NULL;
	RotaryWheelActionDelegate delegatedWheelEvent = NULL;
	Timer rotaryTimer;
	int rotaryChangeCountResolution=4;

	volatile RotaryAction act = RotaryAction::NONE;

	volatile unsigned long threshold = 1000;


	// 'rotaryHalfSteps' is the counter of half-steps. The actual
	// number of steps will be equal to rotaryHalfSteps / 2
	//
	volatile long rotaryHalfSteps = 0;

	// Working variables for the interrupt routines
	//
	volatile unsigned long int0time = 0;
	volatile unsigned long int1time = 0;
	volatile uint8_t int0signal = 0;
	volatile uint8_t int1signal = 0;
	volatile uint8_t int0history = 0;
	volatile uint8_t int1history = 0;
	int lastRotarySteps =0;
	bool rotary = false;

public:
	Rotary(){
		rotaryTimer.initializeMs(100, TimerDelegate(&Rotary::updateInternalRotaryState, this)).start();
	};

	~Rotary(){
		delete (btn);
		rotaryTimer.stop();
	};

	Rotary(int btnPin, int encoderCLK, int encoderDT) {
		Rotary();
		this->init(encoderCLK, encoderDT);
		this->initBtn(btnPin);
	};

	void init(int encoderCLK, int encoderDT) {
		this->encoderCLK = encoderCLK;
		this->encoderDT = encoderDT;

		attachInterrupt(encoderCLK, RotaryButtonActionDelegate(&Rotary::int0, this), CHANGE);
		attachInterrupt(encoderDT, RotaryButtonActionDelegate(&Rotary::int1, this), CHANGE);
	};

	void int0()
		{
		if ( micros() - int0time < threshold )
			return;
		int0history = int0signal;
		int0signal = digitalRead(encoderCLK);
		if ( int0history==int0signal )
			return;
		int0time = micros();
		if ( int0signal == int1signal )
			rotaryHalfSteps++;
		else
			rotaryHalfSteps--;

		showResult();
//		debugf("%i",rotaryHalfSteps);
		}

	void int1()
		{
		if ( micros() - int1time < threshold )
			return;
		int1history = int1signal;
		int1signal = digitalRead(encoderDT);
		if ( int1history==int1signal )
			return;
		int1time = micros();
//		debugf("%i",rotaryHalfSteps);
		}

	void showResult() {
		int tmp = rotaryHalfSteps;
//		if (tmp %2 != 0 ) {
//			return;
//		}

		if (tmp == lastRotarySteps) {
			return;
		}

		if (lastRotarySteps > tmp) {
			act = RotaryAction::PREV;
//			debugf("setting prev, %i, %i", lastRotarySteps,tmp/2);
		}
		else {
			act = RotaryAction::NEXT;
//			debugf("setting next, %i, %i", lastRotarySteps,tmp/2);
		}

//		lastRotarySteps = rotaryHalfSteps /2;

		debugf("%i",tmp );
//		rotary = false;
	}

	MultiFunctionButton* initBtn(int buttonPin, ButtonActionDelegate handler = null, bool pressAndHold = true) {
		if (!btn) {
			btn = new MultiFunctionButton();
		}

		pinMode(buttonPin, INPUT);
		digitalWrite(buttonPin, HIGH); //turn pullup resistor on
		btn->initBtn(buttonPin, handler, pressAndHold);
		return btn;
	};

	//Delegated call when event is triggered
	void setOnWheelEvent(RotaryWheelActionDelegate handler) {
		debugf("set setOnWheelEvent");
		delegatedWheelEvent  = handler;
	}

	MultiFunctionButton* getButton() {
		return btn;
	}

	void updateInternalRotaryState() {
//		int tmp = lastRotarySteps;
//		rotary = true;

//		if (tmp > lastencoderValue && (tmp - lastencoderValue >4)) {
//			act = RotaryAction::PREV;
//			debugf("Prev");
//			lastencoderValue = tmp;
//
//		}
//		else if (tmp < lastencoderValue && (lastencoderValue - tmp >4)){
//			act = RotaryAction::NEXT;
//			debugf("Next");
//			lastencoderValue = tmp;
//		}
//
		if (!delegatedWheelEvent) {
			debugf("no delegatedWheelEvent");
			return;
		}

		if (act != RotaryAction::NONE) {
			debugf("state= %i,%i,%i", (act == RotaryAction::PREV), (act == RotaryAction::NEXT), (act == RotaryAction::NONE));
			delegatedWheelEvent(act);
			act = RotaryAction::NONE;
//			debugf("rotary:none");
			//only change after consume
			lastRotarySteps = rotaryHalfSteps /2;
		}
	}

	void btnClicked(MultiFunctionButtonAction event) {
		switch (event) {
			case BTN_CLICK:
				debugf("click");
//				moveRight();
				break;
			case BTN_DOUBLE_CLICK:
				debugf("BTN_DOUBLE_CLICK");
//				moveLeft();
				break;
			case BTN_LONG_CLICK:
				debugf("BTN_LONG_CLICK");
				break;
			case BTN_HOLD_CLICK:
				debugf("BTN_HOLD_CLICK");
				break;
		}
	}
};

#endif /* INCLUDE_UTILS_ROTARY_H_ */
