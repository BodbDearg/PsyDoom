#pragma once

#include <algorithm>
#include <cassert>
#include <condition_variable>
#include <deque>
#include <functional>
#include <limits>
#include <map>
#include <memory>
#include <mutex>

#include <eventbus/internal/AsyncCallbackVector.h>
#include <eventbus/internal/common.h>

namespace Dexode
{

/**
 * Async version of EventBus. Events are scheduled to queue and processed when called consume method.
 * Methods like listen/unlisten also now will be processed when consume being called.
 *
 */
class AsyncEventBus
{
public:
	AsyncEventBus() = default;

	~AsyncEventBus()
	{
		std::lock_guard<std::mutex> guard {_callbacksMutex};
		_callbacks.clear();
	}

	AsyncEventBus(const AsyncEventBus&) = delete;
	AsyncEventBus(AsyncEventBus&&) = delete;

	AsyncEventBus& operator=(AsyncEventBus&&) = delete;
	AsyncEventBus& operator=(const AsyncEventBus&) = delete;

	/**
	 * Register listener for event. Returns token used for unlisten.
	 * This request will be scheduled into queue. Need at least call 1 consume.
	 * It isn't ASAP event
	 *
	 * @tparam Event - type you want to listen for
	 * @param callback - your callback to handle event
	 * @return token used for unlisten
	 */
	template <typename Event>
	int listen(std::function<void(const Event&)> callback)
	{
		static_assert(Internal::validateEvent<Event>(), "Invalid event");

		const int token = newToken();
		listen<Event>(token, std::move(callback));
		return token;
	}

	/**
	 * Register listener for event.
	 * This request will be scheduled into queue. Need at least call 1 consume.
	 * It isn't ASAP event
	 *
	 * @tparam Event - type you want to listen for
	 * @param token - unique token for identification receiver. Simply pass token from @see EventBus::listen
	 * @param callback - your callback to handle event
	 */
	template <typename Event>
	void listen(const int token, std::function<void(const Event&)> callback)
	{
		static_assert(Internal::validateEvent<Event>(), "Invalid event");

		std::lock_guard<std::mutex> guard {_eventMutex};
		_commandsQueue.push_back([this, token, callback = std::move(callback)]() {
			std::lock_guard<std::mutex> guard {_callbacksMutex};

			using Vector = Internal::AsyncCallbackVector<Event>;

			assert(callback && "callback should be valid"); //Check for valid object

			std::unique_ptr<Internal::CallbackVector>& vector =
				_callbacks[Internal::type_id<Event>()];
			if(vector == nullptr)
			{
				vector.reset(new Vector {});
			}
			assert(dynamic_cast<Vector*>(vector.get()));
			Vector* callbacks = static_cast<Vector*>(vector.get());
			callbacks->add(token, callback);
		});
	}

	/**
	 * This request will be scheduled into queue. Need at least call 1 consume.
	 * It isn't ASAP event
	 *
	 * @param token - token from EventBus::listen
	 */
	void unlistenAll(const int token)
	{
		std::lock_guard<std::mutex> guard {_eventMutex};
		_commandsQueue.emplace_back([this, token]() {
			std::lock_guard<std::mutex> guard {_callbacksMutex};
			for(auto& element : _callbacks)
			{
				element.second->remove(token);
			}
		});
	}

	/**
	 * This request will be scheduled into queue. Need at least call 1 consume.
	 * It isn't ASAP event
	 *
	 * @tparam Event - type you want to unlisten. @see Notiier::listen
	 * @param token - token from EventBus::listen
	 */
	template <typename Event>
	void unlisten(const int token)
	{
		static_assert(Internal::validateEvent<Event>(), "Invalid event");

		std::lock_guard<std::mutex> guard {_eventMutex};
		_commandsQueue.push_back([this, token]() {
			std::lock_guard<std::mutex> guard {_callbacksMutex};

			auto found = _callbacks.find(Internal::type_id<Event>);
			if(found != _callbacks.end())
			{
				found->second->remove(token);
			}
		});
	}

	/**
	 * Schedule event to queue
	 *
	 * @param event your event struct
	 */
	template <typename Event>
	void schedule(Event event)
	{
		static_assert(Internal::validateEvent<Event>(), "Invalid event");
		_eventWaiting.notify_one();

		std::lock_guard<std::mutex> guard {_eventMutex};
		_eventQueue.push_back([this, event = std::move(event)]() {
			std::lock_guard<std::mutex> guard {_callbacksMutex};

			using Vector = Internal::AsyncCallbackVector<Event>;
			auto found = _callbacks.find(Internal::type_id<Event>());
			if(found == _callbacks.end())
			{
				return; // no such notifications
			}

			std::unique_ptr<Internal::CallbackVector>& vector = found->second;
			assert(dynamic_cast<Vector*>(vector.get()));
			Vector* callbacks = static_cast<Vector*>(vector.get());

			for(const auto& element : callbacks->container)
			{
				element.second(event);
			}
		});
	}

	/**
	 * Schedule and consume event. Notify all listeners with event.
	 *
	 * @param event your event struct
	 */
	template <typename Event>
	void notify(const Event& event)
	{
		schedule(event);
		consume(1);
	}

	/**
	 * Process queued events. This should be called always on same thread.
	 * @param max maximum count of events to consume.
	 * If max is higher than available events then only available events will be consumed.
	 * @return number of consumed events
	 */
	int consume(int max = std::numeric_limits<int>::max());
	bool wait();
	bool waitFor(std::chrono::milliseconds timeout);

	std::size_t getQueueEventCount() const
	{
		std::lock_guard<std::mutex> guard {_eventMutex};
		return _eventQueue.size();
	}

private:
	int newToken()
	{
		std::lock_guard<std::mutex> guard {_eventMutex};
		int token = ++_tokener;
		return token;
	}

	std::size_t processCommandsAndGetQueuedEventsCount();

	int _tokener = 0;
	std::map<Internal::type_id_t, std::unique_ptr<Internal::CallbackVector>> _callbacks;
	mutable std::mutex _callbacksMutex;
	mutable std::mutex _eventMutex;

	std::mutex _waitMutex;
	std::condition_variable _eventWaiting;

	std::deque<std::function<void()>> _eventQueue;
	std::deque<std::function<void()>> _commandsQueue;
};

} /* namespace Dexode */
