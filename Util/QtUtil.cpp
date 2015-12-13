/**
 * @file   QtUtil.cpp
 *
 * @date   Jan 30, 2013
 * @author Sam Roth <>
 */

#include "QtUtil.hpp"
#include <QAction>
namespace dbuilder {

QString readFile(const QString &filename)
{
	QFile f(filename);
	f.open(QFile::ReadOnly);
	if(f.isOpen())
	{
		return f.readAll();
	}
	else
	{
		return {};
	}
}

}  // namespace dbuilder

