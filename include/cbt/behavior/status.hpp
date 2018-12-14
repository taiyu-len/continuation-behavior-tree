#ifndef CBT_BEHAVIOR_STATUS_HPP
#define CBT_BEHAVIOR_STATUS_HPP
/* Status codes are used to indicate the return state of a behavior node.
 *
 * status codes are modeled using 3-value weak logic, with an extra null state.
 *
 * Success, and Failure act as true and false, respectively.
 * Aborted is contagious, and propogates across boolean expression
 * Unknown is simply ignored in expressions.
 *
 * +---+---+
 * |   | ! |
 * | S | F |
 * | F | S |
 * | U | U |
 * | A | A |
 * +---+---+
 *
 * +---+---------+   +---+---------+
 * | & | S F U A |   | | | S F U A |
 * +---+---------+   +---+---------+
 * | S | S F S A |   | S | S S S A |
 * | F | F F F A |   | F | S F F A |
 * | U | S F U A |   | U | S F U A |
 * | A | A A A A |   | A | A A A A |
 * +---+---------+   +---+---------+
 *
 */
#include <iosfwd>

namespace cbt
{
enum class status
{
	failure = -1,
	aborted = +0,
	success = +1,
	unknown = +2,
};

auto operator<<(std::ostream& os, status s) -> std::ostream&;

auto operator!(status) noexcept -> status;
auto operator&&(status, status) noexcept -> status;
auto operator||(status, status) noexcept -> status;

} // namespace cbt
#endif // CBT_BEHAVIOR_STATUS_HPP
