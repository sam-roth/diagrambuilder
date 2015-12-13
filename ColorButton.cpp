/**
 * @author "Sam Roth <s>"
 * @date   Sep 25, 2013
 */

#include "ColorButton.hpp"
#include "Util/ColorSwatch.hpp"

namespace dbuilder {

ColorButton::ColorButton(QWidget *parent)
: QToolButton(parent)
{
	connect(this, SIGNAL(clicked()), SLOT(onClicked()));
}

ColorButton::~ColorButton()
{

}

QColor ColorButton::color()
{
	return _color;
}


void ColorButton::setColor(QColor color)
{
	_color = color;
	setIcon(iconForColor(_color));
	emit colorChanged(_color);
}

void ColorButton::onClicked()
{
	auto color = QColorDialog::getColor(_color, this);
	if(color.isValid())
	{
		setColor(color);
	}
}

} /* namespace dbuilder */
