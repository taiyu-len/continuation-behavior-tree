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
	auto result = status::unknown;
	auto cb = [&](status s) { result = s; };
	auto True  = [&]{ ++count; return status::success; };
	auto False = [&]{ ++count; return status::failure; };
	auto Both  = [&, s=status::success]() mutable { ++count; return s=!s; };
	SUBCASE("Repeat 5 times")
	{
		spawn(repeat_n(True, 5), cb);
		REQUIRE(count == 5);
		REQUIRE(result == status::success);
		spawn(until_n(False, 5), cb);
		REQUIRE(count == 10);
		REQUIRE(result == status::failure);
		spawn(for_n(Both, 5), cb);
		REQUIRE(count == 15);
	}
	SUBCASE("Nested Repeat 5*5 times")
	{
		spawn(repeat_n(repeat_n(True, 5), 5), cb);
		REQUIRE(count == 25);
		REQUIRE(result == status::success);
	}
	SUBCASE("Fail at third iteration")
	{
		auto n = [&]{ return ++count == 3 ? status::failure : status::success; };
		spawn(repeat(n), cb);
		REQUIRE(count == 3);
		REQUIRE(result == status::failure);
	}
	SUBCASE("Repeat 0 times")
	{
		spawn(repeat_n(True, 0), cb);
		REQUIRE(count == 0);
		REQUIRE(result == status::success);
	}
}

template<status initial, status exit_on>
struct repeater_t
{
	behavior     child;
	continuation resume{};

	auto operator()(continuation _resume) noexcept -> continues
	{
		resume = std::move(_resume);
		return step(initial);
	}
	auto step(status s) noexcept -> continues
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

template<status initial, status exit_on>
struct repeater_n_t
{
	behavior child;
	size_t     limit;
	size_t     count = 0;
	continuation resume{};

	auto operator()(continuation _resume) noexcept -> continues
	{
		resume = std::move(_resume);
		count = 0;
		return step(initial);
	}
	auto next(status s) noexcept -> continues
	{
		++this->count;
		return step(s);
	}
	auto step(status s) noexcept -> continues
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


using repeat_n_t = repeater_n_t<status::success, status::failure>;
using until_n_t  = repeater_n_t<status::failure, status::success>;
using for_n_t    = repeater_n_t<status::success, status::unknown>;

using repeat_t   = repeater_t<status::success, status::failure>;
using until_t    = repeater_t<status::failure, status::success>;
using forever_t  = repeater_t<status::success, status::unknown>;
}

auto repeat_n(behavior&& x, size_t limit) -> behavior
{ return repeat_n_t{ std::move(x), limit }; }

auto repeat(behavior&& x) -> behavior
{ return repeat_t{ std::move(x) }; }

auto until_n(behavior&& x, size_t limit) -> behavior
{ return until_n_t{ std::move(x), limit }; }

auto until(behavior&& x) -> behavior
{ return until_t{ std::move(x) }; }

auto for_n(behavior&& x, size_t limit) -> behavior
{ return for_n_t{ std::move(x), limit }; }

auto forever(behavior&& x) -> behavior
{ return forever_t{ std::move(x) }; }

} // cbt
