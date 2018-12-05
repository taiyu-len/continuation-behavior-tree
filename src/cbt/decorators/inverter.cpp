#include "cbt/decorators/inverter.hpp"
#include "cbt/behavior.hpp"
#include "cbt/spawn.hpp"
#include <doctest/doctest.h>
namespace cbt
{
namespace {
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
struct inverter_t
{
	behavior_t   child;
	continuation resume{};
	auto operator()(continuation _resume) noexcept -> continues
	{
		resume = std::move(_resume);
		return continues::down(
			child,
			continuation::mem_fn<&inverter_t::next>(*this));
	}
	auto next(Status s) noexcept -> continues
	{
		return continues::up(std::move(resume), !s);
	}
};
}

behavior_t inverter(behavior_t&& x)
{
	return inverter_t{ std::move(x) };
}
} // cbt
