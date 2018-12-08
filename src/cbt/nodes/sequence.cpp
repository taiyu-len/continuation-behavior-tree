#include "cbt/nodes/sequence.hpp"
#include "cbt/nodes/select.hpp"
#include "cbt/nodes/spawn.hpp"
#include <doctest/doctest.h>
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
	SUBCASE("Select first success")
	{
		spawn(select(
			[&]{ ++count[0]; return Failure; },
			[&]{ ++count[1]; return Success; },
			[&]{ ++count[2]; return Failure; }
		), cb);
		REQUIRE(count[0] == 1);
		REQUIRE(count[1] == 1);
		REQUIRE(count[2] == 0);
		REQUIRE(result == Success);
	}
	SUBCASE("Select all fail")
	{
		spawn(select(
			[&]{ ++count[0]; return Failure; },
			[&]{ ++count[1]; return Failure; },
			[&]{ ++count[2]; return Failure; }
		), cb);
		REQUIRE(result == Failure);
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

template<Status S>
struct sequence_while
{
	std::unique_ptr<behavior_t[]> children;
	std::uint8_t size;
	std::uint8_t index = 0;
	continuation resume = {};

	auto operator()(continuation _resume) noexcept -> continues
	{
		resume = std::move(_resume);
		index = 0;
		return step(S);
	}
	auto next(Status s) noexcept -> continues
	{
		++index;
		return step(s);
	}
	auto step(Status s) noexcept -> continues
	{
		if (index == size || s == !S)
		{
			return continues::up(std::move(resume), s);
		}
		return continues::down(
			children[index],
			continuation::mem_fn<&sequence_while::next>(*this));
	}
};
using sequence_t = sequence_while<Success>;
using select_t = sequence_while<Failure>;

}

auto select_impl(behavior_t *data, std::uint8_t size) -> behavior_t
{
	auto p = std::make_unique<behavior_t[]>(size);
	std::move(data, data+size, p.get());
	return select_t{std::move(p), size};
}
auto sequence_impl(behavior_t *data, std::uint8_t size) -> behavior_t
{
	auto p = std::make_unique<behavior_t[]>(size);
	std::move(data, data+size, p.get());
	return sequence_t{std::move(p), size};
}
} // cbt
