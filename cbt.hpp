#ifndef CBT_HPP
#define CBT_HPP
// Continuation Behavior Trees.
//
// A WIP implementation of Behavior tree using continuations.
// Designed primaraly to be used with asio and support async callers and such.
//


#include <vector>
#include <memory>
#include <functional>

// The status of a function
// TODO replace with std::error_code or something
enum Status
{
  Invalid = -1,
  Failure = +0,
  Success = +1
};

class Behavior
{
public:
  // The Continuation is called with a given status
  using Continuation = std::function<void(Status)>;
  void operator()() { (*this)([](Status){});  }
  void operator()(Continuation c)
  {
    run(std::move(c));
  }

  virtual ~Behavior() = default;
protected:
  virtual
  void run(Continuation c) = 0;
};

class Decorator : public Behavior
{
protected:
  std::unique_ptr<Behavior> _child;

public:
  Decorator(std::unique_ptr<Behavior> child): _child(std::move(child)) {}
  Decorator(Decorator &&) = default;
  Decorator& operator=(Decorator &&) = default;

  Behavior& child() { return *_child; }
};

/*
 * The root decorator is designed to be the base of a behavior tree,
 * It moves itself into the continuation that the child will then call.
 * This allows the contiuation stack to be self contained, keeping the nodes
 * alive until the root continuation finishes.
 *
 * [Or]-o[O1]-o[O2]-o[O3] Behavior "tree"
 *     /  |     |     |
 * [-Or-|-r1--|-r2--|-r3-]   Continuation stack
 *
 * Or owns O1, O1 owns O2, etc. via smart pointers.
 *
 * The calling order is
 * Or, O1, O2, O3, r3, r2, r1, Or.
 *
 * rn keeps a pointer to On
 *
 * by storing Or into the continuation stack, we keep all On alive, so that rn
 * can use them safely until the continuation stack finishes.
 */

/* TODO: use alternative to std::function that allows move only types
struct Root : public Decorator
{
  using Decorator::Decorator;
  Root(Root&&) = default;
  Root& operator=(Root&&) = default;

protected:
  void run(Continuation c) override
  {
    child()(Continuation([self=std::move(*this), c=std::move(c)](Status s) {
      c(s);
    }));
  }
};
*/

class Inverter : public Decorator
{
public:
  using Decorator::Decorator;
protected:
  void run(Continuation c) override
  {
    child()(
      [c=std::move(c)] (Status s) {
        switch (s) {
        case Success: return c(Failure);
        case Failure: return c(Success);
        default: return c(s);
        }
      });
  }
};

class Repeator : public Decorator
{
public:
  Repeator(std::unique_ptr<Behavior> b, size_t limit)
  : Decorator(std::move(b))
  , _limit(limit)
  , _count(0)
  {}

protected:
  size_t _limit, _count;

  void run(Continuation c) override
  {
    _count = 0;
    step(std::move(c));
  }
  void step(Continuation c)
  {
    child()(
      [this, c=std::move(c)](Status s) {
        if (++_count == _limit || s == Failure) c(s);
        else step(std::move(c));
      });
  }
};

#endif // CBT_HPP
