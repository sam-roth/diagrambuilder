/**
 * @file   DiagramItemModel.cpp
 *
 * @date   Jan 23, 2013
 * @author Sam Roth <>
 */

#include "moc_DiagramItemModel.cpp"
#include <cassert>
#include "Util/UUIDTranslator.hpp"
#include "DiagramComponent.hpp"
#include <boost/lexical_cast.hpp>
#include "UUIDMapper.hpp"
#include <boost/property_tree/info_parser.hpp>
#include <iostream>

namespace dbuilder {


DiagramItemModel::DiagramItemModel(DiagramContext *ctx, const pt::ptree::value_type &data, QObject *parent)
: QObject(parent)
, _ctx(ctx)
{
	UUIDTranslator ut;
	_uuid = ut.get_value(data.first).get();

	auto kindName = data.second.get<std::string>("kind");
	setKind(ctx->kind(kindName.c_str()));

	_scenePos.setX(data.second.get("scenePosX", 0.0));
	_scenePos.setY(data.second.get("scenePosY", 0.0));
	_rotation = data.second.get("rotation", 0.0);
	_sceneZ = data.second.get("sceneZ", 0.0);

	for(const pt::ptree::value_type &pv : data.second.get_child("dependencies"))
	{
		addDependency(pv.second.get_value<QUuid>());
	}

	_extraData = data.second.get_child("extraData");
}

DiagramItemModel::DiagramItemModel(DiagramContext *ctx, QObject *parent)
: QObject(parent)
, _ctx(ctx)
, _uuid(QUuid::createUuid())
, _kind(nullptr)
, _rotation(0)
, _sceneZ(0)
{

}

boost::optional<Connection> DiagramItemModel::connection() const
{
	if(auto connTree = getTree("connection"))
	{
		Connection conn {
			connTree->get<QUuid>("src"),
			connTree->get<int>("srcPort"),
			connTree->get<QUuid>("dst"),
			connTree->get<int>("dstPort"),
			connTree->get("center", 0.5)
		};

		return conn;
	}
	else
	{
		return {};
	}
}

boost::optional<QString> DiagramItemModel::text() const
{
	if(auto resultString = getData<std::string>("text"))
	{
		return QString(resultString->c_str());
	}
	else
	{
		return {};
	}
}

void DiagramItemModel::setConnection(const optional<Connection> &conn)
{
	if(conn)
	{
		pt::ptree connTree;
		connTree.put("src", conn->src);
		connTree.put("srcPort", conn->srcPort);
		connTree.put("dst", conn->dst);
		connTree.put("dstPort", conn->dstPort);
		connTree.put("center", conn->center);
		setTree("connection", connTree);

		this->addDependency(conn->src);
		this->addDependency(conn->dst);
	}
	else
	{
		this->_extraData.erase("connection");
	}
}
void DiagramItemModel::setText(const optional<QString> &text)
{
	if(text)
	{
		setData("text", text->toStdString());
	}
}

DiagramItemModel::~DiagramItemModel()
{

}

void DiagramItemModel::save(pt::ptree &dst) const
{
	assert(_kind);
	const_cast<DiagramItemModel *>(this)->requestUpdateModel();

	pt::ptree result;
	result.add("kind", kind()->name().toStdString());
	result.add("scenePosX", scenePos().x());
	result.add("scenePosY", scenePos().y());
	result.add("rotation", rotation());
	result.add("sceneZ", sceneZ());

	pt::ptree deptree;
	int i = 0;
	for(const auto &dep : dependencies())
	{
		deptree.add(boost::lexical_cast<std::string>(i), dep);
		++i;
	}

	result.add_child("dependencies", deptree);
	result.add_child("extraData", _extraData);

	UUIDTranslator ut;
	dst.add_child(ut.put_value(uuid()).get(), result);
}

static void mapUUIDs(UUIDMapper *mapper, pt::ptree &tree)
{
	if(auto val = tree.get_value_optional<QUuid>())
	{
		tree.put_value(mapper->map(*val));
	}

	for(pt::ptree::value_type &pv : tree)
	{
		mapUUIDs(mapper, pv.second);
	}
}

DiagramItemModel *DiagramItemModel::clone(UUIDMapper *mapper, QObject *parent) const
{
	const_cast<DiagramItemModel *>(this)->requestUpdateModel();
	DiagramItemModel *result = new DiagramItemModel(_ctx, parent);
	result->_uuid = mapper->map(_uuid);
	result->_kind = _kind;
	result->_scenePos = _scenePos;
	result->_sceneZ = _sceneZ;
	for(const auto &dep : _dependencies)
	{
		result->_dependencies.insert(mapper->map(dep));
	}
	result->_rotation = _rotation;
	result->_extraData = _extraData;

	mapUUIDs(mapper, result->_extraData);

	return result;
}

void DiagramItemModel::requestUpdateView()
{
	_updateViewGuard.enter([&]() { emit updateViewRequested(); });
}

void DiagramItemModel::requestUpdateModel()
{
	_updateModelGuard.enter([&]() { emit updateModelRequested(); });
}
} /* namespace dbuilder */
