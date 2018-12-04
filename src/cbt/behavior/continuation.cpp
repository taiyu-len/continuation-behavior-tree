#include "cbt/behavior/continuation.hpp"
#include "cbt/behavior/continues.hpp"
#include <utility>
#include <cassert>
#include <doctest/doctest.h>

namespace cbt
{
continuation::continuation(continuation_type const& x) noexcept
: _ref(&x) {}

continuation::continuation(continuation&& x) noexcept
: _ref(std::exchange(x._ref, nullptr)) {}

continuation::~continuation() noexcept
{ assert(_ref == nullptr); }

auto continuation::operator=(continuation_type const& x) noexcept -> continuation&
{
	assert(_ref == nullptr);
	_ref = &x;
	return *this;
}

continuation& continuation::operator=(continuation&& x) noexcept
{
	assert(_ref == nullptr);
	_ref = std::exchange(x._ref, nullptr);
	return *this;
}

void continuation::operator()(Status s) noexcept
{
	assert(_ref != nullptr);
	continues::up(std::move(*this), s).run();
}

continuation::operator bool() const noexcept
{ return _ref != nullptr; }

bool operator==(std::nullptr_t, continuation const& x) noexcept
{ return x._ref == nullptr; }

bool operator==(continuation const& x, std::nullptr_t) noexcept
{ return x._ref == nullptr; }

bool operator==(continuation const& x, continuation const& y) noexcept
{ return x._ref == y._ref; }

bool operator!=(std::nullptr_t, continuation const& x) noexcept
{ return x._ref != nullptr; }

bool operator!=(continuation const& x, std::nullptr_t) noexcept
{ return x._ref != nullptr; }

bool operator!=(continuation const& x, continuation const& y) noexcept
{ return x._ref != y._ref; }

void swap(continuation& x, continuation& y) noexcept
{ std::swap(x._ref, y._ref); }

auto continuation::step(Status s) noexcept -> continues
{
	assert(_ref != nullptr);
	return (*std::exchange(_ref, nullptr))(s);
}

TEST_CASE("continuation")
{
	auto result = Invalid;
	auto x = continuation_type([&](Status s) -> continues
	{
		result = s;
		return continues::finished();
	});
	auto c = continuation(x);
	SUBCASE("continuation may only be called once")
	{
		c(Success);
		REQUIRE(result == Success);
		REQUIRE(c == nullptr);
	}
	SUBCASE("continuation is empty after move")
	{
		auto c2 = std::move(c);
		REQUIRE(c == nullptr);
		c2(Failure);
		REQUIRE(result == Failure);
		REQUIRE(c2 == nullptr);
	}
	SUBCASE("continuation may be recreated from original")
	{
		c(Success);
		REQUIRE(c == nullptr);
		c = x;
		REQUIRE(c != nullptr);
		c(Success);
		REQUIRE(c == nullptr);
	}
}

} // cbt
