#include <cbt/status.hpp>
#include "cbt/unique_function.hpp"
#include <type_traits>
#include <cstddef>
#include <cassert>

namespace cbt
{
// the object storing the continuation type within behavior_t
using continuation_type = unique_function<void(Status)>;

/*
 * a linear function reference.
 * - must be called exactly once before destruction
 * - or moved into another continuation object.
 *
 * In non-debug mode, the object is trivially copyable, simply a pointer, which
 * is required to allow tail-call optimizations in the callbacks.
 * however, it loses the linear behavior described above.
 *
 * in debug mode, it keeps the behavior, but is less efficient.
 */
struct continuation
{
	continuation() = default;
	continuation(continuation_type const& x) noexcept;
	continuation& operator=(continuation_type const& x) noexcept;
#if !defined(DEBUG)
	continuation(continuation&& x) noexcept = default;
	continuation& operator=(continuation&& x) noexcept = default;
#else // defined(DEBUG)
	continuation(continuation&& x) noexcept
	:_ref(std::exchange(x._ref, nullptr))
	{}
	continuation& operator=(continuation&& x) noexcept
	{
		assert(_ref == nullptr);
		_ref = std::exchange(x._ref, nullptr);
		return *this;
	}
	~continuation() noexcept
	{
		assert(_ref == nullptr);
	}
#endif

	void operator()(Status);

	// helper functions
	operator bool() const noexcept { return _ref != nullptr; }
	friend bool operator==(continuation const& x, continuation const& y) noexcept
	{
		return x._ref == y._ref;
	}
	friend bool operator==(std::nullptr_t, continuation const& x) noexcept
	{
		return x._ref == nullptr;
	}
	friend bool operator==(continuation const& x, std::nullptr_t) noexcept
	{
		return x._ref == nullptr;
	}
	friend bool operator!=(continuation const& x, continuation const& y) noexcept
	{
		return !(x == y);
	}
	friend bool operator!=(std::nullptr_t, continuation const& x) noexcept
	{
		return x._ref != nullptr;
	}
	friend bool operator!=(continuation const& x, std::nullptr_t) noexcept
	{
		return x._ref != nullptr;
	}
	void swap(continuation &x) noexcept
	{
		std::swap(_ref, x._ref);
	}

private:
	continuation_type const* _ref = nullptr;
};
} // cbt

