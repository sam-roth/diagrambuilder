/**
 * @file   MainWindow.cpp
 *
 * @date   Jan 3, 2013
 * @author Sam Roth <>
 */

#include "moc_MainWindow.cpp"
#include "DiagramScene.h"
#include <QGraphicsView>
#include <QPainter>
#include "Kinds/ConnectorKind.h"
#include "Kinds/SVGKind.hpp"
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/info_parser.hpp>
#include <QToolBar>
#include <QFileDialog>
#include <QMenuBar>
#include "DiagramItem.h"
#include "Commands/InsertItemCommand.h"
#include "Commands/DeleteItemCommand.h"
#include <QMainWindow>
#include "UUIDMapper.h"
#include <QSvgGenerator>
#include <QGLWidget>
#include <QPainter>
#include "ui_MainWindowUI.h"
#include "Kinds/TextKind.h"
#include <QPrintDialog>
#include <QPrinter>
#include <QPushButton>
#include "qgraphicsitem.h"
#include <QPainter>
#include <QPixmap>
#include <QBitmap>
#include <QSvgRenderer>
#include <QMessageBox>
#include <QFile>
#include <QDomDocument>
#include <QCloseEvent>
#include "Util/FixedWidthStroke.hpp"
#include "ContextMenu.h"
#include <QGraphicsSceneMouseEvent>
#include "Commands/RotateItemCommand.h"
#include "DiagramIO/InfoDiagramLoader.hpp"
#include "ComponentFileReader.hpp"
#include "DiagramContext.hpp"
#include "Clipboard.hpp"
#include "Util/ListUtil.hpp"
#include "TestTableModel.hpp"
#include "Kinds/PathKind.hpp"
#include "Kinds/PathConnectorKind.hpp"
#include <QAction>
#include "Kinds/GroupKind.hpp"
#include "DiagramView.hpp"
#include "Kinds/BoxKind.hpp"
#include "Commands/InsertItemsCommand.hpp"
#include "Util/Log.hpp"
#include <QWidgetAction>
#include <QVBoxLayout>
#include <QToolButton>
using dbuilder::PathKind;
using dbuilder::DiagramView;

