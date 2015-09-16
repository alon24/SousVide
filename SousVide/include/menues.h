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
	MNU_MOVE = 1, MNU_VALUES = 2, SELECT =3
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

//typedef void (*cb_use)(ButtonUseEvent);

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

	void setDelegate() {

	}

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
	MenuPage* createPage(String id) {
		MenuPage* p = new MenuPage(id);
		addChild(p);
	}

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

#endif /* INCLUDE_MENUES_H_ */
