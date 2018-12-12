#ifndef CBT_NODES_SEQUENCE_HPP
#define CBT_NODES_SEQUENCE_HPP
/* Models AND over return status,
 * returns first failure, or abort, otherwise success.
 *
 * however, abort is not contagious in this case due to short circuiting.
 * that is, we dont continue processing just to check if any abort.
 */
#include "cbt/behavior.hpp"
#include <array>
namespace cbt
{
auto sequence_impl(behavior *, std::uint8_t) -> behavior;

template<typename ...T>
auto sequence(T&&... xs) -> behavior
{
	static_assert(sizeof...(T) < 256, "Too many nodes in sequence");
	std::array<behavior, sizeof...(T)> a = { std::forward<T>(xs)... };
	return sequence_impl(a.data(), static_cast<std::uint8_t>(a.size()));
}
} // cbt
#endif // CBT_NODES_SEQUENCE_HPP
