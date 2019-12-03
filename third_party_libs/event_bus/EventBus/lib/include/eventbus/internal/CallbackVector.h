#pragma once

namespace Dexode
{
namespace Internal
{

struct CallbackVector
{
	virtual ~CallbackVector() = default;

	virtual void remove(const int token) = 0;
};

} // namespace Internal
} // namespace Dexode
