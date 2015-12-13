/**
 * @date   Jun 28, 2013
 * @author Sam Roth <>
 */

#include "PropertyWidget.hpp"
#include <QtGui>
#include "DiagramItem.hpp"
#include "DiagramItemModel.hpp"
#include "DiagramComponent.hpp"

namespace dbuilder
{

PropertyWidget::PropertyWidget(QWidget *parent)
: QWidget(parent)
{
}

PropertyWidget::~PropertyWidget()
{
}

void PropertyWidget::itemsChanged()
{
}

} /* namespace dbuilder */
