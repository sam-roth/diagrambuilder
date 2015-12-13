/**
 * @file   Demangle.cpp
 *
 * @date   Mar 30, 2013
 * @author Sam Roth <>
 */
#include <cxxabi.h>
#include <cstdlib>
#include "Util/ScopeExit.hpp"
#include "Util/Demangle.hpp"

namespace dbuilder {

std::string demangle(const char *symbol)
{
	int status;
	char *dmresult = nullptr;
	DBScopeExit([&]() { std::free(dmresult); });

	dmresult = abi::__cxa_demangle(symbol, nullptr, nullptr, &status);
	if(status != 0)
	{
		return symbol;
	}
	else
	{
		std::string result = dmresult;
		return result;
	}

}

}  // namespace dbuilder
