#pragma once
/**
 * @file   GridMenu.hpp
 *
 * @date   Mar 13, 2013
 * @author Sam Roth <>
 */

#include <QMenu>

namespace dbuilder
{

class GridMenu: public QMenu
{
	Q_OBJECT
	QAction *_hoverItem;
public:
	GridMenu(QWidget *parent=nullptr);
	virtual ~GridMenu();
protected:
	virtual bool event(QEvent *);
private slots:
	void menuItemRequestedClose();
	void menuItemHoverChanged(QAction *, bool);
};

} /* namespace dbuilder */
