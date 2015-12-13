/**
 * @file   MainWindow.cpp
 *
 * @date   Mar 10, 2013
 * @author Sam Roth <>
 */

#include <fstream>
#include <QtGui>
#include <QtSvg>
#include <QtCore>
#include <QMdiArea>
#include <QAction>

#include "Clipboard.hpp"
#include "Commands/DeleteItemCommand.hpp"
#include "Commands/InsertItemCommand.hpp"
#include "Commands/InsertItemsCommand.hpp"
#include "Commands/RotateItemCommand.hpp"
#include "DiagramContext.hpp"
#include "DiagramIO/InfoDiagramLoader.hpp"
#include "DiagramItem.hpp"
#include "DiagramComponent.hpp"
#include "DiagramItemModel.hpp"
#include "Main/Application.hpp"
#include "Main/GridMenu.hpp"
#include "Main/GridMenuAction.hpp"
#include "MainWindow.hpp"
#include "ui_MainWindowUI.h"
#include "Util/ListUtil.hpp"
#include "Util/Log.hpp"
#include "Util/ScopeExit.hpp"
#include "UUIDMapper.hpp"
#include "Toolbox.hpp"
#include "PreferencesDialog.hpp"
#include <QTextCursor>
#include "DiagramIO/ComponentFile.hpp"
#include "ExportComponentOptions.hpp"
namespace dbuilder {

void MainWindow::populateToolDock(const QList<QAction*>& contextActions)
{
	auto toolDockWidget = new Toolbox(this);
	auto &layout = dynamic_cast<QVBoxLayout &>(*_ui->toolDockContents->layout());
	toolDockWidget->addActions(contextActions);
	layout.addWidget(toolDockWidget);
	layout.addStretch();
	layout.setContentsMargins(0, 0, 0, 0);
}

MainWindow::MainWindow(Application *app)
: _ui(new Ui::MainWindow)
, _app(app)
, _prefsDialog(new PreferencesDialog(this))
, _propWidget(new GenericPropertyWidget(this))
{
	_ctx = app->createContext();
	_ctx->setParent(this);
	_loader = new InfoDiagramLoader(_ctx, this);

	_ui->setupUi(this);
	_ui->propDock->setWidget(_propWidget);

	_scene = new DiagramScene(_ctx, this);
	_view = new DiagramView(this);
	_view->setScene(_scene);

	this->setCentralWidget(_view);


	QList<QAction*> contextActions = createInsertItemActions();

	_contextMenu = new GridMenu(this);
	auto contextMenuGrid = new GridMenuAction(contextActions, this);
	_contextMenu->addAction(contextMenuGrid);
	_ui->menuInsert->addActions(contextActions);


	populateToolDock(contextActions);
	connect(_scene, SIGNAL(contextMenu(QGraphicsSceneContextMenuEvent *)), this, SLOT(contextMenu(QGraphicsSceneContextMenuEvent *)));
	connect(_scene, SIGNAL(modifiedChanged(bool)), this, SLOT(sceneModifiedChanged()));
	connect(_scene, SIGNAL(selectionChanged()), this, SLOT(sceneSelectionChanged()));
	connect(_ui->actionDrag_Lock, SIGNAL(triggered(bool)), _scene, SLOT(setDragLock(bool)));

	QAction *toggleViewAction = _ui->propDock->toggleViewAction();
	toggleViewAction->setText(tr("Inspector"));
	toggleViewAction->setShortcut(QKeySequence("Ctrl+Shift+I"));

	_ui->menuView->addAction(toggleViewAction);

	connect(qApp, SIGNAL(focusChanged(QWidget *, QWidget *)), this, SLOT(onApplicationFocusChange(QWidget *, QWidget *)));

	setCurrentFilePath({});

	QTimer::singleShot(0, this, SLOT(deferredInit()));
//	this->setWindowTitle("[*]");
//	this->setWindowFilePath({});
}


MainWindow::~MainWindow()
{
}

QList<QAction*> MainWindow::createInsertItemActions()
{
	QList<QAction *> actions;
	for(auto kind : _ctx->kinds())
	{
		if(!kind->hidden())
		{
			auto action = new QAction(this);
			action->setText(kind->name());
			action->setIcon(kind->icon());
			actions << action;
			connect(action, SIGNAL(triggered()), this, SLOT(insertItemTriggered()));
		}

	}
	return actions;
}

void MainWindow::closeEvent(QCloseEvent *event)
{
	if(!displaySaveChangesPrompt())
	{
		event->ignore();
	}
	else
	{
		event->accept();
	}

	_app->settings().setValue("MainWindow/geometry", saveGeometry());
	_app->settings().setValue("MainWindow/state", saveState());
}

void MainWindow::insertItemTriggered()
{
	auto action = static_cast<QAction *>(sender());
	auto cmd = new InsertItemCommand(_scene, action->text(), _scene->cursorPos());
	_scene->undoStack().push(cmd);
}

void MainWindow::contextMenu(QGraphicsSceneContextMenuEvent *event)
{
	this->_scene->setCursorPos(event->scenePos());
	this->_contextMenu->exec(QCursor::pos());
}

void MainWindow::sceneModifiedChanged()
{
	if(_scene->clean())
	{
		this->setWindowModified(false);
	}
	else
	{
		this->setWindowModified(true);
	}
}

void MainWindow::on_actionNew_triggered()
{
	_app->addMainWindow();
}

bool MainWindow::displaySaveChangesPrompt()
{
	if(_scene->clean()) return true;

	QMessageBox m{this};
	m.setWindowFlags(Qt::Sheet);
	m.setText("The document has been modified");
	m.setInformativeText("Do you want to save your changes?");
	m.setStandardButtons(QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);
	m.setDefaultButton(QMessageBox::Save);
	m.setWindowModality(Qt::WindowModal);

	int r = m.exec();
	if(r == QMessageBox::Save)
	{
		return save();
	}
	else if(r == QMessageBox::Discard)
	{
		return true;
	}
	else
	{
		return false;
	}
}

bool MainWindow::makeNew()
{
	if(!displaySaveChangesPrompt())
	{
		return false;
	}

	_scene->clearDiagram();
	_scene->undoStack().clear();
	setCurrentFilePath({});
	return true;
}

bool MainWindow::save()
{
//	if(_scene->clean()) return true;
	if(windowFilePath().isEmpty())
	{
		return saveAs();
	}
	else
	{
		return saveFile(windowFilePath());
	}
}

bool MainWindow::saveAs()
{
	QFileDialog d{this};
	d.setFilter("*.dbuilder");
	d.setWindowModality(Qt::ApplicationModal);
	d.setAcceptMode(QFileDialog::AcceptSave);
	if(d.exec())
	{
		auto fn = d.selectedFiles().empty()? QString() : d.selectedFiles().front();
		if(fn.isEmpty())
		{
			return false;
		}
		else
		{
			return saveFile(fn);
		}
	}
	else
	{
		return false;
	}
}


bool MainWindow::saveFile(QString where)
{
	std::ofstream os(where.toStdString());
	if(!os)
	{
		DBError("Failed to open output file ", where.toStdString());
		return false;
	}
	_loader->save(os, _scene->diagramItems().values());
	setCurrentFilePath(where);
	_scene->setClean(true);
	return true;
}

void MainWindow::on_actionOpen_triggered()
{
	auto fname = QFileDialog::getOpenFileName(this, {}, {}, "*.dbuilder");
	if(!fname.isEmpty())
	{
		openFile(fname);
	}
}

bool MainWindow::openFile(QString where)
{
	makeNew();
	std::ifstream is(where.toStdString());
	if(!is)
	{
		DBError("Failed to open input file ", where.toStdString());
		return false;
	}

	auto loaded = _loader->load(is, _scene);
	_scene->addDiagramItemsInOrder(loaded);
	this->setCurrentFilePath(where);
	_scene->setClean(true);
	return true;
}

void MainWindow::on_actionSave_triggered()
{
	save();
}

void MainWindow::on_actionSave_As_triggered()
{
	saveAs();
}
void MainWindow::on_actionPrint_triggered()
{
	QPrinter printer(QPrinter::HighResolution);
	QPrintDialog printDialog(&printer, this);
	if(printDialog.exec() != QDialog::Accepted)
	{
		return;
	}

	_scene->setPrintMode(true);
	DBScopeExit([&](){ _scene->setPrintMode(false); });

	QPainter painter(&printer);
	_scene->render(&painter);


}

void MainWindow::on_actionCut_triggered()
{
	// same as copy -> delete
	on_actionCopy_triggered();
	on_actionDelete_triggered();
}

void MainWindow::on_actionCopy_triggered()
{
	copyToClipboard(_loader, filterCast<DiagramItem>(_scene->selectedItems()));
}

void MainWindow::on_actionRotate_Left_triggered()
{
	rotateSelectedItems(-90);
}

void MainWindow::on_actionRotate_Right_triggered()
{
	rotateSelectedItems(90);
}

void MainWindow::on_actionZoom_In_triggered()
{
	_view->scale(1.3, 1.3);
}

void MainWindow::on_actionZoom_Out_triggered()
{
	_view->scale(1/1.3, 1/1.3);
}

void MainWindow::on_actionUndo_triggered()
{
	_scene->undoStack().undo();
}

void MainWindow::on_actionRedo_triggered()
{
	_scene->undoStack().redo();
}

void MainWindow::rotateSelectedItems(qreal angle)
{
	auto rootUndoCommand = new QUndoCommand;
	for(auto item : filterCast<DiagramItem>(_scene->selectedItems()))
	{
		new RotateItemCommand(_scene, item, angle, rootUndoCommand);
	}

	_scene->undoStack().push(rootUndoCommand);
}

void MainWindow::on_actionPaste_triggered()
{
	auto items = pasteFromClipboard(_loader, _scene);
	_scene->undoStack().push(new InsertItemsCommand(_scene, items));
	_scene->clearSelection();
	for(auto item : items)
	{
		item->setSelected(true);
	}
}

void MainWindow::on_actionDuplicate_triggered()
{
	auto sources = filterCast<DiagramItem>(_scene->selectedItems());
	QList<DiagramItem *> targets;
	UUIDMapper mapper;
	for(auto source : sources)
	{
		auto targetModel = source->model()->clone(&mapper, _scene);
		auto target = targetModel->kind()->createFromModel(targetModel);
		targets << target;
	}

	_scene->undoStack().push(new InsertItemsCommand(_scene, targets));
	_scene->clearSelection();
	for(auto target : targets)
	{
		target->setSelected(true);
	}
}

void MainWindow::sceneSelectionChanged()
{
	_propWidget->setItems({});

	auto items = filterCast<DiagramItem>(_scene->selectedItems());
	_propWidget->setItems(items);
}

void MainWindow::on_actionInsert_Item_triggered()
{
	_contextMenu->exec(QCursor::pos(), _ui->actionInsert_Item);
}

void MainWindow::exportToSVG(QString where)
{
	QSvgGenerator gen;
	gen.setFileName(where);
	gen.setSize(_scene->sceneRect().size().toSize());
	_scene->setPrintMode(true);
	DBScopeExit([&](){ _scene->setPrintMode(false); });

	QPainter painter(&gen);
	_scene->render(&painter);
}

void MainWindow::on_actionExport_as_SVG_triggered()
{
	auto filename = QFileDialog::getSaveFileName(this, "Export to SVG", {}, "*.svg");
	if(!filename.isEmpty())
	{
		exportToSVG(filename);
	}
}

void MainWindow::setCurrentFilePath(const QString &filePath)
{
	this->setWindowFilePath(filePath);
	this->setWindowTitle(QFileInfo{filePath}.fileName() + "[*]");
}

void MainWindow::on_actionPreferences_triggered()
{
	_prefsDialog->exec();
}

void MainWindow::on_actionDelete_triggered()
{
	auto rootCmd = new QUndoCommand;
	for(auto item : filterCast<DiagramItem>(_scene->selectedItems()))
	{
		new DeleteItemCommand(_scene, item, rootCmd);
	}
	_scene->undoStack().push(rootCmd);
}

void MainWindow::on_actionPolyline_Connector_triggered()
{
	_scene->setConnectorType("connector");
}

void MainWindow::on_actionPath_Connector_triggered()
{
	_scene->setConnectorType("path-connector");
}


void MainWindow::on_actionClose_triggered()
{
	this->close();
}


QFont MainWindow::currentFont()
{
	if(auto textEdit = dynamic_cast<QTextEdit *>(qApp->focusWidget()))
	{
		return textEdit->currentCharFormat().font();

	}
	else
	{
		return {};
	}
}

void MainWindow::setCurrentFont(const QFont &font)
{
	if(auto textEdit = dynamic_cast<QTextEdit *>(qApp->focusWidget()))
	{
		auto currentCharFormat = textEdit->currentCharFormat();
		currentCharFormat.setFont(font);
		textEdit->setCurrentCharFormat(currentCharFormat);
	}
}

void MainWindow::on_actionFont_triggered()
{
	if(auto textEdit = dynamic_cast<QTextEdit *>(qApp->focusWidget()))
	{
		bool ok;
		auto font = QFontDialog::getFont(&ok, currentFont(), this);
		if(ok)
		{
			textEdit->setCurrentFont(font);
		}
	}
}

void MainWindow::on_actionBold_triggered()
{
	auto font = currentFont();
	DBDebug(DBDump(_ui->actionBold->isChecked(), _ui->actionBold->isCheckable()));
	font.setBold(_ui->actionBold->isChecked());
	setCurrentFont(font);
}

void MainWindow::on_actionItalic_triggered()
{
	auto font = currentFont();
	font.setItalic(_ui->actionItalic->isChecked());
	setCurrentFont(font);
}

void MainWindow::onApplicationFocusChange(QWidget *old, QWidget *now)
{
	// automatically connect any focused QTextEdit to the font controls
	if(auto oldEdit = dynamic_cast<QTextEdit *>(old))
	{
		disconnect(oldEdit, SIGNAL(currentCharFormatChanged(const QTextCharFormat &)), this, SLOT(updateTextStyle()));
	}
	if(auto newEdit = dynamic_cast<QTextEdit *>(now))
	{
		connect(newEdit, SIGNAL(currentCharFormatChanged(const QTextCharFormat &)), this, SLOT(updateTextStyle()));
	}
	updateTextStyle();

}

void MainWindow::updateTextStyle()
{
	auto f = currentFont();
	_ui->actionBold->setChecked(f.bold());
	_ui->actionItalic->setChecked(f.italic());
	_ui->actionUnderline->setChecked(f.underline());
}

void MainWindow::deferredInit()
{
	auto geom = _app->settings().value("MainWindow/geometry");
	auto state = _app->settings().value("MainWindow/state");

	if(!geom.isNull()) restoreGeometry(geom.toByteArray());
	if(!state.isNull()) restoreState(state.toByteArray());

}

void MainWindow::on_actionUnderline_triggered()
{
	auto font = currentFont();
	font.setUnderline(_ui->actionUnderline->isChecked());
	setCurrentFont(font);
}

void MainWindow::exportAsComponent(const QString &componentName,
                                   const QString &where,
                                   const QString &authorName)
{
	auto sceneRect = _scene->itemsBoundingRect();

	// create a svg of the currently visible items
	QString svgData;
	{
		QBuffer svgBuffer;
		{
			QSvgGenerator gen;
			gen.setOutputDevice(&svgBuffer);
			_scene->setPrintMode(true);
			DBScopeExit([&](){ _scene->setPrintMode(false); });


			QPainter painter(&gen);
			_scene->render(&painter, sceneRect, sceneRect);

		}

		svgData.append(svgBuffer.data());
	}

//	{
//		std::ofstream os("/tmp/a.svg");
//		os << svgData;
//	}

	// find all of the ports in the scene
	QList<QPointF> ports;
	for(auto item : _scene->diagramItems())
	{
		for(auto port : item->portLocations())
		{

			auto scenePortLoc = item->mapToScene(port);
			ports << scenePortLoc - sceneRect.topLeft();
		}
	}

	// add collected information to a component file
	ComponentFile cf;
	if(!authorName.isNull())
	{
		cf.setAuthor(authorName);
	}
	cf.addComponent(componentName, svgData, ports);

	std::ofstream outputFile(where.toStdString());
	cf.write(outputFile);

}

void MainWindow::on_actionExport_as_Component_triggered()
{
	if(auto options = ExportComponentOptions::getOptions(this))
	{

		QFileDialog fd(this);
		fd.setWindowTitle("Export as Component");
		fd.setFilter("*.dbcomponent");
		fd.setAcceptMode(QFileDialog::AcceptSave);
		fd.selectFile(options->componentName);
		if(fd.exec() && fd.selectedFiles().size() == 1)
		{
			auto filename = fd.selectedFiles().first();

			exportAsComponent(options->componentName,
			                  filename,
			                  options->author);
		}
	}

}

void MainWindow::on_actionSend_to_Back_triggered()
{
	_scene->sendSelection(_scene->ToBack);
}

void MainWindow::on_actionSend_to_Front_triggered()
{
	_scene->sendSelection(_scene->ToFront);
}

void MainWindow::on_actionSend_Backward_triggered()
{
	_scene->sendSelection(_scene->Backward);
}

void MainWindow::on_actionSend_Frontward_triggered()
{
	_scene->sendSelection(_scene->Frontward);
}
} /* namespace dbuilder */

