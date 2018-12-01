#include "cbt/behavior.hpp"
#include <utility>
#include <doctest/doctest.h>
namespace cbt {
void behavior_t::operator()(continue_t c) const {
	_object->_continue = std::move(c);
	operator()();
}

void behavior_t::operator()() const {
	_object->start(_object->_continue);
}

TEST_CASE("behavior") {
	auto count = 0;
	auto result = Status::Invalid;
	auto cb = [&](Status s) { result = s; };
	SUBCASE("leaf model") {
		auto b = behavior_t([&]{ ++count; return Success; });
		REQUIRE(count == 0);
		REQUIRE(result == Invalid);
		b(cb);
		REQUIRE(result == Success);
		REQUIRE(count == 1);
	}
	SUBCASE("continuation model") {
		auto b = behavior_t([&](continue_t c){ ++count; c(Success); });
		REQUIRE(count == 0);
		REQUIRE(result == Invalid);
		b(cb);
		REQUIRE(result == Success);
		REQUIRE(count == 1);
	}
	SUBCASE("external continuation") {
		continue_t cc;
		auto b = behavior_t([&](continue_t c){ ++count; cc = c; });
		REQUIRE(count == 0);
		REQUIRE(result == Invalid);
		b(cb);
		REQUIRE(result == Invalid);
		REQUIRE(count == 1);
		SUBCASE("Success") {
			cc(Success);
			REQUIRE(count == 1);
			REQUIRE(result == Success);
		}
		SUBCASE("Failure") {
			cc(Failure);
			REQUIRE(count == 1);
			REQUIRE(result == Failure);
		}
	}
}
} // cbt
