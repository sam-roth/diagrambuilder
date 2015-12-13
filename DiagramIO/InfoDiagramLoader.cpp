/**
 * @file   ConfDiagramLoader.cpp
 *
 * @date   Jan 17, 2013
 * @author Sam Roth <>
 */

#include "moc_InfoDiagramLoader.cpp"
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/info_parser.hpp>
#include "DiagramItem.hpp"
#include "DiagramScene.hpp"
#include "DiagramItemModel.hpp"
#include "UUIDMapper.hpp"
#include "DiagramComponent.hpp"

using boost::property_tree::ptree;
using namespace boost::property_tree::info_parser;

namespace dbuilder {
InfoDiagramLoader::InfoDiagramLoader(DiagramContext *ctx, QObject* parent)
: QObject(parent)
, _ctx(ctx)
{
}

QList<DiagramItem *> InfoDiagramLoader::load(std::istream &is, DiagramScene *scene) const
{
	QList<DiagramItem *> results;

	ptree pt;
	read_info(is, pt);

	for(const auto &pv : pt)
	{
		auto model = new DiagramItemModel(scene->context(), pv);
		auto item = model->kind()->createFromModel(model);
		scene->addDiagramItem(item);

		results << item;
	}

	return results;
}



void InfoDiagramLoader::save(std::ostream &os, const QList<DiagramItem *> &items) const
{
	ptree pt;
	for(auto item : items)
	{
		item->model()->save(pt);
	}

	write_info(os, pt);
}



QList<DiagramItemModel *> InfoDiagramLoader::loadModels(std::istream &is, QObject *parent) const
{
	ptree pt;
	read_info(is, pt);

	QList<DiagramItemModel *> results;
	for(const ptree::value_type &pv : pt)
	{
		auto model = new DiagramItemModel(_ctx, pv, parent);

		results << model;
	}

	return results;
}

void InfoDiagramLoader::saveModels(std::ostream &os, const QList<DiagramItemModel *> &models) const
{
	ptree pt;
	for(auto model : models)
	{
		model->save(pt);
	}

	write_info(os, pt);
}

InfoDiagramLoader::~InfoDiagramLoader()
{
}
} // namespace dbuilder

