#pragma once
#include <unordered_map>
#include "PropertyWidget.hpp"
/**
 * @date   Jun 28, 2013
 * @author Sam Roth <>
 */

class QVBoxLayout;

namespace dbuilder
{

class GenericPropertyWidget: public PropertyWidget
{
	Q_OBJECT
	std::unordered_map<const DiagramComponent *, PropertyWidget *> _propertyWidgetForKind;
	QList<PropertyWidget *> _activePropWidgets;
	QVBoxLayout *_layout;
public:
	GenericPropertyWidget(QWidget *parent=nullptr);
	virtual ~GenericPropertyWidget();
protected:
	virtual void itemsChanged();
};

} /* namespace dbuilder */
