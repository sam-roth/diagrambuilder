#pragma once
#include <iostream>
#include <string>
#include <sstream>
#include "StreamOps.hpp"
#include <boost/optional.hpp>

/**
 * @file   Log.hpp
 *
 * @date   Feb 1, 2013
 * @author Sam Roth <>
 */

namespace dbuilder {

inline void writecat(std::ostream &os) { }

template <typename First, typename... Rest>
void writecat(std::ostream &os, const First &f, const Rest &... r)
{
	os << f;
	writecat(os, r...);
}


namespace log {

enum Level
{
	Debug=0, Info, Warning, Error, Critical, NoLogging
};


void setLevel(Level);
Level level();

// lowest level at which backtrace should occur
// default = NoLogging
void setBacktraceLevel(Level);
Level backtraceLevel();

const char *levelName(Level);
boost::optional<Level> levelForName(const std::string &name);
size_t numberOfLevels();

void write(Level, const char *file, int line, const std::string &message);

template <typename... Args>
void writev(Level l, const char *file, int line, const Args &... args)
{
	std::stringstream ss;
	writecat(ss, args...);
	write(l, file, line, ss.str());
}

}  // namespace log
}  // namespace dbuilder


#define DBLog(level, msg...)\
	::dbuilder::log::writev(::dbuilder::log:: level, __FILE__, __LINE__, msg)

#define DBDebug(msg...) DBLog(Debug, msg)
#define DBInfo(msg...) DBLog(Info, msg)
#define DBWarning(msg...) DBLog(Warning, msg)
#define DBError(msg...) DBLog(Error, msg)
#define DBCritical(msg...) DBLog(Critical, msg)

#define DBCountArgsAux(a9, a8, a7, a6, a5, a4, a3, a2, a1, a0, rest...) a0
#define DBCountArgs(x...) DBCountArgsAux(x, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0)
#define DBCommaForeach5(m, s, f, r...) m(f), s DBCommaForeach4(m, s, r)
#define DBCommaForeach4(m, s, f, r...) m(f), s DBCommaForeach3(m, s, r)
#define DBCommaForeach3(m, s, f, r...) m(f), s DBCommaForeach2(m, s, r)
#define DBCommaForeach2(m, s, f, r...) m(f), s DBCommaForeach1(m, s, r)
#define DBCommaForeach1(m, s, f) m(f)
#define DBCommaForeach0(m, s)
#define DBCatAux(m, n) m ## n
#define DBCat(m, n) DBCatAux(m, n)
#define DBCommaForeach(m, s, args...) DBCat(DBCommaForeach, DBCountArgs(args))(m, s, args)
#define DBDump1(x) #x "=", x
#define DBDump(rest...) DBCommaForeach(DBDump1, ", ", rest)

