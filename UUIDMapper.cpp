/**
 * @file   UUIDMapper.cpp
 *
 * @date   Jan 5, 2013
 * @author Sam Roth <>
 */

#include "UUIDMapper.hpp"


namespace dbuilder {
UUIDMapper::UUIDMapper(QObject* parent)
: QObject(parent)
{
}

QUuid UUIDMapper::map(const QUuid &oldUUID)
{
	auto it = _uuidMap.find(oldUUID);
	if(it == _uuidMap.end())
	{
		auto newUUID = QUuid::createUuid();
		_uuidMap.insert(oldUUID, newUUID);
		return newUUID;
	}
	else
	{
		return *it;
	}
}

UUIDMapper::~UUIDMapper()
{
}
} // namespace dbuilder

