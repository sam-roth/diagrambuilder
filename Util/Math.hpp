#pragma once
/**
 * @file   Math.hpp
 *
 * @date   Mar 29, 2013
 * @author Sam Roth <>
 */


namespace dbuilder {

inline int modulus(int n, int b)
{
	if(b < 0)
	{
		return modulus(-n, -b);
	}
	else
	{
		const int result = n % b;
		return result >= 0? result : result + b;
	}
}

}  // namespace dbuilder



