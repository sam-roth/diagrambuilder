/**
 * @file   DiagramScene.cpp
 *
 * @date   Jan 2, 2013
 * @author Sam Roth <>
 */

#include "DiagramScene.hpp"
#include "DiagramItem.hpp"
#include "DiagramItemModel.hpp"
#include "DiagramComponent.hpp"
#include <iostream>
#include <cassert>
#include "Commands/ConnectItemCommand.hpp"
#include "Util/UUIDTranslator.hpp"
#include <boost/property_tree/xml_parser.hpp>
#include <QGraphicsSceneMouseEvent>
#include "Components/ConnectorComponent.hpp"
#include <QPainter>
#include <QMenu>
#include "Commands/InsertItemCommand.hpp"
#include "Commands/RotateItemCommand.hpp"
#include "Commands/MoveItemCommand.hpp"
#include <memory>
#include "Util.hpp"
#include "DiagramContext.hpp"
#include "Util/Log.hpp"
#include "Handle.hpp"
#include "Util/ListUtil.hpp"
#include <boost/lexical_cast.hpp>
#include "UUIDMapper.hpp"
#include "Commands/InsertItemsCommand.hpp"
#include <QApplication>
#include <QCloseEvent>
#include <QStyle>
#include "TabFocus/TabFocusable.hpp"
#include "TabFocus/TabFocusRing.hpp"

namespace dbuilder {


static QList<DiagramItem *> duplicateItems(const QList<DiagramItem *> &items, QObject *parent)
{
	QList<DiagramItem *> result;
	UUIDMapper mapper;
	for(auto item : items)
	{
		auto m = item->model()->clone(&mapper, parent);
		auto i = m->kind()->createFromModel(m);
		i->setParent(parent);
		result << i;
	}

	return result;
}


DiagramScene::DiagramScene(DiagramContext *context, QObject* parent)
: QGraphicsScene(parent)
, ctx(context)
, lineItem(new QGraphicsLineItem)
, startPort(0)
, _highlightedItem(nullptr)
, _clean(true)
, _printMode(false)
, _connectorType("connector")
, _highlightedHandle(nullptr)
, _hoverListener(nullptr)
, _focusRing(new SequentialTabFocusRing(this))
, _zoomRectangle(nullptr)
, _dragLock(false)
{
}

void DiagramScene::setHighlightedItem(DiagramItem *item, int port)
{
	if(_highlightedItem)
	{
		_highlightedItem->setHighlightedPort(-1);
	}

	if(item)
	{
		item->setHighlightedPort(port);
	}
	_highlightedItem = item;
}

void DiagramScene::setClean(bool clean)
{
	_clean = clean;
	emit modifiedChanged(!_clean);
}
const QMap<QString, DiagramComponent *> &DiagramScene::kinds() const
{
	return ctx->kinds();
}

void DiagramScene::drawBackground(QPainter * painter, const QRectF &rect)
{
	if(!printMode())
	{
		painter->setBackground(QColor(255, 255, 255));
		painter->eraseRect(rect);
		qreal gridInterval = 5;
		painter->setPen(QColor(230, 230, 255));
		for(qreal x = round(rect.left() / gridInterval) * gridInterval; x < rect.right(); x += gridInterval)
		{
			painter->drawLine(x, rect.top(), x, rect.bottom());
		}

		for(qreal y = round(rect.top() / gridInterval) * gridInterval; y < rect.bottom(); y += gridInterval)
		{
			painter->drawLine(rect.left(), y, rect.right(), y);
		}
	}
}

void DiagramScene::addDiagramItem(DiagramItem* item)
{
	setClean(false);
	DBLog(Debug, "added item with uuid ", item->model()->uuid().toString().toStdString());
	_itemsByUuid.insert(item->model()->uuid(), item);
	for(auto dependency : item->model()->dependencies())
		_dependents.insert(dependency, item);
	if(item->scene() != this)
		this->addItem(item);
	connect(item, SIGNAL(posChanged(QPointF)), this, SLOT(diagramItemMoved(QPointF)));
	connect(item, SIGNAL(connectorDragStart(QPointF, int)), this, SLOT(connectorDragStart(QPointF, int)));
	connect(item, SIGNAL(connectorDragMid(QPointF)), this, SLOT(connectorDragMid(QPointF)));
	connect(item, SIGNAL(connectorDragEnd(QPointF)), this, SLOT(connectorDragEnd(QPointF)));
	for(auto i : item->childItems())
	{
		i->setCacheMode(QGraphicsItem::DeviceCoordinateCache);
	}

	item->setCacheMode(QGraphicsItem::DeviceCoordinateCache);
	item->model()->requestUpdateView();
}

void DiagramScene::contextMenuEvent(QGraphicsSceneContextMenuEvent *contextMenuEvent)
{
	QGraphicsScene::contextMenuEvent(contextMenuEvent);
	if(!contextMenuEvent->isAccepted())
	{
		contextMenuEvent->accept();
		emit contextMenu(contextMenuEvent);
	}
}

void DiagramScene::userFinishedMovingItem(QPointF prev, QPointF next)
{
}

void DiagramScene::contextMenuTriggered(QAction *action)
{
}


QList<DiagramItem *> DiagramScene::removeDiagramItem(DiagramItem *item)
{
	setClean(false);
	item->model()->requestUpdateModel();

	if(item->scene() != this) return {};

	QList<DiagramItem *> result;
	result << item;
	_itemsByUuid.remove(item->model()->uuid());
	for(auto dependent : this->_dependents.values(item->model()->uuid()))
	{
		result.append(this->removeDiagramItem(dependent));
	}
	this->_dependents.remove(item->model()->uuid());

	// disconnect all signals from item
	item->disconnect(this);

	this->removeItem(item);

	return result;
}

DiagramItem* DiagramScene::item(QUuid id)
{
	auto it = _itemsByUuid.find(id);
	if(it != _itemsByUuid.end())
	{
		return *it;
	}
	else
	{
		return nullptr;
	}
}

void DiagramScene::update()
{
	setClean(false);
	for(auto item : _itemsByUuid)
	{
		item->emitPosChanged();
	}
}

void DiagramScene::registerKind(DiagramComponent *kind)
{
	ctx->registerKind(kind);
}

DiagramComponent *DiagramScene::kind(const QString &name)
{
	return ctx->kind(name);
}

DiagramScene::~DiagramScene()
{
}

void DiagramScene::diagramItemMoved(QPointF)
{
	this->setClean(false);
	auto sdr = static_cast<DiagramItem *>(sender());
	auto deps = _dependents.values(sdr->model()->uuid());
	for(auto dependent : deps)
	{
		dependent->emitDependencyPosChanged(sdr);
	}
}

void DiagramScene::connectorDragStart(QPointF point, int port)
{
	if(lineItem->scene() != this)
		this->addItem(lineItem);
	startPoint = point;
	startPort = port;
	lineItem->setLine(point.x(), point.y(), point.x(), point.y());
}

void DiagramScene::connectorDragMid(QPointF point)
{
	lineItem->setLine(startPoint.x(), startPoint.y(), point.x(), point.y());
	auto itemsHere = items(point);
	for(auto item : itemsHere)
	{
		if(auto diagramItem = dynamic_cast<DiagramItem *>(item))
		{
			int port = diagramItem->portAt(diagramItem->mapFromScene(point));
			if(port >= 0)
			{
				if(diagramItem != sender() || port != startPort)
				{
					setHighlightedItem(diagramItem, port);
					return;
				}
			}
		}
	}

	setHighlightedItem(nullptr, -1);
}

void DiagramScene::createConnection(Connection conn)
{
	this->undoStack().push(new ConnectItemCommand(this, conn, _connectorType));
}

void DiagramScene::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
	if(event->buttons().testFlag(Qt::MiddleButton))
	{
		auto items = filterCast<DiagramItem>(this->items(event->scenePos()));
		if(!items.empty())
		{
			replicate(items.front());
			event->accept();
			return;
		}
	}


