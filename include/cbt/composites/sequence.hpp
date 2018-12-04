#ifndef CBT_SEQUENCE_HPP
#define CBT_SEQUENCE_HPP
#include "cbt/behavior.hpp"
#include <vector>
namespace cbt
{
auto sequence(std::vector<behavior_t> &&) -> behavior_t;

template<typename ...T>
auto sequence(T&&... xs) -> behavior_t
{
	std::vector<behavior_t> v;
	v.reserve(sizeof...(T));
	int _[] = { (v.push_back(std::forward<T>(xs)), 0)... };
	(void)_;
	return sequence(std::move(v));
}
} // cbt
#endif // CBT_SEQUENCE_HPP
