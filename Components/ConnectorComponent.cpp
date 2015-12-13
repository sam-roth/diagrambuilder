/**
 * @file   ConnectorComponent.cpp
 *
 * @date   Jan 3, 2013
 * @author Sam Roth <>
 */

// MOC had trouble parsing some of the boost headers
#ifndef Q_MOC_RUN
#include "ConnectorComponent.hpp"
#include "../DiagramComponent.hpp"
#include <QGraphicsPathItem>
#include "DiagramItem.hpp"
#include "DiagramScene.hpp"
#include "DiagramItemModel.hpp"
#include <QGraphicsSceneMouseEvent>
#include <iostream>
#include <algorithm>
#include <boost/lexical_cast.hpp>
#include <cmath>
#include "Handle.hpp"
#include "Util/HandleItem.hpp"
#include <QGraphicsItem>
#include "Util/FunctionSlot.hpp"
#include "Util/HatchItem.hpp"
#include "BasicPropertyWidget.hpp"
#include "Util/Synchronizer.hpp"
#include "Commands/FunctionUndoCommand.hpp"
#include "PropertyBinder.hpp"
#include "Util/Translators.hpp"
#include "ColorButton.hpp"
#endif

namespace dbuilder {

static const char *LineThicknessKey = "connector.style.lineThickness";
static const char *ConnectorTypeKey = "connector.type";
static const char *BusWidthKey      = "connector.busWidth";
static const char *ArrowheadsKey    = "connector.arrowheads";
static const char *StrokeColorKey   = "connector.style.strokeColor";


template <typename T>
void remove(T *&widget)
{
	delete widget;
	widget = nullptr;
}


static const QPolygonF ArrowheadTriangle(QVector<QPointF>{{0.0, 0.0}, {5.0, -5.0}, {5.0, 5.0}, {0.0, 0.0}});

class ConnectionGraphicsItem: public QObject, public QGraphicsPathItem
{
	Q_OBJECT
	friend class ConnectorPropertyWidget;
	DiagramItem *item;
	HandleItem *handle;
	HatchItem *leftHatchItem, *rightHatchItem;
	QGraphicsPolygonItem *leftArrowhead, *rightArrowhead;

	enum Mode
	{
		PolylineMode, LineMode
	};
	Mode mode;

	enum Arrowhead
	{
		NoArrowhead=0, LeftArrowhead=1, RightArrowhead=LeftArrowhead<<1
	};

	int _arrowhead;

	Q_PROPERTY(bool leftArrow READ leftArrow WRITE setLeftArrow)
	Q_PROPERTY(bool rightArrow READ rightArrow WRITE setRightArrow)
	Q_PROPERTY(int busWidth READ busWidth WRITE setBusWidth)
	Q_PROPERTY(int lineThickness READ lineThickness WRITE setLineThickness)
	Q_PROPERTY(QString connectorType READ connectorType WRITE setConnectorType)
	Q_PROPERTY(QColor color READ color WRITE setColor)

public:
	ConnectionGraphicsItem(DiagramItem *item, QObject *parent=nullptr)
	: QObject(parent)
	, item(item)
	, handle(new HandleItem(this))
	, leftHatchItem(nullptr)
	, rightHatchItem(nullptr)
	, leftArrowhead(nullptr)
	, rightArrowhead(nullptr)
	, mode(PolylineMode)
	, _arrowhead(NoArrowhead)
	{
		item->addExtension(this);

		handle->setParentItem(this);
		handle->setFlag(QGraphicsItem::ItemIsMovable);
		connect(item, SIGNAL(dependencyPosChanged(DiagramItem *)), this, SLOT(dependencyPosChanged(DiagramItem *)));
		connect(item, SIGNAL(printModeChanged(bool)), handle, SLOT(setPrintMode(bool)));
		connect(handle, SIGNAL(handleMoved(QPointF)), this, SLOT(handleMove(QPointF)));
		connect(item->model(), SIGNAL(updateViewRequested()), this, SLOT(dependencyPosChanged()));
		this->setBoundingRegionGranularity(0.25);
		auto p = this->pen();
		p.setWidthF(1.0);
		this->setPen(p);

		updateLineType();
	}

