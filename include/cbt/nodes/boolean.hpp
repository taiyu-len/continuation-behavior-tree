#ifndef CBT_NODES_BOOLEAN_HPP
#define CBT_NODES_BOOLEAN_HPP
/* Behavior nodes modeling boolean operations */
namespace cbt
{
class behavior;
auto negate(behavior&&) -> behavior;
auto implies(behavior&&, behavior&&) -> behavior;
auto equals(behavior&&, behavior&&) -> behavior;
auto differs(behavior&&, behavior&&) -> behavior;
} // cbt
#endif // CBT_NODES_BOOLEAN_HPP
