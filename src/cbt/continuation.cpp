#include "cbt/continuation.hpp"
#include <cassert>
#include <doctest/doctest.h>

namespace cbt
{
continuation::continuation(continuation_type const& x) noexcept
:_ref(&x) {};

continuation::continuation(continuation&& x) noexcept
:_ref(x._ref)
{
	x._ref = nullptr;
}

auto continuation::operator=(continuation&& x) noexcept -> continuation&
{
	assert(_ref == nullptr);
	_ref = std::exchange(x._ref, nullptr);
	return *this;
}
auto continuation::operator=(continuation_type const& x) noexcept -> continuation&
{
	assert(_ref == nullptr);
	_ref = &x;
	return *this;
};

continuation::~continuation() noexcept
{
	// must be called once or moved from before destruction
	assert(_ref == nullptr);
}

void continuation::operator()(Status s)
{
	(*std::exchange(_ref, nullptr))(s);
}
TEST_CASE("continuation") {
	auto result = Invalid;
	auto x = continuation_type([&](Status s) { result = s; });
	auto c = continuation(x);
	SUBCASE("continuation may only be called once") {
		c(Success);
		REQUIRE(result == Success);
		REQUIRE(c == nullptr);
	}
	SUBCASE("continuation is empty after move") {
		auto c2 = std::move(c);
		REQUIRE(c == nullptr);
		c2(Failure);
		REQUIRE(result == Failure);
		REQUIRE(c2 == nullptr);
	}
	SUBCASE("continuation may be recreated from original") {
		c(Success);
		REQUIRE(c == nullptr);
		c = x;
		REQUIRE(c != nullptr);
		c(Success);
		REQUIRE(c == nullptr);
	}
}
} // cbt
