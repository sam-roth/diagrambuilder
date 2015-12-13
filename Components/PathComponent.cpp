/**
 * @file   PathComponent.cpp
 *
 * @date   Jan 27, 2013
 * @author Sam Roth <>
 */

#include "moc_PathComponent.cpp"
#include <QGraphicsPathItem>
#include "Util/ScopeExit.hpp"
#include "DiagramItem.hpp"
#include <QPainter>
#include <iostream>
#include <QGraphicsSceneMouseEvent>
#include <QGraphicsRectItem>
#include <QGraphicsScene>
#include "qgraphicsitem.h"
#include <boost/property_tree/ptree.hpp>
#include "DiagramItemModel.hpp"
#include <cassert>
#include "Util/PathTree.hpp"
#include <QAction>
#include "Path/PathItem.hpp"



namespace dbuilder {


PathComponent::PathComponent(QObject* parent)
: DiagramComponent("path", parent)
{
}

void PathComponent::configure(DiagramItem *item) const
{
	item->setFlag(QGraphicsItem::ItemIsSelectable);
	item->setFlag(QGraphicsItem::ItemIsMovable);
	auto pi = new PathItem(item);
	pi->setParentItem(item);
}

PathComponent::~PathComponent()
{
}

} /* namespace dbuilder */
