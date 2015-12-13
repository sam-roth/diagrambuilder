/**
 * @date   Jul 19, 2013
 * @author Sam Roth <>
 */

#include "ExportComponentOptions.hpp"
#include "ui_ExportComponentOptions.h"
#include "Util/Log.hpp"
namespace dbuilder
{


ExportComponentOptions::ExportComponentOptions(QWidget* parent)
: QDialog(parent)
, _ui(new Ui_ExportComponentOptions)
{
	_ui->setupUi(this);
	_ui->buttonBox->button(QDialogButtonBox::Save)->setDefault(true);

	auto icon = style()->standardIcon(QStyle::SP_MessageBoxWarning);
	_ui->warningIcon->setPixmap(icon.pixmap(QSize(16, 16)));
	_ui->warningText->setAttribute(Qt::WA_MacSmallSize);
}

ExportComponentOptions::~ExportComponentOptions()
{
	delete _ui;
}

ExportComponentOptions::Options ExportComponentOptions::options() const
{
	return Options {
		_ui->authorEdit->text(),
		_ui->componentEdit->text()
	};
}

boost::optional<ExportComponentOptions::Options> ExportComponentOptions::getOptions(QWidget* parent)
{
	ExportComponentOptions dialog(parent);
	dialog.setWindowFlags(Qt::Sheet);
	dialog.setWindowModality(Qt::WindowModal);

	if(dialog.exec())
	{
		return dialog.options();
	}
	else
	{
		return {};
	}
}

} /* namespace dbuilder */
