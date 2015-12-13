#pragma once
/**
 * @file   ConnectorComponent.h
 *
 * @date   Jan 3, 2013
 * @author Sam Roth <>
 */

#include "../DiagramComponent.hpp"

class QGraphicsItem;

namespace dbuilder {



class ConnectorComponent: public DiagramComponent
{
	Q_OBJECT
public:
	ConnectorComponent(QObject *parent=nullptr);
	void configure(DiagramItem *) const;
	virtual ~ConnectorComponent();

	static bool selectHandle(QGraphicsItem *item);

	PropertyWidget *makePropertyWidget(QWidget *parent=nullptr) const;
};


}  // namespace dbuilder