	int arrowhead() const
	{
		return _arrowhead;
	}

	void setArrowhead(int arrowhead)
	{
		DBDebug("Arrowheads set: ", std::hex, arrowhead);
		item->model()->setData(ArrowheadsKey, arrowhead);
		updateLineType();
	}

	void setColor(QColor c)
	{
		item->model()->setData(StrokeColorKey, c.name());
		updateLineType();
	}

	QColor color() const
	{
		return pen().color();
	}
public slots:
	void setBusWidth(int w)
	{
		item->model()->setData(BusWidthKey, w);
		updateLineType();
	}

	int busWidth() const
	{
		return item->model()->getData<int>(BusWidthKey).get_value_or(1);
	}

	int lineThickness() const
	{
		return item->model()->getData<int>(LineThicknessKey).get_value_or(1);
	}

	bool leftArrow() const
	{
		return arrowhead() & LeftArrowhead;
	}

	bool rightArrow() const
	{
		return arrowhead() & RightArrowhead;
	}

	void setLeftArrow(bool on)
	{
		setArrowhead(on? arrowhead() | LeftArrowhead: arrowhead() & ~LeftArrowhead);
	}

	void setRightArrow(bool on)
	{
		setArrowhead(on? arrowhead() | RightArrowhead : arrowhead() & ~RightArrowhead);
	}

	void setLineThickness(int t)
	{
		item->model()->setData(LineThicknessKey, t);
		updateLineType();
	}

	QString connectorType() const
	{
		return item->model()->getData<QString>(ConnectorTypeKey).get_value_or("polyline");
	}

	void setConnectorType(const QString &t)
	{
		item->model()->setData(ConnectorTypeKey, t);
		updateLineType();
	}
private slots:

	void showBusMenu()
	{
		QList<QAction *> pointers;
		const int currentValue = item->model()->getData<int>(BusWidthKey).get_value_or(1);
		for(int i = 1; i <= 32; ++i)
		{
			auto iVariant = QVariant::fromValue(i);
			pointers << new QAction(iVariant.toString(), nullptr);
			pointers.back()->setData(iVariant);
			if(i == currentValue)
			{
				pointers.back()->setCheckable(true);
				pointers.back()->setChecked(true);
			}
		}

		auto result = QMenu::exec(pointers, QCursor::pos());

		if(result)
		{
			item->model()->setData(BusWidthKey, result->data().toInt());
			updateLineType();
		}
		qDeleteAll(pointers);
	}

	void showStyleMenu()
	{
		QAction hairline(tr("Hairline"), nullptr);
		QAction normal(tr("Normal"), nullptr);
		QAction thick(tr("Thick"), nullptr);

		QMap<int, QAction *> actions;
		actions[0] = &hairline;
		actions[1] = &normal;
		actions[3] = &thick;

		auto lineThickness = item->model()->getData<int>(LineThicknessKey)
		    .get_value_or(1);
		if(actions.contains(lineThickness))
		{
			auto active = actions[lineThickness];
			active->setCheckable(true);
			active->setChecked(true);
		}

		auto actionSelected = QMenu::exec(actions.values(), QCursor::pos());

		if(actionSelected)
			item->model()->setData(LineThicknessKey,
			                       actions.key(actionSelected));
		updateLineType();
	}

	void setPolyline()
	{
		item->model()->setData(ConnectorTypeKey, "polyline");
		updateLineType();
	}

	void setLine()
	{
		item->model()->setData(ConnectorTypeKey, "line");
		updateLineType();
	}

	void setMode(Mode m)
	{
		if(m == LineMode)
		{
			setLine();
		}
		else if(m == PolylineMode)
		{
			setPolyline();
		}
	}

