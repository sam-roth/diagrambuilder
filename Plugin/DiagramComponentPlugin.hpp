#pragma once
#include "CoreForward.hpp"
#include "Plugin/BasicPlugin.hpp"
/**
 * @date   Jul 27, 2013
 * @author Sam Roth <>
 */


namespace dbuilder {

class DiagramComponentPlugin: public BasicPlugin
{
public:
	virtual ~DiagramComponentPlugin() { }
	virtual QList<DiagramComponent *> createComponents(QObject *parent=0) = 0;
};

}  // namespace dbuilder

Q_DECLARE_INTERFACE(dbuilder::DiagramComponentPlugin,
                    "saroth.dbuilder.DiagramComponentPlugin/1.0");

