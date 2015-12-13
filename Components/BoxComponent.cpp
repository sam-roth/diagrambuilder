/**
 * @file   BoxComponent.cpp
 *
 * @date   Mar 7, 2013
 * @author Sam Roth <>
 */

#include <cmath>
#include "BoxComponent.hpp"
#include "qgraphicsitem.h"
#include <QObject>
#include "Components/Util/HandleItem.hpp"
#include "DiagramItem.hpp"
#include "DiagramItemModel.hpp"
#include "Util/Log.hpp"
#include <QSpinBox>
#include <QWidgetAction>
#include <QLabel>
#include <QTextDocument>
#include <QTextCursor>
#include <memory>
#include "TabFocus/TabFocusRing.hpp"
#include "TabFocus/TabFocusableTextItem.hpp"
#include "BasicPropertyWidget.hpp"
#include <QtGui>
#include "Util/ScopeExit.hpp"
#include "Util/Synchronizer.hpp"
#include "Commands/FunctionUndoCommand.hpp"
#include "DiagramScene.hpp"
#include "PropertyBinder.hpp"
namespace dbuilder {

class BoxKindItem: public QObject, public QGraphicsRectItem
{
	Q_OBJECT
	friend class BoxPropertyWidget;
	DiagramItem *item;
	HandleItem *handle;

	static void enableTextEditor(bool enabled, QGraphicsTextItem *item)
	{
		if(enabled)
		{
			item->setTextInteractionFlags(Qt::TextEditorInteraction);
		}
		else
		{
			auto tc = item->textCursor();
			tc.movePosition(QTextCursor::StartOfLine);
			item->setTextCursor(tc);
			item->setTextInteractionFlags(Qt::NoTextInteraction);
		}
	}

	struct UserPort
	{
		enum Side {
			Left, Right
		};

		qreal y;
		Side s;
		std::shared_ptr<TabFocusableTextItem> label;

		UserPort(BoxKindItem *that, qreal y, Side s)
		: y(y), s(s), label(new TabFocusableTextItem(that))
		{
			QFont helv("Helvetica");
			label->setFocusRing(that->tabFocusRing);
			label->setFont(helv);
			label->setTextInteractionFlags(Qt::NoTextInteraction);
			label->setPlainText("[port]");
		}

		~UserPort()
		{
		}
	};
	QSize minimumSize;
	QList<UserPort> ports;
	TabFocusableTextItem *label;
	TabFocusRing *tabFocusRing;

	Q_PROPERTY(int leftPortCount READ leftPortCount WRITE setLeftPortCount)
	Q_PROPERTY(int rightPortCount READ rightPortCount WRITE setRightPortCount)

public:
	BoxKindItem(DiagramItem *item)
	: QGraphicsRectItem(item)
	, item(item)
	, handle(new HandleItem())
	, minimumSize(50, 50)
	, label(new TabFocusableTextItem(this))
	, tabFocusRing(new SequentialTabFocusRing(this))
	{
		item->addExtension(this);

		label->setFocusRing(tabFocusRing);
		QFont helvb("Helvetica");
		helvb.setWeight(QFont::Bold);
		label->setFont(helvb);
		label->setTextInteractionFlags(Qt::NoTextInteraction);

		label->document()->setDefaultTextOption(QTextOption(Qt::AlignCenter));

		item->setPositionByCenter(false);

		handle->setParentItem(this);
		handle->setOrientation(Qt::Horizontal | Qt::Vertical);
		handle->setSelectsParent(false);
		handle->setLocksParent(true);
		auto p = pen();
		p.setWidth(1);
		this->setPen(p);
		connect(item->model(), SIGNAL(updateViewRequested()), this, SLOT(updateView()));
		connect(item->model(), SIGNAL(updateModelRequested()), this, SLOT(updateModel()));
		connect(handle, SIGNAL(handleMoved(QPointF)), this, SLOT(handleMoved(QPointF)));
		connect(handle, SIGNAL(handleWillMove(QPointF&)), this, SLOT(handleWillMove(QPointF&)));
		connect(item, SIGNAL(selectionChanged(bool)), this, SLOT(selectionChanged(bool)));
		connect(item, SIGNAL(doubleClicked(QPointF)), this, SLOT(doubleClicked(QPointF)));

		connect(label->document(), SIGNAL(contentsChanged()), this, SLOT(updateLabelPos()));

		connect(item, SIGNAL(printModeChanged(bool)), handle, SLOT(setPrintMode(bool)));
	}
private:

