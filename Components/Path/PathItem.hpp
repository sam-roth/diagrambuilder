#pragma once
#include <cmath>
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
#include <QRegion>
#include <QGraphicsSceneMouseEvent>
#include "Handle.hpp"
/**
 * @file   PathItem.hpp
 *
 * @date   Jan 28, 2013
 * @author Sam Roth <>
 */

namespace dbuilder {

class PathHandle: public QObject, public QGraphicsRectItem
{
	Q_OBJECT

public:
	PathHandle(QGraphicsItem *parent)
	: QGraphicsRectItem(-5, -5, 10, 10, parent)
	{
		setFlag(QGraphicsItem::ItemIsMovable);
		setFlag(QGraphicsItem::ItemSendsScenePositionChanges);
		setVisible(false);
	}

	QVariant itemChange(GraphicsItemChange change,
			const QVariant& value)
	{
		if(change == ItemPositionHasChanged)
		{
			emit posChanged();
		}

		return QGraphicsRectItem::itemChange(change, value);
	}
signals:
	void posChanged();
};

class PathItem: public QObject, public QGraphicsPathItem, public dbuilder::HoverListener /*, public dbuilder::Handle*/
{
	Q_OBJECT
	DiagramItem *_item;
	QList<PathHandle *> handles;
	bool _editingPath;
	QGraphicsPolygonItem *_leftArrow, *_rightArrow;
	bool _startConstrained, _endConstrained;
	qreal _oldZ;

	typedef QGraphicsPathItem Super;


	QGraphicsPolygonItem *makeArrow()
	{
		auto result = new QGraphicsPolygonItem(QPolygonF({
			{-5.0, -10}, {5.0, -10.0}, {0.0, 0.0}
		}), this);

		result->setPen(this->pen());
		result->setBrush(QBrush(Qt::black));
		return result;
	}

public:

	PathItem(DiagramItem *parent)
	: QGraphicsPathItem(parent)
	, _item(parent)
	, _editingPath(false)
	, _leftArrow(nullptr)
	, _rightArrow(nullptr)
	, _startConstrained(false)
	, _endConstrained(false)
	, _oldZ(0)
	{
		setAcceptHoverEvents(false);
		connect(_item, SIGNAL(doubleClicked(QPointF)), this, SLOT(containerDoubleClicked()));
		connect(_item, SIGNAL(selectionChanged(bool)), this, SLOT(containerSelectionChanged(bool)));

		QPen p;
		p.setWidthF(1);
		this->setPen(p);

		this->setBoundingRegionGranularity(1);


		assert(_item->model());
		if(auto pathTree = _item->model()->getTree("path"))
		{
			this->setPath(treeToPath(*pathTree));
		}
		else
		{
			QPainterPath path;
			path.quadTo(100, 50, 100, 100);
			this->setPath(path);
		}

		setLeftArrow(_item->model()->getData<bool>("leftArrow").get_value_or(false));
		setRightArrow(_item->model()->getData<bool>("rightArrow").get_value_or(false));

	}

	void beginEditingPath()
	{
		scene()->clearSelection();
		_item->setSelected(true);
		_editingPath = true;
		for(int i = 0; i < handles.size(); i++)
		{
			if(!((i == 0 && _startConstrained) || (i == handles.size() - 1 && _endConstrained)))
			{
				handles[i]->setVisible(true);
			}
		}
	}

	void endEditingPath()
	{
		_editingPath = false;
		for(auto handle : handles)
		{
			handle->setVisible(false);
		}


		boost::property_tree::ptree pt;
		auto path = this->path();
		for(int i = 0; i < path.elementCount(); i++)
		{
			auto element = path.elementAt(i);
			boost::property_tree::ptree elementTree;
			elementTree.put("x", element.x);
			elementTree.put("y", element.y);
			elementTree.put("type", int(element.type));

			pt.add_child("element", elementTree);
		}

		_item->model()->setTree("path", pt);
	}

	void updateArrows()
	{
		auto path = this->path();
		if(path.elementCount() >= 2 && _leftArrow)
		{
			auto x = path.elementAt(0);
			auto y = path.elementAt(1);
			double angle = std::atan2(y.y - x.y, y.x - x.x) * 180.0 / M_PI;
			_leftArrow->setRotation(angle + 90);
			_leftArrow->setPos(x.x, x.y);
		}
		if(path.elementCount() >= 2 && _rightArrow)
		{
			auto x = path.elementAt(path.elementCount() - 1);
			auto y = path.elementAt(path.elementCount() - 2);
			double angle = std::atan2(y.y - x.y, y.x - x.x) * 180.0 / M_PI;
			_rightArrow->setRotation(angle + 90);
			_rightArrow->setPos(x.x, x.y);
		}
	}