	this->setCursorPos(event->scenePos());


	if(event->modifiers().testFlag(Qt::ControlModifier))
	{
		auto items = filterCast<DiagramItem>(this->items(event->scenePos()));
		if(!items.empty())
		{
			auto item = items.front();
			auto dup = duplicateItems({item}, this);
			auto dupItem = dup.front();
			this->undoStack().push(new dbuilder::InsertItemsCommand(this, {dup}, nullptr));

			item->setSelected(false);
			dupItem->setSelected(true);
			this->send({dupItem}, ToFront);

			this->setFocusItem(dupItem, Qt::OtherFocusReason);
		}
	}

	QGraphicsScene::mousePressEvent(event);

	for(auto item : this->selectedItems())
	{
		if(auto diagramItem = dynamic_cast<DiagramItem *>(item))
		{
			diagramItem->markUserBeganMovingItem();
		}
	}
}

void DiagramScene::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
	if(event->buttons().testFlag(Qt::MiddleButton)) return;

	auto cmd = make_unique<QUndoCommand>();
	bool anyMoved = false;

	for(auto item : this->selectedItems())
	{
		if(auto diagramItem = dynamic_cast<DiagramItem *>(item))
		{
			auto userMove = diagramItem->markUserFinishedMovingItem();
			if(userMove.wasMoved)
			{
				new MoveItemCommand(diagramItem, userMove.from, userMove.to, cmd.get());
				anyMoved = true;
			}
		}
	}

	if(anyMoved)
	{
		this->undoStack().push(cmd.release());
	}

	QGraphicsScene::mouseReleaseEvent(event);
}

