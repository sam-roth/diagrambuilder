#pragma once
/**
 * @file   PathConnectorComponent.hpp
 *
 * @date   Jan 28, 2013
 * @author Sam Roth <>
 */

#include "DiagramComponent.hpp"

namespace dbuilder
{

class PathConnectorComponent: public DiagramComponent
{
public:
	PathConnectorComponent(QObject *parent=0);
	void configure(DiagramItem *) const;
	virtual ~PathConnectorComponent();
};

} /* namespace dbuilder */
