#pragma once
/**
 * @file   Optional.hpp
 *
 * @date   Mar 14, 2013
 * @author Sam Roth <>
 */

#include <boost/optional.hpp>
#include <map>
#include <QMap>
#include <unordered_map>

namespace dbuilder {

template <typename T>
struct MapTraits;

template <typename Map, typename K, typename V>
struct BasicStdMapTraits
{
	static boost::optional<const V &> get(const Map &m, const K &k)
	{
		auto it = m.find(k);
		if(it != m.end()) return it->second;
		else return {};
	}

	static boost::optional<V &> get(Map &m, const K &k)
	{
		auto it = m.find(k);
		if(it != m.end()) return it->second;
		else return {};
	}

	typedef K Key;
	typedef V Value;
};

template <typename K, typename V>
struct MapTraits<std::map<K, V>>
: public BasicStdMapTraits<std::map<K, V>, K, V> { };

template <typename K, typename V>
struct MapTraits<std::unordered_map<K, V>>
: public BasicStdMapTraits<std::unordered_map<K, V>, K, V> { };

template <typename K, typename V>
struct MapTraits<QMap<K, V>>
{
	typedef K Key;
	typedef V Value;
	typedef QMap<K, V> Map;

	static boost::optional<const V &> get(const Map &m, const K &k)
	{
		auto it = m.find(k);
		if(it != m.end()) return *it;
		else return {};
	}

	static boost::optional<V &> get(Map &m, const K &k)
	{
		auto it = m.find(k);
		if(it != m.end()) return *it;
		else return {};
	}
};

/**
 * Get a reference to a map entry wrapped in boost::optional<>
 * @param m   The map in which to search
 * @param k   The key for which to search
 * @return    A boost::optional<> containing the reference iff k is in m.
 */
template <typename Map, typename K=typename MapTraits<Map>::Key, typename V=typename MapTraits<Map>::Value>
boost::optional<V &> get(Map &m, const K &k)
{
	return MapTraits<Map>::get(m, k);
}
/**
 * Get a const reference to a map entry wrapped in boost::optional<>
 * @param m   The map in which to search
 * @param k   The key for which to search
 * @return    A boost::optional<> containing the reference iff k is in m.
 */
template <typename Map, typename K=typename MapTraits<Map>::Key, typename V=typename MapTraits<Map>::Value>
boost::optional<const V &> get(const Map &m, const K &k)
{
	return MapTraits<Map>::get(m, k);
}


/**
 * Get a copy of a map entry wrapped in boost::optional<>
 * @param m   The map in which to search
 * @param k   The key for which to search
 * @return    A boost::optional<> containing the value iff k is in m.
 */
template <typename Map, typename K=typename MapTraits<Map>::Key, typename V=typename MapTraits<Map>::Value>
boost::optional<V> getCopy(const Map &m, const K &k)
{
	if(auto result = get(m, k))
	{
		return *result;
	}
	else
	{
		return {};
	}
}

}  // namespace dbuilder


