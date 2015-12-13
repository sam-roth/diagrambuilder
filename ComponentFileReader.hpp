#pragma once
/**
 * @file   ComponentFileReader.hpp
 *
 * @date   Jan 19, 2013
 * @author Sam Roth <>
 */

#include <QObject>
#include <istream>
#include "DiagramContext.hpp"
#include "CoreForward.hpp"

namespace dbuilder {



class ComponentFileReader: public QObject
{
	Q_OBJECT
public:
	ComponentFileReader(QObject *parent=nullptr);

	void read(std::istream &, DiagramContext *scene);

	virtual ~ComponentFileReader();
};


}  // namespace dbuilder
