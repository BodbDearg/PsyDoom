#pragma once

#include <algorithm>
#include <functional>
#include <vector>

#include "CallbackVector.h"

namespace Dexode
{
namespace Internal
{

template <typename Event>
struct AsyncCallbackVector : public CallbackVector
{
	using CallbackType = std::function<void(const Event&)>;
	using ContainerElement = std::pair<int, CallbackType>;
	std::vector<ContainerElement> container;

	virtual void remove(const int token) override
	{
		auto removeFrom = std::remove_if(
			container.begin(), container.end(), [token](const ContainerElement& element) {
				return element.first == token;
			});
		if(removeFrom != container.end())
		{
			container.erase(removeFrom, container.end());
		}
	}

	void add(const int token, CallbackType callback)
	{
		container.emplace_back(token, std::move(callback));
	}
};

} // namespace Internal
} // namespace Dexode
