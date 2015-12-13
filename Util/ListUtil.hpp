#pragma once
#include <QList>
/**
 * @file   ListUtil.hpp
 *
 * @date   Jan 21, 2013
 * @author Sam Roth <>
 */


namespace dbuilder {
template <typename Dst, typename Src>
QList<Dst *> filterCast(const QList<Src *> &l)
{
	QList<Dst *> results;
	for(auto item : l)
	{
		if(auto castedItem = dynamic_cast<Dst *>(item))
		{
			results << castedItem;
		}
	}

	return results;
}

template <typename Dst, typename Src>
QList<Dst> listCast(const QList<Src> &l)
{
	QList<Dst> results;
	for(auto item : l) results << static_cast<Dst>(item);
	return results;
}

} // namespace dbuilder

