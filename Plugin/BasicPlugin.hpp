#pragma once
/**
 * @date   Jul 27, 2013
 * @author Sam Roth <>
 */

namespace dbuilder {

struct PluginInfo
{
	QString name;
	QString author;
	QUrl url;

	uint majorVersion, minorVersion;
};

std::ostream& operator <<(std::ostream& os, const PluginInfo& what);

class BasicPlugin
{
public:
	virtual ~BasicPlugin() { }

	virtual PluginInfo pluginInfo() const = 0;

};

}  // namespace dbuilder

Q_DECLARE_INTERFACE(dbuilder::BasicPlugin,
                    "saroth.dbuilder.BasicPlugin/1.0");


