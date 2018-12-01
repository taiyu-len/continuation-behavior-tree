#include "cbt/behavior.hpp"
#include <utility>
#include <doctest/doctest.h>
#include <cassert>
namespace cbt
{

void behavior_t::operator()(continuation c) const
{
	assert(_object != nullptr);
	assert(c       != nullptr);
	_object->start(std::move(c));
}

TEST_CASE("behavior")
{
	auto count = 0;
	auto result = Status::Invalid;
	auto cb = continuation_type([&](Status s) { result = s; });
	SUBCASE("leaf model")
	{
		auto b = behavior_t([&]{ ++count; return Success; });
		REQUIRE(count == 0);
		REQUIRE(result == Invalid);
		b(cb);
		REQUIRE(result == Success);
		REQUIRE(count == 1);
	}
	SUBCASE("continuation model")
	{
		auto b = behavior_t([&](continuation c){ ++count; c(Success); });
		REQUIRE(count == 0);
		REQUIRE(result == Invalid);
		b(cb);
		REQUIRE(result == Success);
		REQUIRE(count == 1);
	}
	SUBCASE("external continuation")
	{
		continuation cc;
		auto b = behavior_t([&](continuation c){ ++count; cc = std::move(c); });
		REQUIRE(count == 0);
		REQUIRE(result == Invalid);
		b(cb);
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
