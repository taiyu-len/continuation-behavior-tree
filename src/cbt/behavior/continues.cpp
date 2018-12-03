#include "cbt/behavior/continues.hpp"
#include "cbt/behavior/continuation.hpp"
#include "cbt/behavior.hpp"
#include <cassert>
namespace cbt
{
auto continues::up(continuation&& c, Status s) noexcept -> continues
{
	continues x;
	x._continue = std::move(c);
	x._status   = s;
	return x;
}

auto continues::down(behavior_t const& b, continuation&& c) noexcept -> continues
{
	continues x;
	x._behavior = &b;
	x._continue = std::move(c);
	return x;
}

auto continues::elsewhere() noexcept -> continues
{ return continues{}; }
auto continues::finished() noexcept -> continues
{ return continues{}; }

void continues::run() noexcept
{
	while (true) switch (state()) {
	case state_e::up:   *this = go_up(); break;
	case state_e::down: *this = go_down(); break;
	case state_e::elsewhere: return;
	}
}

auto continues::state() const noexcept -> state_e
{
	if (_behavior) return state_e::down;
	if (_continue) return state_e::up;
	else           return state_e::elsewhere;
}

auto continues::go_up() noexcept -> continues
{ return _continue.step(_status); }

auto continues::go_down() noexcept -> continues
{ return _behavior->step(std::move(_continue)); }
} // cbt
