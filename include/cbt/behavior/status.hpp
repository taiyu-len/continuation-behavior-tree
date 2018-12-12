#ifndef CBT_BEHAVIOR_STATUS_HPP
#define CBT_BEHAVIOR_STATUS_HPP
/* Status codes are used to indicate the return state of a behavior node.
 *
 * status codes are modeled using 3-value weak logic.
 * with success being true, failure being false, abort being the third.
 * +---+---+
 * |   | ! |
 * | S | F |
 * | A | A |
 * | F | S |
 * +---+---+
 *
 * +---+---+---+---+   +---+---+---+---+
 * | & | S | A | F |   | | | S | A | F |
 * +---+---+---+---+   +---+---+---+---+
 * | S | S | A | F |   | S | S | A | S |
 * | A | A | A | A |   | A | A | A | A |
 * | F | F | A | F |   | F | S | A | F |
 * +---+---+---+---+   +---+---+---+---+
 *
 * status codes have an unknown initial state as well, but use of it outside of
 * comparisons is undefined
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

} // cbt
#endif // CBT_BEHAVIOR_STATUS_HPP
