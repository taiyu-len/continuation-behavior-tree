#include "cbt.hpp"
#include <doctest/doctest.h>

// empty function passed as final callback to break on in debugger.
struct MockBehavior : public Behavior
{
  std::function<Status()> func = []{ return Success; };
  MockBehavior() = default;
  MockBehavior(std::function<Status()> f):func(f) {}

  int _count = 0;
  void start() override
  {
    CBT_LOG();
    _status = func();
    resume();
  }
};

TEST_CASE("Behavior")
{
  SUBCASE("calls once") {
    auto count = 0;
    auto m = MockBehavior([&]{ count++; return Success; });

    REQUIRE(count == 0);
    m.run();
    REQUIRE(count == 1);
  }
  SUBCASE("pass status to final continuation") {
    auto s = Invalid;
    auto cb = [&](Status _s) { s = _s; };
    SUBCASE("Success") {
      auto m = MockBehavior([&]{ return Success; });
      REQUIRE(s == Invalid);
      m.run(cb);
      REQUIRE(s == Success);
    }
    SUBCASE("Failure") {
      auto m = MockBehavior([&]{ return Failure; });
      REQUIRE(s == Invalid);
      m.run(cb);
      REQUIRE(s == Failure);
    }
  }
}

struct MockBehaviorContinue : public Behavior
{
  MockBehaviorContinue(Continuation& c, int& i):_next{c}, _count{i} {}
  Continuation &_next;
  int &_count;

  void start() override
  {
    _count++;
    _next = [this](Status s) { _status = s; resume(); };
  }
};

TEST_CASE("Behavior with Continues")
{
  Behavior::Continuation next;
  int count = 0;
  auto r = Repeater(MockBehaviorContinue{next, count}, 5);
  SUBCASE("Manual continue till end") {
    r.run([&](Status){ next = nullptr; });
    REQUIRE(count == 1); next(Success);
    REQUIRE(count == 2); next(Success);
    REQUIRE(count == 3); next(Success);
    REQUIRE(count == 4); next(Success);
    REQUIRE(count == 5); next(Success);
    REQUIRE(next == nullptr);
  }
  SUBCASE("Fail early") {
    r.run([&](Status){ next = nullptr; });
    REQUIRE(next != nullptr);
    REQUIRE(count == 1); next(Success);
    REQUIRE(count == 2); next(Success);
    REQUIRE(count == 3); next(Failure);
    REQUIRE(next == nullptr);
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
    i.run(cb);
    REQUIRE(result == Failure);
  }
  SUBCASE("Double") {
    auto i = Inverter(Inverter(std::move(md)));
    REQUIRE(result == Invalid);
    i.run(cb);
    REQUIRE(result == Success);
  }
}

TEST_CASE("Decorator::Repeater")
{
  auto count = 0;
  auto result = Invalid;
  auto cb = [&](Status s) { result = s; };
  auto md = MockBehavior{[&]{ ++count; return Success; }};
  SUBCASE("Repeat 5 times") {
    auto r = Repeater(std::move(md), 5);
    CBT_LOG("==== start repeat 5 ====");
    r.run(cb);
    CBT_LOG("==== stop  repeat 5 ====");
    REQUIRE(count == 5);
    REQUIRE(result == Success);
  }
  SUBCASE("Nested Repeat 5*5 times") {
    auto r = Repeater(Repeater(std::move(md), 5), 5);
    CBT_LOG("==== start repeat 25 ====");
    r.run(cb);
    CBT_LOG("==== stop  repeat 25 ====");
    REQUIRE(count == 25);
    REQUIRE(result == Success);
  }
  SUBCASE("Fail at third iteration") {
    md = MockBehavior{[&]{ return ++count == 3 ? Failure : Success; }};
    auto r = Repeater(std::move(md), 5);
    r.run(cb);
    REQUIRE(count == 3);
    REQUIRE(result == Failure);
  }
  SUBCASE("Repeat 0 times") {
    auto r = Repeater(std::move(md), 0);
    r.run(cb);
    REQUIRE(count == 0);
    REQUIRE(result == Success);
  }
}

TEST_CASE("Composite::Sequence")
{
  int count[3] = {0, 0, 0};
  auto result = Invalid;
  auto cb = [&](Status s) { result = s; };
  SUBCASE("Succeed sequence 3 times") {
    auto bt = Sequence(
      MockBehavior{[&]{ ++count[0]; return Success; }},
      MockBehavior{[&]{ ++count[1]; return Success; }},
      MockBehavior{[&]{ ++count[2]; return Success; }}
    );
    bt.run(cb);
    REQUIRE(result == Success);
    REQUIRE(count[0] == 1);
    REQUIRE(count[1] == 1);
    REQUIRE(count[2] == 1);
  }
  SUBCASE("Fail in middle of sequence ") {
    auto bt = Sequence(
      MockBehavior{[&]{ ++count[0]; return Success; }},
      MockBehavior{[&]{ ++count[1]; return Failure; }},
      MockBehavior{[&]{ ++count[2]; return Success; }}
    );
    bt.run(cb);
    REQUIRE(result == Failure);
    REQUIRE(count[0] == 1);
    REQUIRE(count[1] == 1);
    REQUIRE(count[2] == 0);
  }
}
