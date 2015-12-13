#pragma once
#include <QGraphicsScene>
#include <QUuid>
#include <QMap>
#include <qundostack.h>
#include "DiagramItemModel.hpp"
#include "Handle.hpp"
#include "qgraphicsitem.h"
#include "CoreForward.hpp"
class QGraphicsLineItem;
namespace dbuilder {

class TabFocusRing;

}  // namespace dbuilder
/**
 * @file   DiagramScene.h
 *
 * @date   Jan 2, 2013
 * @author Sam Roth <>
 */

namespace dbuilder {



/**
 * @class DiagramScene
 * A QGraphicsScene that manages instances of DiagramItem.
 *
 */
class DiagramScene: public QGraphicsScene
{
	Q_OBJECT
	DiagramContext *ctx;
	QMap<QUuid, DiagramItem *> _itemsByUuid;
	QMultiMap<QUuid, DiagramItem *> _dependents;
	QSet<QGraphicsItem *> _temporarilyDisabledItems;
//	QMap<QString, DiagramComponent *> _kinds;

	// in progress line
	QGraphicsLineItem *lineItem;
	QPointF startPoint;
	int startPort;

	QUndoStack _undoStack;
	DiagramItem *_highlightedItem;

	bool _clean;
	bool _printMode;

	QPointF insertLoc;

	QString _connectorType;
	dbuilder::Handle *_highlightedHandle;
	dbuilder::HoverListener * _hoverListener;


	QPointF _cursorPos;

	dbuilder::TabFocusRing *_focusRing;

	QGraphicsRectItem *_zoomRectangle;
	bool _dragLock;

	void setHighlightedItem(DiagramItem *item, int port);

public:

	DiagramContext *context() const { return ctx; }

	DiagramScene(DiagramContext *context, QObject *parent=nullptr);

	/**
	 * Add a DiagramItem to the scene.
	 *
	 * @warning Do not use addItem(QGraphicsItem*) for this purpose.
	 * @param item
	 */
	void addDiagramItem(DiagramItem *item);
	/**
	 * Removes the given DiagramItem from the scene along with all of its
	 * dependents.
	 *
	 * @return list of diagram items removed
	 */
	QList<DiagramItem *> removeDiagramItem(DiagramItem *item);

	void addDiagramItemsInOrder(const QList<DiagramItem *> &items);

	/**
	 * Register a DiagramComponent for use with the DiagramScene.
	 * @param kind
	 */
	void registerKind(DiagramComponent *kind);
	/**
	 * @param name
	 * @return the DiagramComponent with the given name
	 */
	DiagramComponent *kind(const QString &name);
	/**
	 * @return a map of all registered DiagramComponent instances by name
	 */
	const QMap<QString, DiagramComponent *> &kinds() const;

	/**
	 * @param id
	 * @return the DiagramItem with the given UUID or nullptr if no such item exists
	 */
	DiagramItem *item(QUuid id);
	/**
	 * @return a map of all DiagramItem instances in this scene by UUID
	 */
	const QMap<QUuid, DiagramItem *> &diagramItems() const { return _itemsByUuid; }

	/**
	 * Creates the given connection in the scene
	 * @param conn
	 */
	void createConnection(Connection conn);

	void replicateAndConnect(Connection conn);
	void replicate(DiagramItem *item);

//	void replicateAndInsert(const QList<DiagramItem *> &items);

	/**
	 * Causes all contained DiagramItem instances to emit posChanged() as if
	 * they had been moved and clears the clean flag.
	 */
	void update();

	/**
	 * Removes all DiagramItems from the scene.
	 */
	void clearDiagram();

	QUndoStack &undoStack() { return _undoStack; }

	virtual ~DiagramScene();

	/**
	 * @return false if the scene has been modified since the clean flag was last set,
	 * true otherwise
	 */
	bool clean() const
	{
		return _clean;
	}

	void setClean(bool clean=true);

	bool printMode() const
	{
		return _printMode;
	}

	void setPrintMode(bool printMode);

	const QString& connectorType() const
	{
		return _connectorType;
	}

	void setConnectorType(const QString& connectorType)
	{
		_connectorType = connectorType;
	}

	const QPointF& cursorPos() const
	{
		return _cursorPos;
	}

	void setCursorPos(const QPointF& cursorPos)
	{
		_cursorPos = cursorPos;
	}


	void placeZoomRectangle(QRectF rect);
	void clearZoomRectangle();

	enum ZMotion
	{
		Backward=1,
		Frontward=1<<1,
		ZMFully=1<<2,
		ToBack=ZMFully|Backward,
		ToFront=ZMFully|Frontward
	};

	void send(const QList<DiagramItem *> &items, ZMotion motion);
	void sendSelection(ZMotion motion);

	QList<DiagramItem *> selectedDiagramItems();

	bool dragLock() const { return _dragLock; }
private:
	qreal targetZForSending(const QList<DiagramItem *> &items, ZMotion motion);

public slots:
	void group();
	void ungroup();
	void setDragLock(bool d) { _dragLock = d; }

public:
	dbuilder::TabFocusRing* focusRing() const
	{
		return _focusRing;
	}

signals:
	/**
	 * emitted upon change of the clean() flag
	 *
	 * @param modified     equivalent to !clean()
	 */
	void modifiedChanged(bool modified);
	void contextMenu(QGraphicsSceneContextMenuEvent *);
protected:
	void mousePressEvent(QGraphicsSceneMouseEvent *mouseEvent);
	void mouseReleaseEvent(QGraphicsSceneMouseEvent *mouseEvent);
	void mouseMoveEvent(QGraphicsSceneMouseEvent *mouseEvent);
	void keyPressEvent(QKeyEvent *event);
	void drawBackground(QPainter * painter, const QRectF & rect);
	virtual void contextMenuEvent(QGraphicsSceneContextMenuEvent *contextMenuEvent);
private slots:
	void contextMenuTriggered(QAction *);
	void diagramItemMoved(QPointF);
	void userFinishedMovingItem(QPointF, QPointF);

	void connectorDragStart(QPointF, int port);
	void connectorDragMid(QPointF);
	void connectorDragEnd(QPointF);
};


}  // namespace dbuilder
