#include "cbt/spawn.hpp"
#include <doctest/doctest.h>

namespace cbt
{
TEST_CASE("spawn")
{
	auto count = 0;
	auto result = Status::Invalid;
	auto cb = [&](behavior_t, Status s) {
		++count;
		result = s;
	};
	SUBCASE("spawn behavior tree")
	{
		spawn([]{ return Success; }, cb);
		REQUIRE(count == 1);
		REQUIRE(result == Success);
	}
	SUBCASE("spawn and continue")
	{
		continuation c;
		spawn([&](continuation cc){ ++count; c = std::move(cc); }, cb);
		REQUIRE(count == 1);
		REQUIRE(result == Invalid);
		REQUIRE(c != nullptr);
		c(Success);
		REQUIRE(count == 2);
		REQUIRE(result == Success);
	}
}
} // cbt
