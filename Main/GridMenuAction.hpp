#pragma once
/**
 * @file   GridMenuAction.hpp
 *
 * @date   Mar 10, 2013
 * @author Sam Roth <>
 */

#include <QWidgetAction>
#include <QList>

class QToolButton;

namespace dbuilder {

class FlowLayout;

class GridMenuAction: public QWidgetAction
{
	Q_OBJECT
	QList<QToolButton *> toolbuttons;
	QWidget *palette;
	FlowLayout *layout;
	QMap<QToolButton *, QAction *> actionForButton;
public:
	GridMenuAction(
		const QList<QAction *> &actions,
		QObject *parent=nullptr
	);

	virtual ~GridMenuAction();
private slots:
	void buttonHoverChanged(bool);

signals:
	void closeMenu();
	void hoverChanged(QAction *, bool);
};

} /* namespace dbuilder */
