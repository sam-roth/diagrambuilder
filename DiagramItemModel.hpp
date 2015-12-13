#pragma once
#include <QObject>
#include <boost/property_tree/ptree.hpp>
#include <QUuid>
#include <QSet>
#include "DiagramContext.hpp"
#include <QPoint>
#include "CoreForward.hpp"
#include "Util/ReentrancyGuard.hpp"
/**
 * @file   DiagramItemModel.hpp
 *
 * @date   Jan 23, 2013
 * @author Sam Roth <>
 */


namespace dbuilder {
struct Connection
{
	QUuid src;
	int srcPort;
	QUuid dst;
	int dstPort;
	double center;
};
namespace pt = boost::property_tree;
using boost::optional;

class DiagramItemModel: public QObject
{
	Q_OBJECT
	DiagramContext *_ctx;
	QUuid _uuid;
	const DiagramComponent *_kind;
	QPointF _scenePos;
	qreal _sceneZ;
	QSet<QUuid> _dependencies;
	double _rotation;
	pt::ptree _extraData;
	ReentrancyGuard _updateViewGuard, _updateModelGuard;
public:
	DiagramItemModel(DiagramContext *ctx, const pt::ptree::value_type &data, QObject *parent=nullptr);
	DiagramItemModel(DiagramContext *ctx, QObject *parent=nullptr);
	virtual ~DiagramItemModel();

	void save(pt::ptree &dst) const;
	DiagramItemModel *clone(UUIDMapper *mapper, QObject *parent=nullptr) const;

	boost::optional<Connection> connection() const;
	boost::optional<QString> text() const;

	void setConnection(const optional<Connection> &);
	void setText(const optional<QString> &text);

	DiagramContext *ctx() const
	{
		return _ctx;
	}

	const DiagramComponent *kind() const
	{
		return _kind;
	}

	const QPointF& scenePos() const
	{
		return _scenePos;
	}

	const QUuid& uuid() const
	{
		return _uuid;
	}

	double rotation() const
	{
		return _rotation;
	}

	void addDependency(const QUuid &dep)
	{
		_dependencies.insert(dep);
	}

	void removeDependency(const QUuid &dep)
	{
		_dependencies.remove(dep);
	}

	const QSet<QUuid> &dependencies() const
	{
		return _dependencies;
	}

	template <typename T>
	void setData(const pt::ptree::path_type &path, T &&value)
	{
		_extraData.put(path, std::forward<T>(value));
	}


	void removeData(const std::string &key)
	{
		_extraData.erase(key);
	}

	template <typename T>
	optional<T> getData(const pt::ptree::path_type &path) const
	{
		return _extraData.get_optional<T>(path);
	}

	template <typename Tree>
	void setTree(const pt::ptree::path_type &path, Tree &&tree)
	{
		_extraData.put_child(path, std::forward<Tree>(tree));
	}

	optional<const pt::ptree &> getTree(const pt::ptree::path_type &path) const
	{
		return _extraData.get_child_optional(path);
	}


public slots:
	void setKind(const DiagramComponent *kind)
	{
		_kind = kind;
	}

	void setScenePos(const QPointF& scenePos)
	{
		_scenePos = scenePos;
	}

	void setUuid(const QUuid& uuid)
	{
		_uuid = uuid;
	}

	void setRotation(double rotation)
	{
		_rotation = rotation;
	}
signals:
	/**
	 * signal: request that the view re-reads the model
	 */
	void updateViewRequested();
	/**
	 * signal: request that the view commit its changes to the model
	 */
	void updateModelRequested();
public slots:
	void requestUpdateView();
	void requestUpdateModel();

	qreal sceneZ() const
	{
		return _sceneZ;
	}

	void setSceneZ(qreal sceneZ)
	{
		_sceneZ = sceneZ;
	}
};
} /* namespace dbuilder */

