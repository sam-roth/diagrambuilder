/**
 * @date   Jun 29, 2013
 * @author Sam Roth <>
 */

#include "BasicPropertyWidget.hpp"
#include <QtGui>
#include "DiagramItem.hpp"
#include "DiagramItemModel.hpp"
#include "DiagramComponent.hpp"
#include "ui_BasicPropertyWidget.h"
#include <QtCore>
#include "Util/Optional.hpp"
#include "Util/Log.hpp"
#include "Util/ScopeExit.hpp"
#include "DiagramScene.hpp"

static const char *sectionStylesheet = R"(
QPushButton {
	background-color: qlineargradient(spread:pad, x1:0, y1:0, x2:0, y2:1, stop:0 palette(light), stop:1 palette(dark));
	padding-top: 3px;
	padding-bottom: 3px;
	border-bottom: 2px groove palette(dark);
	border-top: 2px groove palette(dark);
	max-height: 20px;
	font-weight: bold;
	text-align: left;
}
)";

namespace dbuilder
{
BasicPropertyWidget::BasicPropertyWidget(QWidget *parent)
: PropertyWidget(parent)
, _layout(new QVBoxLayout(this))
, _ui(new Ui_BasicPropertyWidget)
{
	_layout->setContentsMargins(0, 0, 0, 0);
	_layout->setSpacing(0);
	_layout->addStretch();
	auto mainSec = addSection("Item");
	_ui->setupUi(mainSec);
	_kindNameLabel = _ui->itemKindLabel;

	typedef std::numeric_limits<double> DL;

	_ui->xSpinner->setRange(DL::lowest(), DL::max());
	_ui->ySpinner->setRange(DL::lowest(), DL::max());


	_pb.bind(_ui->xSpinner, "x");
	_pb.bind(_ui->ySpinner, "y");

}

BasicPropertyWidget::~BasicPropertyWidget()
{
	delete _ui;
}

QWidget *BasicPropertyWidget::addSection(const QString& title)
{
	auto label = new QPushButton(title, this);
	label->setIcon(QIcon(":/resources/addl-icons/disclosure-open.png"));
	label->setFlat(true);
	label->setCheckable(true);
	label->setChecked(true);
	label->setStyleSheet(sectionStylesheet);
	_layout->insertWidget(_layout->count() - 1, label);
	connect(label, SIGNAL(clicked()), this, SLOT(sectionHeaderClicked()));

	auto result = new QWidget(this);
	_layout->insertWidget(_layout->count() - 1, result);

	sectionForHeader[label] = result;

	return result;
}

bool BasicPropertyWidget::event(QEvent *event)
{
	// Macs use smaller widgets in tool windows; set the appropriate flags.

	if(event && event->type() == QEvent::Polish)
	{
		for(auto childWidget : this->findChildren<QWidget *>())
		{
			childWidget->setAttribute(Qt::WA_MacSmallSize, true);
		}
	}
	return PropertyWidget::event(event);
}

void BasicPropertyWidget::itemsChanged()
{
	PropertyWidget::itemsChanged();
	for(auto item : _items)
		if(item)
			disconnect(item, nullptr, this, nullptr);

	QObjectList objs;
	auto items = this->items();

	const DiagramComponent *kind = nullptr;
	bool multipleKinds = false;

	for(auto item : items)
	{
		_items << item;
		objs << item;
		if(!kind)
		{
			kind = item->model()->kind();
		}
		else if(item->model()->kind() != kind)
		{
			multipleKinds = true;
		}
		connect(item, SIGNAL(posChanged(QPointF)), &_pb, SLOT(update()));
	}

	if(!items.empty())
	{
		_pb.setUndoStack(&items.front()->scene()->undoStack());
	}
	_pb.setObjects(objs);


	if(!multipleKinds && kind)
	{
		_kindNameLabel->setText(kind->name());
	}
	else if(kind)
	{
		_kindNameLabel->setText(tr("[multiple selection]"));
	}
	else
	{
		_kindNameLabel->setText(tr("[no selection]"));
	}

}

void BasicPropertyWidget::sectionHeaderClicked()
{
	auto sdr = dynamic_cast<QPushButton *>(sender());
	if(!sdr)
	{
		return;
	}

	if(auto section = get(sectionForHeader, sdr))
	{
		if(!sdr->isChecked())
		{
			sdr->setIcon(QIcon(":/resources/addl-icons/disclosure-closed.png"));
			auto anim = new QPropertyAnimation(*section, "maximumHeight");
			anim->setDuration(100);
			anim->setStartValue((**section).height());
			anim->setEndValue(0);
			anim->start(QAbstractAnimation::DeleteWhenStopped);
			connect(anim, SIGNAL(finished()), *section, SLOT(hide()));
		}
		else
		{
			sdr->setIcon(QIcon(":/resources/addl-icons/disclosure-open.png"));
			(**section).setVisible(true);
			auto anim = new QPropertyAnimation(*section, "maximumHeight");
			anim->setDuration(100);
			anim->setKeyValueAt(0, 0);
			anim->setKeyValueAt(0.99, 500);
			anim->setKeyValueAt(1, QWIDGETSIZE_MAX);
			anim->start(QAbstractAnimation::DeleteWhenStopped);
		}
	}
}

} /* namespace dbuilder */
#include "BasicPropertyWidget.moc"
