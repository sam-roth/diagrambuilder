/**
 * @author "Sam Roth <s>"
 * @date   Sep 25, 2013
 */

#pragma once


namespace dbuilder {

inline QIcon iconForColor(QColor color)
{
	QPixmap pixmap(QSize(16, 16));
	pixmap.fill(color);
	return QIcon(pixmap);

}

}  // namespace dbuilder
