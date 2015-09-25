//#include "menues.h";
#include "menues.h"
#include <SmingCore/SmingCore.h>

struct MenuChangeEvent
{
	const MenuItem &from;
	const MenuItem &to;
};

struct MenuUseEvent
{
	const MenuItem &item;
};

typedef void (*cb_change)(MenuChangeEvent);
typedef void (*cb_use)(MenuUseEvent);

String BaseMenuElement::getId()
{
	return m_id;
};

void BaseMenuElement::setParent(BaseMenuElement *parent)
{
	m_parent = parent;
}
;

BaseMenuElement* BaseMenuElement::getParent() const
{
	return m_parent;
}
;

Vector<BaseMenuElement*> BaseMenuElement::getChildren()
{
	return m_children;
};

int BaseMenuElement::getIndexInParent()
{
//	Serial.println("getIndexInParent p= " + m_parent->getId());

	if (m_parent == NULL)
		return -1;
	Vector<BaseMenuElement*> children = m_parent->getChildren();
//	Serial.println(
//			"getIndexInParent children.size= " + String(children.size()));
	if (children.size() == 0)
	{
		return -1;
	}

	for (int i = 0; i < children.size(); i++)
	{
//		Serial.println(
//				"getIndexInParent,for children.elementAt(i)= "
//						+ children.elementAt(i)->getId());

//		Serial.println(", elementAt(i) == this = " + String(elementAt(i) == this));
		if (children.elementAt(i) == this)
			return i;
	}

	return -1;
};

void BaseMenuElement::addChild(BaseMenuElement* child)
{
	m_children.add(child);
	child->setParent(this);
};
BaseMenuElement* BaseMenuElement::elementAt(int index)
{
	return m_children.elementAt(index);
};

Type BaseMenuElement::getType()
{
	return m_type;
};

//////////////////////////// MenuItem ///////////////////

// bool MenuItem::operator!= ( MenuItem& item1)
// {
//     return !(this->operator== (item1));
// }

//////////////////////////// MenuPage ///////////////////

//void MenuPage::add(MenuItem* item)
//{
//	addChild(item);
//	Serial.println("for item = " + item->getId() + ",p =" + getId());
//}

bool MenuPage::isCircular()
{
	return m_circular;
};

MenuItem* MenuPage::createItem(String id)
{

	MenuItem* item = new MenuItem(id);
	if (id != "..") {
		addChild(item);
	}
	return item;
};

void MenuPage::setPrevItem(BaseMenuElement* prev)
{
	m_prevItem = prev;
}

BaseMenuElement* MenuPage::getPrev()
{
	return m_prevItem;
}

//Vector<MenuItem*> MenuPage::getItems()
//{
//	return m_items;
//}
//
//int MenuPage::count()
//{
//	return m_items.count();
//}

//////////////////////////// Menu ///////////////////
//Menu::Menu(String id): BaseMenuElement(id)
//{
//}

//void Menu::addPage(MenuPage* page)
//{
//	m_pages.add(page);
//}
//
//
//void Menu::setCurrentItem(int pn, int itn)
//{
//	moveto(pn, itn);
//}
//
void Menu::setCurrentItem(BaseMenuElement* cur)
{
	m_currentItem = cur;
//	Serial.println("setting current =" + cur->getId() + ",p =" + cur->getParent()->getId());
};

BaseMenuElement* Menu::getCurrent()
{
	return m_currentItem;
}
;

MenuPage* Menu::getPage(int index)
{
	if (index > getChildren().count())
	{
		return NULL;
	}
	return (MenuPage*) getChildren().get(index - 1);
}
;

