#ifndef CBT_COMPOSITES_HPP
#define CBT_COMPOSITES_HPP
#include "cbt/behavior.hpp"
#include <vector>
namespace cbt
{
behavior_t sequence(std::vector<behavior_t>);

template<typename ...T>
behavior_t sequence(T&&... xs)
{
	std::vector<behavior_t> v;
	v.reserve(sizeof...(T));
	int _[] = { (v.push_back(std::move(xs)), 0)... };
	(void)_;
	return sequence(std::move(v));
}

} // cbt
#endif // CBT_COMPOSITES_HPP
