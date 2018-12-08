#include "cbt/nodes/repeat.hpp"
#include "cbt/nodes/spawn.hpp"
#include "cbt/behavior.hpp"
#include <doctest/doctest.h>
namespace cbt
{
namespace {
TEST_CASE("repeat")
{
	auto count = 0;
	auto result = Invalid;
	auto cb = [&](Status s) { result = s; };
	auto True  = [&]{ ++count; return Success; };
	auto False = [&]{ ++count; return Failure; };
	auto Both  = [&, s=Success]() mutable { ++count; return s=!s; };
	SUBCASE("Repeat 5 times")
	{
		spawn(repeat_n(True, 5), cb);
		REQUIRE(count == 5);
		REQUIRE(result == Success);
		spawn(until_n(False, 5), cb);
		REQUIRE(count == 10);
		REQUIRE(result == Failure);
		spawn(for_n(Both, 5), cb);
		REQUIRE(count == 15);
	}
	SUBCASE("Nested Repeat 5*5 times")
	{
		spawn(repeat_n(repeat_n(True, 5), 5), cb);
		REQUIRE(count == 25);
		REQUIRE(result == Success);
	}
	SUBCASE("Fail at third iteration")
	{
		auto n = [&]{ return ++count == 3 ? Failure : Success; };
		spawn(repeat(n), cb);
		REQUIRE(count == 3);
		REQUIRE(result == Failure);
	}
	SUBCASE("Repeat 0 times")
	{
		spawn(repeat_n(True, 0), cb);
		REQUIRE(count == 0);
		REQUIRE(result == Success);
	}
}

template<Status initial, Status exit_on>
struct repeater_t
{
	behavior_t     child;
	continuation resume{};

	auto operator()(continuation _resume) noexcept -> continues
	{
		resume = std::move(_resume);
		return step(initial);
	}
	auto step(Status s) noexcept -> continues
	{
		if (s == exit_on)
		{
			return continues::up(std::move(resume), s);
		}
		return continues::down(
			child,
			continuation::mem_fn<&repeater_t::step>(*this));
	}
};

template<Status initial, Status exit_on>
struct repeater_n_t
{
	behavior_t child;
	size_t     limit;
	size_t     count = 0;
	continuation resume{};

	auto operator()(continuation _resume) noexcept -> continues
	{
		resume = std::move(_resume);
		count = 0;
		return step(initial);
	}
	auto next(Status s) noexcept -> continues
	{
		++this->count;
		return step(s);
	}
	auto step(Status s) noexcept -> continues
	{
		if (s == exit_on || count == limit)
		{
			return continues::up(std::move(resume), s);
		}
		return continues::down(
			child,
			continuation::mem_fn<&repeater_n_t::next>(*this));
	}
};


using repeat_n_t = repeater_n_t<Success, Failure>;
using until_n_t  = repeater_n_t<Failure, Success>;
using for_n_t    = repeater_n_t<Success, Invalid>;

using repeat_t   = repeater_t<Success, Failure>;
using until_t    = repeater_t<Failure, Success>;
using forever_t  = repeater_t<Success, Invalid>;
}

auto repeat_n(behavior_t&& x, size_t limit) -> behavior_t
{ return repeat_n_t{ std::move(x), limit }; }

auto repeat(behavior_t&& x) -> behavior_t
{ return repeat_t{ std::move(x) }; }

auto until_n(behavior_t&& x, size_t limit) -> behavior_t
{ return until_n_t{ std::move(x), limit }; }

auto until(behavior_t&& x) -> behavior_t
{ return until_t{ std::move(x) }; }

auto for_n(behavior_t&& x, size_t limit) -> behavior_t
{ return for_n_t{ std::move(x), limit }; }

auto forever(behavior_t&& x) -> behavior_t
{ return forever_t{ std::move(x) }; }

} // cbt
