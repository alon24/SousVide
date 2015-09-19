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

struct paramStruct {
	textRect t;
//	String text;
	String id;

	void init(String id, String text, textRect t) {
		this->id = id;
//		this->text = text;
		this->t = t;
	}
};

class BaseScreenElement
{

protected:
	String id;
	BaseScreenElement* parent;

public:

	BaseScreenElement(String id) {
		this->id = id;
	};

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
	virtual String getParamText(String id){};
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
	void print(Extended_SSD1306 &display);
//	void updateData(Extended_SSD1306 display, paramStruct* param, String newData);
//	void updateData(Extended_SSD1306 display, int index, String newData);
//	void updateDataForId(Extended_SSD1306 display, String id, String newData);

//	paramStruct* getParam(int index);
	paramStruct* getParamById(String id);

	bool canUpdateDisplay(bool newState) {
		return parent->canUpdateDisplay();
	}

	String getParamText(String id){
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
		addElemenet(el);
		return el;
	}

	void addElemenet(InfoPageLine* el){
		el->setParent(this);
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

	void print(Extended_SSD1306 display) {
		if(!canUpdateDisplay()) {
			Serial.println("Cannot Update display, flag is false");
			return;
		}
		display.clearDisplay();
//		display.setCursor(0,0);
		for(int i=0; i< mChildren.size(); i++){
			InfoPageLine* child = mChildren.get(i);
			child->print(display);
		}

		display.display();
	}

	bool canUpdateDisplay() {
		return getParent()->canUpdateDisplay();
	}

	String getParamText(String id){
		return parent->getParamText(id);
	}
};

class InfoScreens : public BaseScreenElement{
	int mCurrent = 0;
	Vector<InfoScreenPage*> mChildern;
	Vector<paramStruct*> dirtyElements;
	HashMap<String, String> paramValueMap;
	bool updateDisplay = false;

public:
	InfoScreens(String id) : BaseScreenElement(id) {
	}

	void showCurrent( Extended_SSD1306 display) {
		this->updateDisplay = true;
		print(mCurrent, display);
	}

	void show(int pNum, Extended_SSD1306 display) {
		mCurrent = pNum;
		showCurrent(display);
	}

//	//Do not print to screen anymore(so no updates to data)
//	void hide(){
//		this->updateDisplay = false;
//	}

	InfoScreenPage* createScreen(String id, String header){
		InfoScreenPage* el = new InfoScreenPage(id, header);
		el->setParent(this);
		mChildern.add(el);
		return el;
	}

	void addPage(InfoScreenPage* page) {
		page->setParent(this);
		mChildern.add(page);
	}

	InfoScreenPage* get(int index) {
		if (mChildern.size() >index) {
			return mChildern.get(index);
		}
		return NULL;
	}

	InfoScreenPage* getCurrent() {
		return mChildern.get(mCurrent);
	}

	void print(int pIndex, Extended_SSD1306 display) {
		if(!canUpdateDisplay()) {
			Serial.println("Cannot Update display, flag is false");
			return;
		}
//		Serial.println("Printing index = " +String(pIndex));
		InfoScreenPage* p = get(pIndex);
		p->print(display);
	}

	String getParamText(String id) {
		return paramValueMap[id];
	}

	//no scren update
	void updateParamValue(String id, String newData) {
		if (paramValueMap.contains(id)) {
			paramValueMap.remove(id);
		}

		paramValueMap[id] = newData;
	}

	void updateParamValue(String id, String newData, Extended_SSD1306 display) {
		updateParamValue(id, newData);
		if (canUpdateDisplay()) {
			Vector<paramStruct*> params = getCurrent()->getAllParamsForId(id);
			for (int i = 0; i < params.size(); ++i) {
				paramStruct* param = params.get(i);
				display.writeover(param->t, newData);
//				param->text = newData;
			}
		}
	}

	void setCanUpdateDisplay(bool newState){
		this->updateDisplay = newState;
	}

	bool canUpdateDisplay(){
		return updateDisplay;
	}

};

#endif /* INCLUDE_INFOSCREENS_H_ */