MainWindow::MainWindow(QWidget* parent)
: QMainWindow(parent)
, ctx(new DiagramContext(this))
, _ui(new Ui::MainWindow)
, _scene(new DiagramScene(ctx, this))
, _gview(new DiagramView(this))
, _scale(1)
, _contextMenu(new ContextMenu(this))
, _diagramLoader(new InfoDiagramLoader(ctx, this))
, _contextActionToolbar(new QToolBar(this))
{
	_gview->setScene(_scene);
	_ui->setupUi(this);
	setCentralWidget(_gview);
	_gview->setViewport(new QGLWidget(QGLFormat(QGL::SampleBuffers)));
	_gview->setRenderHints(
		QPainter::Antialiasing |
		QPainter::TextAntialiasing |
		QPainter::SmoothPixmapTransform |
		QPainter::HighQualityAntialiasing
	);
	_gview->setDragMode(QGraphicsView::RubberBandDrag);
	this->addToolBar(Qt::TopToolBarArea, _contextActionToolbar);
	_contextActionToolbar->hide();
	_contextActionToolbar->setIconSize(QSize(16, 16));


	connect(_scene, SIGNAL(modifiedChanged(bool)), this, SLOT(setWindowModified(bool)));

	_scene->registerKind(new ConnectorKind(_scene));
	_scene->registerKind(new dbuilder::PathConnectorKind(_scene));
	_scene->registerKind(new TextKind(_scene));
	_scene->registerKind(new dbuilder::GroupKind(_scene));
	_scene->registerKind(new dbuilder::BoxKind(_scene));

	ctx->registerKind(new PathKind(_scene));


	ComponentFileReader cfr;

	std::string path;
	QFile compsFile("://components.info");
	compsFile.open(QFile::ReadOnly);
	auto compsFileContents = compsFile.readAll();
	std::stringstream compsFileStream(std::string(compsFileContents.begin(), compsFileContents.end()));
	cfr.read(compsFileStream, this->_scene);

	QDir homeDir = QDir::home();
	if(homeDir.cd(".dbuilder"))
	{
		auto userCompFileName = homeDir.filePath("components.info");
		std::ifstream is(userCompFileName.toStdString());
		if(is)
		{
			cfr.read(is, this->_scene);
		}
	}

	_itemMenu = new QMenu(this); // = _ui->menuInsert;

//	connect(_itemMenu, SIGNAL(aboutToShow()), this, SLOT(itemMenuAboutToShow()));
//	connect(_itemMenu, SIGNAL(triggered(QAction*)), this, SLOT(itemMenuTriggered(QAction*)));
	connect(_ui->actionDelete, SIGNAL(triggered()), this, SLOT(del()));
	connect(_ui->actionDuplicate, SIGNAL(triggered()), this, SLOT(duplicate()));
	connect(_ui->actionExport_as_SVG, SIGNAL(triggered()), this, SLOT(exportSVG()));
	connect(_ui->actionSave, SIGNAL(triggered()), this, SLOT(save()));
	connect(_ui->actionSave_As, SIGNAL(triggered()), this, SLOT(saveAs()));
	connect(_ui->actionNew, SIGNAL(triggered()), this, SLOT(newDoc()));
	connect(_ui->actionOpen, SIGNAL(triggered()), this, SLOT(load()));
	connect(_ui->actionInsert_Item, SIGNAL(triggered()), this, SLOT(openInsertMenu()));
	connect(_ui->actionZoom_In, SIGNAL(triggered()), this, SLOT(zoomIn()));
	connect(_ui->actionZoom_Out, SIGNAL(triggered()), this, SLOT(zoomOut()));
	connect(_ui->actionPrint, SIGNAL(triggered()), this, SLOT(print()));
	connect(_ui->actionRotate_Left, SIGNAL(triggered()), this, SLOT(rotateCCW()));
	connect(_ui->actionRotate_Right, SIGNAL(triggered()), this, SLOT(rotateCW()));
	connect(_scene, SIGNAL(contextMenu(QGraphicsSceneContextMenuEvent *)), this, SLOT(contextMenu(QGraphicsSceneContextMenuEvent *)));
	connect(_ui->actionUndo, SIGNAL(triggered()), &_scene->undoStack(), SLOT(undo()));
	connect(_ui->actionRedo, SIGNAL(triggered()), &_scene->undoStack(), SLOT(redo()));
	connect(_ui->actionCopy, SIGNAL(triggered()), this, SLOT(copyItems()));
	connect(_ui->actionPaste, SIGNAL(triggered()), this, SLOT(pasteItems()));
	connect(_ui->actionGroup, SIGNAL(triggered()), _scene, SLOT(group()));
	connect(_ui->actionUngroup, SIGNAL(triggered()), _scene, SLOT(ungroup()));


	auto connectorTypeGroup = new QActionGroup(this);
	connectorTypeGroup->addAction(_ui->actionPath_Connector);
	connectorTypeGroup->addAction(_ui->actionPolyline_Connector);
	connectorTypeGroup->setExclusive(true);
	connect(connectorTypeGroup, SIGNAL(triggered(QAction*)), this, SLOT(connectorTypeChanged(QAction*)));
	connect(_scene, SIGNAL(selectionChanged()), this, SLOT(sceneSelectionChanged()));

	auto toolDockContents = _ui->toolDockContents;
	auto itemMenuPaletteAction = new QWidgetAction(this);
	auto itemMenuPalette = new QWidget;
	itemMenuPaletteAction->setDefaultWidget(itemMenuPalette);
	auto itemMenuPaletteLayout = new QGridLayout(itemMenuPalette);
	_itemMenu->addAction(itemMenuPaletteAction);
	itemMenuPalette->setStyleSheet(R"EOF(
QToolButton {
  border: none;
}

QToolButton:hover {
  background-color: palette(highlight);
}

)EOF");
	int col = 0;
	const int ncols = 8;
	for(auto kind : _scene->kinds())
	{
		if(kind->hidden()) continue;

		auto button = new QPushButton(kind->name(), toolDockContents);
//		button->setFixedHeight(100);
		auto itemMenuButton = new QToolButton(itemMenuPalette);
		itemMenuButton->setText(kind->name());
		itemMenuButton->setToolButtonStyle(Qt::ToolButtonIconOnly);
		connect(itemMenuButton, SIGNAL(clicked()), this, SLOT(addItemFromToolbox()));
		connect(itemMenuButton, SIGNAL(clicked()), _itemMenu, SLOT(close()));
		itemMenuButton->setToolTip(kind->name());
		if(auto svgKind = dynamic_cast<SVGKind *>(kind))
		{
//			QImage img(QSize(32, 32), QImage::Format_ARGB32);
//			img.fill(QColor(0, 0, 0, 0));
//
//			QPixmap pixmap = QPixmap::fromImage(img);
//			QPainter painter(&pixmap);
//
//			auto document = makeFixedWidthStroke(svgKind->filename());
//
//
//			QSvgRenderer renderer(document.toAscii());
//
//			auto viewBox = renderer.viewBoxF();
//			auto aspect = viewBox.width() / viewBox.height();
//			QSizeF drawSize;
//			if(aspect > 1.0)
//			{
//				drawSize = QSizeF(32, 32.0 / aspect);
//			}
//			else
//			{
//				drawSize = QSizeF(32.0 * aspect, 32.0);
//			}
//
//			QRectF drawArea({(32.0-drawSize.width())/2.0, (32.0-drawSize.height())/2.0}, drawSize);
//
//			renderer.render(&painter, drawArea);
//			QIcon icon(pixmap);


			QIcon icon = kind->icon();
			button->setIconSize({32,32});
			button->setIcon(icon);


			itemMenuButton->setIcon(icon);
			itemMenuPaletteLayout->addWidget(itemMenuButton, col/ncols, col % ncols);
			++col;

		}
		else
		{
			itemMenuButton->setToolButtonStyle(Qt::ToolButtonTextOnly);
			int itemcol, itemrow;
			for(;;)
			{
				itemrow = col / ncols;
				itemcol = col % ncols;
				if(ncols - itemcol >= 2) break;
				++col;
			}
			itemMenuButton->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

			itemMenuPaletteLayout->addWidget(itemMenuButton, col/ncols, col % ncols, 1, 2, Qt::AlignLeft);
			col += 2;
//			_itemMenu->addAction(kind->name(), this, SLOT(itemMenuTriggered()));
		}

		toolDockContents->layout()->addWidget(button);

		connect(button, SIGNAL(clicked()), this, SLOT(addItemFromToolbox()));

	}

	static_cast<QVBoxLayout *>(toolDockContents->layout())->addStretch();
//	this->setUnifiedTitleAndToolBarOnMac(false);
}


