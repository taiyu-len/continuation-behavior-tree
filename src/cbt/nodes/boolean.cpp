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
	auto test = [&](behavior &&b, status s)
	{
		spawn(std::move(b), cb);
		return result == s;
	};
	int count[] = {0, 0, 0};
	auto callcount = [&](int i, int j, int k)
	{
		return count[0] == i && count[1] == j && count[2] == k;
	};
	auto True  = [&]{ count[0]++; return status::success; };
	auto False = [&]{ count[1]++; return status::failure; };
	auto Abort = [&]{ count[2]++; return status::aborted; };
	SUBCASE("negate")
	{
		test(negate(True),  status::failure);
		test(negate(False), status::success);
		test(negate(Abort), status::aborted);
	};
	SUBCASE("double negate")
	{
		test(negate(negate(True)), status::success);
		test(negate(negate(False)), status::failure);
		test(negate(negate(Abort)), status::aborted);
	};
	SUBCASE("implies")
	{
		REQUIRE(test(implies(True, True), status::success));
		REQUIRE(callcount(2, 0, 0));
		REQUIRE(test(implies(True, False), status::failure));
		REQUIRE(callcount(3, 1, 0));
		REQUIRE(test(implies(True, Abort), status::aborted));
		REQUIRE(callcount(4, 1, 1));
		REQUIRE(test(implies(False, True), status::success));
		REQUIRE(callcount(4, 2, 1));
		REQUIRE(test(implies(False, False), status::success));
		REQUIRE(callcount(4, 3, 1));
		REQUIRE(test(implies(False, Abort), status::success));
		REQUIRE(callcount(4, 4, 1));
		REQUIRE(test(implies(Abort, True), status::aborted));
		REQUIRE(callcount(4, 4, 2));
		REQUIRE(test(implies(Abort, False), status::aborted));
		REQUIRE(callcount(4, 4, 3));
		REQUIRE(test(implies(Abort, Abort), status::aborted));
		REQUIRE(callcount(4, 4, 4));
	};
	SUBCASE("equals")
	{
		REQUIRE(test(equals(True, True), status::success));
		REQUIRE(callcount(2, 0, 0));
		REQUIRE(test(equals(True, False), status::failure));
		REQUIRE(callcount(3, 1, 0));
		REQUIRE(test(equals(True, Abort), status::aborted));
		REQUIRE(callcount(4, 1, 1));
		REQUIRE(test(equals(False, True), status::failure));
		REQUIRE(callcount(5, 2, 1));
		REQUIRE(test(equals(False, False), status::success));
		REQUIRE(callcount(5, 4, 1));
		REQUIRE(test(equals(False, Abort), status::aborted));
		REQUIRE(callcount(5, 5, 2));
		REQUIRE(test(equals(Abort, True), status::aborted));
		REQUIRE(callcount(5, 5, 3));
		REQUIRE(test(equals(Abort, False), status::aborted));
		REQUIRE(callcount(5, 5, 4));
		REQUIRE(test(equals(Abort, Abort), status::aborted));
		REQUIRE(callcount(5, 5, 5));
	};
	SUBCASE("differs")
	{
		REQUIRE(test(differs(True, True), status::failure));
		REQUIRE(callcount(2, 0, 0));
		REQUIRE(test(differs(True, False), status::success));
		REQUIRE(callcount(3, 1, 0));
		REQUIRE(test(differs(True, Abort), status::aborted));
		REQUIRE(callcount(4, 1, 1));
		REQUIRE(test(differs(False, True), status::success));
		REQUIRE(callcount(5, 2, 1));
		REQUIRE(test(differs(False, False), status::failure));
		REQUIRE(callcount(5, 4, 1));
		REQUIRE(test(differs(False, Abort), status::aborted));
		REQUIRE(callcount(5, 5, 2));
		REQUIRE(test(differs(Abort, True), status::aborted));
		REQUIRE(callcount(5, 5, 3));
		REQUIRE(test(differs(Abort, False), status::aborted));
		REQUIRE(callcount(5, 5, 4));
		REQUIRE(test(differs(Abort, Abort), status::aborted));
		REQUIRE(callcount(5, 5, 5));
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
	behavior x, y;
	continuation resume{};
	auto operator()(continuation _resume) noexcept -> continues
	{
		resume = std::move(_resume);
		auto self = static_cast<T*>(this);
		auto c = continuation::mem_fn<&T::next>(*self);
		return continues::down(x, std::move(c));
	}
};

struct implies_t : base_t<implies_t>
{
	auto next(status s) noexcept -> continues
	{
		if (s == status::success)
		{
			return continues::down(y, std::move(this->resume));
		}
		return continues::up(std::move(this->resume), !s);
	}
};

struct equals_t : base_t<equals_t>
{
	status first = status::unknown;
	auto next(status s) noexcept -> continues
	{
		if (s == status::aborted)
		{
			return last(s);
		}
		first = s;
		return CBT_DOWN(y, last);
	}
	auto last(status s) noexcept -> continues
	{
		if (s != status::aborted)
		{
			s = (s == first) ? status::success : status::failure;
		}
		return continues::up(std::move(resume), s);
	}
};

struct differs_t : base_t<differs_t>
{
	status first = status::unknown;
	auto next(status s) noexcept -> continues
	{
		if (s == status::aborted)
		{
			return last(s);
		}
		first = s;
		return CBT_DOWN(y, last);
	}
	auto last(status s) noexcept -> continues
	{
		if (s != status::aborted)
		{
			s = (s != first) ? status::success : status::failure;
		}
		return continues::up(std::move(resume), s);
	}
};
} // namespace

auto negate(behavior&& x) -> behavior
{
	return negate_t{ std::move(x) };
}

auto implies(behavior&& x, behavior&& y) -> behavior
{
	return implies_t{{ std::move(x), std::move(y) }};
}

auto equals(behavior&& x, behavior&& y) -> behavior
{
	return equals_t{{ std::move(x), std::move(y) }};
}

auto differs(behavior&& x, behavior&& y) -> behavior
{
	return differs_t{{ std::move(x), std::move(y) }};
}

} // namespace cbt
