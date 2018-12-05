#include "cbt/decorators/repeater.hpp"
#include "cbt/behavior.hpp"
#include "cbt/spawn.hpp"
#include <doctest/doctest.h>
namespace cbt
{
namespace {
TEST_CASE("repeater")
{
	auto count = 0;
	auto result = Invalid;
	auto cb = [&](Status s) { result = s; };
	auto leaf = behavior_t([&]{ ++count; return Success; });
	SUBCASE("Repeat 5 times")
	{
		spawn(repeater(std::move(leaf), 5), cb);
		REQUIRE(count == 5);
		REQUIRE(result == Success);
	}
	SUBCASE("Nested Repeat 5*5 times")
	{
		spawn(repeater(repeater(std::move(leaf), 5), 5), cb);
		REQUIRE(count == 25);
		REQUIRE(result == Success);
	}
	SUBCASE("Fail at third iteration")
	{
		leaf = [&]{ return ++count == 3 ? Failure : Success; };
		spawn(repeater(std::move(leaf), 5), cb);
		REQUIRE(count == 3);
		REQUIRE(result == Failure);
	}
	SUBCASE("Repeat 0 times")
	{
		spawn(repeater(std::move(leaf), 0), cb);
		REQUIRE(count == 0);
		REQUIRE(result == Success);
	}
}
struct repeater_t
{
	behavior_t child;
	size_t limit;
	size_t count = 0;
	continuation resume{};
	auto operator()(continuation _resume) noexcept -> continues
	{
		resume = std::move(_resume);
		count = 0;
		return step(Success);
	}
	auto next(Status s) noexcept -> continues
	{
		++count;
		return step(s);
	}
	auto step(Status s) noexcept -> continues
	{
		if (count == limit || s == Failure)
		{
			return continues::up(std::move(resume), s);
		}
		return continues::down(
			child,
			continuation::mem_fn<&repeater_t::next>(*this));
	}
};
}

behavior_t repeater(behavior_t&& x, size_t limit)
{
	return repeater_t{ std::move(x), limit };
}
} // cbt
