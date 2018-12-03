#ifndef CBT_SPAWN_HPP
#define CBT_SPAWN_HPP
#include "cbt/behavior.hpp"
#include <utility>
#include <type_traits>
namespace cbt
{
template<
	typename T,
	bool = std::is_invocable<T, behavior_t&&, Status>::value,
	bool = std::is_invocable<T, Status>::value>
 struct call_cleanup;

template<typename T>
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

	spawn_t(behavior_t&& tree, T cleanup);
};

void spawn(behavior_t&& x);

template<typename T>
void spawn(behavior_t&& x, T&& y)
{
	spawn_t<std::decay_t<T>>::spawn(std::move(x), std::forward<T>(y));
}


template<typename T>
struct call_cleanup<T, true, false>
{ void operator()(T &x, behavior_t &&b, Status s) { x(std::move(b), s); } };

template<typename T>
struct call_cleanup<T, false, true>
{ void operator()(T &x, behavior_t &&, Status s) { x(s); } };

template<typename T, bool x, bool y>
struct call_cleanup
{
	static_assert(sizeof(T) < 0, "spawn called with invalid type");
	static_assert(!x, "is invocable as void(behavior_t&&, Status)");
	static_assert(!y, "is invocable as void(Status)");
	static_assert(x,  "is not invocable as void(behavior_t&&, Status)");
	static_assert(y,  "is not invocable as void(Status)");
};

template<typename T>
spawn_t<T>::spawn_t(behavior_t&& tree, T cleanup)
: _tree(std::move(tree)), _cleanup(cleanup)
{
	// passes behavior_t into cleanup function, and delete the
	// spawn_t as final continuation
	_continue = [this] (Status s) -> continues
	{
		call_cleanup<T>{}(_cleanup, std::move(_tree), s);
		delete this;
		return continues::finished();
	};
	_tree.run(_continue);
}

} // cbt

#endif // CBT_SPAWN_HPP
