#include "cbt/behavior.hpp"
#include "cbt/nodes/spawn.hpp"
#include <utility>
#include <doctest/doctest.h>
#include <cassert>
namespace cbt
{

void behavior::run(continuation c) const noexcept
{ continues::down(*this, std::move(c)).run(); }

auto behavior::step(continuation c) const noexcept -> continues
{
	assert(_object != nullptr);
	assert(c);
	return _object->start(std::move(c));
}

TEST_CASE("behavior")
{
	auto count = 0;
	auto result = Status::Invalid;
	auto cb = [&](Status s) { result = s; };
	SUBCASE("leaf model")
	{
		auto bt = behavior([&]{ ++count; return Success; });
		REQUIRE(count == 0);
		REQUIRE(result == Invalid);
		spawn(std::move(bt), cb);
		REQUIRE(result == Success);
		REQUIRE(count == 1);
	}
	SUBCASE("continuation model")
	{
		auto bt = behavior([&](continuation c)
		{
			++count;
			return continues::up(std::move(c), Success);
		});
		REQUIRE(count == 0);
		REQUIRE(result == Invalid);
		spawn(std::move(bt), cb);
		REQUIRE(result == Success);
		REQUIRE(count == 1);
	}
	SUBCASE("external continuation")
	{
		continuation cc;
		auto bt = behavior([&](continuation c)
		{
			++count;
			cc = std::move(c);
		});
		REQUIRE(count == 0);
		REQUIRE(result == Invalid);
		spawn(std::move(bt), cb);
		REQUIRE(result == Invalid);
		REQUIRE(count == 1);
		SUBCASE("Success")
		{
			cc(Success);
			REQUIRE(count == 1);
			REQUIRE(result == Success);
		}
		SUBCASE("Failure")
		{
			cc(Failure);
			REQUIRE(count == 1);
			REQUIRE(result == Failure);
		}
	}
}
} // cbt
