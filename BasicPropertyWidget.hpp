#pragma once
#include "PropertyWidget.hpp"
#include <QMap>
#include "Util/ReentrancyGuard.hpp"
#include "PropertyBinder.hpp"
/**
 * @date   Jun 29, 2013
 * @author Sam Roth <>
 */

class QVBoxLayout;
class QPropertyAnimation;
class Ui_BasicPropertyWidget;

namespace dbuilder
{

class BasicPropertyWidget: public PropertyWidget
{
	Q_OBJECT
	QVBoxLayout *_layout;
	QLabel *_kindNameLabel;
	Ui_BasicPropertyWidget *_ui;
	QMap<QWidget *, QWidget *> sectionForHeader;
	QList<QPointer<DiagramItem>> _items;
	PropertyBinder _pb;
public:
	BasicPropertyWidget(QWidget *parent=0);
	virtual ~BasicPropertyWidget();
protected:
	virtual bool event(QEvent *);
	QWidget *addSection(const QString &title);
	virtual void itemsChanged();
private slots:
	void sectionHeaderClicked();
};

} /* namespace dbuilder */
