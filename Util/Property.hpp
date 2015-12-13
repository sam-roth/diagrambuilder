#pragma once
/**
 * @file   Property.h
 *
 * @date   Jan 12, 2013
 * @author Sam Roth <>
 */


#define DBArgCountAux(a1, a2, a3, a4, a5, a6, rest...) a6
#define DBArgCount(args...) DBArgCountAux(args, 5, 4, 3, 2, 1, 0)
#define DBConcatAux(a, b) a ## b
#define DBConcat(a, b) DBConcatAux(a, b)
#define DBProperty(args...) DBConcat(DBProperty, DBArgCount(args))(args)
#define DBProperty2(T, getter)\
	private:\
	T _ ## getter;\
	public:\
	const T &getter() const { return this -> _ ## getter; }

#define DBProperty3(T, getter, setter)\
	private:\
	T _ ## getter;\
	public:\
	const T &getter() const { return this -> _ ## getter; }\
	void setter(const T &value) { this -> _ ## getter = value; }

