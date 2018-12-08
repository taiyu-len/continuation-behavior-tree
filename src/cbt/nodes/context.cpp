#include "cbt/behavior.hpp"
#include "cbt/nodes/context.hpp"
#include "cbt/nodes/sequence.hpp"
#include "cbt/nodes/repeat.hpp"
#include "cbt/nodes/spawn.hpp"
#include <doctest/doctest.h>
namespace cbt
{
static auto spawn_counter(continuation &c, int init, int count) -> int&
{
	// create context node that initializes type with init value
	auto [node, state] = context<int>([init](int &i){ i = init; });
	auto inc = [&state = *state, &c](continuation _c)
	{
		state++;
		c = std::move(_c);
	};
	// create a sequence that initializes the state, and calls increment a
	// number of times
	spawn(sequence(std::move(node), repeat_n(inc, count)));
	return *state;
}

TEST_CASE("context")
{
	continuation c;
	SUBCASE("0-3")
	{
		// the state is only safely dereferencable as long as we have
		// continuation to call
		int &state = spawn_counter(c, 0, 3);
		// we hit the first leaf, incrementing state, and setting the
		// continuation. and continue with success
		REQUIRE_UNARY(c && state == 1);
		c(Success);

		// as above
		REQUIRE_UNARY(c && state == 2);
		c(Success);

		// as above
		REQUIRE_UNARY(c && state == 3);
		// this time the continuation will finish and free the behavior
		// tree, leaving it invalid
		c(Success);
		REQUIRE_FALSE(c);
	}
	SUBCASE("4-5")
	{
		int &state = spawn_counter(c, 4, 2);
		REQUIRE(state == 5);
		REQUIRE(c); c(Success);
		REQUIRE(state == 6);
		REQUIRE(c); c(Success);
		REQUIRE(!c);
	}
}
}
