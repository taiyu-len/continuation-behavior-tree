#include "cbt/nodes/select.hpp"
#include "cbt/nodes/sequence.hpp"
#include "cbt/nodes/spawn.hpp"
#include <algorithm>
#include <doctest/doctest.h>
namespace cbt
{
namespace {
TEST_CASE("sequence")
{
	int  count[3] = {0, 0, 0};
	auto result = status::unknown;
	auto cb = [&](status s) { result = s; };
	SUBCASE("Succeed sequence all 3 times")
	{
		spawn(sequence(
			[&]{ ++count[0]; return status::success; },
			[&]{ ++count[1]; return status::success; },
			[&]{ ++count[2]; return status::success; }
		), cb);
		REQUIRE(result == status::success);
		REQUIRE(count[0] == 1);
		REQUIRE(count[1] == 1);
		REQUIRE(count[2] == 1);
	}
	SUBCASE("Fail in middle of sequence ")
	{
		spawn(sequence(
			[&]{ ++count[0]; return status::success; },
			[&]{ ++count[1]; return status::failure; },
			[&]{ ++count[2]; return status::success; }
		), cb);
		REQUIRE(result == status::failure);
		REQUIRE(count[0] == 1);
		REQUIRE(count[1] == 1);
		REQUIRE(count[2] == 0);
	}
	SUBCASE("abort early")
	{
		spawn(sequence(
			[&]{ ++count[0]; return status::success; },
			[&]{ ++count[1]; return status::aborted; },
			[&]{ ++count[2]; return status::success; }
		), cb);
		REQUIRE(result == status::aborted);
		REQUIRE(count[0] == 1);
		REQUIRE(count[1] == 1);
		REQUIRE(count[2] == 0);
	}
}
TEST_CASE("select")
{
	int  count[3] = {0, 0, 0};
	auto result = status::unknown;
	auto cb = [&](status s) { result = s; };
	SUBCASE("Select first success")
	{
		spawn(select(
			[&]{ ++count[0]; return status::failure; },
			[&]{ ++count[1]; return status::success; },
			[&]{ ++count[2]; return status::failure; }
		), cb);
		REQUIRE(count[0] == 1);
		REQUIRE(count[1] == 1);
		REQUIRE(count[2] == 0);
		REQUIRE(result == status::success);
	}
	SUBCASE("Select all fail")
	{
		spawn(select(
			[&]{ ++count[0]; return status::failure; },
			[&]{ ++count[1]; return status::failure; },
			[&]{ ++count[2]; return status::failure; }
		), cb);
		REQUIRE(result == status::failure);
		REQUIRE(count[0] == 1);
		REQUIRE(count[1] == 1);
		REQUIRE(count[2] == 1);
	}
	SUBCASE("abort early")
	{
		spawn(select(
			[&]{ ++count[0]; return status::failure; },
			[&]{ ++count[1]; return status::aborted; },
			[&]{ ++count[2]; return status::failure; }
		), cb);
		REQUIRE(result == status::aborted);
		REQUIRE(count[0] == 1);
		REQUIRE(count[1] == 1);
		REQUIRE(count[2] == 0);
	}
}

template<status S>
struct sequence_while
{
	std::unique_ptr<behavior[]> children = {};
	std::uint8_t size = 0;
	std::uint8_t index = 0;
	continuation resume = {};

	auto operator()(continuation _resume) noexcept -> continues
	{
		resume = std::move(_resume);
		index = 0;
		return step(S);
	}
	auto next(status s) noexcept -> continues
	{
		++index;
		return step(s);
	}
	auto step(status s) noexcept -> continues
	{
		if (s == S && index < size)
		{
			return CBT_DOWN(children[index], next);
		}
		return continues::up(std::move(resume), s);
	}
};
} // namespace
using sequence_t = sequence_while<status::success>;
using select_t   = sequence_while<status::failure>;

auto select_impl(behavior *data, std::uint8_t size) -> behavior
{
	auto p = std::make_unique<behavior[]>(size);
	std::move(data, data+size, p.get());
	return select_t{std::move(p), size};
}
auto sequence_impl(behavior *data, std::uint8_t size) -> behavior
{
	auto p = std::make_unique<behavior[]>(size);
	std::move(data, data+size, p.get());
	return sequence_t{std::move(p), size};
}
} // namespace cbt
