/**
 * @file   DiagramContext.cpp
 *
 * @date   Jan 20, 2013
 * @author Sam Roth <>
 */

#include "moc_DiagramContext.cpp"
#include "DiagramComponent.hpp"
#include <QMap>


namespace dbuilder {




DP_DEFINE(DiagramContext)
{
	QMap<QString, DiagramComponent *> kinds;
};

DiagramContext::DiagramContext(QObject *parent)
: QObject(parent)
, DP_INIT
{

}

void DiagramContext::registerKind(DiagramComponent* kind)
{
	if(dat->kinds.count(kind->name()))
	{
		throw KindAlreadyExistsException();
	}

	kind->setParent(this);
	dat->kinds[kind->name()] = kind;
}

DiagramComponent* DiagramContext::kind(const QString& name)
{
	auto it = dat->kinds.find(name);
	if(it == dat->kinds.end())
	{
		throw KindDoesNotExistException(("the type of diagram item " + name + " does not exist").toStdString());
	}

	return it.value();
}

const QMap<QString, DiagramComponent *> &DiagramContext::kinds() const
{
	return dat->kinds;
}

DiagramContext::~DiagramContext()
{
}



}  // namespace dbuilder