void DiagramScene::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
	auto hoverListeners = filterCast<dbuilder::HoverListener>(items(event->scenePos()));
	dbuilder::HoverListener *nextHoverListener = nullptr;
	for(dbuilder::HoverListener *l: hoverListeners)
	{
		if(l->acceptHover(event->scenePos()))
		{
			nextHoverListener = l;
			break;
		}
	}

	if(nextHoverListener != _hoverListener)
	{
		if(_hoverListener)
		{
			_hoverListener->endHover();
		}

		if(nextHoverListener)
		{
			nextHoverListener->startHover();
		}

		_hoverListener = nextHoverListener;
	}

	QGraphicsScene::mouseMoveEvent(event);
}


void DiagramScene::clearDiagram()
{
	clear();
	_itemsByUuid.clear();
	_dependents.clear();
	_highlightedItem = nullptr;
}

void DiagramScene::group()
{
	DBWarning("Grouping has been disabled since it doesn't work properly.");
	return;

	auto itemsToGroup = filterCast<DiagramItem>(this->selectedItems());

	if(itemsToGroup.size() > 1)
	{
		auto parentItem = itemsToGroup[0];
		for(int i = 1; i < itemsToGroup.size(); i++)
		{
			itemsToGroup[i]->setParentDiagramItem(parentItem);
		}
	}
}

void DiagramScene::ungroup()
{
	DBWarning("Grouping has been disabled since it doesn't work properly.");
	return;

	auto selectedItems = filterCast<DiagramItem>(this->selectedItems());
	auto kind = this->kind("group");
	for(auto item : selectedItems)
	{
		if(item->model()->kind() == kind)
		{
			this->removeDiagramItem(item);
		}
	}
}

class DiagramItemLoader
{
	DiagramScene &scene;
	const QList<DiagramItem *> &items;
	QSet<DiagramItem *> itemsBeingLoaded;
	QSet<DiagramItem *> itemsLoaded;
	QMap<QUuid, DiagramItem *> itemsByUUID;
public:
	DiagramItemLoader(DiagramScene &scene, const QList<DiagramItem *> &items)
	: scene(scene)
	, items(items)
	{
		for(auto item : items)
		{
			itemsByUUID[item->model()->uuid()] = item;
		}
	}

	void load(DiagramItem *item)
	{
		if(itemsBeingLoaded.contains(item))
		{
			DBWarning("Circular reference ignored for ", item->model()->uuid(), " of kind ", item->model()->kind()->name());
			return;
		}

		if(itemsLoaded.contains(item))
		{
			return;
		}

		itemsBeingLoaded.insert(item);
		for(auto uuid : item->model()->dependencies())
		{
			if(!itemsByUUID.contains(uuid))
			{
				DBWarning("Item ", item->model()->uuid(), " depends on unknown item ", uuid);
			}
			else
			{
				load(itemsByUUID[uuid]);
			}
		}
		scene.addDiagramItem(item);
		itemsLoaded.insert(item);
		itemsBeingLoaded.remove(item);
	}

	void loadAll()
	{
		for(auto item : items)
		{
			load(item);
		}
	}
};

void DiagramScene::addDiagramItemsInOrder(const QList<DiagramItem*>& items)
{
	DiagramItemLoader(*this, items).loadAll();
}



void DiagramScene::replicateAndConnect(Connection conn)
{
	createConnection(conn);

	auto src = item(conn.src);
	auto dst = item(conn.dst);
	assert(src && dst);

	auto srcPortLoc = src->portLocations()[conn.srcPort];



	int port = 0;
	for(auto portLoc : src->portLocations())
	{
//		if(port != 0)
//		{
			if(portLoc.x() == srcPortLoc.x() && portLoc.y() != srcPortLoc.y())
			{
				UUIDMapper mapper;
				auto model = dst->model()->clone(&mapper, this);
				auto item = new DiagramItem(this);
				item->setModel(model);
				this->undoStack().push(new dbuilder::InsertItemsCommand(this, {item}, nullptr));
				item->setPos(dst->pos() + (portLoc - srcPortLoc));
				this->createConnection(Connection{src->model()->uuid(), port, item->model()->uuid(), conn.dstPort, conn.center});
			}
//		}

		port++;
	}
}

void DiagramScene::replicate(DiagramItem* item)
{
	auto originallySelectedItems = filterCast<DiagramItem>(selectedItems());
	this->clearSelection();

	int i = 0;
	for(auto correspondingItem : originallySelectedItems)
	{
		DiagramItem *replItem;
		if(i == 0)
		{
			replItem = item;
		}
		else
		{
			auto dup = duplicateItems({item}, this);
			replItem = dup.front();
			this->undoStack().push(new dbuilder::InsertItemsCommand(this, {replItem}, nullptr));
		}

		replItem->setPos(correspondingItem->pos());
		replItem->setSelected(true);
		++i;
	}
}

