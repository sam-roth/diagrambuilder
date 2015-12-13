#pragma once
/**
 * @file   Levenshtein.hpp
 *
 * @date   Mar 13, 2013
 * @author Sam Roth <>
 */

#include <boost/range.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/tuple/tuple.hpp>
#include <map>
namespace dbuilder {

struct LevenshteinDistanceMemoEntry
{
	size_t lenS, lenT;

	bool operator <(const LevenshteinDistanceMemoEntry &that) const
	{
		return this->lenS < that.lenS || (this->lenS == that.lenS && this->lenT < that.lenT);
	}
};

typedef std::map<LevenshteinDistanceMemoEntry, size_t> LevenshteinDistanceMemo;
template <typename I>
inline size_t levenshteinDistance(I si, I se, I ti, I te, LevenshteinDistanceMemo &ldm)
{
	const size_t lenS = se - si;
	const size_t lenT = te - ti;

	auto memoIt = ldm.find(LevenshteinDistanceMemoEntry{lenS, lenT});
	if(memoIt != ldm.end())
	{
		return memoIt->second;
	}

	if(lenS == 0) return lenT;
	if(lenT == 0) return lenS;

	const size_t cost = (se[-1] == te[-1])? 0 : 1;

	auto result = std::min<size_t>({
		levenshteinDistance(si, se - 1, ti, te, ldm) + 1,
		levenshteinDistance(si, se, ti, te - 1, ldm) + 1,
		levenshteinDistance(si, se - 1, ti, te - 1, ldm) + cost
	});


//	std::tuple<size_t, size_t> key(lenS, lenT);
	ldm[LevenshteinDistanceMemoEntry{lenS, lenT}] = result;
	return result;
}

template <typename T, typename I=typename boost::range_const_iterator<T>::type>
inline size_t levenshteinDistance(const T &s, const T &t)
{
	LevenshteinDistanceMemo ldm;
	return levenshteinDistance<I>(std::begin(s), std::end(s), std::begin(t), std::end(t), ldm);
}

}  // namespace dbuilder



