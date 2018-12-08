#ifndef CBT_NODES_BOOLEAN_HPP
#define CBT_NODES_BOOLEAN_HPP
/* Behavior nodes modeling boolean operations */
namespace cbt
{
class behavior_t;
auto negate(behavior_t&&) -> behavior_t;
auto implies(behavior_t&&, behavior_t&&) -> behavior_t;
auto equals(behavior_t&&, behavior_t&&) -> behavior_t;
auto differs(behavior_t&&, behavior_t&&) -> behavior_t;
} // cbt
#endif // CBT_NODES_BOOLEAN_HPP
