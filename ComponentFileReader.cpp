/**
 * @file   ComponentFileReader.cpp
 *
 * @date   Jan 19, 2013
 * @author Sam Roth <>
 */

#include "moc_ComponentFileReader.cpp"
#include "DiagramScene.hpp"
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/info_parser.hpp>
#include <unordered_map>
#include <string>
#include <iostream>
#include <sstream>
#include <QPointF>
#include "Util/StreamOps.hpp"
#include "Components/SVGComponent.hpp"
#include <QList>

namespace dbuilder {



ComponentFileReader::ComponentFileReader(QObject* parent)
: QObject(parent)
{
}


void ComponentFileReader::read(std::istream& is, DiagramContext* scene)
{
	using boost::property_tree::ptree;
	using namespace boost::property_tree::info_parser;

	ptree pt;
	read_info(is, pt);

	std::unordered_map<std::string, ptree> abstracts;

	for(const ptree::value_type &x : pt)
	{
		if(x.first == "abstract")
		{
			abstracts[x.second.get_value<std::string>()] = x.second;
		}
		else if(x.first == "component")
		{
			ptree compTree = x.second;

			while(auto base = compTree.get_optional<std::string>("extends"))
			{
				auto baseTree = abstracts[*base];
				compTree.erase("extends");
				for(const auto &y : baseTree)
				{
					compTree.push_back(y);
				}
			}

			auto compName = x.second.get_value<std::string>();
			auto portsTree = compTree.get_child("ports");


			QList<QPointF> ports;

			for(const ptree::value_type &port : portsTree)
			{
				std::stringstream ss(port.second.get_value<std::string>());
				qreal x, y;
				ss >> x >> y;
				ports.push_back(QPointF(x, y));

			}
			if(auto optionalSvgFile = compTree.get_optional<std::string>("svg-file"))
			{
				auto svgFile = *optionalSvgFile;

				if(scene)
				{
					scene->registerKind(new SVGComponent(compName.c_str(), svgFile.c_str(), ports, scene));
				}
			}
			else
			{
				auto svgData = compTree.get<std::string>("svg-data");

				if(scene)
				{
					scene->registerKind(new SVGComponent(compName.c_str(),
					                                SVGData{svgData.c_str()},
					                                ports,
					                                scene));
				}

			}
		}
		else if(x.first == "resource-info")
		{
			// todo
		}
		else
		{
			throw std::runtime_error("unexpected key \"" + x.first + "\"");
		}
	}

}

ComponentFileReader::~ComponentFileReader()
{
}


}  // namespace dbuilder
