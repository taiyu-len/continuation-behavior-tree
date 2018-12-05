#ifndef CBT_SPAWN_HPP
#define CBT_SPAWN_HPP
namespace cbt
{
class behavior_t;

void spawn(behavior_t&& x);

template<typename T>
void spawn(behavior_t&& x, T&& y);
} // cbt
#include "cbt/spawn_t.hpp"
#endif // CBT_SPAWN_HPP
