#include "cbt/composites/sequence.hpp"
#include "cbt/behavior.hpp"
#include "cbt/spawn.hpp"
#include <doctest/doctest.h>
#include <memory>
#include <algorithm>
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
	std::unique_ptr<behavior_t[]> children;
	std::uint8_t size;
	std::uint8_t index = 0;
	continuation resume = {};

	auto operator()(continuation _resume) noexcept -> continues
	{
		resume = std::move(_resume);
		index = 0;
		return step(Success);
	}
	auto next(Status s) noexcept -> continues
	{
		++index;
		return step(s);
	}
	auto step(Status s) noexcept -> continues
	{
		if (index == size || s == Failure)
		{
			return continues::up(std::move(resume), s);
		}
		return continues::down(
			children[index],
			continuation::mem_fn<&sequence_t::next>(*this));
	}
};
}

auto sequence(behavior_t *data, std::uint8_t size) -> behavior_t
{
	auto p = std::make_unique<behavior_t[]>(size);
	std::move(data, data+size, p.get());
	return sequence_t{std::move(p), size};
}
} // cbt
