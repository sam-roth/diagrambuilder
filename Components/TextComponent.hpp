#pragma once
#include "DiagramComponent.hpp"
#include "Plugin/DiagramComponentPlugin.hpp"
/**
 * @file   TextComponent.h
 *
 * @date   Jan 5, 2013
 * @author Sam Roth <>
 */

namespace dbuilder {
class TextComponent: public DiagramComponent
{
	Q_OBJECT
public:
	TextComponent(QObject *parent=nullptr);
	void configure(DiagramItem *) const;

	PropertyWidget *makePropertyWidget(QWidget *parent=nullptr) const;

	virtual ~TextComponent();
};

class TextComponentPlugin: public QObject, public DiagramComponentPlugin
{
	Q_OBJECT
	Q_INTERFACES(dbuilder::DiagramComponentPlugin: dbuilder::BasicPlugin)
public:
	PluginInfo pluginInfo() const;
	QList<DiagramComponent *> createComponents(QObject *parent=0);

};

} // namespace dbuilder