	void updateLineType()
	{
		auto connectorType = item->model()->getData<std::string>(
		        ConnectorTypeKey).get_value_or("");
		if(connectorType == "line")
		{
			mode = LineMode;
			handle->setOrientation(0);
		}
		else // assume polyline
		{
			mode = PolylineMode;
			handle->setOrientation(Qt::Horizontal);
		}

		_arrowhead = item->model()->getData<int>(ArrowheadsKey).get_value_or(NoArrowhead);
		DBDebug("Read arrowhead value from model: ", std::hex, _arrowhead);


		if(_arrowhead & LeftArrowhead)
		{
			if(!leftArrowhead)
			{
				leftArrowhead = new QGraphicsPolygonItem(ArrowheadTriangle, this);
				leftArrowhead->setPos(0, 0);
			}
		}
		else
		{
			remove(leftArrowhead);
		}

		if(_arrowhead & RightArrowhead)
		{
			if(!rightArrowhead)
			{
				rightArrowhead = new QGraphicsPolygonItem(ArrowheadTriangle, this);
				rightArrowhead->setRotation(180);
			}
		}
		else
		{
			remove(rightArrowhead);
		}

		auto color = QColor(item->model()->getData<QString>(StrokeColorKey).get_value_or("#000"));

		for(auto ah : {leftArrowhead, rightArrowhead})
		{
			if(ah)
			{
				ah->setBrush(color);
				QPen p;
				p.setStyle(Qt::NoPen);
				p.setWidth(0);
				p.setCapStyle(Qt::FlatCap);
				ah->setPen(p);
			}
		}

		auto lineThickness = item->model()->getData<int>(LineThicknessKey)
		    .get_value_or(1);

		auto pen = this->pen();
		pen.setCapStyle(Qt::FlatCap);
		pen.setWidth(lineThickness);
		pen.setColor(color);
		this->setPen(pen);

		int busWidth = item->model()->getData<int>(BusWidthKey).get_value_or(1);
		if(busWidth > 1)
		{
			if(!leftHatchItem)
			{
				leftHatchItem = new HatchItem(this);
				rightHatchItem = new HatchItem(this);
			}
			auto busWidthStr = QString::number(busWidth);
			leftHatchItem->setText(busWidthStr);
			rightHatchItem->setText(busWidthStr);
		}
		else if(leftHatchItem)
		{
			delete leftHatchItem;
			delete rightHatchItem;
			leftHatchItem = rightHatchItem = nullptr;
		}

		dependencyPosChanged();
	}

	void handleMove(QPointF point)
	{
		if(mode == PolylineMode)
		{
			if(auto conn = item->model()->connection())
			{
				conn->center = point.x() / this->boundingRect().width();
				if(fabs(conn->center - 0.5) < 0.05) conn->center = 0.5;
				item->model()->setConnection(*conn);

			}
			dependencyPosChanged(nullptr);
		}
	}

