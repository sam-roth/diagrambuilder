/**
 * @file   PreferencesDialog.cpp
 *
 * @date   Mar 28, 2013
 * @author Sam Roth <>
 */

#include "PreferencesDialog.hpp"
#include "ui_Preferences.h"
#include "Util/Log.hpp"
#include <QtGui>
#include "Util/Optional.hpp"
#include <QtCore>
#include "Main/Application.hpp"
#include "Util/ColorSwatch.hpp"
namespace dbuilder
{

PreferencesDialog::PreferencesDialog(QWidget* parent)
: QDialog(parent)
, _ui(new Ui_PreferencesDialog)
, _colorDialog(new QColorDialog(this))
{
	_ui->setupUi(this);
	for(int i = 0; i < log::numberOfLevels(); ++i)
	{
		_ui->cmbLoggingLevel->addItem(log::levelName(log::Level(i)), i);
	}


	_ui->buttonBox->button(QDialogButtonBox::Ok)->setDefault(true);

	connect(this, SIGNAL(accepted()), this, SLOT(onAccepted()));
}

PreferencesDialog::~PreferencesDialog()
{
	delete _ui;
}

void PreferencesDialog::updateView()
{
	auto app = Application::instance();

	_ui->cmbLoggingLevel->setCurrentIndex(int(log::level()));

	setButtonColor(_ui->btnConnectionPointColor, app->portOutlineColor());
	_libraries = app->libraries();

	updateLibraryList();
}

void PreferencesDialog::updateModel()
{
	auto app = Application::instance();

	log::setLevel(log::Level(_ui->cmbLoggingLevel->currentIndex()));

	app->setPortOutlineColor(buttonColor(_ui->btnConnectionPointColor));
	app->setLibraries(_libraries);
	app->saveSettings();
}

void PreferencesDialog::showEvent(QShowEvent* event)
{
	updateView();
}

void PreferencesDialog::onAccepted()
{
	updateModel();
}

void PreferencesDialog::on_buttonBox_clicked(QAbstractButton *button)
{
	if(_ui->buttonBox->buttonRole(button) == QDialogButtonBox::ApplyRole)
	{
		updateModel();
	}
	else if(_ui->buttonBox->standardButton(button) == QDialogButtonBox::RestoreDefaults)
	{
		auto app = Application::instance();
		app->setPortOutlineColor(app->defaultPortOutlineColor());
		updateView();
	}
}

void PreferencesDialog::onColorButtonClicked()
{
	auto btn = static_cast<QToolButton*>(sender());
	_colorDialog->setCurrentColor(buttonColor(btn));
	if(_colorDialog->exec() == QDialog::Accepted)
	{
		setButtonColor(btn, _colorDialog->currentColor());
	}
}

void PreferencesDialog::setButtonColor(QToolButton *b, QColor color)
{
	_colorForWidget[b] = color;
	b->setIcon(iconForColor(color));
}

QColor PreferencesDialog::buttonColor(QToolButton *b)
{
	return getCopy(_colorForWidget, b).get_value_or({});
}

void PreferencesDialog::on_addLibraryButton_clicked()
{
	auto path = QFileDialog::getExistingDirectory(this, "Select Library");
	if(path.isNull()) return;

	_libraries << path;
	updateLibraryList();
}

void PreferencesDialog::on_removeLibraryButton_clicked()
{
	auto items = _ui->libraryList->selectedItems();
	if(items.empty()) return;

	QMessageBox msg(this);
	msg.setWindowFlags(Qt::Sheet);
	msg.setWindowModality(Qt::WindowModal);
	msg.setText(tr("Remove selected libraries?"));
	msg.setInformativeText(tr("The libraries will not be deleted; they will only be removed from this list."));
	msg.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
	msg.setDefaultButton(QMessageBox::Yes);
	int rv = msg.exec();



	if(rv == QMessageBox::Yes)
	{
		for(auto item : items)
		{
			_libraries.remove(item->text());
		}

		updateLibraryList();
	}
}

void PreferencesDialog::updateLibraryList()
{
	_ui->libraryList->clear();
	_ui->libraryList->addItems(_libraries.toList());
}

} /* namespace dbuilder */

