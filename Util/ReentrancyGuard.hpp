#pragma once
/**
 * @file   ReentrancyGuard.hpp
 *
 * @date   Apr 28, 2013
 * @author Sam Roth <>
 */


namespace dbuilder {

class ReentrancyGuard
{
	mutable bool _within;
	class SetWithin
	{
		const ReentrancyGuard &r;
	public:
		SetWithin(const ReentrancyGuard &r)
		: r(r)
		{
			r._within = true;
		}

		~SetWithin()
		{
			r._within = false;
		}
	};
public:
	ReentrancyGuard()
	: _within(false) { }

	template <typename F>
	bool enter(F &&f) const
	{
		if(_within)
		{
			return false;
		}
		else
		{
			SetWithin w(*this);
			f();
			return true;
		}
	}
};

}  // namespace dbuilder


