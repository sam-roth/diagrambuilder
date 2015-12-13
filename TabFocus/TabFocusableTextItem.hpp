#pragma once
/**
 * @file   TabFocusableTextItem.hpp
 *
 * @date   Mar 29, 2013
 * @author Sam Roth <>
 */

#include <qgraphicsitem.h>
#include "TabFocusable.hpp"
#include "TabFocus/TabFocusRing.hpp"
#include "Util/Printable.hpp"

namespace dbuilder
{

class TabFocusableTextItem: public QGraphicsTextItem, public BasicTabFocusable<TabFocusableTextItem>, public Printable
{
	Q_OBJECT
	TabFocusRing *_focusRing;
public:
	TabFocusableTextItem(QGraphicsItem *parentItem=nullptr);



	virtual TabFocusable *nextFocusable();
	virtual TabFocusable *prevFocusable();
	virtual ~TabFocusableTextItem();

	virtual void print(std::ostream &) const;

	TabFocusRing* focusRing() const
	{
		return _focusRing;
	}

	void setFocusRing(TabFocusRing* focusRing);
};

} /* namespace dbuilder */
