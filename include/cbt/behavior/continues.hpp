#ifndef CBT_BEHAVIOR_RETURN_HPP
#define CBT_BEHAVIOR_RETURN_HPP
#include <cbt/behavior/continuation.hpp>
#include <cbt/behavior/status.hpp>

namespace cbt
{
struct behavior_t;

// the return value of the functions that are used to go up, and down the
// behavior tree.
// used by behavior_t::run to simulate tail-call recursion due to its
// unreliability in non-optimized code
//
// has 3 states.
// - continues::down
//   represents the state where we are going down the tree. that is, passing
//   continuations to that behavior
//   { behavior_t&, continuation }  =>  b.run(c)
//
// - continues::up
//   represents the state where we are going up the tree. that is, calling
//   continuations with a status
//   { continuation, Status }  =>  c(status)
//
// - continues::elsewhere
//   we passed a continuation off to some other location. so we can stop running
//
struct continues
{
	static auto up(continuation&&, Status s) -> continues;
	static auto down(behavior_t const&, continuation&&) -> continues;
	continues() = default;

	// run the continues until we reach an elsewhere state
	void run();

private:
	behavior_t const* _behavior = nullptr;
	continuation      _continue = {};
	Status            _status   = Invalid;

	enum class state_e { down, up, elsewhere };
	auto state()   -> state_e;
	auto go_up()   -> continues;
	auto go_down() -> continues;
};

} // cbt


#endif // CBT_BEHAVIOR_RETURN_HPP
