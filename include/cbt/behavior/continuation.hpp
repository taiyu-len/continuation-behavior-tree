#ifndef CBT_BEHAVIOR_CONTINUATION_HPP
#define CBT_BEHAVIOR_CONTINUATION_HPP
#include "cbt/behavior/status.hpp"
#include <type_traits>
#define CBT_DOWN(C,FN) \
continues::down((C), continuation::mem_fn<&std::decay_t<decltype(*this)>::FN>(*this))

namespace cbt
{
struct continues;
/* A linear function reference to a continuation_type function pointer.
 * must be called exactly once, or moved into another continuation.
 *
 * may be reconstructed multiple times from a single continuation_type
 */
struct continuation
{
	using that_t = void*;
	using func_t = continues(*)(that_t, status);
	using func1_t = continues(*)(status);

	// construct from member functions, or regular functions.
	template<auto M, typename T>
	static auto mem_fn(T& x) -> continuation;

	continuation() = default;
	~continuation() noexcept;

	continuation(continuation&&) noexcept;
	auto operator=(continuation&&) noexcept -> continuation&;

	continuation(func1_t) noexcept;
	auto operator=(func1_t) noexcept -> continuation&;

	// call operator for external continuation of user code.
	void operator()(status) noexcept;

	explicit operator bool() const noexcept;
	friend bool operator==(std::nullptr_t, continuation const&) noexcept;
	friend bool operator==(continuation const&, std::nullptr_t) noexcept;
	friend bool operator==(continuation const&, continuation const&) noexcept;
	friend bool operator!=(std::nullptr_t, continuation const&) noexcept;
	friend bool operator!=(continuation const&, std::nullptr_t) noexcept;
	friend bool operator!=(continuation const&, continuation const&) noexcept;
	friend void swap(continuation&, continuation&) noexcept;

private:
	friend continues;
	auto step(status) noexcept -> continues;

	/*
	 * if _that == nullptr
	 *     _func1 is active
	 * else
	 *     _func is active
	 */
	union
	{
		func_t  _func;
		func1_t _func1 = nullptr;
	};
	that_t _that = nullptr;
};

} // cbt
#include "cbt/behavior/continuation_t.hpp"
#endif // CBT_BEHAVIOR_CONTINUATION_HPP