//void DiagramScene::replicateAndInsert(const QList<DiagramItem*>& items)
//{
//	auto previouslySelectedItems = filterCast<DiagramItem *>(selectedItems());
//	this->clearSelection();
//
//	int i = 0;
//	QList<DiagramItem *> itemsToInsert = items;
//
//	for(auto item : previouslySelectedItems)
//	{
//		if(i != 0)
//		{
//			itemsToInsert = duplicateItems(items, this);
//		}
//
//		for(auto i : itemsToInsert)
//		{
//
//		}
//
//		++i;
//	}
//}

void DiagramScene::keyPressEvent(QKeyEvent* event)
{
	if(event->key() == Qt::Key_Tab || event->key() == Qt::Key_Backtab)
	{
		event->accept();
		if(auto tabFocusable = dynamic_cast<dbuilder::TabFocusable *>(this->focusItem()))
		{
			if(auto nextFocusable =
					event->modifiers().testFlag(Qt::ShiftModifier)?
							tabFocusable->prevFocusable() :
							tabFocusable->nextFocusable())
			{
				nextFocusable->makeFocused();
			}
		}
	}
	else
	{
		QGraphicsScene::keyPressEvent(event);
	}
}

void DiagramScene::placeZoomRectangle(QRectF rect)
{
	if(!_zoomRectangle)
	{
		_zoomRectangle = new QGraphicsRectItem;
		this->addItem(_zoomRectangle);
		QPen pen(Qt::green);
		pen.setWidth(0);
		_zoomRectangle->setPen(pen);
	}

	_zoomRectangle->setRect(rect);
}

void DiagramScene::clearZoomRectangle()
{
	delete _zoomRectangle;
	_zoomRectangle = nullptr;
}

static bool zLess(const DiagramItem *a, const DiagramItem *b)
{
	return a->zValue() < b->zValue();
}



qreal DiagramScene::targetZForSending(const QList<DiagramItem *> &items, ZMotion motion)
{
//	auto items = motion & ZMFully?
//		  diagramItems().values()
//		: filterCast<DiagramItem>(this->items(itemsInRect));

	auto it = motion & Backward?
		  std::min_element(items.begin(), items.end(), &zLess)
		: std::max_element(items.begin(), items.end(), &zLess);
	qreal z = it == items.end()?
		  0
		: (**it).zValue();
	return motion & Backward? z - 1 : z + 1;
}

void DiagramScene::send(const QList<DiagramItem *> &items,
                        ZMotion motion)
{

	QRectF boundingRect;
	for(auto item : items)
	{
		boundingRect = boundingRect.united(item->boundingRect());
	}

	auto otherItems = motion & ZMFully?
		  diagramItems().values()
		: filterCast<DiagramItem>(this->items(boundingRect));

	for(auto item : items)
	{
		otherItems.removeAll(item);
	}

	const qreal targetZ = targetZForSending(otherItems, motion);

	for(auto item : items)
	{
		DBDebug("setting item: ", Printable::repr(item), " zValue: ", targetZ);
		item->setZValue(targetZ);
	}

}

void DiagramScene::sendSelection(ZMotion motion)
{
	send(selectedDiagramItems(), motion);
}

QList<DiagramItem*> DiagramScene::selectedDiagramItems()
{
	return filterCast<DiagramItem>(selectedItems());
}

void DiagramScene::connectorDragEnd(QPointF point)
{
	if(lineItem->scene() == this)
		this->removeItem(lineItem);
	auto endItems = items(point);
	auto sdr = static_cast<DiagramItem *>(sender());
	for(auto item : endItems)
	{
		if(auto diagramItem = dynamic_cast<DiagramItem *>(item))
		{
			auto itemPoint = diagramItem->mapFromScene(point);
			int port = diagramItem->portAt(itemPoint);
			if(port >= 0 && (port != startPort || diagramItem != sender()))
			{
				Connection conn{sdr->model()->uuid(), startPort, diagramItem->model()->uuid(), port, 0.5};
				if(QApplication::keyboardModifiers().testFlag(Qt::ControlModifier))
				{
					replicateAndConnect(conn);
				}
				else
				{
					createConnection(conn);
				}
				return;
			}
		}
	}
}

void DiagramScene::setPrintMode(bool printMode)
{
	this->clearSelection();
	_printMode = printMode;
	for (auto item : items())
	{
		item->setCacheMode(
				printMode ?
						QGraphicsItem::NoCache :
						QGraphicsItem::DeviceCoordinateCache);

		if(auto di = dynamic_cast<DiagramItem *>(item))
		{
			di->setPrintMode(printMode);
		}
	}
	update();
}



}  // namespace dbuilder
