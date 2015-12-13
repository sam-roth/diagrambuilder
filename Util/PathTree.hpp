#pragma once
#include <boost/property_tree/ptree.hpp>
#include <QPainterPath>
/**
 * @file   PathTree.hpp
 *
 * @date   Jan 28, 2013
 * @author Sam Roth <>
 */




namespace dbuilder {

namespace pt = boost::property_tree;

inline QPainterPath treeToPath(const pt::ptree &p)
{
	QPainterPath result;

	QList<QPointF> curve;

	auto finishCurve = [&]() {
		if(curve.size() == 1)
		{
			result.lineTo(curve.front());
		}
		else if(curve.size() == 2)
		{
			result.quadTo(curve.front(), curve.back());
		}
		else if(curve.size() == 3)
		{
			result.cubicTo(curve[0], curve[1], curve[2]);
		}
		else if(curve.size() != 0)
		{
			throw std::logic_error("invalid curve size");
		}

		curve.clear();
	};

	for(const pt::ptree::value_type &pv : p)
	{
		auto elementType = QPainterPath::ElementType(pv.second.get<int>("type"));
		double x = pv.second.get<double>("x");
		double y = pv.second.get<double>("y");


		if(elementType == QPainterPath::CurveToDataElement)
		{
			curve << QPointF(x, y);
		}
		else
		{
			finishCurve();
			if(elementType == QPainterPath::CurveToElement)
			{
				curve << QPointF(x, y);
			}
			else if(elementType == QPainterPath::LineToElement)
			{
				result.lineTo(x, y);
			}
			else if(elementType == QPainterPath::MoveToElement)
			{
				result.moveTo(x, y);
			}
		}
	}

	finishCurve();

	return result;
}


}  // namespace dbuilder