	void updatePorts()
	{
		item->clearPorts();
		qreal nextMinimumHeight = 50;

		for(const auto &port : ports)
		{
			QPointF point, textOrigin;
			port.label->setTextWidth(this->rect().width()/2-5);
			if(port.s == UserPort::Left)
			{
				point = {0.0, port.y};
				textOrigin = {0.0, port.y-10};
			}
			else
			{
				point = {this->rect().width(), port.y};
				textOrigin = {this->rect().width()/2+5, port.y-10};
				QTextBlockFormat format;
				format.setAlignment(Qt::AlignRight);
				QTextCursor cursor = port.label->textCursor();
				cursor.select(QTextCursor::Document);
				cursor.mergeBlockFormat(format);
				cursor.clearSelection();
				port.label->setTextCursor(cursor);
			}

			item->addPort(point);
			port.label->setPos(textOrigin);

			nextMinimumHeight = std::max(nextMinimumHeight, port.y + 20);
		}

		minimumSize.setHeight(nextMinimumHeight);
		if(this->rect().height() < minimumSize.height())
		{
			this->setRect(0, 0, this->rect().width(), minimumSize.height());
		}
		this->handle->overridePos(this->rect().bottomRight());

		updateLabelPos();

		item->updateBoundingBox();
	}


	int leftPortCount() const
	{
		return portCount(UserPort::Left);
	}

	int rightPortCount() const
	{
		return portCount(UserPort::Right);
	}

	void setLeftPortCount(int c)
	{
		setPortCount(UserPort::Left, c);
	}

	void setRightPortCount(int c)
	{
		setPortCount(UserPort::Right, c);
	}

	int portCount(UserPort::Side s) const
	{
		int total = 0;
		for(const auto &port : ports)
		{
			if(port.s == s)
			{
				total++;
			}
		}

		return total;
	}

	void setPortCount(const UserPort::Side s, const int newCount)
	{
		assert(newCount >= 0);

		int count = portCount(s);

		assert(count >= 0);

		while(count < newCount)
		{
			addPort(s);
			count++;
		}

		while(count > newCount)
		{
			removePort(s);
			count--;
		}
	}
private slots:

	void selectionChanged(bool s)
	{
		if(!s)
		{
			enableTextEditor(false, label);
			for(const auto &port : ports) enableTextEditor(false, port.label.get());
		}
	}

	void doubleClicked(QPointF p)
	{
			enableTextEditor(true, label);
			for(const auto &port : ports) enableTextEditor(true, port.label.get());
	}

	void updateLabelPos()
	{
		this->label->setTextWidth(this->rect().width());
		this->label->setPos(this->rect().center() - this->label->boundingRect().center());
	}

	void addPort(UserPort::Side s)
	{
		qreal y = 20;
		for(int i = ports.size() - 1; i >= 0; i--)
		{
			if(ports[i].s == s)
			{
				y = ports[i].y + 20;
				break;
			}
		}

		ports << UserPort{this, y, s};
		updatePorts();
	}

	void removePort(UserPort::Side s)
	{
		for(int i = ports.size() - 1; i >= 0; i--)
		{
			if(ports[i].s == s)
			{
				ports.removeAt(i);
				break;
			}
		}

		updatePorts();
	}

	void addLeftPort()
	{
		addPort(UserPort::Left);
	}

	void addRightPort()
	{
		addPort(UserPort::Right);
	}

	void removeLeftPort()
	{
		removePort(UserPort::Left);
	}

	void removeRightPort()
	{
		removePort(UserPort::Right);
	}

