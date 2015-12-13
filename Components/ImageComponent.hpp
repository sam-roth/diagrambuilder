#pragma once
/**
 * @date   Jul 19, 2013
 * @author Sam Roth <>
 */

#include "DiagramComponent.hpp"
#include "Plugin/DiagramComponentPlugin.hpp"

namespace dbuilder
{

class ImageComponent: public DiagramComponent
{
	Q_OBJECT
public:
	ImageComponent(QObject *parent=nullptr);
	void configure(DiagramItem *) const;
	PropertyWidget *makePropertyWidget(QWidget *parent) const;

	virtual ~ImageComponent();
};

class ImageComponentPlugin: public QObject, public DiagramComponentPlugin
{
	Q_OBJECT
	Q_INTERFACES(dbuilder::DiagramComponentPlugin: dbuilder::BasicPlugin)
public:
	PluginInfo pluginInfo() const;
	QList<DiagramComponent *> createComponents(QObject *parent=0);

};


} /* namespace dbuilder */
