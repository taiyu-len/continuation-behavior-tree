#include "cbt/behavior.hpp"
#include "cbt/nodes/boolean.hpp"
#include "cbt/nodes/spawn.hpp"
#include <doctest/doctest.h>
namespace cbt
{
namespace {
TEST_CASE("boolean-nodes")
{
	auto result = status::unknown;
	auto cb    = [&](status s) { result = s; };
	auto test = [&](behavior &&b, status s) {
		spawn(std::move(b), cb);
		REQUIRE(result == s);
	};
	auto True  = []{ return status::success; };
	auto False = []{ return status::failure; };
	SUBCASE("negate")
	{
		test(negate(True),  status::failure);
		test(negate(False), status::success);
	};
	SUBCASE("double negate")
	{
		test(negate(negate(True)), status::success);
		test(negate(negate(False)), status::failure);
	};
	SUBCASE("implies")
	{
		test(implies(True, True), status::success);
		test(implies(True, False), status::failure);
		test(implies(False, True), status::success);
		test(implies(False, False), status::success);
	};
	SUBCASE("equals")
	{
		test(equals(True, True), status::success);
		test(equals(True, False), status::failure);
		test(equals(False, True), status::failure);
		test(equals(False, False), status::success);
	};
	SUBCASE("differs")
	{
		test(differs(True, True), status::failure);
		test(differs(True, False), status::success);
		test(differs(False, True), status::success);
		test(differs(False, False), status::failure);
	};
}

struct negate_t
{
	behavior   child;
	continuation resume{};
	auto operator()(continuation _resume) noexcept -> continues
	{
		resume = std::move(_resume);
		return continues::down(
			child,
			continuation::mem_fn<&negate_t::next>(*this));
	}
	auto next(status s) noexcept -> continues
	{
		return continues::up(std::move(resume), !s);
	}
};

template<typename T>
struct base_t
{
	behavior   x, y;
	continuation resume{};
	auto operator()(continuation _resume) noexcept -> continues
	{
		resume = std::move(_resume);
		auto self = static_cast<T*>(this);
		return continues::down(
			x,
			continuation::mem_fn<&T::next>(*self));
	}
};

struct implies_t : base_t<implies_t>
{
	auto next(status s) noexcept -> continues
	{
		if (s == status::failure)
		{
			return continues::up(std::move(this->resume), status::success);
		}
		return continues::down(y, std::move(this->resume));
	}
};

struct equals_t : base_t<equals_t>
{
	status first = status::unknown;
	auto next(status s) noexcept -> continues
	{
		first = s;
		return continues::down(
			y,
			continuation::mem_fn<&equals_t::last>(*this));
	}
	auto last(status s) noexcept -> continues
	{
		return continues::up(
			std::move(resume),
			s == first ? status::success : status::failure);
	}
};

struct differs_t : base_t<differs_t>
{
	status first = status::unknown;
	auto next(status s) noexcept -> continues
	{
		first = s;
		return continues::down(
			y,
			continuation::mem_fn<&differs_t::last>(*this));
	}
	auto last(status s) noexcept -> continues
	{
		return continues::up(
			std::move(resume),
			s != first ? status::success : status::failure);
	}
};
}

auto negate(behavior&& x) -> behavior
{ return negate_t{ std::move(x) }; }

auto implies(behavior&& x, behavior&& y) -> behavior
{ return implies_t{ std::move(x), std::move(y) }; }

auto equals(behavior&& x, behavior&& y) -> behavior
{ return equals_t{ std::move(x), std::move(y) }; }

auto differs(behavior&& x, behavior&& y) -> behavior
{ return differs_t{ std::move(x), std::move(y) }; }

} // cbt
