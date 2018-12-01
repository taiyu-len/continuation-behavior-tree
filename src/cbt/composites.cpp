#include "cbt/composites.hpp"
#include <doctest/doctest.h>
namespace cbt
{
struct sequence_t
{
	std::vector<behavior_t> children;
	size_t index = 0;
	continuation resume = {};
	continuation_type c = {};
	void operator()(continuation _resume)
	{
		index = 0;
		resume = std::move(_resume);
		if (children.size() == 0) return resume(Success);
		c = [this](Status s) {
			if (++index == children.size() || s == Failure)
			{
				return resume(s);
			}
			return children[index](c);
		};
		children[0](c);
	}
};

behavior_t sequence(std::vector<behavior_t> v)
{
	return sequence_t{ std::move(v) };
}
TEST_CASE("sequence")
{
	int  count[3] = {0, 0, 0};
	auto result = Invalid;
	auto cb = continuation_type([&](Status s) { result = s; });
	SUBCASE("Succeed sequence all 3 times")
	{
		auto seq = sequence(
			[&]{ ++count[0]; return Success; },
			[&]{ ++count[1]; return Success; },
			[&]{ ++count[2]; return Success; }
		);
		seq(cb);
		REQUIRE(result == Success);
		REQUIRE(count[0] == 1);
		REQUIRE(count[1] == 1);
		REQUIRE(count[2] == 1);
	}
	SUBCASE("Fail in middle of sequence ")
	{
		auto seq = sequence(
			[&]{ ++count[0]; return Success; },
			[&]{ ++count[1]; return Failure; },
			[&]{ ++count[2]; return Success; }
		);
		seq(cb);
		REQUIRE(result == Failure);
		REQUIRE(count[0] == 1);
		REQUIRE(count[1] == 1);
		REQUIRE(count[2] == 0);
	}
}
} // cbt
