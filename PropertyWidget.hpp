#pragma once
#include <QWidget>
#include "CoreForward.hpp"
/**
 * @date   Jun 28, 2013
 * @author Sam Roth <>
 */

class QLabel;
namespace dbuilder
{

class PropertyWidget: public QWidget
{
	Q_OBJECT
	QList<DiagramItem *> _items;
public:
	PropertyWidget(QWidget *parent=0);
	virtual ~PropertyWidget();

	QList<DiagramItem *> items() const
	{
		return _items;
	}

	void setItems(QList<DiagramItem *> items)
	{
		_items = std::move(items);
		itemsChanged();
	}
protected:
	virtual void itemsChanged();
};

} /* namespace dbuilder */
