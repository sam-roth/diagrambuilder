/**
 * @date   Jul 19, 2013
 * @author Sam Roth <>
 */

#include "ImageComponent.hpp"
#include "DiagramItem.hpp"
#include <QGraphicsSvgItem>
#include "Util/Log.hpp"
#include "BasicPropertyWidget.hpp"
#include "DiagramItemModel.hpp"
#include "DiagramScene.hpp"
#include <qundostack.h>
#include "Commands/FunctionUndoCommand.hpp"
#include "PropertyBinder.hpp"
#include <QtCore>
#include "Util/ListUtil.hpp"

namespace dbuilder
{

static const char *ImageSourceKey = "image.source";

class ImageGraphicsItem: public QObject, public QGraphicsRectItem
{
	Q_OBJECT
	Q_INTERFACES(QGraphicsItem)
	friend class ImagePropertyWidget;
	DiagramItem *_item;
	QGraphicsSvgItem *_svgItem;
	QGraphicsTextItem *_placeholder;
	QImage _image;
	Q_PROPERTY(QString imageSource READ imageSource WRITE setImageSource)
public:
	ImageGraphicsItem(DiagramItem *parent)
	: QGraphicsRectItem(parent)
	, _item(parent)
	, _svgItem(nullptr)
	, _placeholder(nullptr)
	{
		updateImage();

		QPen invisible;
		invisible.setColor(QColor(0, 0, 0, 0));

		_item->setPen(invisible);
		_item->setFlag(ItemIsSelectable);
		_item->setFlag(ItemIsMovable);
	}

	/**
	 * Set the filename of the image to display.
	 * If empty or invalid, a placeholder message will appear.
	 * @param filename the filename of the image
	 */
	void setImageSource(const QString &filename)
	{
		if(filename != imageSource())
		{
			_item->model()->setData(ImageSourceKey, filename.toStdString());
			updateImage();
		}
	}

	QString imageSource() const
	{
		return _item->model()->getData<std::string>(ImageSourceKey).get_value_or("").c_str();
	}
private:

	void updateImage()
	{
		auto filename = imageSource();
		clearImage();

		if(QFileInfo(filename).exists())
		{
			if(filename.endsWith(".svg"))
			{
				showSvg(filename);
			}
			else
			{
				showImage(filename);
			}
		}

		if(!_svgItem && _image.isNull())
		{
			_placeholder = new QGraphicsTextItem(this);
			_placeholder->setDefaultTextColor(Qt::red);
			_placeholder->setPlainText(tr("[no image selected]"));
			this->setRect(_placeholder->boundingRect());
		}

	}

	/**
	 * Reset to the initial state.
	 */
	void clearImage()
	{
		delete _svgItem;
		_svgItem = nullptr;

		_image = QImage();

		delete _placeholder;
		_placeholder = nullptr;
	}

	/**
	 * Switch to displaying an SVG.
	 * @param filename  the filename of the SVG file
	 */
	void showSvg(const QString &filename)
	{
		_svgItem = new QGraphicsSvgItem(filename, this);
		this->setRect(_svgItem->boundingRect());
		_item->updateBoundingBox();
	}

	/**
	 * Switch to displaying a raster image.
	 * @param filename the filename of the image
	 */
	void showImage(const QString &filename)
	{
		_image = QImage(filename);
		this->setRect(_image.rect());
		_item->updateBoundingBox();
	}

protected:

