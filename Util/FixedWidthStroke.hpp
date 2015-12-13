#pragma once
/**
 * @file   FixedWidthStroke.hh
 *
 * @date   Jan 10, 2013
 * @author Sam Roth <>
 */


#include <QDomDocument>
#include <QFile>
#include <QString>


namespace dbuilder {
inline QString makeFixedWidthStroke(QString filename)
{
	QFile file(filename);
	QDomDocument doc;
	doc.setContent(&file);
	doc.documentElement().setAttribute("vector-effect", "non-scaling-stroke");
	return doc.toString();
}

inline QString makeFixedWidthStrokeWithData(const QString &data)
{
	QDomDocument doc;
	doc.setContent(data);
	doc.documentElement().setAttribute("vector-effect", "non-scaling-stroke");
	return doc.toString();
}

} // namespace dbuilder
