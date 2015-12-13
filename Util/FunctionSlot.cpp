/**
 * @file   FunctionSlot.cpp
 *
 * @date   Dec 31, 2012
 * @author Sam Roth <>
 */

#include "FunctionSlot.hpp"
#include <QObject>
#include <functional>


namespace dbuilder {
FunctionSlot::FunctionSlot(QObject* parent, std::function<void()> func)
: QObject(parent)
, _func(std::move(func))
{
}

FunctionSlot::~FunctionSlot()
{
}

void FunctionSlot::triggered()
{
	_func();
}

void connectFunc(QObject *target, const char *signal, std::function<void()> func)
{
	QObject::connect(target, signal, new FunctionSlot(target, std::move(func)), SLOT(triggered()));
}
} // namespace dbuilder
