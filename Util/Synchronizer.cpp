/**
 * @date   Jul 17, 2013
 * @author Sam Roth <>
 */

#include "Synchronizer.hpp"
#include "Util/ScopeExit.hpp"

namespace dbuilder
{

Synchronizer::Synchronizer(QObject* parent)
: QObject(parent)
, _syncSender(nullptr)
{
}

Synchronizer::~Synchronizer()
{
}

QObject *Synchronizer::syncSender()
{
	return _syncSender;
}

void Synchronizer::viewUpdate()
{
	if(_syncSender) return;
	_syncSender = sender();
	if(!_syncSender) _syncSender = this;
	DBScopeExit([&]() { _syncSender = nullptr; });
	emit viewUpdated();
}

Synchronizer* Synchronizer::whenUpdated(QObject *receiver,
                                        const char *viewUpdatedSlot,
                                        const char *modelUpdatedSlot)
{
	connect(this, SIGNAL(viewUpdated()), receiver, viewUpdatedSlot);
	connect(this, SIGNAL(modelUpdated()), receiver, modelUpdatedSlot);
	return this;
}

void Synchronizer::modelUpdate()
{
	if(_syncSender) return;
	_syncSender = sender();
	if(!_syncSender) _syncSender = this;
	DBScopeExit([&]() { _syncSender = nullptr; });
	emit modelUpdated();
}

} /* namespace dbuilder */
