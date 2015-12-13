/**
 * @author "Sam Roth <s>"
 * @date   Sep 25, 2013
 */

#pragma once

namespace dbuilder {

class ColorButton: public QToolButton
{
	Q_OBJECT
	QColor _color;
	Q_PROPERTY(QColor color READ color WRITE setColor NOTIFY colorChanged)
public:
	ColorButton(QWidget *parent=0);

	virtual ~ColorButton();
	QColor color();
signals:
	void colorChanged(QColor);
public slots:
	void setColor(QColor);
private slots:
	void onClicked();
};

} /* namespace dbuilder */
