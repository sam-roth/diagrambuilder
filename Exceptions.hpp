#pragma once
#include <stdexcept>
/**
 * @file   Exceptions.hpp
 *
 * @date   Jan 20, 2013
 * @author Sam Roth <>
 */


namespace dbuilder {

class Exception: public std::runtime_error
{
public:
	Exception(const std::string &message)
	: std::runtime_error(message)
	{ }
};

}

#define DBDeriveException(Derived, Base, defaultMessage)\
	class Derived: public Base\
	{\
	public:\
		Derived(const std::string &message=defaultMessage)\
		: Base(message)\
		{ }\
	}

#define DBDefineException(T, defaultMessage)\
		DBDeriveException(T, ::dbuilder::Exception, defaultMessage)


