/*
 * InfoScreens.cpp
 *
 *  Created on: Aug 20, 2015
 *      Author: ilan
 */
#include <InfoScreens.h>

InfoPageLine::InfoPageLine(String id, String text, int size = 1) : BaseScreenElement(id)
{
	m_text = text;
	if (size != 1)
	{
		m_textSize = size;
	}
	else
	{
		m_textSize = 1;
	}
}
;

int InfoPageLine::InfoPageLine::getTextSize()
{
	return m_textSize;
}
;

String InfoPageLine::InfoPageLine::getText()
{
	return m_text;
}

paramStruct* InfoPageLine::addParam(String id, String text)
{
	paramStruct* ret = new paramStruct();
	getParent()->updateParamValue(id, text);
//	ret->text = text;
	ret->t.x = -1;
	ret->t.y = -1;
	ret->t.h = -1;
	ret->t.w = -1;

	ret->id = id;
	params.add(ret);
	return ret;
}

paramStruct* InfoPageLine::addParam(String id, String text, textRect initial)
{
	paramStruct* ret = addParam(id, text);
	ret->t.x = initial.x;
	ret->t.y = initial.y;
	ret->t.h = initial.h;
	ret->t.w = initial.w;
	return ret;
}

//prints the element
void InfoPageLine::print(Extended_SSD1306 &display)
{
	textRect *t = display.print(getText());
	int y = display.getCursorY();
	for (int s = 0; s < params.size(); ++s)
	{
		paramStruct* param = params.get(s);
		String str = getParamText(param->id);
//		String str = param->text;
		if (param->t.x != -1)
		{
			display.setCursor(param->t.x, y);
		}
		textRect* t = display.print(str);
		param->t = *t;
		Serial.printf("x %i, y %i, w %i\n", t->x, t->y, t->w);
	}

	display.println();
	Serial.printf("line end: %s, %s\n", String(t->x).c_str(), String(display.getCursorY()).c_str());
}

//void InfoPageLine::updateData(Extended_SSD1306 display, paramStruct* param,
//		String newData)
//{
//	//		uint32_t free = system_get_free_heap_size();
//	//		int ff = (int)free;
//	//		Serial.printf("free1= %i", ff );
//
//	display.writeover(param->t, newData);
//	//		free = system_get_free_heap_size();
//	//		ff = (int)free;
//	//		Serial.printf(", 2= %i", ff );
//
//	param->text = newData;
//	//		free = system_get_free_heap_size();
//	//		ff = (int)free;
//	//		Serial.printf(", 3= %i\n", ff );
//}

//void InfoPageLine::updateData(Extended_SSD1306 display, int index,
//		String newData)
//{
//	paramStruct* p = params.get(index);
//	this->updateData(display, p, newData);
//}
//
//void InfoPageLine::updateDataForId(Extended_SSD1306 display, String id,
//		String newData)
//{
//	paramStruct* p = getParamById(id);
//	if (p != NULL)
//	{
//		this->updateData(display, p, newData);
//	}
//}

//paramStruct* InfoPageLine::getParam(int index)
//{
//	return params.get(index);
//}

paramStruct* InfoPageLine::getParamById(String id)
{
	for (int i = 0; i < params.size(); ++i)
	{
		if (params.get(i)->id == id)
		{
			return params.get(i);
		}
	}
	return NULL;
}

