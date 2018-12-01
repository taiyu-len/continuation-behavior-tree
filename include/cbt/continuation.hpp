#include <cbt/status.hpp>
#include "cbt/unique_function.hpp"
#include <type_traits>

namespace cbt
{
// the object storing the continuation type within behavior_t
using continuation_type = unique_function<void(Status)>;

/*
 * a linear function reference.
 * - must be called exactly once before destruction
 * - or moved into another continuation object.
 */
struct continuation
{
	continuation() = default;
	continuation(continuation_type const& x) noexcept;
	continuation(continuation&& x) noexcept;
	continuation& operator=(continuation&& x) noexcept;
	continuation& operator=(continuation_type const& x) noexcept;
	~continuation() noexcept;

	void operator()(Status);

	// helper functions
	operator bool() const noexcept { return _ref != nullptr; }
	friend bool operator==(continuation const& x, continuation const& y) noexcept
	{
		return x._ref == y._ref;
	}
	friend bool operator==(nullptr_t, continuation const& x) noexcept
	{
		return x._ref == nullptr;
	}
	friend bool operator==(continuation const& x, nullptr_t) noexcept
	{
		return x._ref == nullptr;
	}
	friend bool operator!=(continuation const& x, continuation const& y) noexcept
	{
		return !(x == y);
	}
	friend bool operator!=(nullptr_t, continuation const& x) noexcept
	{
		return x._ref != nullptr;
	}
	friend bool operator!=(continuation const& x, nullptr_t) noexcept
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

template<typename T, typename = void>
struct takes_continuation : std::false_type {};

template<typename T>
struct takes_continuation<T,
	std::void_t<decltype(
		std::declval<T>() ( // T is callable with continuation{}
			std::declval<continuation>()))>>
	: std::true_type{};

} // cbt

