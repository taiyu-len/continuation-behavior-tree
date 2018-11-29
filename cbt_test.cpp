#include "cbt.hpp"
#include <doctest/doctest.h>

// empty function passed as final callback to break on in debugger.
static void bp_func(Status) {};

struct MockBehavior : public Behavior
{
  Status _status = Success;
  int _count = 0;
  void run() override
  {
    ++_count;
    call_cc(_status);
  }
};

TEST_CASE("Behavior")
{
  MockBehavior m;
  SUBCASE("calls once") {
    REQUIRE(m._count == 0);
    m();
    REQUIRE(m._count == 1);
  }
  SUBCASE("continuation with status") {
    Status s = Invalid;
    auto cb = [&](Status _s) { s = _s; };
    SUBCASE("Success") {
      m(cb);
      REQUIRE(s == Success);
    }
    SUBCASE("Failure") {
      m._status = Failure;
      m(cb);
      REQUIRE(s == Failure);
    }
  }
}

struct MockDecorator : public Decorator
{
  MockDecorator(std::unique_ptr<Behavior> b, int& r, int& c)
  : Decorator(std::move(b))
  , _run_count(r)
  , _cb_count(c)
  {}

  int& _run_count;
  int& _cb_count;

  void run() override
  {
    _run_count++;
    child()([this](Status s) {
      _cb_count++;
      call_cc(s);
    });
  }
};

TEST_CASE("Decorator")
{
  int run_count = 0;
  int cb_count = 0;
  auto _b = std::make_unique<MockBehavior>();
  auto md = std::make_unique<MockDecorator>(std::move(_b), run_count, cb_count);
  SUBCASE("Single") {
    (*md)();
    REQUIRE(run_count == 1);
    REQUIRE(cb_count  == 1);
  }
  SUBCASE("Double") {
    auto md2 = MockDecorator(std::move(md), run_count, cb_count);
    md2();
    REQUIRE(run_count == 2);
    REQUIRE(cb_count  == 2);
  }
  SUBCASE("Double with continuation") {
    int i = 0;
    auto md2 = MockDecorator(std::move(md), run_count, cb_count);
    md2([&](Status){ i = 1; });
    REQUIRE(i == 1);
  }
}

TEST_CASE("Repeater")
{
  auto _b = std::make_unique<MockBehavior>();
  auto bp = _b.get();
  auto rp = std::make_unique<Repeator>(std::move(_b), 5);
  SUBCASE("Repeat 5 times") {
    (*rp)(bp_func);
    REQUIRE(bp->_count == 5);
  }
  SUBCASE("Nested Repeat 5*5 times") {
    auto rp2 = std::make_unique<Repeator>(std::move(rp), 5);
    (*rp2)(bp_func);
    REQUIRE(bp->_count == 25);
  }
  SUBCASE("Nested repeat Fail first") {
    bp->_status = Failure;
    auto rp2 = std::make_unique<Repeator>(std::move(rp), 5);
    (*rp2)();
    REQUIRE(bp->_count == 1);
  }

}

struct MockBehaviorContinue : public Behavior
{
  MockBehaviorContinue(std::function<void()>& n):next(n) {}
  std::function<void()> &next;
  int count = 0;

  void run() override
  {
    count++;
    next = [this]() { call_cc(Success); };
  }
};

TEST_CASE("Behavior with Continues")
{
  std::function<void()> next;
  auto _b = std::make_unique<MockBehaviorContinue>(next);
  auto bp = _b.get();
  auto rp = std::make_unique<Repeator>(std::move(_b), 5);
  SUBCASE("Repeater") {
    (*rp)([&](Status){ next = nullptr; });
    REQUIRE(bp->count == 1); next();
    REQUIRE(bp->count == 2); next();
    REQUIRE(bp->count == 3); next();
    REQUIRE(bp->count == 4); next();
    REQUIRE(bp->count == 5); next();
    REQUIRE(next == nullptr);
  }
}
