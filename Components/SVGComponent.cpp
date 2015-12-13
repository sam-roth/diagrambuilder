/**
 * @file   SVGComponent.cpp
 *
 * @date   Jan 3, 2013
 * @author Sam Roth <>
 */

#include "SVGComponent.hpp"
#include <QGraphicsSvgItem>
#include "DiagramItem.hpp"
#include <QTemporaryFile>
#include <QTextStream>
#include "Util/FixedWidthStroke.hpp"
#include <QPainter>
#include <QSvgRenderer>
namespace dbuilder {
namespace {

QIcon iconFromSVG(QSvgRenderer &renderer)
{
	QImage img(QSize(32, 32), QImage::Format_ARGB32);
	img.fill(QColor(0, 0, 0, 0));

	QPixmap pixmap = QPixmap::fromImage(img);
	QPainter painter(&pixmap);


	auto viewBox = renderer.viewBoxF();
	auto aspect = viewBox.width() / viewBox.height();
	QSizeF drawSize;
	if(aspect > 1.0)
	{
		drawSize = QSizeF(32, 32.0 / aspect);
	}
	else
	{
		drawSize = QSizeF(32.0 * aspect, 32.0);
	}

	QRectF drawArea({(32.0-drawSize.width())/2.0, (32.0-drawSize.height())/2.0}, drawSize);

	renderer.render(&painter, drawArea);

	return QIcon(pixmap);

}

}  // anonymous namespace

SVGComponent::SVGComponent(QString kindName, QString filename, QList<QPointF> ports, QObject* parent)
: DiagramComponent(kindName, parent)
, _filename(filename)
, _ports(ports)
{
	auto document = makeFixedWidthStroke(this->filename());
	QSvgRenderer renderer(document.toAscii());
	_icon = iconFromSVG(renderer);
	_sharedRenderer = new QSvgRenderer(_filename, this);
}

SVGComponent::SVGComponent(QString kindName,
                 const SVGData &svgData,
                 const QList<QPointF> &ports,
                 QObject *parent)
: DiagramComponent(kindName)
, _ports(ports)
, _svgData(svgData.data)
{
	auto document = makeFixedWidthStrokeWithData(_svgData);
	QSvgRenderer renderer(document.toAscii());
	_icon = iconFromSVG(renderer);
	_sharedRenderer = new QSvgRenderer(_svgData.toUtf8(), this);
}

QIcon SVGComponent::icon() const
{
	return _icon;
}

void SVGComponent::configure(DiagramItem *item) const
{
	item->setFlag(QGraphicsItem::ItemIsSelectable);
	item->setFlag(QGraphicsItem::ItemIsMovable);
	auto svgItem = new QGraphicsSvgItem(item);
	svgItem->setSharedRenderer(_sharedRenderer);

	for(auto port : _ports)
	{
		item->addPort(port);
	}
}

SVGComponent::~SVGComponent()
{
}


} // namespace dbuilder

