#include <QObject>
#include <QList>
#include <QtCore>
#pragma once
/**
 * @file   TabFocusRing.hpp
 *
 * @date   Mar 29, 2013
 * @author Sam Roth <>
 */

namespace dbuilder
{

class TabFocusable;

class TabFocusRing
{
public:
	virtual ~TabFocusRing() { }

	virtual void add(TabFocusable *) = 0;
	virtual void remove(TabFocusable *) = 0;

	virtual TabFocusable *next(TabFocusable *) = 0;
	virtual TabFocusable *prev(TabFocusable *) = 0;
};


class SequentialTabFocusRing: public QObject, public TabFocusRing
{
	Q_OBJECT
	QList<TabFocusable *> _items;
	// Necessary in order to be able to remove items upon destruction:
	// Since we're only notified after the derived part of the class is already
	// destroyed, dynamic_cast<> won't work.
	QMap<QObject *, TabFocusable *> _itemForObject;
public:
	SequentialTabFocusRing(QObject *parent);
	virtual ~SequentialTabFocusRing();

	virtual void add(TabFocusable *);
	virtual void remove(TabFocusable *);

	virtual TabFocusable *next(TabFocusable *);
	virtual TabFocusable *prev(TabFocusable *);
private:
	void remove(QObject *obj);
private slots:
	void participantDestroyed();

};
} /* namespace dbuilder */
