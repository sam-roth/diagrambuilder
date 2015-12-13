#pragma once
/**
 * @file   DataPtr.hh
 *
 * @date   Dec 27, 2012
 * @author Sam Roth <>
 */



namespace dbuilder {
template <typename T>
struct BasicData
{
	T *that;

	virtual ~BasicData() { }
};

#define DP_DECLARE(T)  struct Data; DataPtr<T, Data> dat
#define DP_DEFINE(T)   struct T::Data: public BasicData<T>
#define DP_INIT        dat(this)

template <typename T, typename D>
class DataPtr
{
	BasicData<T> *data;
public:
	DataPtr(const DataPtr &) = delete;

	DataPtr(T *t)
	{
		data = new D;
		data->that = t;
	}

	D *operator ->() const
	{
		return static_cast<D *>(data);
	}

	~DataPtr()
	{
		delete data;
	}
};
} // namespace dbuilder



