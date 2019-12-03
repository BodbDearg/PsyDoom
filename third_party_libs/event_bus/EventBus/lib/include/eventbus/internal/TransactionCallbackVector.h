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
struct TransactionCallbackVector : public CallbackVector
{
	using CallbackType = std::function<void(const Event&)>;
	using ContainerElement = std::pair<int, CallbackType>;
	using ContainerType = std::vector<ContainerElement>;
	ContainerType container;
	ContainerType toAdd;
	std::vector<int> toRemove;
	int inTransaction = 0;

	virtual void remove(const int token) override
	{
		if(inTransaction > 0)
		{
			toRemove.push_back(token);
			return;
		}

		//Invalidation rules: https://stackoverflow.com/questions/6438086/iterator-invalidation-rules
		auto removeFrom = std::remove_if(
			container.begin(), container.end(), [token](const ContainerElement& element) {
				return element.first == token;
			});
		if(removeFrom != container.end())
		{
			container.erase(removeFrom, container.end());
		}
	}

	void add(const int token, const CallbackType& callback)
	{
		if(inTransaction > 0)
		{
			toAdd.emplace_back(token, callback);
		}
		else
		{
			container.emplace_back(token, callback);
		}
	}

	void beginTransaction()
	{
		++inTransaction;
	}

	void commitTransaction()
	{
		--inTransaction;
		if(inTransaction > 0)
		{
			return;
		}
		inTransaction = 0;

		if(toAdd.empty() == false)
		{
			container.insert(container.end(), toAdd.begin(), toAdd.end());
			toAdd.clear();
		}
		if(toRemove.empty() == false)
		{
			for(auto token : toRemove)
			{
				remove(token);
			}
			toRemove.clear();
		}
	}
};

} // namespace Internal
} // namespace Dexode
