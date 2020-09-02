/**
 * @file   TabFocusableTextItem.cpp
 *
 * @date   Mar 29, 2013
 * @author Sam Roth <>
 */

#include "TabFocusableTextItem.hpp"
#include "Util/Log.hpp"
#include "Util/Printable.hpp"

namespace dbuilder
{

TabFocusableTextItem::TabFocusableTextItem(QGraphicsItem* parentItem)
: QGraphicsTextItem(parentItem)
, _focusRing(nullptr)
{
	this->setDefaultTextColor(QColor(0, 0, 0));
}

TabFocusable* TabFocusableTextItem::nextFocusable()
{
	return _focusRing? _focusRing->next(this) : this;
}

TabFocusable* TabFocusableTextItem::prevFocusable()
{
	return _focusRing? _focusRing->prev(this) : this;
}

TabFocusableTextItem::~TabFocusableTextItem()
{
}

void TabFocusableTextItem::print(std::ostream& os) const
{
	writecat(os, "text=", toPlainText(), ", toplevel=", Printable::repr(this->topLevelItem()));
}

void TabFocusableTextItem::setFocusRing(TabFocusRing* focusRing)
{
	if(_focusRing)
	{
		_focusRing->remove(this);
	}

	_focusRing = focusRing;

	if(_focusRing)
	{
		_focusRing->add(this);
	}
}

} /* namespace dbuilder */
