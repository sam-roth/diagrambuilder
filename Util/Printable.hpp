#pragma once
#include <string>
#include <ostream>
#include <typeinfo>
#include <sstream>
#include "Util/Demangle.hpp"
/**
 * @file   Printable.hpp
 *
 * @date   Mar 30, 2013
 * @author Sam Roth <>
 */



namespace dbuilder {

class Printable
{
public:
	virtual void print(std::ostream &os) const = 0;
	virtual std::string str() const;

	virtual ~Printable();

	template <typename T>
	static std::string str(T *p)
	{
		if(auto printable = dynamic_cast<const Printable *>(p))
		{
			return printable->str();
		}
		else
		{
			std::stringstream res;
			res << p;
			return res.str();
		}
	}


	template <typename T>
	static std::string repr(T *p)
	{
		if(auto printable = dynamic_cast<Printable *>(p))
		{
			std::stringstream ss;
			ss << demangle(typeid(*p).name()) << "{ ";
			printable->print(ss);
			ss << " }";
			return ss.str();
		}
		else
		{
			std::stringstream ss;
			ss << "<" << demangle(typeid(*p).name()) << " @ " << p << ">";
			return ss.str();
		}
	}
};


}  // namespace dbuilder

