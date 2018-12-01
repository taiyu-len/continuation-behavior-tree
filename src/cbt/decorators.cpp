#include "cbt/decorators.hpp"
#include <cassert>
#include <doctest/doctest.h>
#include <cmath>

namespace cbt
{
struct inverter_t
{
	behavior_t child;
	continuation resume{};
	continuation_type c{};
	void operator()(continuation&& _resume) noexcept
	{
		assert(_resume != nullptr);
		resume = std::move(_resume);
		c = [this](Status s)
		{
			return resume(!s);
		};
		return child.run(c);
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
		auto bt = inverter(std::move(leaf));
		REQUIRE(result == Invalid);
		bt.run(cb);
		REQUIRE(result == Failure);
	};
	SUBCASE("double inversion")
	{
		auto bt = inverter(inverter(std::move(leaf)));
		REQUIRE(result == Invalid);
		bt.run(cb);
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
	void operator()(continuation&& _resume) noexcept
	{
		assert(_resume != nullptr);
		count = 0;
		resume = std::move(_resume);
		if (limit == 0) return resume(Success);
		c = [this](Status s)
		{
			assert(count < limit);
			if (++count == limit || s == Failure) return resume(s);
			else return child.run(c);
		};
		return child.run(c);
	}
};

behavior_t repeater(behavior_t&& x, size_t limit)
{
	return repeater_t{ std::move(x), limit };
}

intptr_t stack_pointer()
{
	char p;
#if defined(__GNUC__)
	// gcc has, and requires std::launder for this to work
	return reinterpret_cast<intptr_t>(std::launder(&p));
#else // defined(__clang__)
	// clang does not have std::launder, but works without it
	return reinterpret_cast<intptr_t>(&p);
#endif
}

TEST_CASE("repeater")
{
	auto bottom = stack_pointer();
	auto top    = bottom;

	auto count = 0;
	auto result = Invalid;
	auto cb = continuation_type([&](Status s)
	{
		result = s; top = stack_pointer();
	});
	auto leaf = behavior_t([&]{ ++count; return Success; });
	SUBCASE("Repeat 5 times")
	{
		auto bt = repeater(std::move(leaf), 5);
		bt.run(cb);
		REQUIRE(count == 5);
		REQUIRE(result == Success);
		MESSAGE("stack usage = " << std::abs(bottom - top));
		// using continuation by rvalue reference
		// g++     -Og -ggdb: 3792
		// g++     -O3      : 745
		// g++     -Os      : 753
		// clang++ -O3      : 769
		// clang++ -Os      : 745
		// using continuation by value
		// g++     -Os      : 1041
	}
	SUBCASE("Nested Repeat 5*5 times")
	{
		auto bt = repeater(repeater(std::move(leaf), 5), 5);
		bt.run(cb);
		REQUIRE(count == 25);
		REQUIRE(result == Success);
		MESSAGE("stack usage = " << std::abs(bottom - top));
		// using continuation by rvalue reference
		// g++     -Og -ggdb: 20672
		// g++     -O3      : 3225
		// g++     -Os      : 3153
		// clang++ -O3      : 3489
		// clang++ -Os      : 3305
		// using continuation by value
		// g++     -Os      : 4641
	}
	SUBCASE("Fail at third iteration")
	{
		leaf = [&]{ return ++count == 3 ? Failure : Success; };
		auto bt = repeater(std::move(leaf), 5);
		bt.run(cb);
		REQUIRE(count == 3);
		REQUIRE(result == Failure);
	}
	SUBCASE("Repeat 0 times")
	{
		auto bt = repeater(std::move(leaf), 0);
		bt.run(cb);
		REQUIRE(count == 0);
		REQUIRE(result == Success);
	}
}
} // cbt
