#ifndef CBT_NODES_SPAWN_T_HPP
#define CBT_NODES_SPAWN_T_HPP
#include "cbt/behavior.hpp"
#include <utility>
#include <type_traits>
namespace cbt
{
template<typename T, bool C1, bool C2>
struct _spawn_t
{
	static void spawn(behavior&& x, T y)
	{
		(void) new _spawn_t(std::move(x), std::move(y));
	}
private:
	behavior _tree;
	T _cleanup;
	auto finish(status s) noexcept -> continues
	{
		if constexpr (C1) _cleanup(std::move(_tree), s);
		else if constexpr (C2) _cleanup(s);
		delete this;
		return continues::finished();
	}

	_spawn_t(behavior&& tree, T cleanup)
	: _tree(std::move(tree))
	, _cleanup(std::move(cleanup))
	{
		_tree.run(continuation::mem_fn<&_spawn_t::finish>(*this));
	}
};

template<typename T>
void spawn(behavior&& x, T&& y)
{
	constexpr bool a = std::is_invocable<T, behavior&&, status>::value;
	constexpr bool b = std::is_invocable<T, status>::value;
	constexpr bool valid = a+b == 1;
	static_assert(valid,       "spawn called with invalid type");
	static_assert(valid || !a, "invocable as void(behavior&&, status)");
	static_assert(valid || !b, "invocable as void(status)");
	static_assert(valid || a, "not invocable as void(behavior&&, status)");
	static_assert(valid || b, "not invocable as void(status)");
	if constexpr (valid)
	{
		_spawn_t<std::decay_t<T>, a, b>::spawn(std::move(x), std::forward<T>(y));
	}
}
} // cbt
#endif // CBT_NODES_SPAWN_T_HPP
