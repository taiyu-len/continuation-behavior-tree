#ifndef CBT_NODES_CONTEXT_HPP
#define CBT_NODES_CONTEXT_HPP
/* behavior node used to store and initialize state.
 *
 * return a behavoir node storing an object of type T, with a function that
 * initializes it when the node is called, and a pointer the internal object.
 * [tree, pointer]
 */
#include <utility>

namespace cbt
{
class behavior;
template<typename T>
using context_result = std::pair<behavior, T*>;

template<typename T, typename F>
auto context(F&&) -> context_result<T>;

template<typename T>
auto context() -> context_result<T>;
} // cbt
#include "cbt/nodes/context_t.hpp"
#endif // CBT_NODES_CONTEXT_HPP
