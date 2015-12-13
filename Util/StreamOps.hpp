#pragma once
/**
 * @file   StreamOps.hh
 *
 * @date   Jan 16, 2013
 * @author Sam Roth <>
 */




#include <QPointF>
#include <iostream>
#include <QString>
#include <QUuid>

namespace dbuilder {



inline std::ostream &operator <<(std::ostream &os, const QPointF &obj)
{
	return os << "QPointF{" << obj.x() << ", " << obj.y() << "}";
}

inline std::ostream &operator <<(std::ostream &os, const QString &x)
{
	return os << x.toStdString();
}

inline std::ostream &operator <<(std::ostream &os, const QUuid &x)
{
	return os << x.toString();
}

}  // namespace dbuilder
