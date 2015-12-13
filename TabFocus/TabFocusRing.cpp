/**
 * @file   TabFocusRing.cpp
 *
 * @date   Mar 29, 2013
 * @author Sam Roth <>
 */

#include "TabFocusRing.hpp"
#include "TabFocusable.hpp"
#include "Util/Log.hpp"
#include "Util/Math.hpp"
#include "Util/Printable.hpp"
#include "Util/Optional.hpp"

namespace dbuilder
{

SequentialTabFocusRing::SequentialTabFocusRing(QObject *parent)
: QObject(parent)
{

}

SequentialTabFocusRing::~SequentialTabFocusRing()
{
}

void SequentialTabFocusRing::add(TabFocusable *t)
{
	_items.push_back(t);
	_itemForObject[t->object()] = t;
	connect(t->object(), SIGNAL(destroyed()), this, SLOT(participantDestroyed()));
	DBDebug("SequentialTabFocusRing: Added item to focus ring: ", Printable::repr(t));
}

void SequentialTabFocusRing::remove(TabFocusable *t)
{
	this->remove(t->object());
}

TabFocusable* SequentialTabFocusRing::next(TabFocusable *t)
{
	if(_items.empty())
	{
		return t;
	}


	const int idx = _items.indexOf(t);
	if(idx >= 0)
	{
		return _items[modulus(idx + 1, _items.size())];
	}
	else
	{
		return _items.front();
	}
}

TabFocusable* SequentialTabFocusRing::prev(TabFocusable *t)
{
	if(_items.empty())
	{
		return t;
	}


	const int idx = _items.indexOf(t);
	if(idx >= 0)
	{
		const int nextIdx = modulus(idx - 1, _items.size());
		return _items[nextIdx];
	}
	else
	{
		return _items.front();
	}
}

void SequentialTabFocusRing::participantDestroyed()
{
	remove(sender());
}

void SequentialTabFocusRing::remove(QObject* obj)
{
	if(auto t = get(_itemForObject, obj))
	{
		_itemForObject.remove(obj);
		_items.removeAll(*t);
		disconnect(obj, nullptr, this, nullptr);
	}
}
} /* namespace dbuilder */
