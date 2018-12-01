#ifndef CBT_SPAWN_HPP
#define CBT_SPAWN_HPP
#include "cbt/behavior.hpp"
#include <utility>
namespace cbt
{
template<typename T>
struct spawn_t
{
	static
	void spawn(behavior_t&& x, T y)
	{
		(void) new spawn_t(std::move(x), std::move(y));
	}
private:
	behavior_t _child;
	T _cleanup;
	continuation_type _continue{};

	spawn_t(behavior_t&& child, T cleanup)
	: _child(std::move(child)), _cleanup(cleanup)
	{
		// passes behavior_t into cleanup function, and delete the
		// spawn_t as final continuation
		_continue = [this](Status s)
		{
			_cleanup(std::move(_child), s);
			delete this;
		};
		_child.run(_continue);
	}
};

void spawn(behavior_t&& x)
{
	auto noop = [](behavior_t, Status) { };
	spawn_t<decltype(noop)>::spawn(std::move(x), noop);
}

template<typename T>
void spawn(behavior_t&& x, T&& y)
{
	spawn_t<std::decay_t<T>>::spawn(std::move(x), std::forward<T>(y));
}

} // cbt

#endif // CBT_SPAWN_HPP
