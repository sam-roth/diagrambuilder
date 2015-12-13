#pragma once
/**
 * @file   DiagramView.hpp
 *
 * @date   Mar 3, 2013
 * @author Sam Roth <>
 */

#include <qgraphicsview.h>
#include <QTimeLine>

namespace dbuilder
{

/**
 * QGraphicsView subclass for holding a DiagramScene.
 *
 * Improves upon QGraphicsView by adding smooth zooming
 * and pinch/stretch gestures.
 */
class DiagramView: public QGraphicsView
{
	Q_OBJECT
	qreal z;
	qreal targetZ;
	QTimer *zoomTimer;
public:
	DiagramView(QWidget *parent=nullptr);
	virtual ~DiagramView();

protected:
	virtual void wheelEvent(QWheelEvent*);
	virtual bool event(QEvent *);
private slots:
	void zoomTimerTimeout();
private:
	void beginZoom(qreal steps);
};


} /* namespace dbuilder */
