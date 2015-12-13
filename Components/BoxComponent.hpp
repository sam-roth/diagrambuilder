#pragma once
/**
 * @file   BoxComponent.hpp
 *
 * @date   Mar 7, 2013
 * @author Sam Roth <>
 */

#include "DiagramComponent.hpp"

namespace dbuilder
{

class BoxComponent: public DiagramComponent
{
	Q_OBJECT
public:
	BoxComponent(QObject *parent=nullptr);
	void configure(DiagramItem *) const;
	virtual ~BoxComponent();

	PropertyWidget *makePropertyWidget(QWidget *parent=nullptr) const;
};

} /* namespace dbuilder */
