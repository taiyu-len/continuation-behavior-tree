#include "cbt/decorators/inverter.hpp"
#include "cbt/behavior.hpp"
#include "cbt/spawn.hpp"
#include <doctest/doctest.h>
namespace cbt
{
struct inverter_t
{
	behavior_t   child;
	continuation resume{};
	continuation_type c{};
	auto operator()(continuation _resume) noexcept -> continues
	{
		resume = std::move(_resume);
		c = [this](Status s) -> continues
		{
			return continues::up(std::move(resume), !s);
		};
		return continues::down(child, c);
	}
};

behavior_t inverter(behavior_t&& x)
{
	return inverter_t{ std::move(x) };
}

TEST_CASE("inverter")
{
	auto result = Invalid;
	auto cb   = [&](Status s) { result = s; };
	auto leaf = behavior_t([]{ return Success; });
	SUBCASE("single inversion")
	{
		spawn(inverter(std::move(leaf)), cb);
		REQUIRE(result == Failure);
	};
	SUBCASE("double inversion")
	{
		spawn(inverter(inverter(std::move(leaf))), cb);
		REQUIRE(result == Success);
	};
}
} // cbt
