/*
 * menues.h
 *
 *  Created on: May 25, 2015
 *      Author: ilan
 */

#ifndef INCLUDE_MENUES_H_
#define INCLUDE_MENUES_H_

#include <SmingCore/SmingCore.h>

enum MenuActionEnum
{
	MNU_MOVE = 1, MNU_VALUES = 2
};
enum class Type
{
	Base = 0, Item = 1, Page = 2, Menu = 3
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
	}
	;
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
	int m_maxPerPage = 2;
public:
	Menu(String id) :
			BaseMenuElement(id, Type::Menu)
	{
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

	int getCurrIndex()
	{
		return getCurrent()->getIndexInParent();
	}

	void movetolinked()
	{
		BaseMenuElement* linked = ((MenuItem*) getCurrent())->getLinkedItem();
		if (linked == NULL)
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

private:
	void moveto(int pn, int itn);
	void moveto(BaseMenuElement* el);
};

#endif /* INCLUDE_MENUES_H_ */