	void paint(QPainter *painter,
	           const QStyleOptionGraphicsItem *option,
	           QWidget *widget)
	{
		if(!_image.isNull())
		{
			// use item coordinate cache for images, since they're not scalable
			this->setCacheMode(ItemCoordinateCache);
			painter->drawImage(boundingRect(), _image);
		}
	}
};
class ImagePropertyWidget: public BasicPropertyWidget
{
	Q_OBJECT
	QList<ImageGraphicsItem *> _items;
//	ImageGraphicsItem *_item;
	QLineEdit *_imageSourceEdit;
	QToolButton *_browseButton;
	PropertyBinder *_pb;
public:
	ImagePropertyWidget(QWidget *parent=nullptr)
	: BasicPropertyWidget(parent)
	, _pb(new PropertyBinder(this))
	{
		auto section = addSection(tr("Image"));
		auto layout = new QFormLayout(section);

		auto row = new QHBoxLayout;
		_imageSourceEdit = new QLineEdit(this);
		_pb->bind(_imageSourceEdit, "imageSource");
		_browseButton = new QToolButton(this);
		_browseButton->setText("...");
		row->addWidget(_imageSourceEdit);
		row->addWidget(_browseButton);
		layout->addRow(tr("Image Source:"), row);

		connect(_imageSourceEdit, SIGNAL(editingFinished()),
				this,             SLOT(updateItem()));

		connect(_browseButton,    SIGNAL(clicked()),
				this,             SLOT(browseButtonClicked()));

	}
private slots:
	void browseButtonClicked()
	{
		auto filterText = tr("Images and Vector Graphics (*.svg *.xpm *.xbm *.png *.jpg *.jpeg)");
		auto filename = QFileDialog::getOpenFileName(this,
		                                             tr("Choose Image"),
		                                             QString(),
		                                             filterText);
		if(!filename.isNull())
		{
			_imageSourceEdit->setText(filename);
			_pb->update(_imageSourceEdit);
			updateItem();
		}
	}

	/**
	 * Update the DiagramItem with the selected settings
	 */
	void updateItem()
	{
		return;
		auto parentCommand = new QUndoCommand;

		for(auto item : _items)
		{
			auto oldFilename = item->imageSource();
			auto newFilename = _imageSourceEdit->text();
			QPointer<ImageGraphicsItem> pitem = item;
			QPointer<ImagePropertyWidget> self = this;


			auto action = [pitem, self](const QString &filename) {
				if(!pitem) return;
				pitem->setImageSource(filename);
				if(self)
				{
					self->updateControls();
				}

			};

			new FunctionUndoCommand(
				[newFilename, action]() { action(newFilename); },
				[oldFilename, action]() { action(oldFilename); },
				parentCommand
			);
		}

		doCommand(parentCommand);
	}

	/**
	 * Update the property widget with the current state of
	 * the diagram item.
	 */
	void updateControls()
	{
		return;
		QString source;
		bool multiple = false;
		for(auto item : _items)
		{
			if(source.isNull())
			{
				source = item->imageSource();
			}
			else if(source != item->imageSource())
			{
				multiple = true;
			}
		}

		if(multiple)
		{
			_imageSourceEdit->setPlaceholderText(tr("[multiple selection]"));
			_imageSourceEdit->setText({});
		}
		else
		{
			_imageSourceEdit->setPlaceholderText({});
			_imageSourceEdit->setText(source);
		}
	}
private:
	void doCommand(QUndoCommand *cmd)
	{
		if(!_items.empty())
		{
			_items.front()->_item->scene()->undoStack().push(cmd);
		}
	}
protected:
	virtual void itemsChanged()
	{
		BasicPropertyWidget::itemsChanged();
		_items.clear();
		for(auto item : items())
		{
			if(auto ext = item->extensionOrNull<ImageGraphicsItem>())
			{
				_items << ext;
			}
		}
		if(!_items.empty())
		{
			_pb->setUndoStack(&_items.front()->_item->scene()->undoStack());
		}
		_pb->setObjects(listCast<QObject *>(_items));
		updateControls();
	}
};

ImageComponent::ImageComponent(QObject* parent)
: DiagramComponent("Image", parent)
{
}

void ImageComponent::configure(DiagramItem *item) const
{
	item->addExtension(new ImageGraphicsItem(item));
}

PropertyWidget *ImageComponent::makePropertyWidget(QWidget *parent) const
{
	return new ImagePropertyWidget(parent);
}

ImageComponent::~ImageComponent()
{
}

PluginInfo ImageComponentPlugin::pluginInfo() const
{
	PluginInfo result;
	result.author = "Sam Roth <>";
	result.name = "ImageComponentPlugin";
	result.url = QUrl("https://bitbucket.org/saroth/diagramming");
	result.minorVersion = 0;
	result.majorVersion = 0;
	return result;
}

QList<DiagramComponent *> ImageComponentPlugin::createComponents(QObject *parent)
{
	return {new ImageComponent(parent)};
}



} /* namespace dbuilder */

Q_EXPORT_PLUGIN2(dbuilder_ImageComponentPlugin,
                 dbuilder::ImageComponentPlugin)

#include "ImageComponent.moc"
