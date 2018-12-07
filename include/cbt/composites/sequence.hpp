#ifndef CBT_SEQUENCE_HPP
#define CBT_SEQUENCE_HPP
#include "cbt/behavior.hpp"
#include <array>
namespace cbt
{
auto sequence_impl(behavior_t *, std::uint8_t) -> behavior_t;

template<typename ...T>
auto sequence(T&&... xs) -> behavior_t
{
	static_assert(sizeof...(T) < 256, "Too many nodes in sequence");
	std::array<behavior_t, sizeof...(T)> a = { std::forward<T>(xs)... };
	return sequence_impl(a.data(), static_cast<std::uint8_t>(a.size()));
}
} // cbt
#endif // CBT_SEQUENCE_HPP
