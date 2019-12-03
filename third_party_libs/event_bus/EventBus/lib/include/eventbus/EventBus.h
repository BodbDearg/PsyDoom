#pragma once

#include <algorithm>
#include <cassert>
#include <functional>
#include <map>
#include <memory>

#include <eventbus/internal/TransactionCallbackVector.h>
#include <eventbus/internal/common.h>

namespace Dexode
{

class EventBus
{
public:
	EventBus() = default;

	~EventBus()
	{
		_callbacks.clear();
	}

	EventBus(const EventBus&) = delete;
	EventBus(EventBus&&) = delete;

	EventBus& operator=(EventBus&&) = delete;
	EventBus& operator=(const EventBus&) = delete;

	/**
	 * Register listener for event. Returns token used for unlisten.
	 *
	 * @tparam Event - type you want to listen for
	 * @param callback - your callback to handle event
	 * @return token used for unlisten
	 */
	template <typename Event>
	int listen(const std::function<void(const Event&)>& callback)
	{
		static_assert(Internal::validateEvent<Event>(), "Invalid event");

		const int token = ++_tokener;
		listen<Event>(token, callback);
		return token;
	}

	/**
	 * @tparam Event - type you want to listen for
	 * @param token - unique token for identification receiver. Simply pass token from @see EventBus::listen
	 * @param callback - your callback to handle event
	 */
	template <typename Event>
	void listen(const int token, const std::function<void(const Event&)>& callback)
	{
		static_assert(Internal::validateEvent<Event>(), "Invalid event");

		using Vector = Internal::TransactionCallbackVector<Event>;

		assert(callback && "callback should be valid"); //Check for valid object

		std::unique_ptr<Internal::CallbackVector>& vector = _callbacks[Internal::type_id<Event>()];
		if(vector == nullptr)
		{
			vector.reset(new Vector {});
		}
		assert(dynamic_cast<Vector*>(vector.get()));
		Vector* vectorImpl = static_cast<Vector*>(vector.get());
		vectorImpl->add(token, callback);
	}

	/**
	 * @param token - token from EventBus::listen
	 */
	void unlistenAll(const int token)
	{
		for(auto& element : _callbacks)
		{
			element.second->remove(token);
		}
	}

	/**
	 * @tparam Event - type you want to unlisten. @see Notiier::listen
	 * @param token - token from EventBus::listen
	 */
	template <typename Event>
	void unlisten(const int token)
	{
		static_assert(Internal::validateEvent<Event>(), "Invalid event");

		auto found = _callbacks.find(Internal::type_id<Event>());
		if(found != _callbacks.end())
		{
			found->second->remove(token);
		}
	}

	/**
	 * Notify all listeners for event
	 *
	 * @param event your event struct
	 */
	template <typename Event>
	void notify(const Event& event)
	{
		using CleanEventType = typename std::remove_const<Event>::type;
		static_assert(Internal::validateEvent<Event>(), "Invalid event");

		using Vector = Internal::TransactionCallbackVector<CleanEventType>;
		auto found = _callbacks.find(Internal::type_id<CleanEventType>());
		if(found == _callbacks.end())
		{
			return; // no such notifications
		}

		std::unique_ptr<Internal::CallbackVector>& vector = found->second;
		assert(dynamic_cast<Vector*>(vector.get()));
		Vector* vectorImpl = static_cast<Vector*>(vector.get());

		vectorImpl->beginTransaction();
		for(const auto& element : vectorImpl->container)
		{
			element.second(event);
		}
		vectorImpl->commitTransaction();
	}

private:
	int _tokener = 0;
	std::map<Internal::type_id_t, std::unique_ptr<Internal::CallbackVector>> _callbacks;
};

} /* namespace Dexode */
