#pragma once
/**
 * @file   Handle.hpp
 *
 * @date   Feb 28, 2013
 * @author Sam Roth <>
 */

namespace dbuilder {
class HoverListener
{
public:
	virtual void startHover() = 0;
	virtual void endHover() = 0;
	virtual bool acceptHover(QPointF) { return true; }
	virtual ~HoverListener() { }
};
class Handle: public HoverListener
{
public:
	virtual ~Handle(){ }
};

}  // namespace dbuilder
