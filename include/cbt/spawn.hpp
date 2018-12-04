#ifndef CBT_SPAWN_HPP
#define CBT_SPAWN_HPP
#include "cbt/behavior.hpp"
#include <utility>
#include <type_traits>
namespace cbt
{
void spawn(behavior_t&& x);

template<typename T>
void spawn(behavior_t&& x, T&& y);

template<typename T, bool C1, bool C2>
struct spawn_t
{

	static void spawn(behavior_t&& x, T y)
	{
		(void) new spawn_t(std::move(x), std::move(y));
	}
private:
	behavior_t _tree;
	T _cleanup;
	continuation_type _continue{};

	spawn_t(behavior_t&& tree, T cleanup)
	: _tree(std::move(tree))
	, _cleanup(std::move(cleanup))
	, _continue([this](Status s) -> continues
	{
		if constexpr (C1) _cleanup(std::move(_tree), s);
		else if constexpr (C2) _cleanup(s);
		delete this;
		return continues::finished();
	})
	{ _tree.run(_continue); }
};

template<typename T>
void spawn(behavior_t&& x, T&& y)
{
	constexpr bool a = std::is_invocable<T, behavior_t&&, Status>::value;
	constexpr bool b = std::is_invocable<T, Status>::value;
	constexpr bool valid = a+b == 1;
	static_assert(valid,       "spawn called with invalid type");
	static_assert(valid || !a, "invocable as void(behavior&&, Status)");
	static_assert(valid || !b, "invocable as void(Status)");
	static_assert(valid || a, "not invocable as void(behavior&&, Status)");
	static_assert(valid || b, "not invocable as void(Status)");
	if constexpr (valid)
	{
		spawn_t<std::decay_t<T>, a, b>::spawn(std::move(x), std::forward<T>(y));
	}
}
} // cbt

#endif // CBT_SPAWN_HPP
