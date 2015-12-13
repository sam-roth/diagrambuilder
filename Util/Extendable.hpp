#pragma once
#include <typeindex>
/**
 * @date   Jul 15, 2013
 * @author Sam Roth <>
 */

namespace dbuilder {

class Extendable
{
	std::unordered_map<std::type_index, QObject *> _extensions;
public:
	struct NoSuchExtension: public std::runtime_error
	{
	public:
		NoSuchExtension(const std::string &msg)
		: std::runtime_error(msg) { }
	};

	const std::unordered_map<std::type_index, QObject*> &extensions() const
	{
		return _extensions;
	}

	template <typename T>
	T *extensionOrNull() const
	{
		auto it = _extensions.find(typeid(T));
		if(it != _extensions.end())
		{
			return static_cast<T *>(it->second);
		}
		else
		{
			return nullptr;
		}
	}


	template <typename T>
	T &getExtension() const
	{
		if(auto result = extensionOrNull<T>())
		{
			return *result;
		}
		else
		{
			throw NoSuchExtension(std::string("No such extension: ") + typeid(T).name());
		}
	}


	template <typename T>
	void addExtension(T *ext)
	{
		_extensions[typeid(T)] = ext;
	}
};

}  // namespace dbuilder


