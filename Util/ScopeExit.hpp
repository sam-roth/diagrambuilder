#pragma once
#include <functional>
/**
 * @file   ScopeExit.h
 *
 * @date   Jan 16, 2013
 * @author Sam Roth <>
 */


namespace dbuilder {
class ScopeExitImpl
{
	std::function<void()> f;
public:
	ScopeExitImpl(std::function<void()> f)
	: f(f) { }

	~ScopeExitImpl()
	{
		f();
	}
};
} // namespace dbuilder


#define DBConcatAux(a, b) a ## b
#define DBConcat(a, b) DBConcatAux(a, b)
#define DBScopeExit(f...) ::dbuilder::ScopeExitImpl DBConcat(_scopeExit, __COUNTER__)(f)



