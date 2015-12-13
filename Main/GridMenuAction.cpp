/**
 * @file   GridMenuAction.cpp
 *
 * @date   Mar 10, 2013
 * @author Sam Roth <>
 */

#include "GridMenuAction.hpp"
#include <QtGui>
#include "ThirdParty/FlowLayout.hpp"
#include "Util/Log.hpp"
#include <QCloseEvent>

namespace dbuilder {

class GridMenuButton: public QToolButton
{
	Q_OBJECT
public:
	GridMenuButton(QWidget *parent)
	: QToolButton(parent)
	{
		setAttribute(Qt::WA_Hover);
	}

protected:
	bool event(QEvent *event)
	{
		if(event->type() == QEvent::HoverEnter)
		{
			emit hoverChanged(true);
		}
		else if(event->type() == QEvent::HoverLeave)
		{
			emit hoverChanged(false);
		}
		return QToolButton::event(event);
	}


signals:
	void hoverChanged(bool);
};

GridMenuAction::GridMenuAction(const QList<QAction*>& actions, QObject *parent)
: QWidgetAction(parent)
, palette(new QWidget)
, layout(new FlowLayout(palette, -1, 0, 0))
{
	palette->setLayout(layout);
	palette->setMinimumWidth(300);
	palette->setMaximumWidth(300);

	palette->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Expanding);
	for(auto action : actions)
	{
		auto button = new GridMenuButton(palette);
		button->setDefaultAction(action);
		button->setIconSize(QSize(24, 24));
		button->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
		button->setMinimumHeight(24);
		layout->addWidget(button);
		actionForButton[button] = action;
		connect(button, SIGNAL(clicked()), this, SIGNAL(closeMenu()));
		connect(button, SIGNAL(hoverChanged(bool)), this, SLOT(buttonHoverChanged(bool)));
	}

	palette->setStyleSheet(R"EOF(
QToolButton {
  border: none;
}

QToolButton:hover {
  background-color: palette(highlight);
}
)EOF");
	palette->setMaximumHeight(layout->minimumHeightForWidth(palette->minimumWidth()));
	this->setDefaultWidget(palette);

}

GridMenuAction::~GridMenuAction()
{
}

void GridMenuAction::buttonHoverChanged(bool enter)
{
	auto sdr = static_cast<GridMenuButton *>(sender());
	emit hoverChanged(actionForButton[sdr], enter);
}

} /* namespace dbuilder */


#include "GridMenuAction.moc"
