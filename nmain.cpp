/**
 * @file   nmain.cpp
 *
 * @date   Jan 2, 2013
 * @author Sam Roth <>
 */
#include <QApplication>
#include <QGraphicsSvgItem>
#include <QGraphicsPathItem>
#include "DiagramItem.hpp"
#include "DiagramComponent.hpp"
#include <QGraphicsView>
#include <QGraphicsScene>
#include <QMainWindow>
#include <QPainterPath>
#include <cassert>
#include <iostream>
#include "DiagramScene.hpp"
#include "DiagramItemModel.hpp"
#include "qgraphicsitem.h"
#include <array>
#include <QDockWidget>
#include <QListWidget>
#include <qabstractitemmodel.h>
#include "Util/Log.hpp"

//class DiagramItemAndGateKind: public DiagramItemKind
//{
//	Q_OBJECT
//public:
//	DiagramItemAndGateKind(QObject *parent=nullptr)
//	: DiagramItemKind("and-gate", parent)
//	{ }
//
//	void configure(DiagramItem *item) const
//	{
////		item->setFlags(QGraphicsItem::ItemSendsScenePositionChanges | QGraphicsItem::ItemIsSelectable | QGraphicsItem::ItemIsMovable);
//		item->setFlag(QGraphicsItem::ItemIsSelectable);
//		item->setFlag(QGraphicsItem::ItemIsMovable);
//
//		item->addPort({50.0, 15.0});
//		item->addPort({0.0, 25.0});
//		item->addPort({0.0, 5.0});
//		auto svgItem = new QGraphicsSvgItem("andgate.svg", item);
//		(void)svgItem;
//	}
//};
//
//class ConnectionGraphicsItem: public QObject, public QGraphicsPathItem
//{
//	Q_OBJECT
//	DiagramItem *item;
//public:
//	ConnectionGraphicsItem(DiagramItem *item, QObject *parent=nullptr)
//	: QObject(parent)
//	, item(item)
//	{
//		connect(item, SIGNAL(dependencyPosChanged(DiagramItem *)), this, SLOT(dependencyPosChanged(DiagramItem *)));
//	}
//
//private slots:
//	void dependencyPosChanged(DiagramItem *dep)
//	{
//		if(item->scene())
//		{
//			auto scene = static_cast<DiagramScene *>(item->scene());
//			auto model = item->model();
//			if(!model->connection()) return;
//			auto conn = *model->connection();
//
//			auto p1 = scene->item(conn.src);
//			auto p2 = scene->item(conn.dst);
//
//			if(p1 && p2)
//			{
//				auto srcPt = p1->scenePortLocation(conn.srcPort);
//				auto dstPt = p2->scenePortLocation(conn.dstPort);
//
//				qreal xc = srcPt.x() + (dstPt.x() - srcPt.x()) / 2;
//
//				QPainterPath path(srcPt);
//				path.lineTo(xc, srcPt.y());
//				path.lineTo(xc, dstPt.y());
//				path.lineTo(dstPt);
//				this->setPath(path);
//			}
//		}
//	}
//};
//
//class DiagramItemConnectorKind: public DiagramItemKind
//{
//	Q_OBJECT
//public:
//	DiagramItemConnectorKind(QObject *parent=nullptr)
//	: DiagramItemKind("connector", parent)
//	{
//	}
//
//	void configure(DiagramItem *item) const
//	{
//		auto cgi = new ConnectionGraphicsItem(item, item);
//		cgi->setParentItem(item);
//	}
//};

//class KindListModel: public QAbstractListModel
//{
//	Q_OBJECT
//	DiagramScene *scene;
//public:
//	KindListModel(DiagramScene *scene, QObject *parent=nullptr)
//	: QAbstractListModel(parent)
//	, scene(scene) { }
//
//	int rowCount(const QModelIndex &parent=QModelIndex())
//	{
//		return scene->kinds().size();
//	}
//
//
//};

class App: public QApplication
{
	Q_OBJECT
public:
	App(int &argc, char **argv)
	: QApplication(argc, argv) { }

	virtual bool notify(QObject *receiver, QEvent *e)
	{
		try
		{
			return QApplication::notify(receiver, e);
		}
		catch(std::exception &exc)
		{
			DBLog(Error, "Caught exception: ", exc.what());
			return false;
		}
	}
};
#include "nmain.moc"

#include "DiagramItemModel.hpp"
#include "DiagramScene.hpp"
#include "Commands/InsertItemCommand.h"
#include "Commands/ConnectItemCommand.h"
#include "MainWindow.h"
#include "ComponentFileReader.hpp"
#include <fstream>
#include "Util/Log.hpp"


int main(int argc, char **argv)
{
	DBLog(Info, "DiagramBuilder started");
	dbuilder::log::setLevel(dbuilder::log::Debug);

	if(argc == 3 && std::string("compfile") == argv[1])
	{
		ComponentFileReader fr;
		std::ifstream is(argv[2]);
		fr.read(is, nullptr);
		return 0;
	}


	App app(argc, argv);
	MainWindow mainWindow;

	auto scene = mainWindow.scene();

//	QMainWindow mainWindow;
//
//
//
//	auto scene = new DiagramScene(&mainWindow);
//	auto graphicsView = new QGraphicsView(scene, &mainWindow);
//	mainWindow.setCentralWidget(graphicsView);
//	graphicsView->setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing | QPainter::SmoothPixmapTransform);
//	auto andGateKind = new DiagramItemAndGateKind(&app);
//	scene->registerKind(andGateKind);
//	scene->registerKind(new DiagramItemConnectorKind(scene));
//
//	{
//		auto sidePanel = new QDockWidget(&mainWindow);
//		auto listview = new QListWidget(sidePanel);
//		sidePanel->setWidget(listview);
//		for(auto kind : scene->kinds().keys())
//		{
//			listview->addItem(kind);
//		}
//
//		mainWindow.addDockWidget(Qt::LeftDockWidgetArea, sidePanel);
//
//	}
//
	auto &undoStack = scene->undoStack();
//	auto insertAndGate1 = new InsertItemCommand(scene, "and-gate", {0, 0});
//	auto insertAndGate2 = new InsertItemCommand(scene, "and-gate", {100, 100});
//	auto connectAndGates = new ConnectItemCommand(scene, Connection {
//		insertAndGate1->item()->model()->uuid(), 0,
//		insertAndGate2->item()->model()->uuid(), 1
//	});

//	undoStack.push(insertAndGate1);
//	undoStack.push(insertAndGate2);
//	undoStack.push(connectAndGates);
//
//
	mainWindow.show();
	app.setActiveWindow(&mainWindow);
	mainWindow.raise();
	mainWindow.setWindowState(Qt::WindowMaximized);
	return app.exec();
}



