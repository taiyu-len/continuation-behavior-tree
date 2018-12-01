#ifndef CBT_DECORATORS_HPP
#define CBT_DECORATORS_HPP
#include "cbt/behavior.hpp"
namespace cbt
{
behavior_t inverter(behavior_t x);
behavior_t repeater(behavior_t x, size_t limit);
} // cbt
#endif // CBT_DECORATORS_HPP
