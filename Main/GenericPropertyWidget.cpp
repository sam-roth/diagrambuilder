/**
 * @date   Jun 28, 2013
 * @author Sam Roth <>
 */

#include "GenericPropertyWidget.hpp"
#include "DiagramItem.hpp"
#include "DiagramItemModel.hpp"
#include <QtGui>
#include "Util/Optional.hpp"
#include "DiagramComponent.hpp"

namespace dbuilder
{
GenericPropertyWidget::GenericPropertyWidget(QWidget* parent)
: PropertyWidget(parent)
, _layout(new QVBoxLayout(this))
{
	_layout->setContentsMargins(0, 0, 0, 0);
}

GenericPropertyWidget::~GenericPropertyWidget()
{
}

void GenericPropertyWidget::itemsChanged()
{
	for(auto w : _activePropWidgets)
	{
		w->setVisible(false);
	}
	_activePropWidgets.clear();

	auto items = this->items();
//	auto item = !items.empty()? items.front() : nullptr;

	QSet<const DiagramComponent *> components;

	for(auto item : items)
	{
		components << item->model()->kind();
	}

	if(components.size() == 1) // todo: implement better multiple selection support
	{
		auto kind = *components.begin();

		auto optionalWidget = get(_propertyWidgetForKind, kind);
		PropertyWidget *widget;
		if(optionalWidget)
		{
			widget = *optionalWidget;
		}
		else
		{
			widget = kind->makePropertyWidget(this);
			_propertyWidgetForKind[kind] = widget;
			_layout->addWidget(widget);
		}

		widget->setVisible(true);
		widget->setItems(items);
		_activePropWidgets << widget;
	}
}

} /* namespace dbuilder */
