/**
 * @file   DiagramView.cpp
 *
 * @date   Mar 3, 2013
 * @author Sam Roth <>
 */

#include "moc_DiagramView.cpp"
#include <QTimeLine>
#include <QCloseEvent>
#include "Util/Log.hpp"
#include <qgraphicsview.h>
#include <QScrollBar>
#include <cmath>
#include <QGLWidget>
#include <QTouchEvent>
#include <algorithm>
#include <QtCore>
#include <QApplication>
#include "DiagramScene.hpp"
#include <QtGui>


namespace dbuilder
{

namespace {

qreal weightedAverage(qreal a, qreal b, qreal weight)
{
	return b * weight + a * (1.0 - weight);
}

}  // anonymous namespace

DiagramView::DiagramView(QWidget *parent)
: QGraphicsView(parent)
, z(1.0)
, targetZ(1.0)
, zoomTimer(new QTimer(this))
{
	zoomTimer->setInterval(1000/30);

	connect(zoomTimer, SIGNAL(timeout()), this, SLOT(zoomTimerTimeout()));

	this->setTransformationAnchor(QGraphicsView::AnchorUnderMouse);
	this->setDragMode(RubberBandDrag);
	this->setRenderHints(QPainter::Antialiasing |
	                     QPainter::SmoothPixmapTransform |
	                     QPainter::HighQualityAntialiasing |
	                     QPainter::TextAntialiasing);
	this->setViewport(new QGLWidget(QGLFormat(QGL::SampleBuffers)));

	grabGesture(Qt::PinchGesture);
}

DiagramView::~DiagramView()
{
}

void DiagramView::wheelEvent(QWheelEvent *event)
{
	// zoom only when Ctrl/Cmd is held down
	if(event->modifiers().testFlag(Qt::ControlModifier) ||
			event->modifiers().testFlag(Qt::MetaModifier))
	{
		event->accept();

		qreal numDegrees = event->delta() / 8.0;
		// zoom using 60 degrees of mouse wheel rotation per step
		beginZoom(numDegrees / 60.0);
	}
	else
	{
		QGraphicsView::wheelEvent(event);
	}
}

void DiagramView::beginZoom(qreal steps)
{
	if(!zoomTimer->isActive())
	{
		// sync up z with current transform
		z = targetZ = std::log(transform().m11());
	}

	// Flood of wheel events prevents timer from being triggered
	// when using certain mice. Call slot here to get around this.
	if(zoomTimer->isActive())
	{
		zoomTimerTimeout();
	}

	// limit zoom to (-5,5)
	targetZ += steps;
	targetZ = std::max(std::min(targetZ, 5.0), -5.0);

	zoomTimer->start();
}

bool DiagramView::event(QEvent *event)
{
	// handle pinch/stretch gestures
	if(auto gestureEvent = dynamic_cast<QGestureEvent *>(event))
	{
		for(auto gesture : gestureEvent->gestures())
		{
			if(auto pinchGesture = dynamic_cast<QPinchGesture *>(gesture))
			{
				auto steps = std::log(pinchGesture->scaleFactor() /
				                        pinchGesture->lastScaleFactor());
				beginZoom(steps);
			}
		}
	}
	return QGraphicsView::event(event);
}

void DiagramView::zoomTimerTimeout()
{
	// use a zoom level that is the average of the current
	// zoom level and the target zoom level
	z = weightedAverage(z, targetZ, 0.5);
	if(std::abs(z - targetZ) < 0.01)
	{
		z = targetZ;
		zoomTimer->stop();
	}

	// z, the absolute zoom level, is the logarithm of the scale
	auto sc = std::exp(z);

	// since z is the absolute zoom level, we can't just use the scale() method;
	// we need to start with an empty QTransform instead
	auto currentTransform = transform();
	QTransform t;
	t.scale(sc, sc).translate(currentTransform.dx(), currentTransform.dy());
	setTransform(t, false);
}

} /* namespace dbuilder */
