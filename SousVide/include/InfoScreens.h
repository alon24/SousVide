/*
 * InfoScreens.h
 *
 *  Created on: Aug 20, 2015
 *      Author: ilan
 */

#ifndef INCLUDE_INFOSCREENS_H_
#define INCLUDE_INFOSCREENS_H_

#include <SmingCore/SmingCore.h>
#include <Extended_SSD1306.h>

#define TIME_BETWEEN_SCREEN_CHANGE 300

struct paramState {
	bool dirty = false;
	String val;

	void update(String newVal) {
		val = newVal;
		dirty = true;
	}

	void clearDirty() {
		dirty = false;
	}
};

struct paramStruct{
	textRect t;
//	String text;
	String id;

	void init(String id, String text, textRect t) {
		this->id = id;
		this->t = t;
	}
};

class BaseScreenElement
{

protected:
	String id;
	BaseScreenElement* parent;
	Extended_SSD1306* display;

public:

	BaseScreenElement(String id) {
		this->id = id;
	};

	void setDisplay(Extended_SSD1306* disp) {
		this->display = disp;
	}

	String getId() {
		return id;
	};

	void setParent(BaseScreenElement* parent) {
		this->parent = parent;
	};

	BaseScreenElement* getParent() {
		return parent;
	}
	virtual void setCanUpdateDisplay(bool newState) {};
	virtual bool canUpdateDisplay() {};
	virtual void updateParamValue(String id, String newData) {};
	virtual paramState getParamText(String id){};
};

class InfoPageLine : public BaseScreenElement
{
	String m_text;
	int m_textSize;
	bool initialized = false;
	Vector<paramStruct*> params;
public:

	int mX, mY, mWidth;
	InfoPageLine(String id, String text, int size);
	int getTextSize();
	String getText();
	paramStruct* addParam(String id, String text);
	paramStruct* addParam(String id, String text, textRect initial);

	//prints the element
	void print();
//	void updateData(Extended_SSD1306 display, paramStruct* param, String newData);
//	void updateData(Extended_SSD1306 display, int index, String newData);
//	void updateDataForId(Extended_SSD1306 display, String id, String newData);

//	paramStruct* getParam(int index);
	paramStruct* getParamById(String id);

	bool canUpdateDisplay(bool newState) {
		return parent->canUpdateDisplay();
	}

	paramState getParamText(String id){
		return parent->getParamText(id);
	}

//	void updateParamValue(String id, String value) {
//		getParent()->getParent()->updateParamValue(id, value);
//	}
};

class InfoScreenPage : public BaseScreenElement{
	Vector<InfoPageLine*> mChildren;
	String m_header;
public:
	InfoScreenPage(String id, String header) : BaseScreenElement(id) {
		m_header = header;
	};

	/**
	 * creates and adds to parent
	 */
	InfoPageLine* createLine(String id, String text) {
		InfoPageLine* el =  new InfoPageLine(id, text, 1);
		el->setParent(this);
		el->setDisplay(&*display);
		addElemenet(el);
		return el;
	}

	void addElemenet(InfoPageLine* el){
		el->setParent(this);
		el->setDisplay(&*display);
		mChildren.add(el);
	};

	InfoPageLine* itemAt(int index) {
		return mChildren.get(index);
	};

	Vector<InfoPageLine*> getItems()
	{
		return mChildren;
	};

	InfoPageLine* getChildById(String id) {
		for (int i = 0; i < mChildren.size(); ++i) {
			if(mChildren.get(i)->getId() == id) {
				return mChildren.get(i);
			}
		}
		return NULL;
	}

//	//Does NOT call display.display();
//	void updateParam( String id, String newVal, Extended_SSD1306 display) {
//		Vector<InfoPageLine*> ret = getAllLinesParamsForId(id);
//		for (int z = 0; z < ret.size(); ++z) {
//			ret.elementAt(z)->updateDataForId(display, id, newVal);
//		}
//
//		ret.removeAllElements();
//	}
//
//	void updateAllParams( Vector<String> ids, Vector<String> newVals, Extended_SSD1306 display) {
//		for (int i = 0; i < ids.size(); ++i) {
//			updateParam(ids.elementAt(i), newVals.elementAt(i), display);
//		}
//
//		display.display();
//	}

	Vector<paramStruct*> getAllParamsForId(String id) {
		Vector<paramStruct*> ret;
		for (int i = 0; i < mChildren.size(); ++i) {
			InfoPageLine* l = mChildren.elementAt(i);
			paramStruct* p = l->getParamById(id);
			if ( p != NULL) {
				ret.add(p);
			}
		}

		return ret;
	}

	//No screen update
	void updateParamValue(String id, String newData) {
		getParent()->updateParamValue(id, newData);
	}

	void print() {
		if(!canUpdateDisplay()) {
			Serial.println("Cannot Update display, flag is false");
			return;
		}
//		debugf("print,3.1 ");
		display->clearDisplay();
//		debugf("print,3.2 ");
		display->setCursor(0,0);
		for(int i=0; i< mChildren.size(); i++){
//			debugf("print,3.3 - %i ", i);
			InfoPageLine* child = mChildren.get(i);
			child->print();
//			debugf("print,3.4 - %i ", i);
		}
//		debugf("print,3.6 ");
		display->display();
//		debugf("print,3.7 ");
	}