	void dependencyPosChanged(DiagramItem *dep=nullptr)
	{
		if(item->scene())
		{
			auto scene = static_cast<DiagramScene *>(item->scene());
			auto model = item->model();
			if(!model->connection()) return;
			auto conn = *model->connection();

			auto p1 = scene->item(conn.src);
			auto p2 = scene->item(conn.dst);

			if(p1 && p2)
			{
				auto srcPt = p1->scenePortLocation(conn.srcPort);
				auto dstPt = p2->scenePortLocation(conn.dstPort);

				if(srcPt.x() > dstPt.x())
				{
					std::swap(srcPt, dstPt);
				}

				auto startPoint = ((_arrowhead & LeftArrowhead) && dstPt.x() - srcPt.x() > 2.5)? QPointF{2.5, 0.0} : QPointF{0.0, 0.0};
				QPainterPath path(startPoint);

				if(mode == LineMode)
				{
					this->setPos(mapToParent(mapFromScene(srcPt)));

					qreal w = dstPt.x() - srcPt.x();
					qreal h = dstPt.y() - srcPt.y();

					qreal xc = w / 2.0;
					qreal yc = h / 2.0;

					path.lineTo(w, h);

					this->handle->overridePos(QPointF(xc, yc));

					this->setPath(path);
					item->updateBoundingBox();

					if(leftHatchItem)
					{
						rightHatchItem->setVisible(false);
						leftHatchItem->setPos(xc - 5, yc - 5);
					}

					if(leftArrowhead)
					{
						leftArrowhead->setRotation(-path.angleAtPercent(0.5));
					}

					if(rightArrowhead)
					{
						rightArrowhead->setPos(w, h);
						rightArrowhead->setRotation(180 - path.angleAtPercent(0.5));
					}
					return;
				}

				qreal xc = conn.center * (dstPt.x() - srcPt.x());
				if(abs(xc) > abs(dstPt.x() - srcPt.x()))
				{
					xc = dstPt.x() - srcPt.x();
				}
				else if(xc < .01)
				{
					xc = 0;
				}

				QPointF p(xc, (dstPt.y() - srcPt.y())/2);
				this->handle->overridePos(p);

				this->setPos(mapToParent(mapFromScene(srcPt)));
				if(std::abs(dstPt.y() - srcPt.y()) > 0.5)
				{
					path.lineTo(xc, 0);
					path.lineTo(xc, dstPt.y() - srcPt.y());
					path.lineTo(dstPt.x() - srcPt.x(), dstPt.y() - srcPt.y());
				}
				else
				{
					path.lineTo(dstPt.x() - srcPt.x(), 0);
				}
				this->setPath(path);
				qreal w = dstPt.x() - srcPt.x();
				qreal h = dstPt.y() - srcPt.y();

				if(leftHatchItem)
				{
					rightHatchItem->setVisible(true);


					qreal x1 = xc / 2 - 5;
					qreal x2 = xc + (w - xc) / 2 - 5;
					qreal y1 = -5;
					qreal y2 = h - 5;
					if(std::abs(x1) >= 10)
					{
						leftHatchItem->setVisible(true);
						leftHatchItem->setPos(x1, y1);
					}
					else
					{
						leftHatchItem->setVisible(false);
					}

					if(std::abs(x2 - w) >= 10)
					{
						rightHatchItem->setVisible(true);
						rightHatchItem->setPos(x2, y2);
					}
					else
					{
						rightHatchItem->setVisible(false);
					}
				}

				if(leftArrowhead)
				{
					leftArrowhead->setRotation(0);
				}

				if(rightArrowhead)
				{
					rightArrowhead->setRotation(180);
					rightArrowhead->setPos(w, h);
				}
			}

			item->updateBoundingBox();

		}
	}

};

//template <typename A>
//using Accessor = ConnectionGraphicsItem



class ConnectorPropertyWidget: public BasicPropertyWidget
{
	Q_OBJECT
	Synchronizer *_sync;
	QButtonGroup *_lineTypeGroup;
	QButtonGroup *_arrowheadButtonGroup;
	QSlider *_lineThicknessSlider;
	QSpinBox *_busWidthSpinBox;
	QList<ConnectionGraphicsItem *> _items;
	QCheckBox *_leftArrowCheckBox, *_rightArrowCheckBox;
	ColorButton *_strokeColorButton;
	PropertyBinder *_pb;
public:
	ConnectorPropertyWidget(QWidget *parent=nullptr)
	: BasicPropertyWidget(parent)
	, _sync(new Synchronizer(this))
	, _pb(new PropertyBinder(this))
	{
		_sync->whenUpdated(this,
		                   SLOT(updateItem()),
		                   SLOT(updateControls()));

		auto section = addSection(tr("Connector"));

		auto layout = new QFormLayout(section);

		{
			auto optGroup = new QButtonGroup(section);
			_lineTypeGroup = optGroup;
			optGroup->setExclusive(true);

			auto lineButton = new QRadioButton(tr("Line"), section);
			auto polylineButton = new QRadioButton(tr("Polyline"), section);

			_pb->bind(lineButton, "connectorType", "line");
			_pb->bind(polylineButton, "connectorType", "polyline");

			optGroup->addButton(lineButton, ConnectionGraphicsItem::LineMode);
			optGroup->addButton(polylineButton, ConnectionGraphicsItem::PolylineMode);

			connect(optGroup, SIGNAL(buttonClicked(int)),
			        _sync,    SLOT(viewUpdate()));

			auto subLayout = new QHBoxLayout;
			subLayout->addWidget(lineButton);
			subLayout->addWidget(polylineButton);
			layout->addRow(tr("Line Type"), subLayout);
		}

		{
			auto g = _arrowheadButtonGroup = new QButtonGroup(section);
			g->setExclusive(false);

			auto leftArrowCheckbox = new QCheckBox(tr("Start"), section);
			auto rightArrowCheckbox = new QCheckBox(tr("End"), section);

			_leftArrowCheckBox = leftArrowCheckbox;
			_rightArrowCheckBox = rightArrowCheckbox;

			g->addButton(leftArrowCheckbox, ConnectionGraphicsItem::LeftArrowhead);
			g->addButton(rightArrowCheckbox, ConnectionGraphicsItem::RightArrowhead);

			_pb->bind(_leftArrowCheckBox, "leftArrow");
			_pb->bind(_rightArrowCheckBox, "rightArrow");

//			for(auto c : {leftArrowCheckbox, rightArrowCheckbox}) c->setTristate();


//			connect(g, SIGNAL(buttonClicked(int)), _sync, SLOT(viewUpdate()));
			connect(leftArrowCheckbox, SIGNAL(clicked()), _sync, SLOT(viewUpdate()));
			connect(rightArrowCheckbox, SIGNAL(clicked()), _sync, SLOT(viewUpdate()));



			auto subLayout = new QHBoxLayout;
			subLayout->addWidget(leftArrowCheckbox);
			subLayout->addWidget(rightArrowCheckbox);
			layout->addRow(tr("Arrowheads"), subLayout);
		}

		{
			auto indicator = new QLabel(section);

			_lineThicknessSlider = new QSlider(Qt::Horizontal, section);
			_lineThicknessSlider->setRange(0, 10);
			_pb->bind(_lineThicknessSlider, "lineThickness");

			connect(_lineThicknessSlider, SIGNAL(valueChanged(int)),
					_sync,                SLOT(viewUpdate()));

			connect(_lineThicknessSlider, SIGNAL(valueChanged(int)),
					indicator,            SLOT(setNum(int)));

			auto subLayout = new QHBoxLayout;
			subLayout->addWidget(_lineThicknessSlider);
			subLayout->addWidget(indicator);

			layout->addRow(tr("Line Thickness"), subLayout);
		}

		_busWidthSpinBox = new QSpinBox(section);
		_busWidthSpinBox->setRange(0, std::numeric_limits<int>::max());
		_pb->bind(_busWidthSpinBox, "busWidth");

		connect(_busWidthSpinBox, SIGNAL(valueChanged(int)),
				_sync,            SLOT(viewUpdate()));

		layout->addRow(tr("Bus Width"), _busWidthSpinBox);
		_strokeColorButton = new ColorButton(this);
		_pb->bind(_strokeColorButton, "color");
		layout->addRow(tr("Stroke Color"), _strokeColorButton);
	}

private:

