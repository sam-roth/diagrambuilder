/**
 * @file   DiagramItem.cpp
 *
 * @date   Jan 2, 2013
 * @author Sam Roth <>
 */

#include <cmath>
#include "DiagramItem.hpp"
#include <cassert>
#include "DiagramComponent.hpp"
#include <QPen>
#include <QGraphicsSceneMouseEvent>
#include "DiagramItemModel.hpp"
#include <iostream>
#include "DiagramScene.hpp"
#include <QMenu>
#include <QAction>
#include "Commands/DeleteItemCommand.hpp"
#include "Util/StreamOps.hpp"
#include <QApplication>
#include "Util/Log.hpp"
#include "Util/UUIDTranslator.hpp"
#include "Main/Application.hpp"


namespace dbuilder {

DiagramItem::DiagramItem(QObject *parent)
: QObject(parent)
{
	init();
}

void DiagramItem::init()
{
	_model = nullptr;
	_dragging = false;
	_printMode = false;
	_explicitSelectionOnly = false;
	_highlightedPort = -1;
	_positionBeingSet = false;
	_itemSnaps = true;
	_settingsModel = nullptr;
	_positionByCenter = true;
	_app = dbuilder::Application::instance();
	_editMode = false;

	connect(_app, SIGNAL(settingsChanged()), this, SLOT(settingsChanged()));

	this->setPen(QPen(QColor(0, 0, 0, 0)));
	this->setAcceptHoverEvents(true);
	this->setFlag(ItemSendsScenePositionChanges, true);
	this->setFlag(ItemSendsGeometryChanges);
}

void DiagramItem::setEditMode(bool e)
{
	if(_editMode != e)
	{
		_editMode = e;
		if(this->isSelected() != e)
		{
			this->setSelected(e);
		}

		emit editModeChanged(e);
		if(e)
		{
			emit editingBegan();
		}
		else
		{
			emit editingFinished();
		}
	}
}

bool DiagramItem::editMode() const
{
	return _editMode;
}

void DiagramItem::settingsChanged()
{
	// update port outline color
	setPrintMode(printMode());
}

void DiagramItem::setHighlightedPort(int port)
{
	if(_highlightedPort != -1)
	{
		_portSymbols[_highlightedPort]->setBrush(QBrush(QColor(0, 0, 0, 0))); //QColor("SlateBlue")));
	}

	_highlightedPort = port;
	if(_highlightedPort != -1)
	{
		_portSymbols[_highlightedPort]->setBrush(QBrush(_app->portHighlightColor()));
	}

	update();
}

void DiagramItem::addPort(QPointF loc)
{
	_portLocations.push_back(loc);
	_portSymbols.push_back(new QGraphicsEllipseItem(loc.x()-5, loc.y()-5, 10, 10, this));
	_portSymbols.back()->setPen(QPen(_app->portOutlineColor()));

	emit posChanged(this->scenePos());
}

void DiagramItem::setPrintMode(bool printMode)
{
	_printMode = printMode;
	if(printMode)
	{
		for(auto sym : _portSymbols)
		{
			sym->setPen(QPen(QColor(0,0,0,0)));
		}
	}
	else
	{
		for(auto sym : _portSymbols)
		{
			sym->setPen(QPen(_app->portOutlineColor()));
		}
	}
	emit printModeChanged(printMode);
}

const QList<QPointF>& DiagramItem::portLocations() const
{
	return _portLocations;
}

void DiagramItem::explicitlySelect()
{
	auto tmp = _explicitSelectionOnly;
	setExplicitSelectionOnly(false);
	this->setSelected(true);
	setExplicitSelectionOnly(tmp);
}
QVariant DiagramItem::itemChange(GraphicsItemChange change,
		const QVariant& value)
{
	if(change == ItemScenePositionHasChanged)
	{
		// update the model with the new scene position
//		this->model()->setScenePos(this->scenePos());
		emit posChanged(this->scenePos());
	}
	else if(change == ItemPositionChange)
	{
		auto refpos = positionByCenter()? this->boundingRect().center() : this->boundingRect().topLeft();
		if(_itemSnaps)
		{
			// use coarse snapping when alt/option is pressed
			bool useCoarseSnapping = QApplication::keyboardModifiers().testFlag(Qt::AltModifier);
			qreal snapResolution = useCoarseSnapping? 50.0 : 5.0;

			// snap item to five pixel grid
			auto nextPos = value.toPointF() + refpos;
			nextPos.setX(round(nextPos.x() / snapResolution) * snapResolution);
			nextPos.setY(round(nextPos.y() / snapResolution) * snapResolution);

			nextPos = nextPos - refpos;

			// constrain to axis when shift is pressed
			bool constrainToAxis = QApplication::keyboardModifiers().testFlag(Qt::ShiftModifier);
			if(constrainToAxis)
			{
				auto diff = nextPos - _originalPosition;
				if(std::abs(diff.y()) > std::abs(diff.x()))
				{
					diff.setX(0);
				}
				else
				{
					diff.setY(0);
				}

				nextPos = diff + _originalPosition;
			}

			return nextPos;
		}
		else
		{
			return value;
		}
	}
	else if(change == ItemSelectedHasChanged)
	{
		if(!this->isSelected())
		{
			this->setEditMode(false);
		}
		emit selectionChanged(this->isSelected());
	}
	else if(change == ItemRotationHasChanged)
	{
		// update the model with the new rotation
//		this->model()->setRotation(this->rotation());
		emit posChanged(this->scenePos());
	}
	else if(change == ItemSceneHasChanged && scene())
	{
		emit addedToScene();
	}
	else if(change == ItemSceneChange)
	{
		if(!qvariant_cast<QGraphicsScene *>(value) && scene())
		{
			emit willBeRemovedFromScene();
		}
	}

	return QGraphicsRectItem::itemChange(change, value);
}

void DiagramItem::hoverMoveEvent(QGraphicsSceneHoverEvent* event)
{
	for(auto item : _portSymbols)
	{
		if(item->contains(event->pos()))
		{
			item->setBrush(QBrush(Qt::black));
		}
		else
		{
			item->setBrush(QBrush(QColor(0, 0, 0, 0)));
		}
	}
	QGraphicsRectItem::hoverMoveEvent(event);
}

void DiagramItem::hoverLeaveEvent(QGraphicsSceneHoverEvent *event)
{
	for(auto item : _portSymbols)
	{
		item->setBrush(QBrush(QColor(0,0,0,0)));
	}
}
void DiagramItem::emitPosChanged()
{
	emit posChanged(this->scenePos());
}

void DiagramItem::contextMenuEvent(QGraphicsSceneContextMenuEvent *event)
{
	QMenu menu;
	if(_model && _model->kind())
	{
		auto nameAction = menu.addAction(_model->kind()->name());
		nameAction->setEnabled(false);
		menu.addSeparator();
	}
	menu.addAction("Delete", this, SLOT(deleteDiagramItem()));
	menu.addAction("Edit", this, SLOT(beginEditing()));
	menu.exec(event->screenPos());
}

void DiagramItem::beginEditing()
{
	this->setEditMode(true);
}

void DiagramItem::deleteDiagramItem()
{
	if(scene())
	{
		auto diagramScene = static_cast<DiagramScene *>(scene());
		auto deleteCommand = new DeleteItemCommand(diagramScene, this);
		diagramScene->undoStack().push(deleteCommand);
	}
}


void DiagramItem::updateBoundingBox()
{
	this->setRect(this->childrenBoundingRect());
}
int DiagramItem::portAt(QPointF point)
{
	int i = 0;
	for(auto item : _portSymbols)
	{
		if(item->contains(point)) return i;
		i++;
	}

	return -1;
}

void DiagramItem::setModel(DiagramItemModel *model)
{
	assert(!_model);
	_model = model;
	_model->setParent(this);
	connect(_model, SIGNAL(updateViewRequested()), this, SLOT(updateViewRequested()));
	connect(_model, SIGNAL(updateModelRequested()), this, SLOT(updateModelRequested()));

	model->kind()->configure(this);
	updateBoundingBox();
	model->requestUpdateView();
}

void DiagramItem::updateViewRequested()
{
	this->setRotation(this->model()->rotation());
	this->setPos(this->model()->scenePos());
	this->setZValue(this->model()->sceneZ());
}

void DiagramItem::clearPorts()
{
	for(auto portSymbol : this->_portSymbols)
	{
		delete portSymbol;
	}

	this->_portSymbols.clear();
	this->_portLocations.clear();
	emit posChanged(this->scenePos());
}

void DiagramItem::print(std::ostream& os) const
{
	if(model())
	{
		writecat(os, "kind=", Printable::str(model()->kind()), ", uuid=", model()->uuid());
	}
	else
	{
		writecat(os, "<no model>");
	}
}

void DiagramItem::updateModelRequested()
{
	this->model()->setScenePos(this->scenePos());
	this->model()->setRotation(this->rotation());
	this->model()->setSceneZ(this->zValue());
}

QPointF DiagramItem::scenePortLocation(int port) const
{
	assert(port >= 0);
	if(port < _portLocations.size())
	{
		return this->mapToScene(_portLocations[port]);
	}
	else if(!_portLocations.empty())
	{
		return this->mapToScene(_portLocations.back());
	}
	else
	{
		return this->scenePos();
	}
}

void DiagramItem::mouseMoveEvent(QGraphicsSceneMouseEvent* event)
{
	if(_dragging)
	{
		emit connectorDragMid(event->scenePos());
	}
	else
	{
		QGraphicsRectItem::mouseMoveEvent(event);
	}
}

void DiagramItem::mousePressEvent(QGraphicsSceneMouseEvent* event)
{
	if(event->buttons().testFlag(Qt::RightButton))
	{
		QGraphicsItem::mousePressEvent(event);
		return;
	}

	if(explicitSelectionOnly())
	{
		event->ignore();
		return;
	}

	QGraphicsRectItem::mousePressEvent(event);
	int port = portAt(event->pos());
	if(port >= 0 && !_dragging)
	{
		_dragging = true;
		emit connectorDragStart(event->scenePos(), port);
		if(scene()->dragLock())
		{
			grabMouse();
		}
	}
	else if(_dragging)
	{
		_dragging = false;
		emit connectorDragEnd(event->scenePos());
		ungrabMouse();
	}

}

void DiagramItem::mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event)
{
	emit doubleClicked(event->scenePos());
	setEditMode(true);
}

