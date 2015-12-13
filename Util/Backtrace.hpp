#pragma once
#include <string>
#include <cxxabi.h>
#include <wordexp.h>
#include <execinfo.h>
#include <dlfcn.h>
#include <exception>
#include <iostream>
#include <iomanip>
#include <typeinfo>
#include <signal.h>
#include <unistd.h>

/**
 * @date   Jul 1, 2013
 * @author Sam Roth <>
 */

namespace sroth {

inline std::string demangle(const char *name)
{
	int ok;

	char *buff = abi::__cxa_demangle(name, nullptr, nullptr, &ok);
	if(ok == 0)
	{
		std::string result = buff;
		std::free(buff);
		return result;
	}
	else
	{
		return name;
	}
}

inline std::string elideTemplateArgs(const std::string &str)
{
	std::string result;

	int openBracketCount = 0;
	for(char c : str)
	{
		if(c == '>')
		{
			--openBracketCount;
		}

		if(!openBracketCount)
		{
			result.push_back(c);
		}

		if(c == '<')
		{
			openBracketCount++;
		}
	}

	return result;
}

inline std::string elidedDemangle(const char *name)
{
	return elideTemplateArgs(demangle(name));
}


inline void printBacktrace(int omitFrames=1, std::ostream &stream=std::cerr)
{
	void *buffer[128];
	int n = backtrace(buffer, 128);

	stream << "backtrace:\n";

	for(int i = omitFrames; i < n; ++i)
	{
		Dl_info info;
		int rv = dladdr(buffer[i], &info);
		if(rv)
		{
			stream << "    " << std::setw(4) << i << " " << demangle(info.dli_sname)  << "[" << buffer[i] << "]"<< std::endl;
		}
		else
		{
			stream << "    " << std::setw(4) << i  << " " << buffer[i] << std::endl;
		}
	}
}

inline std::string backtraceAsString(int omitFrames=1)
{
	std::stringstream ss;
	printBacktrace(omitFrames+1, ss);
	return ss.str();
}

inline void terminateHandler()
{
	static bool terminateCalled = false;

	if(!terminateCalled)
	{
		terminateCalled = true;
		try
		{
			throw;
		}
		catch(std::exception &exc)
		{
			std::cerr << "terminate() called with " << demangle(typeid(exc).name()) <<
					"\n    what(): " << exc.what() << std::endl;
		}
		catch(...)
		{
			std::cerr << "terminate() called with an unknown exception" << std::endl;
		}
	}
	else
	{
		// we got back here by trying to throw an exception where there wasn't one
		std::cerr << "terminate() called without an exception" << std::endl;
	}

	printBacktrace(1, std::cerr);

	raise(SIGKILL); // suppress crash reporter
}


}  // namespace sroth
