#include "cbt/decorators.hpp"
#include <doctest/doctest.h>
namespace cbt
{
behavior_t inverter(behavior_t x)
{
	return [child = std::move(x)](continuation resume)
	{
		return child([&](Status s) { return resume(! s); });
	};
}
TEST_CASE("inverter")
{
	auto result   = Invalid;
	auto cb = [&](Status s) { result = s; };
	auto leaf = behavior_t([]{ return Success; });
	SUBCASE("single inversion")
	{
		auto i = inverter(std::move(leaf));
		REQUIRE(result == Invalid);
		i(cb);
		REQUIRE(result == Failure);
	};
	SUBCASE("double inversion")
	{
		auto i = inverter(inverter(std::move(leaf)));
		REQUIRE(result == Invalid);
		i(cb);
		REQUIRE(result == Success);
	};
}

behavior_t repeater(behavior_t x, size_t limit)
{
	return [child = std::move(x), limit](continuation resume)
	{
		if (limit == 0) return resume(Success);
		else return child([&, limit=limit] (Status s) mutable
		{
			if (--limit == 0 || s == Failure) return resume(s);
			else return child();
		});
	};
}

TEST_CASE("repeater")
{
	auto count = 0;
	auto result = Invalid;
	auto cb = [&](Status s) { result = s; };
	auto leaf = behavior_t([&]{ ++count; return Success; });
	SUBCASE("Repeat 5 times")
	{
		auto r = repeater(std::move(leaf), 5);
		r(cb);
		REQUIRE(count == 5);
		REQUIRE(result == Success);
	}
	SUBCASE("Nested Repeat 5*5 times")
	{
		auto r = repeater(repeater(std::move(leaf), 5), 5);
		r(cb);
		REQUIRE(count == 25);
		REQUIRE(result == Success);
	}
	SUBCASE("Fail at third iteration")
	{
		leaf = [&]{ return ++count == 3 ? Failure : Success; };
		auto r = repeater(std::move(leaf), 5);
		r(cb);
		REQUIRE(count == 3);
		REQUIRE(result == Failure);
	}
	SUBCASE("Repeat 0 times")
	{
		auto r = repeater(std::move(leaf), 0);
		r(cb);
		REQUIRE(count == 0);
		REQUIRE(result == Success);
	}
}
} // cbt
