#include "cbt/composites.hpp"
#include <doctest/doctest.h>
namespace cbt
{
struct sequence_t
{
	std::vector<behavior_t> children;
	size_t index;
	void operator()(continuation resume)
	{
		if (children.size() == 0) return resume(Success);
		index = 0;
		return next(std::move(resume));
	}
	void next(continuation resume)
	{
		children[index]([this, &resume](Status s)
		{
		if (++index == children.size() || s == Failure) return resume(s);
		else return next(std::move(resume));
		});
	}
};

behavior_t sequence(std::vector<behavior_t> v)
{
	return sequence_t{ std::move(v), 0 };
}
TEST_CASE("sequence")
{
	int  count[3] = {0, 0, 0};
	auto result = Invalid;
	auto cb = [&](Status s) { result = s; };
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
