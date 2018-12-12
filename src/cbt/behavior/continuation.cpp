#include "cbt/behavior/continuation.hpp"
#include "cbt/behavior/continues.hpp"
#include <utility>
#include <cassert>
#include <doctest/doctest.h>

namespace cbt
{
namespace {
struct continue_test
{
	status& result;
	auto fn(status s) -> continues {
		result = s;
		return continues::finished();
	}
};
}
TEST_CASE("continuation")
{
	static auto result = status::unknown;
	auto ct = continue_test{result};
	auto f  = [](status s) -> continues
	{
		result = s;
		return continues::finished();
	};

	auto cmf = continuation::mem_fn<&continue_test::fn>(ct);
	auto cf  = continuation(f);

	SUBCASE("continuation may only be called once")
	{
		auto x = [&](continuation& c)
		{
			c(status::success);
			REQUIRE(result == status::success);
			REQUIRE(c == nullptr);
		};
		x(cmf);
		x(cf);
	}
	SUBCASE("continuation is empty after move")
	{
		auto x = [&](continuation& c)
		{
			auto c2 = std::move(c);
			REQUIRE(c == nullptr);
			c2(status::failure);
			REQUIRE(result == status::failure);
			REQUIRE(c2 == nullptr);
		};
		x(cmf);
		x(cf);
	}
	SUBCASE("continuation may be recreated from original")
	{
		cf(status::success);
		cmf(status::success);
		REQUIRE(cf == nullptr);
		REQUIRE(cmf == nullptr);
		cf = f;
		REQUIRE(cf != nullptr);
		cf(status::success);
		REQUIRE(cf == nullptr);
	}
}

continuation::continuation(continuation&& x) noexcept
: _func(std::exchange(x._func, nullptr))
, _that(std::exchange(x._that, nullptr))
{}

continuation& continuation::operator=(continuation&& x) noexcept
{
	assert(_func == nullptr && _that == nullptr);
	_func = std::exchange(x._func, nullptr);
	_that = std::exchange(x._that, nullptr);
	return *this;
}

continuation::continuation(func1_t f) noexcept
: _func1{f}
, _that{nullptr}
{}

auto continuation::operator=(func1_t f) noexcept -> continuation&
{ return *this = continuation{f}; }

continuation::~continuation() noexcept
{ assert(_func == nullptr && _that == nullptr); }

void continuation::operator()(status s) noexcept
{
	continues::up(std::move(*this), s).run();
}

continuation::operator bool() const noexcept
{ return _func != nullptr; }

bool operator==(std::nullptr_t, continuation const& x) noexcept
{ return x._func == nullptr; }

bool operator==(continuation const& x, std::nullptr_t) noexcept
{ return x._func == nullptr; }

bool operator==(continuation const& x, continuation const& y) noexcept
{ return x._func == y._func && x._that == y._that; }

bool operator!=(std::nullptr_t, continuation const& x) noexcept
{ return !(nullptr == x); }

bool operator!=(continuation const& x, std::nullptr_t) noexcept
{ return !(x == nullptr); }

bool operator!=(continuation const& x, continuation const& y) noexcept
{ return !(x == y); }

void swap(continuation& x, continuation& y) noexcept
{
	std::swap(x._func, y._func);
	std::swap(x._that, y._that);
}

auto continuation::step(status s) noexcept -> continues
{
	if (_that == nullptr)
	{
		assert(_func1 != nullptr);
		return std::exchange(_func1, nullptr)(s);
	}
	else
	{
		assert(_func != nullptr);
		return std::exchange(_func, nullptr)(
			std::exchange(_that, nullptr), s);
	}
}

} // cbt
