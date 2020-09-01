/**
 * @file   PathConnectorComponent.cpp
 *
 * @date   Jan 28, 2013
 * @author Sam Roth <>
 */

#include "PathConnectorComponent.hpp"
#include "Components/Path/PathItem.hpp"
#include <QObject>
#include "DiagramScene.hpp"
#include "Util.hpp"
#include "Util/Log.hpp"

namespace dbuilder {

class PathConnectorController: public QObject
{
	Q_OBJECT
	DiagramItem *item;
	PathItem *view;
public:

	PathConnectorController(DiagramItem *item, PathItem *view)
	: QObject(view)
	, item(item)
	, view(view)
	{
		view->setStartConstrained(true);
		view->setEndConstrained(true);

		connect(item, SIGNAL(dependencyPosChanged(DiagramItem *)), this, SLOT(dependencyPosChanged(DiagramItem *)));
		connect(item->model(), SIGNAL(updateViewRequested()), this, SLOT(itemAddedToScene()));
	}



public slots:

	void equalize()
	{
		auto path = view->path();
		if(path.elementCount() == 4)
		{
			auto e1 = path.elementAt(1);
			auto e2 = path.elementAt(2);
			auto e3 = path.elementAt(3);

			qreal y = (e1.y + e2.y) / 2.0;
			path.setElementPositionAt(1, 0, y);
			path.setElementPositionAt(2, e3.x, y);

			view->setPath(path);
		}
		else
		{
			DBError("cannot equalize path of length ", path.elementCount());
		}
	}

	void leftArrow()
	{
		view->setLeftArrow(!view->leftArrow());
	}

	void rightArrow()
	{
		view->setRightArrow(!view->rightArrow());
	}

	void dependencyPosChanged(DiagramItem *dep)
	{
		if(item->scene())
		{
			auto scene = static_cast<DiagramScene *>(item->scene());
			auto model = item->model();
			if(!model->connection()) return;
			auto conn = *model->connection();

			auto p1 = scene->item(conn.src);
			auto p2 = scene->item(conn.dst);

			if(p1 && p2)
			{
				auto srcPt = p1->scenePortLocation(conn.srcPort);
				auto dstPt = p2->scenePortLocation(conn.dstPort);

				if(srcPt.x() > dstPt.x())
				{
					std::swap(srcPt, dstPt);
				}

				view->setStart(srcPt);
				view->setEnd(dstPt);
			}

			item->updateBoundingBox();
		}

	}

	void itemAddedToScene()
	{
		dependencyPosChanged(nullptr);
	}

};
}

#include "PathConnectorComponent.moc"


namespace dbuilder {


PathConnectorComponent::PathConnectorComponent(QObject* parent)
: DiagramComponent("path-connector", parent)
{
	setHidden(true);
}

void PathConnectorComponent::configure(DiagramItem *item) const
{
	auto pi = new PathItem(item);
	pi->setParentItem(item);
	auto controller = new PathConnectorController(item, pi);
	item->setAcceptedMouseButtons(Qt::NoButton);
	item->setZValue(10);
	item->setAcceptHoverEvents(false);
	item->setFlag(QGraphicsItem::ItemIsSelectable);
	item->setFlag(QGraphicsItem::ItemIsMovable, false);
	item->setItemSnaps(false);

}

PathConnectorComponent::~PathConnectorComponent()
{
}

} /* namespace dbuilder */
