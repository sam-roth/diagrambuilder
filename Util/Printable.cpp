/**
 * @file   Printable.cpp
 *
 * @date   Mar 30, 2013
 * @author Sam Roth <>
 */
#include "Printable.hpp"

namespace dbuilder {


std::string Printable::str() const
{
	std::stringstream ss;
	print(ss);
	return ss.str();
}


Printable::~Printable()
{
}


}  // namespace dbuilder