void MainWindow::connectorTypeChanged(QAction *action)
{
	if(action == _ui->actionPolyline_Connector)
	{
		_scene->setConnectorType("connector");
	}
	else
	{
		_scene->setConnectorType("path-connector");
	}
}

void MainWindow::sceneSelectionChanged()
{
	_contextActionToolbar->clear();
	auto items = _scene->selectedItems();
	if(items.size() == 1)
	{
		if(auto diagramItem = dynamic_cast<DiagramItem*>(items.front()))
		{
			_contextActionToolbar->addActions(diagramItem->contextActions());
			_contextActionToolbar->show();
		}
	}
	else
	{
		_contextActionToolbar->hide();

	}

}

void MainWindow::copyItems()
{
	copyToClipboard(_diagramLoader, filterCast<DiagramItem>(_scene->selectedItems()));
}

void MainWindow::pasteItems()
{
	this->_scene->undoStack().push(new dbuilder::InsertItemsCommand(_scene, pasteFromClipboard(_diagramLoader, _scene)));
}

void MainWindow::rotateCCW()
{
	for(auto item : _scene->selectedItems())
	{
		if(auto diagramItem = dynamic_cast<DiagramItem *>(item))
		{
			_scene->undoStack().push(new RotateItemCommand(_scene, diagramItem, -90));
//			diagramItem->setRotation(diagramItem->rotation() - 90);
		}
	}
}