void DiagramItem::setParentDiagramItem(DiagramItem *item)
{
	if(item)
	{
		auto p = this->scenePos();
		this->model()->setData("parent", item->model()->uuid());
		this->setParentItem(item);
		this->setPos(item->mapFromScene(p));
		item->updateBoundingBox();
		auto flags = this->flags();
		_unparentedSelectable = flags.testFlag(ItemIsSelectable);
		_unparentedMovable = flags.testFlag(ItemIsMovable);

		this->setFlag(ItemIsMovable, false);
	}
	else
	{
		this->model()->removeData("parent");
		this->setParentItem(nullptr);

		this->setFlag(ItemIsMovable, _unparentedMovable);
	}
}

DiagramItem* DiagramItem::parentDiagramItem() const
{
	auto parentUUID = this->model()->getData<QUuid>("parent");
	if(parentUUID && scene())
	{
		return this->scene()->item(*parentUUID);
	}
	else
	{
		return nullptr;
	}
}

DiagramScene* DiagramItem::scene() const
{
	return static_cast<DiagramScene *>(QGraphicsItem::scene());
}

void DiagramItem::markUserBeganMovingItem()
{
	_originalPosition = this->pos();
}

DiagramItem::UserMove DiagramItem::markUserFinishedMovingItem()
{
	if(this->pos() != _originalPosition)
	{
		return {true, _originalPosition, this->pos()};
	}

	return {false, {}, {}};
}

void DiagramItem::mouseReleaseEvent(QGraphicsSceneMouseEvent* event)
{
	QGraphicsRectItem::mouseReleaseEvent(event);
	if(_dragging && !scene()->dragLock())
	{
		_dragging = false;
		emit connectorDragEnd(event->scenePos());
	}

	for(auto item : _portSymbols)
	{
		item->setBrush(QBrush(QColor(0, 0, 0, 0)));
	}

//	if(this->pos() != _originalPosition)
//	{
		// fixme: this isn't sent when moving groups of items
//		emit userFinishedMovingItem(_originalPosition, this->pos());
//		std::cerr << "userFinishedMovingItem(" << _originalPosition << ", " << this->pos() << ")\n";
//	}
}

void DiagramItem::emitDependencyPosChanged(DiagramItem *dependency)
{
	emit dependencyPosChanged(dependency);
}

DiagramItem::~DiagramItem()
{
}


}  // namespace dbuilder
