#ifndef CBT_SELECT_HPP
#define CBT_SELECT_HPP
#include "cbt/behavior.hpp"
#include <array>
namespace cbt
{
auto select_impl(behavior_t *, std::uint8_t) -> behavior_t;

template<typename ...T>
auto select(T&&... xs) -> behavior_t
{
	static_assert(sizeof...(T) < 256, "Too many nodes in sequence");
	std::array<behavior_t, sizeof...(T)> a = { std::forward<T>(xs)... };
	return select_impl(a.data(), static_cast<std::uint8_t>(a.size()));
}
} // cbt
#endif // CBT_SELECT_HPP
