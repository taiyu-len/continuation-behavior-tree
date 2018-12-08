#ifndef CBT_NODES_REPEAT_HPP
#define CBT_NODES_REPEAT_HPP
/* Behavior nodes to repeat until some condition and/or limit */
#include <cstddef>
namespace cbt
{
class behavior_t;

// Repeat until failure or limit
auto repeat_n(behavior_t&&, size_t limit) -> behavior_t;
auto repeat(behavior_t&&) -> behavior_t;

// Repeat until success or limit
auto until_n(behavior_t&&, size_t limit) -> behavior_t;
auto until(behavior_t&&) -> behavior_t;

// Repeat until limit, (TODO: stop on Abort Status)
auto for_n(behavior_t&&, size_t limit) -> behavior_t;
auto forever(behavior_t&&) -> behavior_t;

} // cbt
#endif // CBT_NODES_REPEAT_HPP
