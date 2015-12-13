#pragma once
#include <QMap>
#include <QWidget>
/**
 * @file   Toolbox.hpp
 *
 * @date   Mar 14, 2013
 * @author Sam Roth <>
 */

namespace dbuilder
{
class ToolboxData;

class Toolbox: public QWidget
{
	Q_OBJECT
	ToolboxData *d;
	typedef QWidget Super;
	friend class ToolboxData;
public:
	Toolbox(QWidget *parent=nullptr);
	virtual ~Toolbox();
protected:
	virtual bool event(QEvent *);
};

} /* namespace dbuilder */
