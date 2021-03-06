#ifndef CBT_NODES_REPEAT_HPP
#define CBT_NODES_REPEAT_HPP
/* Behavior nodes to repeat until some condition and/or limit */
#include <cstddef>
namespace cbt
{
struct behavior;

// Repeat until failure or limit
auto repeat_n(behavior&&, size_t limit) -> behavior;
auto repeat(behavior&&) -> behavior;

// Repeat until success or limit
auto until_n(behavior&&, size_t limit) -> behavior;
auto until(behavior&&) -> behavior;

// Repeat until limit
auto for_n(behavior&&, size_t limit) -> behavior;
auto forever(behavior&&) -> behavior;

} // namespace cbt
#endif // CBT_NODES_REPEAT_HPP
