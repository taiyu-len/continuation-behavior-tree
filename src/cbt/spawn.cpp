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
	SUBCASE("respawning")
	{
		struct respawn {
			int    &count;
			Status &result;
			void operator ()(behavior_t b, Status s) {
				if (++count == 5 || s == Failure)
					result = s;
				else return spawn(std::move(b), *this);
			}
		};
		continuation c;
		spawn(
			[&](continuation cc){ c = std::move(cc); },
			respawn{count, result});
		c(Success);
		REQUIRE(c != nullptr);
		REQUIRE(count == 1);
		REQUIRE(result == Invalid);
		SUBCASE("Fail early") {
			c(Failure);
			REQUIRE(c == nullptr);
			REQUIRE(count == 2);
			REQUIRE(result == Failure);
		}
		SUBCASE("Finish loop") {
			while (c) c(Success);
			REQUIRE(c == nullptr);
			REQUIRE(count == 5);
			REQUIRE(result == Success);
		}
	}
}
} // cbt
