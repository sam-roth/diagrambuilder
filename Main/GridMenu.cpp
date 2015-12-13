/**
 * @file   GridMenu.cpp
 *
 * @date   Mar 13, 2013
 * @author Sam Roth <>
 */

#include "GridMenu.hpp"
#include <QCloseEvent>
#include "Main/GridMenuAction.hpp"

namespace dbuilder
{

GridMenu::GridMenu(QWidget *parent)
: QMenu(parent)
, _hoverItem(nullptr)
{

}

GridMenu::~GridMenu()
{
}

bool GridMenu::event(QEvent *event)
{
	// if a GridMenuAction was added or removed, connect or disconnect appropriate signals
	if(auto actionEvent = dynamic_cast<QActionEvent *>(event))
	{
		if(auto gridMenuAction = dynamic_cast<GridMenuAction *>(actionEvent->action()))
		{
			if(actionEvent->type() == QEvent::ActionAdded)
			{
				connect(gridMenuAction, SIGNAL(closeMenu()), this, SLOT(menuItemRequestedClose()));
				connect(gridMenuAction, SIGNAL(hoverChanged(QAction *, bool)), this, SLOT(menuItemHoverChanged(QAction *, bool)));
			}
			else if(actionEvent->type() == QEvent::ActionRemoved)
			{
				disconnect(gridMenuAction, nullptr, this, nullptr); // disconnect all signals
			}
		}
	}
	else if(event->type() == QEvent::Close)
	{
		if(_hoverItem)
		{
			_hoverItem->trigger();
		}
	}
	return QMenu::event(event);
}

void GridMenu::menuItemHoverChanged(QAction *action, bool enter)
{
	if(enter)
	{
		_hoverItem = action;
	}
	else
	{
		_hoverItem = nullptr;
	}
}

void GridMenu::menuItemRequestedClose()
{
	_hoverItem = nullptr;
	close();
}

} /* namespace dbuilder */

