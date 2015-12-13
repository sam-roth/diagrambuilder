/**
 * @file   TextComponent.cpp
 *
 * @date   Jan 5, 2013
 * @author Sam Roth <>
 */

#include "TextComponent.hpp"
#include <QGraphicsTextItem>
#include "DiagramItem.hpp"
#include "DiagramItemModel.hpp"
#include <iostream>
#include <QWidget>
#include "Util/Log.hpp"
#include <QGraphicsSceneMouseEvent>
#include <QTextDocument>
#include <QTextCursor>
#include "TabFocus/TabFocusableTextItem.hpp"
#include <QGraphicsScene>
#include "DiagramScene.hpp"
#include "TabFocus/TabFocusRing.hpp"
#include "Util/Printable.hpp"
#include "PropertyWidget.hpp"
#include <QtGui>
#include "Util/ReentrancyGuard.hpp"
#include "Util/ScopeExit.hpp"
#include "Util/FunctionSlot.hpp"
#include <QAction>
#include "BasicPropertyWidget.hpp"


namespace dbuilder {
class TextEditorItem: public TabFocusableTextItem
{
	Q_OBJECT
	DiagramItem *item;
	ReentrancyGuard _updateViewGuard;
	bool _disableUpdates;
public:
	TextEditorItem(DiagramItem *item)
	: TabFocusableTextItem(item)
	, item(item)
	, _disableUpdates(false)
	{
		this->setFont(QFont("Helvetica"));
		connect(item, SIGNAL(selectionChanged(bool)), this, SLOT(selectionChanged(bool)));
		connect(item, SIGNAL(doubleClicked(QPointF)), this, SLOT(doubleClicked(QPointF)));
		connect(this->document(), SIGNAL(contentsChanged()), this, SLOT(onDocumentContentsChanged()));
		connect(this->document(), SIGNAL(contentsChanged()), this, SIGNAL(documentContentsChanged()));
		connect(item, SIGNAL(addedToScene()), this, SLOT(addedToScene()));
		connect(item, SIGNAL(willBeRemovedFromScene()), this, SLOT(willBeRemovedFromScene()));
		connect(item->model(), SIGNAL(updateViewRequested()), this, SLOT(updateView()));
		connect(item->model(), SIGNAL(updateModelRequested()), this, SLOT(updateModel()));

		updateView();
	}


	virtual void makeFocused()
	{
		if(this->scene())
		{
			this->scene()->clearSelection();
			item->setSelected(true);
			doubleClicked({});
		}
	}
protected:
	QVariant itemChange(GraphicsItemChange change, const QVariant &value)
	{
		return QGraphicsTextItem::itemChange(change, value);
	}

	void mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event)
	{
		doubleClicked(event->scenePos());
		QGraphicsTextItem::mouseDoubleClickEvent(event);
	}
signals:
	void documentContentsChanged();
private slots:
	void addedToScene()
	{
		this->setFocusRing(item->scene()->focusRing());
	}
	void willBeRemovedFromScene()
	{
		this->setFocusRing(nullptr);
	}

	void doubleClicked(QPointF)
	{
		this->setTextInteractionFlags(Qt::TextEditorInteraction);

		this->setFocus(Qt::MouseFocusReason);
		this->setCacheMode(QGraphicsItem::NoCache);
		item->setCacheMode(QGraphicsItem::NoCache);

	}

	void selectionChanged(bool focused)
	{
		if(!focused)
		{
			auto tc = this->textCursor();
			tc.movePosition(QTextCursor::StartOfLine);
			this->setTextCursor(tc);
			this->setTextInteractionFlags(Qt::NoTextInteraction);
			this->setCacheMode(DeviceCoordinateCache);
			item->setCacheMode(DeviceCoordinateCache);
		}
	}

	void updateView()
	{
		if(_disableUpdates) return;

		if(item->model() && item->model()->text())
		{
			this->setHtml(*item->model()->text());
		}
		else
		{
			this->setPlainText("[enter text here]");
		}
	}

	void onDocumentContentsChanged()
	{
		item->updateBoundingBox();
	}

	void updateModel()
	{
		item->model()->setText(this->toHtml());
	}
};

class TextComponentPropertyWidget: public BasicPropertyWidget
{
	Q_OBJECT
	QTextEdit *_textEdit;
	DiagramItemModel *_model;
	QTextDocument *_origTextDoc;
	TextEditorItem *_item;
	bool _disableUpdates;
public:
	TextComponentPropertyWidget(QWidget *parent=nullptr)
	: BasicPropertyWidget(parent)
	, _textEdit(new QTextEdit(this))
	, _model(nullptr)
	, _origTextDoc(_textEdit->document())
	, _item(nullptr)
	, _disableUpdates(false)
	{
		auto section = addSection("Text");
		auto layout = new QVBoxLayout(section);
		layout->addWidget(_textEdit);
		_textEdit->setAcceptRichText(true);
	}


protected:
	virtual void itemsChanged()
	{
		BasicPropertyWidget::itemsChanged();

		if(_model)
		{
			disconnect(_model, nullptr, this, nullptr);
			_model = nullptr;
		}

		if(_item)
		{
			disconnect(_item->document(), nullptr, this, nullptr);
			_item = nullptr;
		}

		auto items = this->items();
		auto item = items.size() == 1? items.front() : nullptr;

		if(item)
		{
			_model = item->model();

			connect(_model, SIGNAL(updateViewRequested()), this, SLOT(updateView()));
			connect(_textEdit->document(), SIGNAL(contentsChanged()), this, SLOT(documentContentsChanged()));
			auto ext = item->extensionOrNull<TextEditorItem>();
			connect(ext->document(), SIGNAL(contentsChanged()), this, SLOT(updateView()));
			_item = ext;

			updateView();
		}
		else
		{
			_textEdit->setDocument(_origTextDoc);
		}
	}
private slots:
	void updateView()
	{
		if(_disableUpdates) return;

		if(_item)
		{
			_disableUpdates = true;
			_textEdit->setHtml(_item->toHtml());
			_disableUpdates = false;
		}
	}

	void documentContentsChanged()
	{
		if(_disableUpdates) return;

		if(_item)
		{
			_disableUpdates = true;
			_item->setHtml(_textEdit->toHtml());
			_disableUpdates = false;
		}
	}

};


TextComponent::TextComponent(QObject* parent)
: DiagramComponent("text", parent)
{
}

void TextComponent::configure(DiagramItem *item) const
{
	item->setFlag(QGraphicsItem::ItemIsMovable);
	item->setFlag(QGraphicsItem::ItemIsSelectable);
	auto textItem = new TextEditorItem(item);
	item->addExtension(textItem);
}

PropertyWidget* TextComponent::makePropertyWidget(QWidget* parent) const
{
	return new TextComponentPropertyWidget(parent);
}

TextComponent::~TextComponent()
{
}

PluginInfo TextComponentPlugin::pluginInfo() const
{
	PluginInfo result;
	result.author = "Sam Roth <>";
	result.name = "TextComponentPlugin";
	result.url = QUrl("https://bitbucket.org/saroth/diagramming");
	result.minorVersion = 0;
	result.majorVersion = 0;
	return result;
}

QList<DiagramComponent *> TextComponentPlugin::createComponents(QObject *parent)
{
	return {new TextComponent(parent)};
}

} // namespace dbuilder

Q_EXPORT_PLUGIN2(dbuilder_TextComponentPlugin,
                 dbuilder::TextComponentPlugin)

#include "TextComponent.moc"
