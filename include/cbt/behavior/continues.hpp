#ifndef CBT_BEHAVIOR_RETURN_HPP
#define CBT_BEHAVIOR_RETURN_HPP
#include <cbt/behavior/continuation.hpp>
#include <cbt/behavior/status.hpp>

namespace cbt
{
class behavior;

/*
 * the return type of functions used to walk up and down the behavior tree.
 * used to avoid relying on unreliable tail-call optimizations.
 *
 * continues::down is used to go down the behavior tree, that is passing a
 * continuation into the next behavior.
 *
 * continues::up is used to go up the behavior tree, that is caling a
 * continuation with a given status.
 *
 * continues::elsewhere is used to tell the run() function that it has passed a
 * continuation to be called somewhere else.
 */
struct continues
{
	static auto up(continuation&&, status s) noexcept -> continues;
	static auto down(behavior const&, continuation&&) noexcept -> continues;
	static auto elsewhere() noexcept -> continues;
	static auto finished() noexcept -> continues;

	// run the continues until we reach an elsewhere state
	void run() noexcept;
private:
	continues() = default;
	behavior const* _behavior = nullptr;
	continuation    _continue = {};
	status          _status   = status::unknown;

	enum class state_e { down, up, elsewhere };
	auto state() const noexcept  -> state_e;
	auto go_up() noexcept  -> continues;
	auto go_down() noexcept -> continues;
};

} // cbt


#endif // CBT_BEHAVIOR_RETURN_HPP
