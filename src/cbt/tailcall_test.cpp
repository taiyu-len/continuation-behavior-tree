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
	auto leaf_2 = []{ return behavior_t([](continuation c){ c(Success); }); };
	/*
	 * Disable test case, dont have a good way to get it to work yet
	 * which can get stack pointer, and avoids segfault in failing cases
	SUBCASE("unique-function")
	{
		intptr_t caller = stack_address();
		intptr_t callee = 0;
		static unique_function<void(int)> fn;
		fn = [](int i)
		{
			if (i > -1) fn(i+1);
			// prevents tail call
			// else callee = stack_address();
		};
		fn(0);
		intptr_t diff_1 = std::abs(caller - callee);
		fn(50);
		intptr_t diff_2 = std::abs(caller - callee);
		CHECK(diff_1 == diff_2);
	}
	*/
	SUBCASE("inverter")
	{
		SUBCASE("return leaf")
		{
			auto x = leaf();
			auto y = leaf();
			for (auto i = 0; i != 4; ++i) { x = inverter(std::move(x)); }
			for (auto i = 0; i != 8; ++i) { y = inverter(std::move(y)); }
			CHECK(stack_usage(x) == stack_usage(y));
		}
		SUBCASE("callback leaf")
		{
			auto x = leaf_2();
			auto y = leaf_2();
			for (auto i = 0; i != 4; ++i) { x = inverter(std::move(x)); }
			for (auto i = 0; i != 8; ++i) { y = inverter(std::move(y)); }
			CHECK(stack_usage(x) == stack_usage(y));
		}
	}
	SUBCASE("repeater")
	{
		SUBCASE("return leaf")
		{
			auto x = leaf();
			auto y = leaf();
			for (auto i = 0; i != 2; ++i) { x = repeater(std::move(x), 2); }
			for (auto i = 0; i != 4; ++i) { y = repeater(std::move(y), 2); }
			CHECK(stack_usage(x) == stack_usage(y));
		}
		SUBCASE("return leaf")
		{
			auto x = leaf_2();
			auto y = leaf_2();
			for (auto i = 0; i != 2; ++i) { x = repeater(std::move(x), 2); }
			for (auto i = 0; i != 4; ++i) { y = repeater(std::move(y), 2); }
			CHECK(stack_usage(x) == stack_usage(y));
		}
	}
	SUBCASE("sequence")
	{
		SUBCASE("return leaf")
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
		SUBCASE("callback leaf")
		{
			auto x = leaf_2();
			auto y = leaf_2();
			for (auto i = 0; i != 4; ++i)
			{
				x = sequence(leaf_2(), std::move(x), leaf_2());
			}
			for (auto i = 0; i != 8; ++i)
			{
				y = sequence(leaf_2(), std::move(y), leaf_2());
			}
			CHECK(stack_usage(x) == stack_usage(y));
		}
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
