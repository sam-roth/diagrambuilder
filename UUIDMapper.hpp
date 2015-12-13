#pragma once
/**
 * @file   UUIDMapper.h
 *
 * @date   Jan 5, 2013
 * @author Sam Roth <>
 */

#include <QObject>
#include <QUuid>
#include <QMap>

namespace dbuilder {
class UUIDMapper: public QObject
{
	Q_OBJECT
	QMap<QUuid, QUuid> _uuidMap;
public:
	UUIDMapper(QObject *parent=nullptr);

	QUuid map(const QUuid &);

	virtual ~UUIDMapper();
};
} // namespace dbuilder

