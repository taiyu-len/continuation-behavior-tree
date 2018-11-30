#ifndef CBT_HPP
#define CBT_HPP
// Continuation Behavior Trees.
//
// A WIP implementation of Behavior tree using continuations.
// Designed primaraly to be used with asio and support async callers and such.
//
//
//
//


#include <vector>
#include <memory>
#include <functional>
#include <doctest/doctest.h>


static inline intptr_t stack_pointer()
{
  char p;
  intptr_t ip = (intptr_t)&p;
  return ip;
}
#define CBT_LOG(FMT, ...) \
  printf("%60s: %lX %-20s " FMT "\n", __PRETTY_FUNCTION__, stack_pointer(), __func__, ##__VA_ARGS__);


// The status of a function
// TODO replace with std::error_code or something
enum Status
{
  Invalid = -1,
  Failure = +0,
  Success = +1
};

const char* to_string(Status s);
static inline doctest::String toString(Status s) { return to_string(s); }

class Behavior
{
public:
  using Continuation = std::function<void(Status)>;

  // Run with existing final continuation, until the current continuation has
  // been passed somewhere else. like an event queue
  void run();

  // Set final continuation, then call run()
  template<typename T>
  void run(T&& x)
  {
    _continue = std::forward<T>(x);
    return run();
  }

  virtual ~Behavior() = default;
protected:
  /* start() first initializes the state of the behavior,
   * it then calls start of the next child nodes, with a continuation for the
   * current node.
   * or if it is a leaf node, it may pass of the continuation somewhere else,
   * such as an event queue, or calls the continuation directly.
   *
   * preferably, the continuation should not be called directly, due to
   * lack of tail-call optimization. which leads to a very big stack.
   */

  virtual
  void start() = 0;

  // Resumes the passed in continuation.
  void resume();

  // The continuation set on operator()(Continuation)
  // defaults to a no-op
  Continuation _continue = [](Status){};

  // The default status that _continue is called with.
  Status _status = Status::Invalid;
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
  Root(std::shared_ptr<Behavior> child): _child(std::move(child)) {}

protected:
  std::shared_ptr<Behavior> _child;
  void start() override;
};

class Decorator : public Behavior
{
protected:
  std::unique_ptr<Behavior> _child;

public:
  Decorator(std::unique_ptr<Behavior> child): _child(std::move(child)) {}
};

class InvertDecorator : public Decorator
{
public:
  using Decorator::Decorator;

protected:
  void start() override;
};

template<typename T>
InvertDecorator Inverter(T x) { return {std::make_unique<T>(std::move(x))}; }

class RepeatDecorator : public Decorator
{
public:
  RepeatDecorator(std::unique_ptr<Behavior> b, size_t limit)
  : Decorator(std::move(b))
  , _limit(limit)
  , _count(0)
  {}

protected:
  size_t _limit, _count;

  void start() override;
};

template<typename T>
RepeatDecorator Repeater(T x, size_t limit)
{
  return {std::make_unique<T>(std::move(x)), limit};
}

class Composite : public Behavior
{
public:
  using Children = std::vector<std::unique_ptr<Behavior>>;
  Composite(Children c): _children(std::move(c)) {}
  auto at(int i) -> Behavior&;

protected:
  Children _children;

};

class SequenceUntil : public Composite
{
public:
  SequenceUntil(Children cs, Status x)
  : Composite(std::move(cs))
  , _exit_on(x)
  {};

  void start() override;
protected:
  void next();
  Status _exit_on;
  size_t _index;
};

template<typename... T>
SequenceUntil Sequence(T&&... xs) {
  // Hack to make it work.
  auto c = Composite::Children{};
  c.reserve(sizeof...(T));
  int _[] = { (c.push_back(std::make_unique<T>(xs)), 0)... };
  return { std::move(c), Failure };
}

#endif // CBT_HPP