void MainWindow::rotateCW()
{
	for(auto item : _scene->selectedItems())
	{
		if(auto diagramItem = dynamic_cast<DiagramItem *>(item))
		{
			_scene->undoStack().push(new RotateItemCommand(_scene, diagramItem, 90));
		}
	}
}

void MainWindow::contextMenu(QGraphicsSceneContextMenuEvent *evt)
{
//	_contextMenu->exec(evt->screenPos());

	_scene->setCursorPos(evt->scenePos());
	DBInfo(DBDump(_scene->cursorPos()));

//	QMenu menu;
//	menu.addMenu(_ui->menuEdit);
//	menu.addMenu(_ui->menuInsert);
//	menu.exec(evt->screenPos());

	_itemMenu->exec(evt->screenPos());
}

void MainWindow::closeEvent(QCloseEvent * event)
{
	if(newDoc() && _scene->clean())
	{
		event->accept();
	}
	else
	{
		event->ignore();
	}
}

void MainWindow::addItemFromToolbox()
{
	auto sdr = static_cast<QAbstractButton *>(QObject::sender());
	auto item = new InsertItemCommand(_scene, sdr->text(), _scene->cursorPos());
	_scene->undoStack().push(item);
}

void MainWindow::zoomIn()
{
	_gview->scale(1.3, 1.3);
}

void MainWindow::zoomOut()
{
	_gview->scale(1/1.3, 1/1.3);
}

void MainWindow::openInsertMenu()
{
	_itemMenu->exec(QCursor::pos());
}

void MainWindow::exportSVG()
{
	auto filename = QFileDialog::getSaveFileName(this, "Export as SVG", {}, "*.svg");
	if(filename.isNull()) return;

	QSvgGenerator gen;
	gen.setFileName(filename);
	for(auto item : _scene->items())
	{
		item->setCacheMode(QGraphicsItem::NoCache);
	}

	for(auto item : _scene->diagramItems())
	{
		item->setPrintMode(true);
	}

	_scene->setPrintMode(true);
	gen.setSize(QSize(_gview->sceneRect().width(), _gview->sceneRect().height()));
//	gen.setViewBox(0, 0, _gview->sceneRect().width(), _gview->sceneRect().height());
//	auto sr = _gview->sceneRect();
//	gen.setViewBox(QRectF(sr.right(), sr.bottom(), sr.width(), sr.height()));
	QPainter painter(&gen);
	_scene->render(&painter);
	_scene->setPrintMode(false);
	for(auto item : _scene->diagramItems())
	{
		item->setPrintMode(false);
	}

	for(auto item : _scene->items())
	{
		item->setCacheMode(QGraphicsItem::DeviceCoordinateCache);
	}

}

void MainWindow::print()
{
	QPrinter printer(QPrinter::HighResolution);

	QPrintDialog printDialog(&printer, this);
	if(printDialog.exec() != QDialog::Accepted)
	{
		return;
	}


	QPainter painter(printDialog.printer());

	for(auto item : _scene->items())
	{
		item->setCacheMode(QGraphicsItem::NoCache);
	}

	for(auto item : _scene->diagramItems())
	{
		item->setPrintMode(true);
	}

	_scene->setPrintMode(true);
	_scene->render(&painter);
	_scene->setPrintMode(false);

	for(auto item : _scene->diagramItems())
	{
		item->setPrintMode(false);
	}

	for(auto item : _scene->items())
	{
		item->setCacheMode(QGraphicsItem::DeviceCoordinateCache);
	}

	painter.end();

}

void MainWindow::duplicate()
{
	UUIDMapper mapper;
	QList<DiagramItem *> createdItems;
	for(auto item : _scene->selectedItems())
	{
		if(auto diagramItem = dynamic_cast<DiagramItem *>(item))
		{
			auto cloneModel = diagramItem->model()->clone(&mapper, _scene);
			auto newDiagramItem = new DiagramItem(_scene);
			newDiagramItem->setModel(cloneModel);
			createdItems << newDiagramItem;
		}
	}

	_scene->undoStack().push(new dbuilder::InsertItemsCommand(_scene, createdItems));

	_scene->clearSelection();
	for(auto item : createdItems)
	{
		item->setSelected(true);
	}
}

