#include "cbt/behavior.hpp"
#include "cbt/nodes/boolean.hpp"
#include "cbt/nodes/spawn.hpp"
#include <doctest/doctest.h>
namespace cbt
{
namespace {
TEST_CASE("boolean-nodes")
{
	auto result = Invalid;
	auto cb    = [&](Status s) { result = s; };
	auto test = [&](behavior_t &&b, Status s) {
		spawn(std::move(b), cb);
		REQUIRE(result == s);
	};
	auto True  = []{ return Success; };
	auto False = []{ return Failure; };
	SUBCASE("negate")
	{
		test(negate(True),  Failure);
		test(negate(False), Success);
	};
	SUBCASE("double negate")
	{
		test(negate(negate(True)), Success);
		test(negate(negate(False)), Failure);
	};
	SUBCASE("implies")
	{
		test(implies(True, True), Success);
		test(implies(True, False), Failure);
		test(implies(False, True), Success);
		test(implies(False, False), Success);
	};
	SUBCASE("equals")
	{
		test(equals(True, True), Success);
		test(equals(True, False), Failure);
		test(equals(False, True), Failure);
		test(equals(False, False), Success);
	};
	SUBCASE("differs")
	{
		test(differs(True, True), Failure);
		test(differs(True, False), Success);
		test(differs(False, True), Success);
		test(differs(False, False), Failure);
	};
}

struct negate_t
{
	behavior_t   child;
	continuation resume{};
	auto operator()(continuation _resume) noexcept -> continues
	{
		resume = std::move(_resume);
		return continues::down(
			child,
			continuation::mem_fn<&negate_t::next>(*this));
	}
	auto next(Status s) noexcept -> continues
	{
		return continues::up(std::move(resume), !s);
	}
};

template<typename T>
struct base_t
{
	behavior_t   x, y;
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
	auto next(Status s) noexcept -> continues
	{
		if (s == Failure)
		{
			return continues::up(std::move(this->resume), Success);
		}
		return continues::down(y, std::move(this->resume));
	}
};
struct equals_t : base_t<equals_t>
{
	Status first = Invalid;
	auto next(Status s) noexcept -> continues
	{
		first = s;
		return continues::down(
			y,
			continuation::mem_fn<&equals_t::last>(*this));
	}
	auto last(Status s) noexcept -> continues
	{
		return continues::up(
			std::move(resume),
			s == first ? Success : Failure);
	}
};
struct differs_t : base_t<differs_t>
{
	Status first = Invalid;
	auto next(Status s) noexcept -> continues
	{
		first = s;
		return continues::down(
			y,
			continuation::mem_fn<&differs_t::last>(*this));
	}
	auto last(Status s) noexcept -> continues
	{
		return continues::up(
			std::move(resume),
			s != first ? Success : Failure);
	}
};
}

auto negate(behavior_t&& x) -> behavior_t
{ return negate_t{ std::move(x) }; }

auto implies(behavior_t&& x, behavior_t&& y) -> behavior_t
{ return implies_t{ std::move(x), std::move(y) }; }

auto equals(behavior_t&& x, behavior_t&& y) -> behavior_t
{ return equals_t{ std::move(x), std::move(y) }; }

auto differs(behavior_t&& x, behavior_t&& y) -> behavior_t
{ return differs_t{ std::move(x), std::move(y) }; }

} // cbt
