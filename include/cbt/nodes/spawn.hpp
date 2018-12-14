#ifndef CBT_NODES_SPAWN_HPP
#define CBT_NODES_SPAWN_HPP
namespace cbt
{
struct behavior;

void spawn(behavior&& x);

template<typename T>
void spawn(behavior&& x, T&& y);
} // namespace cbt
#include "cbt/nodes/spawn_t.hpp"
#endif // CBT_NODES_SPAWN_HPP
