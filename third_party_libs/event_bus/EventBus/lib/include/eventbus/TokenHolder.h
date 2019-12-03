#pragma once

#include <cassert>
#include <functional>
#include <memory>

namespace
{

template <class Bus>
void null_deleter(Bus*)
{}

} // namespace

namespace Dexode
{
//
//template<class Bus>
//class EventBusWrapper : Bus

template <class Bus>
class TokenHolder
{
public:
	TokenHolder(const std::shared_ptr<Bus>& bus)
		: _bus {bus}
	{
		assert(_bus);
	}

	TokenHolder(Bus* bus)
		: _bus(bus, &null_deleter<Bus>)
	{}

	TokenHolder(const TokenHolder& other)
		: _bus(other._bus)
	{}

	TokenHolder(TokenHolder&& other)
		: _token(other._token)
		, _bus(std::move(other._bus))
	{
		other._token = 0;
	}

	~TokenHolder()
	{
		unlistenAll();
	}

	TokenHolder& operator=(const TokenHolder& other)
	{
		if(this == &other)
		{
			return *this;
		}
		if(other._bus.get() != _bus.get())
		{
			unlistenAll();
			_bus = other._bus;
		}

		return *this;
	}

	TokenHolder& operator=(TokenHolder&& other)
	{
		if(this == &other)
		{
			return *this;
		}

		unlistenAll();

		_token = other._token;
		other._token = 0;
		_bus = std::move(other._bus);

		return *this;
	}

	/**
	 * Register listener for event.
	 *
	 * @tparam Event - type you want to listen for
	 * @param callback - your callback to handle event
	 */
	template <typename Event>
	void listen(std::function<void(const Event&)> callback)
	{
		if(!callback || !_bus)
		{
			assert(callback);
			assert(_bus);
			return; //Skip such things
		}
		if(_token == 0)
		{
			_token = _bus->template listen<Event>(std::move(callback));
		}
		else
		{
			_bus->template listen<Event>(_token, std::move(callback));
		}
	}

	void unlistenAll()
	{
		if(_token != 0 && _bus)
		{
			_bus->unlistenAll(_token);
		}
	}

	/**
	 * @tparam Event - type you want to unlisten. @see Notiier::listen
	 */
	template <typename Event>
	void unlisten()
	{
		if(_bus)
		{
			_bus->template unlisten<Event>(_token);
		}
	}

	bool isUsing(const std::shared_ptr<Bus>& bus) const
	{
		return _bus == bus;
	}

private:
	int _token = 0;
	std::shared_ptr<Bus> _bus;
};

} // namespace Dexode