void MainWindow::del()
{
	for(auto item : _scene->selectedItems())
	{
		if(auto diagramItem = dynamic_cast<DiagramItem *>(item))
		{
			auto cmd = new DeleteItemCommand(_scene, diagramItem);
			this->_scene->undoStack().push(cmd);
		}

	}
}

void MainWindow::itemMenuAboutToShow()
{
//	_itemMenu->clear();
//	for(auto kind : this->_scene->kinds())
//	{
//		_itemMenu->addAction(kind->name());
//	}
}

void MainWindow::itemMenuTriggered()
{
	QAction *action = static_cast<QAction *>(sender());
	auto item = new InsertItemCommand(_scene, action->text(), _scene->cursorPos());
	_scene->undoStack().push(item);
}

bool MainWindow::newDoc()
{
	if(!scene()->clean())
	{
		QMessageBox mbox(this);
		mbox.setText("This document contains unsaved changes.");
		mbox.setInformativeText("Do you want to save the changes you made to the document?");
		mbox.setWindowFlags(Qt::Sheet);
		mbox.setStandardButtons(QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);
		mbox.setDefaultButton(QMessageBox::Save);
		mbox.setWindowModality(Qt::WindowModal);
		auto result = mbox.exec();
		switch(result)
		{
			case QMessageBox::Save: {
				save();
				if(!scene()->clean()) return true;
			} break;

			case QMessageBox::Discard: {
				// do nothing
			} break;

			case QMessageBox::Cancel: {
				return false;
			} break;

			default: assert(false);
		}

	}
	scene()->clearDiagram();
	scene()->setClean();
	setFilename(QString());
	return true;
}

void MainWindow::save()
{
	if(!_filename.isNull())
	{
//		saveDiagram(_filename.toStdString(), _scene);
		std::ofstream os(_filename.toStdString());
		_diagramLoader->save(os, _scene->diagramItems().values());
		_scene->setClean();
	}
	else
	{
		saveAs();
	}
}

void MainWindow::setFilename(QString filename)
{
	_filename = filename;
	this->setWindowFilePath(filename);
}

void MainWindow::saveAs()
{
	QFileDialog fileDialog(this);
//	fileDialog.setWindowModality(Qt::WindowModal);
	fileDialog.setFilter("*.dbuilder");
	fileDialog.setAcceptMode(QFileDialog::AcceptSave);

//	auto saveFileName = QFileDialog::getSaveFileName(this, "Save", {}, "*.dbuilder");
	if(fileDialog.exec())
	{
		auto saveFileName = fileDialog.selectedFiles().front();

		std::ofstream os(saveFileName.toStdString());
		_diagramLoader->save(os, _scene->diagramItems().values());

//		saveDiagram(saveFileName.toStdString(), _scene);
		setFilename(saveFileName);
		_scene->setClean();
	}
}

void MainWindow::load()
{
//	auto loadFileName = QFileDialog::getOpenFileName(this, "Load", {}, "*.dbuilder");
	QFileDialog fileDialog(this);
//	fileDialog.setWindowModality(Qt::WindowModal);
	fileDialog.setFilter("*.dbuilder");
	fileDialog.setAcceptMode(QFileDialog::AcceptOpen);
//	if(!loadFileName.isNull())

	if(fileDialog.exec())
	{
		newDoc();
		auto loadFileName = fileDialog.selectedFiles().front();
//		loadDiagram(loadFileName.toStdString(), _scene);
		std::ifstream is(loadFileName.toStdString());
		auto items = _diagramLoader->load(is, _scene);
		_scene->addDiagramItemsInOrder(items);
		setFilename(loadFileName);
		_scene->update();
		_scene->setClean();
	}
}

MainWindow::~MainWindow()
{
	// TODO Auto-generated destructor stub
}

