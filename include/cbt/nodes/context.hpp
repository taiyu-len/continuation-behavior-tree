#ifndef CBT_NODES_CONTEXT_HPP
#define CBT_NODES_CONTEXT_HPP

namespace cbt
{
class behavior_t;

// constructs a behavior node storing an object of type T, and function
// of type Init that initialise T on start, and an out param for T*
// to return the address of the object to be used in other nodes.
//
// second version uses default construction for init.
template<typename T, typename F>
// requires std::is_invocable<F, T>
auto context(T*& out, F&&) -> behavior_t;

template<typename T>
auto context(T*& out) -> behavior_t;
} // cbt
#include "cbt/nodes/context_t.hpp"
#endif // CBT_NODES_CONTEXT_HPP
