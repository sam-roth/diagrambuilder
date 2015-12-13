#pragma once
/**
 * @file   UUIDTranslator.hpp
 *
 * @date   Jan 23, 2013
 * @author Sam Roth <>
 */
#include <QUuid>
#include <boost/property_tree/ptree.hpp>
#include <boost/algorithm/string.hpp>
#include <iostream>
namespace dbuilder {

using boost::optional;
namespace pt = boost::property_tree;

inline bool isUUID(const std::string &s)
{
	return boost::algorithm::starts_with(s, "QUuid(") && boost::algorithm::ends_with(s, ")");
}

struct UUIDTranslator
{
	typedef std::string internal_type;
	typedef QUuid       external_type;

	optional<external_type> get_value(const internal_type &str)
	{
		if(isUUID(str))
		{
			std::string uuidOnly(str.begin() + 6, str.end() - 1);
			return QUuid(uuidOnly.c_str());
		}
		else
		{
			return {};
		}
	}


	boost::optional<internal_type> put_value(const external_type &uuid)
	{
		return "QUuid(" + uuid.toString().toStdString() + ")";
	}
};

}  // namespace dbuilder


namespace boost {
namespace property_tree {

template <typename C, typename T, typename A>
struct translator_between<std::basic_string<C, T, A>, QUuid>
{
	typedef ::dbuilder::UUIDTranslator type;
};

}  // namespace property_tree
}  // namespace boost


