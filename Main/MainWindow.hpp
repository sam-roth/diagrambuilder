#pragma once
/**
 * @file   MainWindow.hpp
 *
 * @date   Mar 10, 2013
 * @author Sam Roth <>
 */

#include <qmainwindow.h>
#include "DiagramView.hpp"
#include "DiagramScene.hpp"
#include "CoreForward.hpp"
#include "GenericPropertyWidget.hpp"

class QMdiArea;

namespace Ui {

class MainWindow;

}  // namespace Ui

namespace dbuilder {

class Application;
class PreferencesDialog;

class MainWindow: public QMainWindow
{
	Q_OBJECT
	Ui::MainWindow *_ui;
	Application *_app;
	DiagramContext *_ctx;
	QMenu *_contextMenu;
	DiagramView *_view;
	DiagramScene *_scene;
	DiagramLoader *_loader;
	PreferencesDialog *_prefsDialog;
	GenericPropertyWidget *_propWidget;
public:
	MainWindow(Application *app);
	virtual ~MainWindow();
	bool openFile(QString where);
private:
	QList<QAction *> createInsertItemActions();
	bool displaySaveChangesPrompt();
	bool makeNew();
	bool save();
	bool saveAs();
	bool saveFile(QString where);
	void setCurrentFilePath(const QString &filename);
	void rotateSelectedItems(qreal angle);
	void populateToolDock(const QList<QAction*>& contextActions);
	void exportToSVG(QString where);
	void exportAsComponent(const QString &componentName,
	                       const QString &where,
	                       const QString &authorName);

//	enum ToggleFont { ToggleNone=0, ToggleBold=1<<0, ToggleItalic=1<<1, ToggleUnderline=1<<2 };
	QFont currentFont();
	void setCurrentFont(const QFont &);
//	void setCurrentFont(ToggleFont toggle);
protected:
	void closeEvent(QCloseEvent *event);
private slots:
	void deferredInit();
	void insertItemTriggered();
	void contextMenu(QGraphicsSceneContextMenuEvent *);
	void sceneModifiedChanged();
	void sceneSelectionChanged();
	void onApplicationFocusChange(QWidget *, QWidget *);
	void updateTextStyle();

	void on_actionPreferences_triggered();
	void on_actionNew_triggered();
	void on_actionOpen_triggered();
	void on_actionSave_triggered();
	void on_actionSave_As_triggered();
	void on_actionPrint_triggered();
	void on_actionCut_triggered();
	void on_actionCopy_triggered();
	void on_actionPaste_triggered();
	void on_actionDuplicate_triggered();
	void on_actionRotate_Left_triggered();
	void on_actionRotate_Right_triggered();
	void on_actionZoom_In_triggered();
	void on_actionZoom_Out_triggered();
	void on_actionUndo_triggered();
	void on_actionRedo_triggered();
	void on_actionInsert_Item_triggered();
	void on_actionExport_as_SVG_triggered();
	void on_actionDelete_triggered();
	void on_actionPolyline_Connector_triggered();
	void on_actionPath_Connector_triggered();
	void on_actionClose_triggered();
	void on_actionFont_triggered();
	void on_actionBold_triggered();
	void on_actionItalic_triggered();
	void on_actionUnderline_triggered();
	void on_actionExport_as_Component_triggered();
	void on_actionSend_to_Back_triggered();
	void on_actionSend_to_Front_triggered();
	void on_actionSend_Backward_triggered();
	void on_actionSend_Frontward_triggered();
};

} /* namespace dbuilder */
