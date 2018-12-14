#ifndef CBT_NODES_SELECT_HPP
#define CBT_NODES_SELECT_HPP
/* Models OR over status codes.
 * returning on first success, or first abort.
 */
#include "cbt/behavior.hpp"
#include <array>
namespace cbt
{
auto select_impl(behavior *, std::uint8_t) -> behavior;

template<typename ...T>
auto select(T&&... xs) -> behavior
{
	static_assert(sizeof...(T) < 256, "Too many nodes in sequence");
	std::array<behavior, sizeof...(T)> a = { std::forward<T>(xs)... };
	return select_impl(a.data(), static_cast<std::uint8_t>(a.size()));
}
} // namespace cbt
#endif // CBT_NODES_SELECT_HPP
