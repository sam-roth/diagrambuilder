#pragma once
/**
 * @file   ConfDiagramLoader.h
 *
 * @date   Jan 17, 2013
 * @author Sam Roth <>
 */

#include <QObject>
#include "DiagramLoader.hpp"
#include "DiagramItemModel.hpp"
#include "CoreForward.hpp"


namespace dbuilder {
class InfoDiagramLoader: public QObject, public DiagramLoader
{
	Q_OBJECT
	DiagramContext *_ctx;
public:
	InfoDiagramLoader(DiagramContext *ctx, QObject *parent=nullptr);

	virtual QList<DiagramItem *> load(std::istream &, DiagramScene *) const;
	virtual void save(std::ostream &, const QList<DiagramItem *> &) const;

	virtual QList<DiagramItemModel *> loadModels(std::istream &, QObject *parent=nullptr) const;
	virtual void saveModels(std::ostream &, const QList<DiagramItemModel *> &) const;

	virtual ~InfoDiagramLoader();
};
} // namespace dbuilder