	void setLeftArrow(bool on)
	{
		if(_leftArrow && !on)
		{
			delete _leftArrow;
			_leftArrow = nullptr;
		}
		else if(on)
		{
			_leftArrow = makeArrow();
		}
		updateArrows();

		_item->model()->setData("leftArrow", on);
	}
	void setRightArrow(bool on)
	{
		if(_rightArrow && !on)
		{
			delete _rightArrow;
			_rightArrow = nullptr;
		}
		else if(on)
		{
			_rightArrow = makeArrow();
		}
		updateArrows();

		_item->model()->setData("rightArrow", on);
	}

	bool leftArrow() const
	{
		return _leftArrow;
	}

	bool rightArrow() const
	{
		return _rightArrow;
	}

	void setPath(const QPainterPath &path)
	{

		for(auto handle : handles)
		{
			delete handle;
		}

		handles.clear();

		for(int i = 0; i < path.elementCount(); i++)
		{
			auto x = path.elementAt(i);
			auto handle = new PathHandle(this);
			handle->setPos(x.x, x.y);
			connect(handle, SIGNAL(posChanged()), this, SLOT(handlePosChanged()));
			handles << handle;
		}

		Super::setPath(path);
		updateArrows();
		_item->updateBoundingBox();
		endEditingPath();
	}

	bool endConstrained() const
	{
		return _endConstrained;
	}

	void setEndConstrained(bool endConstrained)
	{
		_endConstrained = endConstrained;
	}

	bool startConstrained() const
	{
		return _startConstrained;
	}

	void setStartConstrained(bool startConstrained)
	{
		_startConstrained = startConstrained;
	}

	void setStart(QPointF pos)
	{
		if(handles.size() > 0)
		{
			handles.front()->setPos(0, 0);
			this->setPos(0, 0);
			_item->setPos(pos);
		}
	}

	void setEnd(QPointF pos)
	{
		if(handles.size() > 0)
		{
			handles.back()->setPos(pos - _item->pos());
		}
	}

	void startHover()
	{
		auto p = pen();
		p.setColor(Qt::blue);
		this->setPen(p);
	}

	void endHover()
	{
		auto p = pen();
		p.setColor(Qt::black);
		setPen(p);
	}

	bool acceptHover(QPointF p)
	{
		if(!_editingPath)
			return this->boundingRegion(this->sceneTransform()).contains(p.toPoint());
		else
			return false;
	}

public slots:
	void addNode()
	{
		auto path = this->path();
		QPointF base(0,0);
		if(path.elementCount() > 0)
		{
			auto elt = path.elementAt(path.elementCount()-1);
			base.setX(elt.x);
			base.setY(elt.y);
		}

		QPointF off1(30, 0);
		QPointF off2(30, 30);
		path.quadTo(base + off1, base + off2);
		this->setPath(path);
	}

protected:
	QVariant itemChange(GraphicsItemChange change, const QVariant &value)
	{
		if(change == ItemPositionHasChanged)
		{
			_item->updateBoundingBox();
		}

		return Super::itemChange(change, value);
	}

	void hoverEnterEvent(QGraphicsSceneHoverEvent *event)
	{
		if(this->acceptHover(event->scenePos()))
			startHover();
		Super::hoverEnterEvent(event);
	}

	void mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event)
	{
		Super::mouseDoubleClickEvent(event);
		if(this->acceptHover(event->scenePos()))
		{
			event->accept();
			beginEditingPath();
		}
		else
		{
			event->ignore();
		}

	}

	void mousePressEvent(QGraphicsSceneMouseEvent *event)
	{
		Super::mousePressEvent(event);
		if(this->acceptHover(event->scenePos()))
		{
			event->accept();
		}
		else
		{
			event->ignore();
		}

	}

	void hoverLeaveEvent(QGraphicsSceneHoverEvent *event)
	{
		endHover();
		Super::hoverLeaveEvent(event);
	}
protected slots:
	void containerDoubleClicked()
	{
		beginEditingPath();
	}

	void containerSelectionChanged(bool selected)
	{
		if(!selected && _editingPath)
		{
			endEditingPath();
		}
	}

	void handlePosChanged()
	{
		auto handle = static_cast<PathHandle *>(sender());
		int i = handles.indexOf(handle);
		auto path = this->path();
		path.setElementPositionAt(i, handle->pos().x(), handle->pos().y());
		Super::setPath(path);
		updateArrows();
		_item->updateBoundingBox();
	}
};

}  // namespace dbuilder

