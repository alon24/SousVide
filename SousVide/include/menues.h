/*
 * menues.h
 *
 *  Created on: May 25, 2015
 *      Author: ilan
 */

#ifndef INCLUDE_MENUES_H_
#define INCLUDE_MENUES_H_

#include <SmingCore/SmingCore.h>
//#include <Libraries/Adafruit_SSD1306/Adafruit_SSD1306.h>
#include <Extended_SSD1306.h>
//#include <utils.h>

enum MenuActionEnum
{
	MNU_MOVE = 1, MNU_VALUES = 2
};
enum class Type
{
	Base = 0, Item = 1, Page = 2, Menu = 3, MainScreenItem = 4,
};

enum class HighlightMode
{
	pointer = 0,
	Inverse = 1
};

struct MenuParams {
	boolean boxed = 1;
	HighlightMode highlightMode = HighlightMode::pointer;
	boolean showBackItem = true;
};

class BaseMenuElement
{
	String m_id;
	BaseMenuElement *m_parent;
	Vector<BaseMenuElement*> m_children;

	Type m_type;

public:
	BaseMenuElement(String id, Type type)
	{
		m_id = id;
		m_type = type;
	};

	Type getType();
	String getId();
	void setParent(BaseMenuElement *parent);
	BaseMenuElement* getParent() const;
	int getIndexInParent();
	Vector<BaseMenuElement*> getChildren();
	void addChild(BaseMenuElement* child);
	BaseMenuElement* elementAt(int index);
};

class MenuItem: public BaseMenuElement
{
	MenuActionEnum m_actionEnum;
	BaseMenuElement *m_linkerItem = NULL;

public:
	MenuItem(String id) :
			BaseMenuElement(id, Type::Item)
	{
	}
	;

//	MenuItem(String name, MenuActionEnum act) :
//			MenuItem(name, Type::Item)
//	{
//		m_actionEnum = act;
//	}

	void setLinker(BaseMenuElement* itm)
	{
		m_linkerItem = itm;
	}
	;

	BaseMenuElement* getLinkedItem() const
	{
		return m_linkerItem;
	}

	// bool operator ==(MenuItem& item1)
	// {
	// 	return (getId() == item1.getId());
	// }
	// bool operator != ( MenuItem& item1);

};

class MenuPage: public BaseMenuElement
{
	bool m_circular = false;
	BaseMenuElement* m_prevItem = NULL;

public:
	MenuPage(String id) :
			BaseMenuElement(id, Type::Page)
	{
	}
//	void add(MenuItem *item);
	String header();
	bool isCircular();
	Vector<MenuItem*> getItems();
	MenuItem* createItem(String id);
	void setPrevItem(BaseMenuElement* prev);
	BaseMenuElement* getPrev();
};

class Menu: public BaseMenuElement
{
	BaseMenuElement* m_currentItem = NULL;
	BaseMenuElement *m_root;

	Vector<BaseMenuElement*> currentDisplayedElements;
	int m_maxPerPage = 2;

	MenuParams* m_params;

public:
	Menu(String id) :
			BaseMenuElement(id, Type::Menu)
	{
		m_params = new MenuParams();
	}
	;

//	void addPage(MenuPage* page);
	void moveUp();
	void moveDown();
	void moveUpLevel();
	MenuPage* getPage(int index);
	Vector<BaseMenuElement*> getDisplayedItems();
	BaseMenuElement* getCurrent();
	MenuPage* getCurrentPage();
//	void setCurrentItem(int pn, int itn);
	void setCurrentItem(BaseMenuElement* cur);
	void setMaxPerPage(int max);

	void setRoot(BaseMenuElement* root);
	BaseMenuElement* getRoot();
	void moveToRoot();

	void setParams(MenuParams* params);
	MenuParams* getParams();

	int getCurrIndex()
	{
		return getCurrent()->getIndexInParent();
	}

	void movetolinked()
	{
		if (!getCurrent()) {
			return;
		}

		BaseMenuElement* linked = ((MenuItem*) getCurrent())->getLinkedItem();
		if (!linked)
		{
			return;
		}

		BaseMenuElement* tmpPrev = getCurrent();
		Serial.println("lin:= " + linked->getId());
		if (linked->getType() == Type::Page)
		{
			if (linked->getId() != "..") {
				((MenuPage*)linked)->setPrevItem(getCurrent());
			}
			BaseMenuElement* it = ((MenuPage*) linked)->elementAt(0);
			setCurrentItem(it);
		}
		else
		{
			if (linked->getId() != "..") {
				((MenuPage*)linked->getParent())->setPrevItem(getCurrent());
			}
			setCurrentItem(linked);
		}
	}
	;
	void moveto(BaseMenuElement* el);

private:
	void moveto(int pn, int itn);
};



class Screen : public BaseMenuElement
{
	MenuParams* m_params;

public:
	Screen(String id) :
			BaseMenuElement(id, Type::Menu)
	{
		m_params = new MenuParams();
	}
	;

};

struct paramStruct {
	textRect t;
	String text;
	String id;

	void init(String id, String text, textRect t) {
		this->id = id;
		this->text = text;
		this->t = t;
	}
};


class InfoScreenLine
{
	String id;
	String m_text;
	int m_textSize;
	bool initialized = false;
	Vector<paramStruct*> params;

public:

	int mX, mY, mWidth;
	InfoScreenLine(String id, String text, int size = 1)
	{
		this->id = id;
		m_text = text;
		if (size != 1) {
			m_textSize = size;
		} else {
			m_textSize = 1;
		}
	};

