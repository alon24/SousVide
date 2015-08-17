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
#include <ilan1306.h>
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
};

class InfoPageElemet
{
	String m_id;
	String m_text;
	int m_textSize;
	Vector<paramStruct*> params;

public:
	int mX, mY, mWidth;
	InfoPageElemet(String id, String text, int size = 1)
	{
		m_id = id;
		m_text = text;
		if (size != 1) {
			m_textSize = size;
		} else {
			m_textSize = 1;
		}
	};

	String getId();
	int getTextSize(){
		return m_textSize;
	};

	String getText() {
		return m_text;
	}

	void setUpdateRect(int x, int y, int w) {
		mX = x;
		mY = y;
		mWidth = w;
	}

	void print(ilan1306 display){
		display.setTextSize(m_textSize);
		display.print(getText());

		for (int s = 0; s < params.size(); ++s) {
			paramStruct* param = params.get(s);
			String str = param->text;
//			int preX = display.getCursorX();
//			int preY = display.getCursorY();
//			int w = param.length() * 6;
//			 display.print(*param);
//			if()

			textRect t = display.printI(str);
			Serial.printf("x %i, y %i, w %i\n", t.x, t.y, t.w);
//			display.print(param);
			display.println();
		}
	}

	void updateData(ilan1306 display, String data) {
//		display.fillRect( mX, mY, data.length() * 6, 8, BLACK);
//		display.setTextSize(1);
//		display.setTextColor(WHITE);
//		display.setCursor(mX, mY );
//		display.print(data);
//		display.display();
	}
};

class InfoPage {
	Vector<InfoPageElemet*> mChildren;
	String m_header;
	String mID;
public:
	InfoPage(String id, String header) {
		mID = id;
		m_header = header;
	};

	InfoPageElemet* createInfo(String id, String text) {
		InfoPageElemet* el =  new InfoPageElemet(id, text, 1);
		addElemenet(el);
		return el;
	}

	void addElemenet(InfoPageElemet* el){
		mChildren.add(el);
	};

	InfoPageElemet* itemAt(int index) {
		return mChildren.get(index);
	};

	Vector<InfoPageElemet*> getItems()
	{
		return mChildren;
	};

	void print(ilan1306 display) {
		for(int i=0; i< mChildren.size(); i++){
			InfoPageElemet* child = mChildren.get(i);
			child->print(display);
		}
	}
};

class InfoPages{
	String mId;
	int mCurrent = 0;
	Vector<InfoPage*> mChildern;
public:
	InfoPages(String id) {
		mId = id;
	}

	InfoPage* createPage(String id, String header){
		InfoPage* el = new InfoPage(id, header);
		mChildern.add(el);
		return el;
	}

	void addPage(InfoPage* page) {
		mChildern.add(page);
	}

	InfoPage* get(int index) {
		if (mChildern.size() >index) {
			return mChildern.get(index);
		}
		return NULL;
	}


	void print(int pIndex, ilan1306 display) {
		InfoPage* p = get(pIndex);
		p->print(display);
	}
};

#endif /* INCLUDE_MENUES_H_ */
