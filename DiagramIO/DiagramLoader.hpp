#pragma once
#include <iostream>
#include <QList>
#include "CoreForward.hpp"
/**
 * @file   DiagramLoader.h
 *
 * @date   Jan 17, 2013
 * @author Sam Roth <>
 */

class QObject;

namespace dbuilder {
class DiagramLoader
{
public:
	virtual QList<DiagramItem *> load(std::istream &, DiagramScene *) const = 0;
//	virtual QList<DiagramItem *> load(std::istream &, DiagramScene *, UUIDMapper *) const = 0;
	virtual void save(std::ostream &, const QList<DiagramItem *> &) const = 0;

	virtual QList<DiagramItemModel *> loadModels(std::istream &, QObject *parent=nullptr) const = 0;
	virtual void saveModels(std::ostream &, const QList<DiagramItemModel *> &) const = 0;

	virtual ~DiagramLoader() { }
};
} // namespace dbuilder
