#include "cbt.hpp"
#include <doctest/doctest.h>
struct MockBehavior : public Behavior
{
  Status _status = Success;
  int _count = 0;
  void run(Continuation c) override
  {
    ++_count;
    c(_status);
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

  void run(Continuation c) override
  {
    _run_count++;
    child()(
      [this, c=std::move(c)](Status s) {
        _cb_count++;
        c(s);
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
    (*rp)();
    REQUIRE(bp->_count == 5);
  }
  SUBCASE("Nested Repeat 5*5 times") {
    auto rp2 = std::make_unique<Repeator>(std::move(rp), 5);
    (*rp2)();
    REQUIRE(bp->_count == 25);
  }
  SUBCASE("Nested repeat Fail first") {
    bp->_status = Failure;
    auto rp2 = std::make_unique<Repeator>(std::move(rp), 5);
    (*rp2)();
    REQUIRE(bp->_count == 1);
  }

}