	String getId() {
		return id;
	}

	int getTextSize(){
		return m_textSize;
	};

	String getText() {
		return m_text;
	}

	paramStruct* addParam(String id, String text) {
		paramStruct* ret = new paramStruct();
		ret->text = text;
		ret->t.x = -1;
		ret->t.y = -1;
		ret->t.h = -1;
		ret->t.w = -1;

		ret->id = id;
		params.add(ret);
		return ret;
	}

	paramStruct* addParam(String id, String text, textRect initial) {
		paramStruct* ret = addParam(id, text);
		ret->t.x = initial.x;
		ret->t.y = initial.y;
		ret->t.h = initial.h;
		ret->t.w = initial.w;
		return ret;
	}

	//prints the element
	void print(Extended_SSD1306 &display){
		textRect *t = display.print(getText());
		int y = display.getCursorY();
		Serial.print("printing " + m_textSize);
		Serial.println(" at y= " + String(y));
		for (int s = 0; s < params.size(); ++s) {
			paramStruct* param = params.get(s);
			String str = param->text;
			if(param->t.x != -1) {
				display.setCursor(param->t.x, y);
			}
//			else {
//				display.setCursor(display.getCursorX(), y);
//			}

			textRect* t = display.print(str);
			param->t = *t;
			Serial.printf("x %i, y %i, w %i\n", t->x, t->y, t->w);
		}

		display.println();
		Serial.printf("line end: %s, %s\n", String(t->x).c_str(), String(display.getCursorY()).c_str());
	}

	textRect* updateData(Extended_SSD1306 display, paramStruct* param, String newData) {
		textRect* ntr = display.writeover(&param->t, newData);
		param->text = newData;
		param->t = *ntr;
		return ntr;
	}

	textRect* updateData(Extended_SSD1306 display, int index, String newData) {
		paramStruct* p = params.get(index);
		textRect* ntr = this->updateData(display, p, newData);
		return ntr;
	}

	textRect* updateDataForId(Extended_SSD1306 display, String id, String newData) {
		paramStruct* p = getParamById(id);
		if (p != NULL) {
			textRect* ntr = this->updateData(display, p, newData);
			return ntr;
		}

		return NULL;
	}

	paramStruct* getParam(int index) {
		return params.get(index);
	}

	paramStruct* getParamById(String id) {
		for (int i = 0; i < params.size(); ++i) {
			if(params.get(i)->id == id) {
				return params.get(i);
			}
		}
		return NULL;
	}
};

class InfoScreen {
	Vector<InfoScreenLine*> mChildren;
	String m_header;
	String id;
public:
	InfoScreen(String id, String header) {
		this->id = id;
		m_header = header;
	};

	/**
	 * creates and adds to parent
	 */
	InfoScreenLine* createLine(String id, String text) {
		InfoScreenLine* el =  new InfoScreenLine(id, text, 1);
		addElemenet(el);
		return el;
	}

	void addElemenet(InfoScreenLine* el){
		mChildren.add(el);
	};

	InfoScreenLine* itemAt(int index) {
		return mChildren.get(index);
	};

	Vector<InfoScreenLine*> getItems()
	{
		return mChildren;
	};

	InfoScreenLine* getChildById(String id) {
		for (int i = 0; i < mChildren.size(); ++i) {
			if(mChildren.get(i)->getId() == id) {
				return mChildren.get(i);
			}
		}
		return NULL;
	}

	//Does NOT call display.display();
	void updateParam( String id, String newVal, Extended_SSD1306 display) {
		Vector<InfoScreenLine*> ret = getAllLinesParamsForId(id);
		for (int z = 0; z < ret.size(); ++z) {
			ret.elementAt(z)->updateDataForId(display, id, newVal);
		}
	}

	void updateAllParams( Vector<String> ids, Vector<String> newVals, Extended_SSD1306 display) {
		for (int i = 0; i < ids.size(); ++i) {
			updateParam(ids.elementAt(i), newVals.elementAt(i), display);
		}

		display.display();
	}

	Vector<InfoScreenLine*> getAllLinesParamsForId(String id) {
		Vector<InfoScreenLine*> ret;
		for (int i = 0; i < mChildren.size(); ++i) {
			InfoScreenLine* l = mChildren.elementAt(i);
			if (l->getParamById(id) != NULL) {
				ret.add(l);
			}
		}

		return ret;
	}

	void print(Extended_SSD1306 display) {
		display.clearDisplay();
//		display.setCursor(0,0);
		for(int i=0; i< mChildren.size(); i++){
			InfoScreenLine* child = mChildren.get(i);
			child->print(display);
		}

		display.display();
	}
};

class InfoPages{
	String mId;
	int mCurrent = 0;
	Vector<InfoScreen*> mChildern;
public:
	InfoPages(String id) {
		mId = id;
	}

	InfoScreen* createScreen(String id, String header){
		InfoScreen* el = new InfoScreen(id, header);
		mChildern.add(el);
		return el;
	}

	void addPage(InfoScreen* page) {
		mChildern.add(page);
	}

	InfoScreen* get(int index) {
		if (mChildern.size() >index) {
			return mChildern.get(index);
		}
		return NULL;
	}

	void print(int pIndex, Extended_SSD1306 display) {
		Serial.println("Printing index = " +String(pIndex));
		InfoScreen* p = get(pIndex);
		p->print(display);
	}
};

#endif /* INCLUDE_MENUES_H_ */
