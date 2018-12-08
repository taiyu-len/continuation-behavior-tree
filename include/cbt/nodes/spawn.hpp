#ifndef CBT_NODES_SPAWN_HPP
#define CBT_NODES_SPAWN_HPP
namespace cbt
{
class behavior_t;

void spawn(behavior_t&& x);

template<typename T>
void spawn(behavior_t&& x, T&& y);
} // cbt
#include "cbt/nodes/spawn_t.hpp"
#endif // CBT_NODES_SPAWN_HPP
