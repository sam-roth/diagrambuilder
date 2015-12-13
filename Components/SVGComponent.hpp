#pragma once
/**
 * @file   SVGComponent.h
 *
 * @date   Jan 3, 2013
 * @author Sam Roth <>
 */

#include "DiagramComponent.hpp"
#include <QString>
#include <QPoint>
#include <QList>

class QSvgRenderer;

namespace dbuilder {

struct SVGData
{
	QString data;
};

class SVGComponent: public DiagramComponent
{
	Q_OBJECT
	QString _filename;
	QList<QPointF> _ports;
	QIcon _icon;
	QString _svgData;
	QSvgRenderer *_sharedRenderer;
public:
	SVGComponent(QString kindName, QString filename, QList<QPointF> ports, QObject *parent=nullptr);
	SVGComponent(QString kindName, const SVGData &svgData, const QList<QPointF> &ports, QObject *parent=nullptr);
	void configure(DiagramItem *) const;

	QIcon icon() const;

	const QString &filename() const { return _filename; }
	virtual ~SVGComponent();
};
} // namespace dbuilder

