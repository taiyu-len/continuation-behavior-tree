#include "cbt/nodes.hpp"
#include <utility>
#include <new>
#include <cmath>
#include <doctest/doctest.h>
namespace cbt
{
static intptr_t stack_address() noexcept
{
	char p;
#if defined(__clang__)
	// clang does not have std::launder, but works without it
	return reinterpret_cast<intptr_t>(&p);
#else // defined(__GNUC__)
	// gcc has, and requires std::launder for this to work
	return reinterpret_cast<intptr_t>(std::launder(&p));
#endif
}

static intptr_t stack_usage(behavior_t &x)
{
	intptr_t const caller = stack_address();
	intptr_t callee;
	continuation_type cb = [&](Status) { callee = stack_address(); };

	x.run(cb);
	return std::abs(caller - callee);
}

TEST_CASE("tail-calls")
{
	auto leaf = []{ return behavior_t([]{ return Success; }); };
	SUBCASE("inverter")
	{
		auto x = leaf();
		auto y = leaf();
		for (auto i = 0; i != 4; ++i) { x = inverter(std::move(x)); }
		for (auto i = 0; i != 8; ++i) { y = inverter(std::move(y)); }
		CHECK(stack_usage(x) == stack_usage(y));
	}
	SUBCASE("repeater")
	{
		auto x = leaf();
		auto y = leaf();
		for (auto i = 0; i != 2; ++i) { x = repeater(std::move(x), 2); }
		for (auto i = 0; i != 4; ++i) { y = repeater(std::move(y), 2); }
		CHECK(stack_usage(x) == stack_usage(y));
	}
	SUBCASE("sequence")
	{
		auto x = leaf();
		auto y = leaf();
		for (auto i = 0; i != 4; ++i)
		{
			x = sequence(leaf(), std::move(x), leaf());
		}
		for (auto i = 0; i != 8; ++i)
		{
			y = sequence(leaf(), std::move(y), leaf());
		}
		CHECK(stack_usage(x) == stack_usage(y));
	}
	SUBCASE("failure")
	{
		auto x = behavior_t([&](continuation c){ auto _ = leaf(); c(Success); });
		auto y = behavior_t([&](continuation c){ auto _ = leaf(); c(Success); });
		for (auto i = 0; i != 2; ++i) { x = repeater(std::move(x), 2); }
		for (auto i = 0; i != 4; ++i) { y = repeater(std::move(y), 2); }
		CHECK_FALSE(stack_usage(x) == stack_usage(y));
	}

}
} // cbt
