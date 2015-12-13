#pragma once
#include <QObject>
#include <QGraphicsRectItem>
#include <QAction>
#include "Util/Printable.hpp"
#include <iosfwd>
#include "CoreForward.hpp"
#include "Util/Extendable.hpp"
/**
 * @file   DiagramItem.h
 *
 * @date   Jan 2, 2013
 * @author Sam Roth <>
 */
class QGraphicsEllipseItem;
class QAbstractItemModel;

namespace dbuilder {

/**
 * @class DiagramItem
 *
 * Serves as a container for elements in the diagram, as well as
 * painting ports.
 *
 * Uses DiagramComponent to display the diagram symbol and provide
 * port locations.
 */
class DiagramItem
: public QObject
, public QGraphicsRectItem
, public Printable
, public Extendable
{
	Q_OBJECT
	friend class DiagramScene;
	friend class DiagramComponent;
	DiagramItemModel *_model;
	QList<QPointF> _portLocations;
	QList<QGraphicsEllipseItem *> _portSymbols;
	bool _dragging;
	bool _printMode;
	bool _explicitSelectionOnly;
	int _highlightedPort;
	bool _positionBeingSet;
	bool _itemSnaps;
	bool _unparentedSelectable;
	bool _unparentedMovable;
	bool _positionByCenter;

	QPointF _originalPosition;
	QAbstractItemModel *_settingsModel;
	dbuilder::Application *_app;

	bool _editMode;

	void init();

	Q_PROPERTY(QPointF pos READ pos WRITE setPos)
	Q_PROPERTY(qreal x READ x WRITE setX)
	Q_PROPERTY(qreal y READ y WRITE setY)

protected:
	/**
	 * @warning you must call setModel(DiagramItemModel*) before adding this item to a scene
	 * @param parent
	 */
	DiagramItem(QObject *parent=nullptr);
public:

	void setHighlightedPort(int port=-1);

	void addPort(QPointF);
	const QList<QPointF> &portLocations() const;
	QPointF scenePortLocation(int port) const;
	int portAt(QPointF);
	void clearPorts();

	void updateBoundingBox();
	virtual ~DiagramItem();


	DiagramScene *scene() const;

	DiagramItemModel* model() const
	{
		return _model;
	}

	void setModel(DiagramItemModel*);

	bool printMode() const
	{
		return _printMode;
	}

	void setPrintMode(bool printMode);

	bool explicitSelectionOnly() const
	{
		return _explicitSelectionOnly;
	}

	void setExplicitSelectionOnly(bool explicitSelectionOnly)
	{
		_explicitSelectionOnly = explicitSelectionOnly;
	}

	void explicitlySelect();

	QAbstractItemModel* settingsModel() const
	{
		return _settingsModel;
	}

	void setSettingsModel(QAbstractItemModel* settingsModel)
	{
		_settingsModel = settingsModel;
	}


	void setParentDiagramItem(DiagramItem *);
	DiagramItem *parentDiagramItem() const;

	void print(std::ostream &os) const;
public slots:
	void deleteDiagramItem();
	void beginEditing();

	bool itemSnaps() const
	{
		return _itemSnaps;
	}

	void setItemSnaps(bool itemSnaps)
	{
		_itemSnaps = itemSnaps;
	}

	bool positionByCenter() const
	{
		return _positionByCenter;
	}

	void setPositionByCenter(bool positionByCenter)
	{
		_positionByCenter = positionByCenter;
	}

	void setEditMode(bool);
	bool editMode() const;
signals:
	void printModeChanged(bool);
	void posChanged(QPointF);
	void dependencyPosChanged(DiagramItem *dependency);
	void addedToScene();
	void willBeRemovedFromScene();
	void connectorDragStart(QPointF, int port);
	void connectorDragMid(QPointF);
	void connectorDragEnd(QPointF);
	void selectionChanged(bool);
	void doubleClicked(QPointF);

	void editModeChanged(bool);
	void editingBegan();
	void editingFinished();
protected:
	virtual void contextMenuEvent(QGraphicsSceneContextMenuEvent *event);
	virtual QVariant itemChange(GraphicsItemChange change, const QVariant & value);
	virtual void hoverMoveEvent(QGraphicsSceneHoverEvent *event);
	virtual void hoverLeaveEvent(QGraphicsSceneHoverEvent *event);
	virtual void mouseMoveEvent(QGraphicsSceneMouseEvent *event);
	virtual void mousePressEvent(QGraphicsSceneMouseEvent *event);
	virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);
	virtual void mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event);
private:
	struct UserMove
	{
		bool wasMoved;
		QPointF from, to;
	};
	/**
	 * private interface with DiagramScene
	 * @param dependency
	 */
	void emitDependencyPosChanged(DiagramItem *dependency);
	void emitPosChanged();

	/**
	 * Called when the mouse button is released and the item is selected.
	 * @return
	 */
	UserMove markUserFinishedMovingItem();
	void markUserBeganMovingItem();

private slots:
	void updateViewRequested();
	void updateModelRequested();
	void settingsChanged();
};


}  // namespace dbuilder