	void doCommand(QUndoCommand *cmd)
	{
		if(!_items.empty())
		{
			_items.front()->item->scene()->undoStack().push(cmd);
		}
	}

	int lineThickness()
	{
//		return _item->item->model()->getData<int>(LineThicknessKey).get_value_or(1);
		return 0;
	}

	void setLineThickness(int thickness)
	{
//		int oldLineThickness = lineThickness();
//		QPointer<ConnectionGraphicsItem> item = _item;
//		QPointer<ConnectorPropertyWidget> self = this;
//
//		if(oldLineThickness == thickness) return;
//
//		auto action = [item, self](int t) {
//			if(!item) return;
//			item->item->model()->setData(LineThicknessKey, t);
//			item->updateLineType();
//			if(self) self->_sync->modelUpdate();
//		};
//
//		auto cmd = new FunctionUndoCommand(
//			[=]() {
//				action(thickness);
//			},
//			[=]() {
//				action(oldLineThickness);
//			}
//		);
//
//		doCommand(cmd);
	}

	int busWidth()
	{
//		return _item->item->model()->getData<int>(BusWidthKey).get_value_or(1);
		return 0;
	}

	void setBusWidth(int width)
	{
//		int oldWidth = busWidth();
//		QPointer<ConnectionGraphicsItem> item = _item;
//		QPointer<ConnectorPropertyWidget> self = this;
//
//		if(oldWidth == width) return;
//
//		auto action = [item, self](int w) {
//			if(!item) return;
//			item->item->model()->setData(BusWidthKey, w);
//			item->updateLineType();
//			if(self) self->_sync->modelUpdate();
//		};
//
//		auto cmd = new FunctionUndoCommand(
//			[=]() {
//				action(width);
//			},
//			[=]() {
//				action(oldWidth);
//			}
//		);
//
//		doCommand(cmd);
	}