Vector<BaseMenuElement*> Menu::getDisplayedItems()
{
	BaseMenuElement *be = getCurrent()->getParent();
	MenuPage* p = NULL;
	if (be != NULL)
	{
		// if(NewType* v = dynamic_cast<NewType*>(old)) {
		//   // old was safely casted to NewType
		//   v->doSomething();
		// }

		if (MenuPage* v = static_cast<MenuPage*>(be))
		{
			// old was safely casted to NewType
			p = (MenuPage*) be;
		}
	}

	if (p == NULL)
	{
		return Vector<BaseMenuElement*>();
	}

//	Serial.println(
//			"m_maxPerPage = " + String(m_maxPerPage) + ", curr="
//					+ String(getCurrent()->getIndexInParent()) + ", size="
//					+ String(p->getChildren().count()));

	int tmpMaxViewed = m_maxPerPage;
	Vector<BaseMenuElement*> newVec;
	if (p->getPrev() != NULL)
	{
		tmpMaxViewed--;
		MenuItem* it = p->createItem("..");
		it->setLinker(p->getPrev());
		newVec.add(it);
	}

	if (tmpMaxViewed >= p->getChildren().count())
	{
//		Serial.println("m_maxPerPage >= page.count() = true");
		for (int i = 0; i < p->getChildren().count(); ++i)
		{
			newVec.add(p->elementAt(i));
		}
		return newVec;
	}

	int start = getCurrent()->getIndexInParent();
	int end(start + tmpMaxViewed);
//	Serial.println("start = " + String(start) + ", end= " + String(end));
	if (end > p->getChildren().count())
	{
//		start = start -(end - page.count());
		end = p->getChildren().count();
		start = end - tmpMaxViewed;
//		Serial.println("end > page.count()");
	}

//	Serial.println(
//			"result: start = " + String(start) + ", end= " + String(end));
	for (int i = start; i < end; ++i)
	{
//		Serial.print(String(i) + ",");
		newVec.add(p->elementAt(i));
	}
	Serial.println();
	return newVec;
};

void Menu::moveDown()
{
	MenuPage *p = (MenuPage *) m_currentItem->getParent();

	if (p->getChildren().size() == 0)
	{
		return;
	}

	int currI = getCurrIndex();
	currI++;
	if (p->getChildren().size() >= currI + 1)
	{
		Serial.println("trying to set " + String(currI));
		setCurrentItem(p->elementAt(currI));
		return;
	}
	else
	{
		setCurrentItem(p->elementAt(0));
		return;
	}
};

void Menu::moveUp()
{
	MenuPage* p = (MenuPage*) getCurrent()->getParent();
	if (p->getChildren().size() == 0)
	{
		return;
	}

	int currI = getCurrIndex();
	currI--;
	if (currI >= 0)
	{
		Serial.println("trying to set " + String(currI));
		setCurrentItem(p->elementAt(currI));
		return;
	}
	else
	{
		setCurrentItem(p->elementAt(p->getChildren().count() - 1));
		return;
	}
};

MenuPage* Menu::getCurrentPage(){
	BaseMenuElement* el = getCurrent()->getParent();
	if (el) {
		if (el->getType() == Type::Page) {
			Serial.println("el is p = " + el->getId());
			return (MenuPage*)el;
		}
	}
	return NULL;
}

void Menu::moveUpLevel(){
	MenuPage* p = getCurrentPage();
	if (p && p->getPrev()) {
		Serial.println("moveUpLevel prev= = " +p->getPrev()->getId());
		BaseMenuElement* el = getCurrentPage()->getPrev();
		p->setPrevItem(NULL);
		setCurrentItem(el);
	}
}

void Menu::moveto(BaseMenuElement* el)
{
	if (!el) {
		return;
	}

	if (el->getType() == Type::Item) {
		setCurrentItem(el);
	} else if (el->getType() == Type::Page) {
		setCurrentItem(el->elementAt(0));
	}
}

void Menu::moveToRoot()
{
	moveto(getRoot());
}

//void Menu::moveto(int pn, int itn)
//{
//	MenuPage* p = getPage(pn-1);
//	p->getItemAt(itn-1);
//	setCurrentItem(p);
//}

void Menu::setMaxPerPage(int max)
{
	m_maxPerPage = max;
};

void Menu::setRoot(BaseMenuElement* root)
{
	m_root = root;
}

BaseMenuElement* Menu::getRoot()
{
	return m_root;
}

void Menu::setParams(MenuParams* params)
{
	delete (m_params);
	m_params = params;
}

MenuParams* Menu::getParams()
{
	return m_params;
}
