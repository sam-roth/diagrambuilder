#pragma once
/**
 * @file   PathComponent.hpp
 *
 * @date   Jan 27, 2013
 * @author Sam Roth <>
 */
#include "DiagramComponent.hpp"

namespace dbuilder {

class PathComponent: public DiagramComponent
{
	Q_OBJECT
public:
	PathComponent(QObject *parent=0);
	void configure(DiagramItem *) const;
	virtual ~PathComponent();
};

} /* namespace dbuilder */

