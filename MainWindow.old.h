#pragma once
#include <QMainWindow>
#include "DiagramContext.hpp"
/**
 * @file   MainWindow.h
 *
 * @date   Jan 3, 2013
 * @author Sam Roth <>
 */
class DiagramScene;
class QGraphicsView;
class QMenu;
class QAction;
class QToolBar;

namespace Ui {

class MainWindow;

}  // namespace Ui

class ContextMenu;
class DiagramLoader;

class MainWindow: public QMainWindow
{
	Q_OBJECT
	DiagramContext *ctx;
	Ui::MainWindow *_ui;
	DiagramScene *_scene;
	QGraphicsView *_gview;
	QMenu *_itemMenu;
	float _scale;
	QString _filename;
	ContextMenu *_contextMenu;
	DiagramLoader *_diagramLoader;
	QToolBar *_contextActionToolbar;
public:
	MainWindow(QWidget *parent=nullptr);

	DiagramScene *scene() const { return _scene; }
	virtual ~MainWindow();
private:
	const QString& filename() const
	{
		return _filename;
	}

	void setFilename(QString filename);
protected:
	virtual void closeEvent(QCloseEvent * event);
private slots:

	void connectorTypeChanged(QAction *);
	void sceneSelectionChanged();
	void contextMenu(class QGraphicsSceneContextMenuEvent *);
	void addItemFromToolbox();

	void copyItems();
	void pasteItems();

	void rotateCCW();
	void rotateCW();

	void itemMenuAboutToShow();
	void itemMenuTriggered();
	bool newDoc();
	void save();
	void saveAs();
	void load();
	void del();
	void duplicate();
	void exportSVG();
	void print();
	void openInsertMenu();
	void zoomIn();
	void zoomOut();

};

