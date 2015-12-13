/**
 * @date   Jul 19, 2013
 * @author Sam Roth <>
 */

#include "ComponentFile.hpp"
#include <boost/property_tree/info_parser.hpp>
#include <boost/timer.hpp>
#include "Util/Log.hpp"
#include "Main/Application.hpp"
#include "Util/Translators.hpp"
#include "DiagramComponent.hpp"
#include "Components/SVGComponent.hpp"

namespace dbuilder
{

ComponentFile::ComponentFile(std::istream& is)
{
	read(is);
}

ComponentFile::ComponentFile()
{
	pt.put("resource-info.kind", "components");
}

ComponentFile::~ComponentFile()
{
}

void ComponentFile::addComponent(const QString &name,
                                 const QString &svgData,
                                 const QList<QPointF> &ports)
{
	add(ComponentSpec{name, svgData, {}, ports});
}

void ComponentFile::setAuthor(const QString &author)
{
	pt.put("resource-info.author", author.toStdString());
}

QString ComponentFile::author() const
{
	auto result = pt.get_optional<std::string>("resource-info.author");
	return result? result->c_str() : QString();
}

void ComponentFile::add(const ComponentSpec &component)
{
	assert(!component.svgData.isNull() || !component.svgFile.isNull() && "either one must be non-null");
	assert(component.svgData.isNull() != component.svgFile.isNull()   && "may not specify both");

	boost::property_tree::ptree subtree;
	subtree.put_value(component.name.toStdString());
	if(component.svgFile.isNull())
	{
		subtree.put("svg-data", component.svgData.toStdString());
	}
	else
	{
		subtree.put("svg-file", component.svgFile.toStdString());
	}

	boost::property_tree::ptree portTree;
	int i = 0;
	for(const auto &port : component.ports)
	{
		std::stringstream ss;
		ss << port.x() << ' ' << port.y();
		portTree.put(boost::lexical_cast<std::string>(i), ss.str());
		++i;
	}

	subtree.put_child("ports", portTree);
	pt.add_child("component", subtree);
}

QList<ComponentSpec> ComponentFile::components() const
{
	auto range = pt.equal_range("component");
	QList<ComponentSpec> result;

	for(auto it = range.first, end = range.second; it != end; ++it)
	{
		result << this->component(it->second);
	}

	return result;
}

void ComponentFile::write(std::ostream &os) const
{
	boost::property_tree::info_parser::write_info(os, pt);
}

ComponentSpec ComponentFile::component(const boost::property_tree::ptree &subtreeRef) const
{
	ComponentSpec result;
	static const boost::property_tree::ptree EmptyDefault;

	boost::property_tree::ptree subtree = subtreeRef;

	if(auto extends = subtree.get_optional<std::string>("extends"))
	{
		if(auto abstr = findAbstract(*extends))
		{
			for(const auto &kv : *abstr)
			{
				if(!subtree.count(kv.first))
				{
					subtree.put_child(kv.first, kv.second);
				}
			}
		}
		else
		{
			DBError("No such abstract component: ", *extends);
		}
	}

	result.name = QString::fromStdString(subtree.get_value<std::string>());
	DBDebug("Loading component ", result.name);

	result.svgData = subtree.get("svg-data", QString());
	result.svgFile = subtree.get("svg-file", QString());

	if(result.svgData.isNull() && result.svgFile.isNull())
	{
		std::stringstream ss;
		boost::property_tree::info_parser::write_info(ss, subtree);
		DBWarning("Read underspecified component: ", ss.str());
	}


	const auto &portTree = subtree.get_child("ports", EmptyDefault);

	for(const auto &port : portTree)
	{
		result.ports << port.second.get_value<QPointF>(SimplePointFTranslator());
	}


	return result;
}

void ComponentFile::read(std::istream &is)
{
	boost::property_tree::read_info(is, pt);
}

boost::optional<const boost::property_tree::ptree&> ComponentFile::findAbstract(const std::string& name) const
{
	auto range = pt.equal_range("abstract");
	for(auto it = range.first, end = range.second; it != end; ++it)
	{
		if(it->second.get_value<std::string>() == name)
		{
			return it->second;
		}
	}

	return {};
}

DiagramComponent *ComponentSpec::createKind(QObject *parent) const
{
	if(!svgData.isNull())
	{
		return new SVGComponent(name, SVGData{svgData}, ports, parent);
	}
	else if(!svgFile.isNull())
	{
		return new SVGComponent(name, svgFile, ports, parent);
	}
	else
	{
		assert(false && "either svgData or svgFile must be specified");
	}
}


} /* namespace dbuilder */
