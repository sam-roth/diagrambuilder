#pragma once
/**
 * @file   PreferencesDialog.hpp
 *
 * @date   Mar 28, 2013
 * @author Sam Roth <>
 */

#include <QDialog>
#include <QColor>
#include <QMap>

class Ui_PreferencesDialog;
class QAbstractButton;
class QColorDialog;
class QToolButton;

namespace dbuilder
{

class PreferencesDialog: public QDialog
{
	Q_OBJECT
	Ui_PreferencesDialog *_ui;
	QColorDialog *_colorDialog;
	QMap<QWidget *, QColor> _colorForWidget;
	QSet<QString> _libraries;

	void setButtonColor(QToolButton *, QColor);
	QColor buttonColor(QToolButton *);
	void updateLibraryList();
public:
	PreferencesDialog(QWidget *parent=nullptr);
	virtual ~PreferencesDialog();

	void updateView();
	void updateModel();
protected:
	virtual void showEvent(QShowEvent *event);
private slots:
	void onAccepted();
	void onColorButtonClicked();

	void on_buttonBox_clicked(QAbstractButton *);

	void on_addLibraryButton_clicked();
	void on_removeLibraryButton_clicked();
};

} /* namespace dbuilder */
