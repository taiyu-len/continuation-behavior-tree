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
  using Continuation = std::function<void(Status)>;

  // Run with existing continuation
  void operator()()
  {
    run();
  }

  // Set continuation and then run
  void operator()(Continuation c)
  {
    _continue = std::move(c);
    run();
  }

  virtual ~Behavior() = default;
protected:
  // Run is responsible for Initializing the state of the behavior,
  // and calling the set continuation at some point later.
  // either directly (leaf node), or through a continuation passed to the child
  // (decorator, composite)
  virtual void run() = 0;

  // Calls the continuation that has been set.
  void call_cc(Status s)
  {
    _continue(s);
  }

  // The continuation set on operator()(Continuation)
  // defaults to a no-op
  Continuation _continue = [](Status){};
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

struct Root : public Behavior
{
  Root(std::shared_ptr<Behavior> child): _child(child) {}

protected:
  std::shared_ptr<Behavior> _child;
  void run() override
  {
    (*_child)([self=std::move(*this)] (Status s) mutable {
      self.call_cc(s);
    });
  }
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

class Inverter : public Decorator
{
public:
  using Decorator::Decorator;
protected:
  void run() override
  {
    child()([this] (Status s) {
      switch (s) {
      case Success: return call_cc(Failure);
      case Failure: return call_cc(Success);
      default: return call_cc(s);
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

  void run() override
  {
    _count = 0;
    child()([this](Status s) {
      if (++_count == _limit || s == Failure) call_cc(s);
      else child()();
    });
  }
};

#endif // CBT_HPP
