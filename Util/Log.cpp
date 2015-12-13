/**
 * @file   Log.cpp
 *
 * @date   Feb 1, 2013
 * @author Sam Roth <>
 */

#include "Log.hpp"
#include <boost/date_time.hpp>
#include <map>
#include "Util/Optional.hpp"
#include "Util/Backtrace.hpp"
#include <boost/algorithm/string.hpp>


namespace dbuilder {
namespace log {
namespace {

Level logLevel = Info;
Level _backtraceLevel = NoLogging;

const char *levelNames[] = {
	"Debug",
	"Info",
	"Warning",
	"Error",
	"Critical",
	"NoLogging"
};


}  // anonymous namespace


Level level()
{
	return logLevel;
}

void setLevel(Level l)
{
	logLevel = l;
	DBInfo("Log level set to ", levelName(logLevel));
}

const char *levelName(Level l)
{
	return levelNames[l];
}

static std::string relativeName(const std::string &n)
{
	static const std::string filename = __FILE__;
	static const size_t pathLength = std::strlen("Util/Log.cpp");
	static const std::string prefix = filename.substr(0, filename.size() - pathLength);

	if(boost::algorithm::starts_with(n, prefix))
	{
		return n.substr(prefix.size(), n.size() - prefix.size());
	}
	else
	{
		return n;
	}
}

boost::optional<Level> levelForName(const std::string& name)
{
	static std::map<std::string, Level> levelsForName = {
		{"Debug", Debug},
		{"Info", Info},
		{"Warning", Warning},
		{"Error", Error},
		{"Critical", Critical},
		{"NoLogging", NoLogging}
	};

	return getCopy(levelsForName, name);
}

size_t numberOfLevels()
{
	return 5;
}

void write(Level l, const char *file, int line, const std::string &message)
{
	if(l >= level())
	{
		auto time = boost::posix_time::second_clock::local_time();
		std::clog << "[" << levelName(l) << "|"
			<< boost::posix_time::to_simple_string(time) << "|"
			<< relativeName(file) << ":" << line << "] "
			<< message << std::endl;
	}
	if(l >= backtraceLevel())
	{
		std::clog << "  ";
		sroth::printBacktrace(3, std::clog);
	}
}

void setBacktraceLevel(Level level)
{
	_backtraceLevel = level;
}

Level backtraceLevel()
{
	return _backtraceLevel;
}


}  // namespace log
}  // namespace dbuilder

