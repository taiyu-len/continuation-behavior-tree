#ifndef CBT_NODES_SPAWN_HPP
#define CBT_NODES_SPAWN_HPP
namespace cbt
{
class behavior;

void spawn(behavior&& x);

template<typename T>
void spawn(behavior&& x, T&& y);
} // cbt
#include "cbt/nodes/spawn_t.hpp"
#endif // CBT_NODES_SPAWN_HPP