	bool canUpdateDisplay() {
		return getParent()->canUpdateDisplay();
	}

	paramState getParamText(String id){
		return parent->getParamText(id);
	}
};

typedef Delegate<void()> showScreenUpdateDelegate;

class InfoScreens : public BaseScreenElement{

private:
	int mCurrent=0;
	Vector<InfoScreenPage*> mChildern;
//	Vector<paramStruct*> dirtyElements;
	HashMap<String, paramState> paramValueMap;

	bool updateDisplay = false;
	unsigned long lastUpdateTime = 0;
//	Timer updateNextTimer;
	Timer screenupdate;

public:
	void handleUpdateTimer() {
		if(canUpdateDisplay()) {
//			if()
			showCurrent();

		}


	}

//	InfoScreens(String id) : BaseScreenElement(id) {
//
//	}

	InfoScreens(String id, Extended_SSD1306 *dis) : BaseScreenElement(id)
	{
		this->display = dis;
		screenupdate.setCallback(showScreenUpdateDelegate(&InfoScreens::handleUpdateTimer, this));

		display->print("InfoScreens");
		Serial.print(display->getCursorY());
		display->display();
	}

	void setCurrent(int index) {
		if (index >= mChildern.size()) {
			return;
		}

		mCurrent = index;
	}

	void showCurrent() {
		lastUpdateTime = millis();
//		debugf("showCurrent,1");
		this->updateDisplay = true;
//		debugf("showCurrent,2");
		print(mCurrent);
//		debugf("showCurrent,3");
	}

	void show(int pNum) {
		mCurrent = pNum;
//		debugf("show:%i", pNum);
		showCurrent();
	}

	void doMove(boolean ) {

	}

	void moveRight() {
		if (mChildern.size() == 1) {
			return;
		}

		int tmpTime = millis();
		long mils = tmpTime - lastUpdateTime;
		if (mils < TIME_BETWEEN_SCREEN_CHANGE) {
			return;
		}
		lastUpdateTime = tmpTime;
		debugf("moveRight mills=%lu", lastUpdateTime);

		debugf("moveRight mCurrent=%i" , mCurrent);
		if (mCurrent + 1 < mChildern.size()) {
			mCurrent++;
		}
		else {
			mCurrent = 0;
		}
		debugf("moveRight mCurrent after=%i" , mCurrent);

		display->clearDisplay();
		display->setCursor(0,0);
		showCurrent();
	}

	void moveLeft() {
		if (mChildern.size() == 1) {
			return;
		}

		debugf("moveLeft mCurrent=%i" , mCurrent);

		int tmpTime = millis();
		long mils = tmpTime - lastUpdateTime;
		if (mils < TIME_BETWEEN_SCREEN_CHANGE) {
			return;
		}

		lastUpdateTime = tmpTime;
		debugf("moveLeft mills=%lu", lastUpdateTime);

		if (mCurrent - 1 >= 0) {
			mCurrent--;
		}
		else {
			mCurrent = mChildern.size()-1;
		}

		debugf("moveLeft mCurrent after=%i" , mCurrent);

		this->display->clearDisplay();
		display->setCursor(0,0);
		showCurrent();
	}

//	//Do not print to screen anymore(so no updates to data)
//	void hide(){
//		this->updateDisplay = false;
//	}

	InfoScreenPage* createScreen(String id, String header){
		InfoScreenPage* el = new InfoScreenPage(id, header);
		el->setParent(this);
		el->setDisplay(&*display);
		mChildern.add(el);
		return el;
	}

	void addPage(InfoScreenPage* page) {
		page->setParent(this);
		page->setDisplay(&*display);
		mChildern.add(page);
	}

	InfoScreenPage* get(int index) {
		if (mChildern.size() >= index) {
			return mChildern.get(index);
		}
		return NULL;
	}

	InfoScreenPage* getCurrent() {
		return mChildern.get(mCurrent);
	}

	void print(int pIndex) {
		if(!canUpdateDisplay()) {
			Serial.println("Cannot Update display, flag is false");
			return;
		}
//		Serial.println("Printing index = " +String(pIndex));
		InfoScreenPage* p = get(pIndex);
		debugf("print,3 %s", p->getId().c_str() );
		p->print();
		debugf("print, 4");
	}

	paramState getParamText(String id) {
		return paramValueMap[id];
	}

	//no screen update
	void updateParamValue(String id, String newData) {
		if (paramValueMap.contains(id)) {
			paramValueMap[id].update(newData);
//			paramValueMap.remove(id);
		}

		paramState p;
		p.update(newData);
		paramValueMap[id] = p;

//		if (canUpdateDisplay()) {
//			Vector<paramStruct*> params = getCurrent()->getAllParamsForId(id);
//			for (int i = 0; i < params.size(); ++i) {
//				paramStruct* param = params.get(i);
//				display->writeover(param->t, newData);
//			}
//		}
	}

	void setCanUpdateDisplay(bool newState){
		this->updateDisplay = newState;
	}

	bool canUpdateDisplay(){
		return updateDisplay;
	}

};

#endif /* INCLUDE_INFOSCREENS_H_ */
