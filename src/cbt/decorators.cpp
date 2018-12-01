#include "cbt/decorators.hpp"
#include <doctest/doctest.h>
namespace cbt
{
struct inverter_t
{
	behavior_t child;
	continuation resume{};
	continuation_type c{};
	void operator()(continuation _resume)
	{
		resume = std::move(_resume);
		return child(c = [this](Status s)
		{
			return resume(!s);
		});
	}
};

behavior_t inverter(behavior_t&& x)
{
	return inverter_t{ std::move(x) };
}

TEST_CASE("inverter")
{
	auto result   = Invalid;
	auto cb = continuation_type([&](Status s) { result = s; });
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

struct repeater_t
{
	behavior_t child;
	size_t limit;
	size_t count = 0;
	continuation resume{};
	continuation_type c{};
	void operator()(continuation _resume)
	{
		count = 0;
		resume = std::move(_resume);
		if (limit == 0) return resume(Success);
		c = [this](Status s)
		{
			if (++count == limit || s == Failure) return resume(s);
			else return child(c);
		};
		return child(c);
	}
};

behavior_t repeater(behavior_t&& x, size_t limit)
{
	return repeater_t{ std::move(x), limit };
}

TEST_CASE("repeater")
{
	auto count = 0;
	auto result = Invalid;
	auto cb = continuation_type([&](Status s) { result = s; });
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
