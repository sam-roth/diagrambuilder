#pragma once
/**
 * @file   TabFocusable.hpp
 *
 * @date   Mar 29, 2013
 * @author Sam Roth <>
 */

class QGraphicsItem;

namespace dbuilder {

class TabFocusable
{
public:
	virtual QGraphicsItem *graphicsItem() = 0;
	virtual QObject *object() = 0;
	virtual TabFocusable *nextFocusable() = 0;
	virtual TabFocusable *prevFocusable() = 0;
	virtual void makeFocused() = 0;

	virtual ~TabFocusable() { }
};

template <typename Derived>
class BasicTabFocusable: public virtual TabFocusable
{
public:
	virtual QGraphicsItem *graphicsItem()
	{
		return static_cast<QGraphicsItem *>(static_cast<Derived *>(this));
	}

	virtual void makeFocused()
	{
		this->graphicsItem()->setFocus(Qt::TabFocusReason);
	}

	virtual QObject *object()
	{
		return static_cast<QObject *>(static_cast<Derived *>(this));
	}
};

}  // namespace dbuilder



