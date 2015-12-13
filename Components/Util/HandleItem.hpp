#pragma once
/**
 * @file   HandleItem.hpp
 *
 * @date   Mar 7, 2013
 * @author Sam Roth <>
 */


#include <QObject>
#include <QGraphicsPolygonItem>
#include <QPainter>
#include "qgraphicsitem.h"
#include "DiagramItem.hpp"
#include <qglobal.h>
#include <QSvgGenerator>
#include <QGraphicsScene>
#include <QGraphicsSceneMouseEvent>
#include "Util/Log.hpp"
namespace dbuilder {


class HandleItem: public QObject, public QGraphicsPolygonItem /*, public dbuilder::Handle*/
{
	Q_OBJECT
	bool posOverride;
	bool _printMode;
	Qt::Orientations _orientation;
	bool _selectsParent;
	bool _locksParent;
public:
	HandleItem(QObject *parent=nullptr)
	: QObject(nullptr)
	, posOverride(false)
	, _printMode(false)
	, _orientation(Qt::Horizontal)
	, _selectsParent(true)
	, _locksParent(false)
	{
		QPolygonF poly(QVector<QPointF>{{5.0, 0.0}, {0.0, 5.0}, {-5.0, 0.0}, {0.0, -5.0}});
		this->setPolygon(poly);
		setPrintMode(false);
		this->setAcceptHoverEvents(true);
		this->setFlag(ItemSendsScenePositionChanges);
		this->setFlag(ItemIsMovable);
	}

	void overridePos(QPointF p)
	{
		posOverride = true;
		this->setPos(p);
		posOverride = false;
	}

	void startHover()
	{
		this->setBrush(QBrush(QColor(0, 0, 0)));
	}

	void endHover()
	{
		this->setBrush(QBrush(QColor(0, 0, 0, 0)));
	}


public slots:
	bool printMode() const
	{
		return _printMode;
	}

	void setPrintMode(bool printMode)
	{
		_printMode = printMode;
		if(_printMode)
		{
			this->setPen(QPen(QColor(0,0,0,0)));
		}
		else
		{
			QPen pen(QColor("SlateBlue"));
//			pen.setWidth(1);
			this->setPen(pen);
		}
	}

	const Qt::Orientations& orientation() const
	{
		return _orientation;
	}

	void setOrientation(const Qt::Orientations& orientation)
	{
		_orientation = orientation;
	}

	bool selectsParent() const
	{
		return _selectsParent;
	}

	void setSelectsParent(bool selectsParent)
	{
		_selectsParent = selectsParent;
	}

	bool locksParent() const
	{
		return _locksParent;
	}

	void setLocksParent(bool locksParent)
	{
		_locksParent = locksParent;
	}

protected:
	void hoverEnterEvent(QGraphicsSceneHoverEvent *event)
	{
		startHover();
		QGraphicsPolygonItem::hoverEnterEvent(event);
	}
	void hoverLeaveEvent(QGraphicsSceneHoverEvent *event)
	{
		endHover();
		QGraphicsPolygonItem::hoverLeaveEvent(event);
	}
	QVariant itemChange(GraphicsItemChange change,
			const QVariant& value)
	{
		if(change == QGraphicsItem::ItemPositionChange)
		{
			auto pt = value.toPointF();
			QPointF result = pos();
			if(!posOverride)
			{

				if(_orientation.testFlag(Qt::Horizontal)) result.setX(pt.x());
				if(_orientation.testFlag(Qt::Vertical))   result.setY(pt.y());

//				if(!(_orientation.testFlag(Qt::Horizontal) && _orientation.testFlag(Qt::Vertical)))
//				{
//
//					if(_orientation.testFlag(Qt::Horizontal))
//					{
//						pt = QPointF(pt.x(), this->pos().y());
//					}
//					else if(_orientation.testFlag(Qt::Vertical))
//					{
//						pt = QPointF(this->pos().y(), pt.x());
//					}
//					else
//					{
//						pt = pos();
//					}
//				}

				emit handleWillMove(result);
				return result;
			}
		}
		else if(change == QGraphicsItem::ItemPositionHasChanged)
		{
			if(!posOverride)
			{
				emit handleMoved(QPointF(pos().x(), this->pos().y()));
			}
		}

		return QGraphicsPolygonItem::itemChange(change, value);
	}

protected:
	void mousePressEvent(QGraphicsSceneMouseEvent *event)
	{
		if(_locksParent)
		{
			this->topLevelItem()->setFlag(ItemIsMovable, false);
		}
		QGraphicsPolygonItem::mousePressEvent(event);
		if(!_selectsParent) return;
		if(!event->modifiers().testFlag(Qt::ControlModifier))
		{
			this->scene()->clearSelection();
		}

		if(auto parent = this->parentItem())
		{
			if(auto grandparent = parent->parentItem())
			{
				if(auto digp = dynamic_cast<DiagramItem *>(grandparent))
				{
					digp->explicitlySelect();
				}
			}
		}
	}

	void mouseMoveEvent(QGraphicsSceneMouseEvent *event)
	{
		QGraphicsPolygonItem::mouseMoveEvent(event);
	}

	void mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
	{
		if(_locksParent)
		{
			this->topLevelItem()->setFlag(ItemIsMovable, true);
		}
		QGraphicsPolygonItem::mouseReleaseEvent(event);
	}

signals:
	void handleWillMove(QPointF&);
	void handleMoved(QPointF);

private:
	virtual ~HandleItem();
};


}  // namespace dbuilder
