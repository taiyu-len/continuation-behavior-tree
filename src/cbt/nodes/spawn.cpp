#include "cbt/nodes/spawn.hpp"
#include <doctest/doctest.h>

namespace cbt
{
void spawn(behavior&& x)
{ spawn(std::move(x), +[](status){}); }

TEST_CASE("spawn")
{
	auto count = 0;
	auto result = status::unknown;
	auto cb = [&](behavior, status s)
	{
		++count;
		result = s;
	};
	SUBCASE("spawn behavior tree")
	{
		spawn([]{ return status::success; }, cb);
		REQUIRE(count == 1);
		REQUIRE(result == status::success);
	}
	SUBCASE("spawn and continue")
	{
		continuation c;
		spawn([&](continuation cc){ ++count; c = std::move(cc); }, cb);
		REQUIRE(count == 1);
		REQUIRE(result == status::unknown);
		REQUIRE(c != nullptr);
		c(status::success);
		REQUIRE(count == 2);
		REQUIRE(result == status::success);
	}
	SUBCASE("respawning")
	{
		struct respawn
		{
			int    &count;
			status &result;
			void operator ()(behavior b, status s)
			{
				if (++count == 5 || s == status::failure)
				{
					result = s;
				}
				else
				{
					spawn(std::move(b), *this);
				}
			}
		};
		continuation c;
		spawn(
			[&](continuation cc){ c = std::move(cc); },
			respawn{count, result});
		c(status::success);
		REQUIRE(c != nullptr);
		REQUIRE(count == 1);
		REQUIRE(result == status::unknown);
		SUBCASE("Fail early")
		{
			c(status::failure);
			REQUIRE(c == nullptr);
			REQUIRE(count == 2);
			REQUIRE(result == status::failure);
		}
		SUBCASE("Finish loop")
		{
			while (c)
			{
				c(status::success);
			}
			REQUIRE(c == nullptr);
			REQUIRE(count == 5);
			REQUIRE(result == status::success);
		}
	}
}
} // namespace cbt