	void handleMoved(QPointF p)
	{
		this->setRect(0, 0, p.x(), p.y());
		item->updateBoundingBox();
		updatePorts();
	}

	void updateView()
	{
		auto labelText = item->model()->getData<std::string>("box.label").get_value_or("Box");
		this->label->setPlainText(labelText.c_str());
		auto width = item->model()->getData<double>("box.width").get_value_or(100);
		auto height = item->model()->getData<double>("box.height").get_value_or(100);
		this->setRect(0, 0, width, height);
		this->handle->overridePos(QPointF(width, height));
		item->updateBoundingBox();
		this->ports.clear();
		auto portsTree = item->model()->getTree("ports");
		if(portsTree)
		{
			for(const pt::ptree::value_type &pv : *portsTree)
			{
				UserPort p(this, 0, UserPort::Left);

				p.s = UserPort::Side(pv.second.get<int>("s"));
				p.y = pv.second.get<double>("y");
				p.label->setPlainText(pv.second.get_optional<std::string>("label").get_value_or("[port]").c_str());

				ports << p;
			}

		}
		updatePorts();
	}

	void updateModel()
	{
		item->model()->setData("box.label", this->label->toPlainText().toStdString());
		item->model()->setData("box.width", this->rect().width());
		item->model()->setData("box.height", this->rect().height());

		pt::ptree portsTree;
		for(const auto &port : ports)
		{
			pt::ptree portTree;
			portTree.put("s", int(port.s));
			portTree.put("y", port.y);
			portTree.put("label", port.label->toPlainText().toStdString());
			portsTree.add_child("port", portTree);
		}

		item->model()->setTree("ports", portsTree);
	}

	void handleWillMove(QPointF &p)
	{
		// apply constraints to handle movement

		if(p.x() < 0) p.setX(0);
		if(p.y() < 0) p.setY(0);

		if(p.x() < minimumSize.width()) p.setX(minimumSize.width());
		if(p.y() < minimumSize.height()) p.setY(minimumSize.height());

		// snap
		p.setX(std::round(p.x()/5.0) * 5.0);
		p.setY(std::round(p.y()/5.0) * 5.0);
	}

};

class BoxPropertyWidget: public BasicPropertyWidget
{
	Q_OBJECT
	std::array<QSpinBox *, 2> _portCountSpinners;
	PropertyBinder _pb;
public:
	BoxPropertyWidget(QWidget *parent=nullptr)
	: BasicPropertyWidget(parent)
	{

		auto section = addSection(tr("Ports"));
		auto layout = new QFormLayout(section);

		for(QSpinBox *&spinner : _portCountSpinners)
		{
			spinner = new QSpinBox(section);
			spinner->setMinimum(0);
			spinner->setMaximum(std::numeric_limits<int>::max());
		}
		_pb.bind(_portCountSpinners[0], "leftPortCount");
		_pb.bind(_portCountSpinners[1], "rightPortCount");


		layout->addRow(tr("Left"), _portCountSpinners[0]);
		layout->addRow(tr("Right"), _portCountSpinners[1]);
	}
protected:

	virtual void itemsChanged()
	{
		BasicPropertyWidget::itemsChanged();

		QObjectList objs;

		auto items = this->items();

		for(auto item : items)
			if(auto ext = item->extensionOrNull<BoxKindItem>())
				objs << ext;

		if(!items.empty())
			_pb.setUndoStack(&items.front()->scene()->undoStack());

		_pb.setObjects(objs);
	}
};

BoxComponent::BoxComponent(QObject* parent)
: DiagramComponent("box", parent)
{
}


void BoxComponent::configure(DiagramItem *item) const
{
	new BoxKindItem(item);
	item->setFlag(QGraphicsItem::ItemIsSelectable);
	item->setFlag(QGraphicsItem::ItemIsMovable);
}

BoxComponent::~BoxComponent()
{
}

PropertyWidget *BoxComponent::makePropertyWidget(QWidget *parent) const
{
	return new BoxPropertyWidget(parent);
}

} /* namespace dbuilder */

#include "BoxComponent.moc"
