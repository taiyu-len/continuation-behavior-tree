#include "cbt.hpp"
#include <doctest/doctest.h>

// empty function passed as final callback to break on in debugger.
static void bp_func(Status) {};

struct MockBehavior : public Behavior
{
  std::function<Status()> func = []{ return Success; };
  MockBehavior() = default;
  MockBehavior(std::function<Status()> f):func(f) {}

  Status _status = Success;
  int _count = 0;
  void run() override
  {
    call_cc(func());
  }
};

TEST_CASE("Behavior")
{
  SUBCASE("calls once") {
    auto count = 0;
    auto m = MockBehavior([&]{ count++; return Success; });

    REQUIRE(count == 0);
    m();
    REQUIRE(count == 1);
  }
  SUBCASE("pass status to final continuation") {
    auto s = Invalid;
    auto cb = [&](Status _s) { s = _s; };
    SUBCASE("Success") {
      auto m = MockBehavior([&]{ return Success; });
      REQUIRE(s == Invalid);
      m(cb);
      REQUIRE(s == Success);
    }
    SUBCASE("Failure") {
      auto m = MockBehavior([&]{ return Failure; });
      REQUIRE(s == Invalid);
      m(cb);
      REQUIRE(s == Failure);
    }
  }
}

TEST_CASE("Decorator::Inverter")
{
  auto result = Invalid;
  auto cb = [&](Status s) { result = s; };
  auto md = MockBehavior{};
  SUBCASE("Single") {
    auto i = Inverter(std::move(md));
    REQUIRE(result == Invalid);
    i(cb);
    REQUIRE(result == Failure);
  }
  SUBCASE("Double") {
    auto i = Inverter(Inverter(std::move(md)));
    REQUIRE(result == Invalid);
    i(cb);
    REQUIRE(result == Success);
  }
}

TEST_CASE("Repeater")
{
  auto count = 0;
  auto result = Invalid;
  auto cb = [&](Status s) { result = s; };
  auto md = MockBehavior{[&]{ ++count; return Success; }};
  SUBCASE("Repeat 5 times") {
    auto r = Repeater(std::move(md), 5);
    r(cb);
    REQUIRE(count == 5);
    REQUIRE(result == Success);
  }
  SUBCASE("Nested Repeat 5*5 times") {
    auto r = Repeater(Repeater(std::move(md), 5), 5);
    r(cb);
    REQUIRE(count == 25);
    REQUIRE(result == Success);
  }
  SUBCASE("Fail at third iteration") {
    md = MockBehavior{[&]{ return ++count == 3 ? Failure : Success; }};
    auto r = Repeater(std::move(md), 5);
    r(cb);
    REQUIRE(count == 3);
    REQUIRE(result == Failure);
  }
}

struct MockBehaviorContinue : public Behavior
{
  MockBehaviorContinue(Continuation& c, int& i):_next{c}, _count{i} {}
  Continuation &_next;
  int &_count;

  void run() override
  {
    _count++;
    _next = [this](Status s) { call_cc(s); };
  }
};

TEST_CASE("Behavior with Continues")
{
  Behavior::Continuation next;
  int count = 0;
  auto r = Repeater(MockBehaviorContinue{next, count}, 5);
  SUBCASE("Manual continue till end") {
    r([&](Status){ next = nullptr; });
    REQUIRE(count == 1); next(Success);
    REQUIRE(count == 2); next(Success);
    REQUIRE(count == 3); next(Success);
    REQUIRE(count == 4); next(Success);
    REQUIRE(count == 5); next(Success);
    REQUIRE(next == nullptr);
  }
  SUBCASE("Fail early") {
    r([&](Status){ next = nullptr; });
    REQUIRE(next != nullptr);
    REQUIRE(count == 1); next(Success);
    REQUIRE(count == 2); next(Success);
    REQUIRE(count == 3); next(Failure);
    REQUIRE(next == nullptr);
  }
}
