#include "cbt/behavior.hpp"
#include "cbt/nodes/spawn.hpp"
#include <utility>
#include <doctest/doctest.h>
#include <cassert>
namespace cbt
{

void behavior::run(continuation c) const noexcept
{
	continues::down(*this, std::move(c)).run();
}

auto behavior::step(continuation c) const noexcept -> continues
{
	assert(_object != nullptr);
	assert(c);
	return _object->start(std::move(c));
}

TEST_CASE("behavior")
{
	auto count = 0;
	auto result = status::unknown;
	auto cb = [&](status s) { result = s; };
	SUBCASE("leaf model")
	{
		auto bt = [&]{ ++count; return status::success; };
		REQUIRE(count == 0);
		REQUIRE(result == status::unknown);
		spawn(bt, cb);
		REQUIRE(result == status::success);
		REQUIRE(count == 1);
	}
	SUBCASE("boolean model")
	{
		auto bt = [&]{ ++count; return true; };
		REQUIRE(count == 0);
		REQUIRE(result == status::unknown);
		spawn(bt, cb);
		REQUIRE(result == status::success);
		REQUIRE(count == 1);
	}
	SUBCASE("continuation model")
	{
		auto bt = [&](continuation c)
		{
			++count;
			return continues::up(std::move(c), status::success);
		};
		REQUIRE(count == 0);
		REQUIRE(result == status::unknown);
		spawn(bt, cb);
		REQUIRE(result == status::success);
		REQUIRE(count == 1);
	}
	SUBCASE("external continuation")
	{
		continuation cc;
		auto bt = [&](continuation c)
		{
			++count;
			cc = std::move(c);
		};
		REQUIRE(count == 0);
		REQUIRE(result == status::unknown);
		spawn(bt, cb);
		REQUIRE(result == status::unknown);
		REQUIRE(count == 1);
		cc(status::success);
		REQUIRE(count == 1);
		REQUIRE(result == status::success);
	}
}
} // cbt
