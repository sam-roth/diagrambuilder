#pragma once
#include <QObject>
#include "DataPtr.hpp"
#include "Exceptions.hpp"
#include <QString>
#include <QMap>
#include "CoreForward.hpp"

/**
 * @file   DiagramContext.hpp
 *
 * @date   Jan 20, 2013
 * @author Sam Roth <>
 */


namespace dbuilder {



DBDefineException(KindAlreadyExistsException, "the kind of diagram item already exists");
DBDefineException(KindDoesNotExistException, "the kind of diagram item does not exist");

class DiagramContext: public QObject
{
	Q_OBJECT
	DP_DECLARE(DiagramContext);
public:
	DiagramContext(QObject *object=nullptr);

	void registerKind(DiagramComponent *kind);
	DiagramComponent *kind(const QString &name);
	const QMap<QString, DiagramComponent *> &kinds() const;

	virtual ~DiagramContext();
};



}  // namespace dbuilder

