#pragma once
#include <memory>
/**
 * @file   Util.h
 *
 * @date   Dec 29, 2012
 * @author Sam Roth <>
 */


namespace dbuilder {
template <typename T, typename... Args>
std::unique_ptr<T> make_unique(Args &&... args)
{
	std::unique_ptr<T> result(new T(std::forward<Args>(args)...));
	return result;
}
} // namespace dbuilder

namespace dbuilder {

template <typename T>
int sgn(T val)
{
	return (T(0) < val) - (val < T(0));
}


}  // namespace dbuilder

