/**
 * @file   DiagramComponent.cpp
 *
 * @date   Jan 2, 2013
 * @author Sam Roth <>
 */

#include "DiagramComponent.hpp"
#include "DiagramItemModel.hpp"
#include "DiagramItem.hpp"
#include "DiagramScene.hpp"
#include "PropertyWidget.hpp"
#include "BasicPropertyWidget.hpp"

namespace dbuilder {




DiagramComponent::DiagramComponent(QString name, QObject* parent)
: QObject(parent)
, _name(name)
, _hidden(false)
{
}

QIcon DiagramComponent::icon() const
{
	return {};
}

DiagramComponent::~DiagramComponent()
{
}

void DiagramComponent::print(std::ostream& os) const
{
	os << name().toStdString();
}

DiagramItem *DiagramComponent::create(DiagramScene *scene) const
{
	QObject localCtx;

	auto model = new DiagramItemModel(scene->context(), &localCtx);
	model->setKind(this);

	auto result = this->createFromModel(model);
//	scene->addDiagramItem(result);

	model->setParent(result);

	return result;
}

DiagramItem* DiagramComponent::createFromModel(DiagramItemModel* model) const
{
	assert(model->kind() == this);
	auto result = new DiagramItem;
	result->setModel(model);

	return result;
}

PropertyWidget *DiagramComponent::makePropertyWidget(QWidget *parent) const
{
	return new BasicPropertyWidget(parent);
}

}  // namespace dbuilder
