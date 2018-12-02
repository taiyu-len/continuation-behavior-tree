#include "cbt/continuation.hpp"
#include <doctest/doctest.h>

namespace cbt
{
continuation::continuation(continuation_type const& x) noexcept
:_ref(&x) {};

auto continuation::operator=(continuation_type const& x) noexcept -> continuation&
{
	assert(_ref == nullptr);
	_ref = &x;
	return *this;
};

void continuation::operator()(Status s)
{
	(*std::exchange(_ref, nullptr))(s);
}
TEST_CASE("continuation") {
	auto result = Invalid;
	auto x = continuation_type([&](Status s) { result = s; });
	auto c = continuation(x);
	bool trivial = std::is_trivially_copyable<continuation>::value;
	SUBCASE("continuation may only be called once") {
		c(Success);
		REQUIRE(result == Success);
		REQUIRE_UNARY(trivial || c == nullptr);
	}
	SUBCASE("continuation is empty after move") {
		auto c2 = std::move(c);
		REQUIRE_UNARY(trivial || c == nullptr);
		c2(Failure);
		REQUIRE(result == Failure);
		REQUIRE_UNARY(trivial || c2 == nullptr);
	}
	SUBCASE("continuation may be recreated from original") {
		c(Success);
		REQUIRE_UNARY(trivial || c == nullptr);
		c = x;
		REQUIRE(c != nullptr);
		c(Success);
		REQUIRE_UNARY(trivial || c == nullptr);
	}
}
} // cbt