	ConnectionGraphicsItem::Mode lineMode()
	{
//		return _item->mode;
		return {};
	}

	void setLineMode(ConnectionGraphicsItem::Mode mode)
	{
//		auto oldMode = lineMode();
//		QPointer<ConnectionGraphicsItem> item = _item;
//		QPointer<ConnectorPropertyWidget> self = this;
//
//		if(oldMode == mode) return;
//
//		auto action = [item, self](ConnectionGraphicsItem::Mode m) {
//			if(!item) return;
//			item->setMode(m);
//			if(self) self->_sync->modelUpdate();
//		};
//
//		auto cmd = new FunctionUndoCommand([=]() { action(mode); },
//		                                   [=]() { action(oldMode); });
//
//		doCommand(cmd);
	}

//	template <typename Accessor>
//	auto property(Accessor accessor) -> boost::optional<decltype((std::declval<ConnectionGraphicsItem>().*accessor)())>

//	template <typename Accessor>
//	auto property(Accessor accessor) -> boost::optional<decltype(accessor(std::declval<ConnectionGraphicsItem>()))>
//	{
//		boost::optional<decltype(accessor(std::declval<ConnectionGraphicsItem>()))> result;
//
//		for(auto item : _items)
//		{
//			if(!result)
//			{
//				result = accessor(*item);
//			}
//			else if(*result != accessor(*item))
//			{
//				result = boost::none;
//				break;
//			}
//		}
//
//		return result;
//	}

	boost::optional<int> arrowheads()
	{
//		return property(&ConnectionGraphicsItem::arrowhead);
		return {};
	}



	template <typename G, typename S, typename T>
	void setProperty(G get, S set, T value)
	{
		QPointer<ConnectorPropertyWidget> self = this;

		auto rootCmd = new QUndoCommand;

		for(auto item : _items)
		{
			auto oldValue = get(*item);
			QPointer<ConnectionGraphicsItem> itemp = item;

			auto action = [itemp, self, get, set](T v) {
				if(!itemp) return;
				set(*itemp, v);
				if(self) self->_sync->modelUpdate();
			};


			new FunctionUndoCommand([=]() { action(value); },
			                        [=]() { action(oldValue); },
			                        rootCmd);
		}

		doCommand(rootCmd);
	}

	void setArrowheads(int arrowheads)
	{

		setProperty(std::mem_fn(&ConnectionGraphicsItem::arrowhead),
		            [](ConnectionGraphicsItem &item, int a) { item.setArrowhead(a); },
		            arrowheads);
//		int oldArrowheads = this->arrowheads();
//		QPointer<ConnectionGraphicsItem> item = _item;
//		QPointer<ConnectorPropertyWidget> self = this;
//		auto action = [item, self](int a) {
//			if(!item) return;
//			item->setArrowhead(a);
//			if(self) self->_sync->modelUpdate();
//		};
//
//		auto cmd = new FunctionUndoCommand([=]() { action(arrowheads); },
//		                                   [=]() { action(oldArrowheads); });
//
//		doCommand(cmd);
	}
private slots:

	void updateItem()
	{
//		if(_item)
		{
//			if(_sync->syncSender() == _lineTypeGroup)
//			{
//				setLineMode(ConnectionGraphicsItem::Mode(_lineTypeGroup->checkedId()));
//			}
//			else if(_sync->syncSender() == _lineThicknessSlider)
//			{
//				setLineThickness(_lineThicknessSlider->value());
//			}
//			else if(_sync->syncSender() == _busWidthSpinBox)
//			{
//				setBusWidth(_busWidthSpinBox->value());
//			}
//			else if(_sync->syncSender() == _arrowheadButtonGroup)
//			{
//				int checked = ConnectionGraphicsItem::NoArrowhead;
//
//
//				for(auto button : _arrowheadButtonGroup->buttons())
//				{
//					if(button->isChecked())
//						checked |= _arrowheadButtonGroup->id(button);
//				}
//
//				setArrowheads(checked);
//
//			}
		}
	}

	void updateControls()
	{
//		if(_item)
//		for(auto item : items())
//		{
//
//			_lineTypeGroup->button(lineMode())->setChecked(true);
//			_lineThicknessSlider->setValue(lineThickness());
//			_busWidthSpinBox->setValue(busWidth());
//
////			if(auto ah = arrowheads())
////			{
////				for(auto button : _arrowheadButtonGroup->buttons())
////				{
////					button->setChecked(*ah & _arrowheadButtonGroup->id(button));
////				}
////			}
////			else
////			{
////				for(auto button : _arrowheadButtonGroup->buttons())
////				{
////					static_cast<QCheckBox *>(button)->setCheckState(Qt::PartiallyChecked);
////				}
////			}
//		}
	}
protected:
	virtual void itemsChanged()
	{
		BasicPropertyWidget::itemsChanged();
		_items.clear();
		QObjectList objects;
		for(auto item : items())
		{
			if(auto ext = item->extensionOrNull<ConnectionGraphicsItem>())
			{
				_items << ext;
				objects << ext;
			}
		}


		if(!_items.empty())
		{
			_pb->setUndoStack(&_items.front()->item->scene()->undoStack());
		}

		_pb->setObjects(objects);
		auto items = this->items();
		_sync->modelUpdate();
	}
};


ConnectorComponent::ConnectorComponent(QObject* parent)
: DiagramComponent("connector", parent)
{
	setHidden(true);
}

void ConnectorComponent::configure(DiagramItem *item) const
{



	item->setFlag(QGraphicsItem::ItemIsSelectable);
	item->setAcceptedMouseButtons(Qt::NoButton);
	item->setAcceptsHoverEvents(false);
	item->setZValue(100);
	item->setBoundingRegionGranularity(0.25);
	item->setItemSnaps(false);
	auto cgi = new ConnectionGraphicsItem(item, item);
	cgi->setParentItem(item);
}

ConnectorComponent::~ConnectorComponent()
{
}

bool ConnectorComponent::selectHandle(QGraphicsItem *item)
{
	if(auto handle = dynamic_cast<HandleItem *>(item))
	{
		if(auto parent = handle->parentItem())
		{
			if(auto grandparent = parent->parentItem())
			{
				if(auto digp = dynamic_cast<DiagramItem *>(grandparent))
				{
					digp->explicitlySelect();
					return true;
				}
			}
		}
	}

	return false;
}


PropertyWidget* ConnectorComponent::makePropertyWidget(QWidget* parent) const
{
	return new ConnectorPropertyWidget(parent);
}
}  // namespace dbuilder

#include "ConnectorComponent.moc"
