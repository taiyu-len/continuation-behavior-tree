#include "cbt/composites/sequence.hpp"
#include "cbt/behavior.hpp"
#include "cbt/spawn.hpp"
#include <doctest/doctest.h>
namespace cbt
{
namespace {
TEST_CASE("sequence")
{
	int  count[3] = {0, 0, 0};
	auto result = Invalid;
	auto cb = [&](Status s) { result = s; };
	SUBCASE("Succeed sequence all 3 times")
	{
		spawn(sequence(
			[&]{ ++count[0]; return Success; },
			[&]{ ++count[1]; return Success; },
			[&]{ ++count[2]; return Success; }
		), cb);
		REQUIRE(result == Success);
		REQUIRE(count[0] == 1);
		REQUIRE(count[1] == 1);
		REQUIRE(count[2] == 1);
	}
	SUBCASE("Fail in middle of sequence ")
	{
		spawn(sequence(
			[&]{ ++count[0]; return Success; },
			[&]{ ++count[1]; return Failure; },
			[&]{ ++count[2]; return Success; }
		), cb);
		REQUIRE(result == Failure);
		REQUIRE(count[0] == 1);
		REQUIRE(count[1] == 1);
		REQUIRE(count[2] == 0);
	}
}
struct sequence_t
{
	std::vector<behavior_t> children;
	size_t index = 0;
	continuation resume = {};
	continuation_type c = {};
	auto operator()(continuation _resume) noexcept -> continues
	{
		resume = std::move(_resume);
		index = 0;
		c = [this](Status s) -> continues
		{
			++index;
			return step(s);
		};
		return step(Success);
	}
	auto step(Status s) noexcept -> continues
	{
		if (index == children.size() || s == Failure)
		{
			return continues::up(std::move(resume), s);
		}
		return continues::down(children[index], c);
	}
};
}

behavior_t sequence(std::vector<behavior_t>&& v)
{
	return sequence_t{ std::move(v) };
}
} // cbt
