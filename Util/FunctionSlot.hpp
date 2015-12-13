#pragma once
#include <QObject>
#include <functional>
/**
 * @file   FunctionSlot.h
 *
 * @date   Dec 31, 2012
 * @author Sam Roth <>
 */

namespace dbuilder {
class FunctionSlot: public QObject
{
	Q_OBJECT
	std::function<void()> _func;
public:
	FunctionSlot(QObject *parent, std::function<void()> func);
	virtual ~FunctionSlot();
public slots:
	void triggered();
};

void connectFunc(QObject *target, const char *signal, std::function<void()> func);
} // namespace dbuilder
