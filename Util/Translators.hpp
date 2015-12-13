#pragma once
/**
 * @date   Jul 21, 2013
 * @author Sam Roth <>
 */


namespace dbuilder {

struct StringTranslator
{
	typedef std::string internal_type;
	typedef QString     external_type;

	boost::optional<external_type> get_value(const internal_type &str)
	{
		return QString::fromStdString(str);
	}


	boost::optional<internal_type> put_value(const external_type &str)
	{
		return str.toStdString();
	}
};


struct SimplePointFTranslator
{
	typedef std::string  internal_type;
	typedef QPointF      external_type;

	boost::optional<external_type> get_value(const internal_type &str)
	{
		qreal x, y;
		std::stringstream ss;
		ss.str(str);
		if(ss >> x >> y)
		{
			return QPointF(x, y);
		}
		else
		{
			return {};
		}
	}

	boost::optional<internal_type> put_value(const external_type &point)
	{
		std::stringstream ss;
		ss << point.x() << ' ' << point.y();
		return ss.str();
	}

};

}  // namespace dbuilder

namespace boost {
namespace property_tree {

template <typename C, typename T, typename A>
struct translator_between<std::basic_string<C, T, A>, QString>
{
	typedef ::dbuilder::StringTranslator type;
};

}  // namespace property_tree
}  // namespace boost
