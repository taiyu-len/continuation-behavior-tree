#ifndef CBT_REPEATER_HPP
#define CBT_REPEATER_HPP
#include <cstddef>
namespace cbt
{
class behavior_t;
auto repeater(behavior_t&&, size_t limits) -> behavior_t;
} // cbt
#endif // CBT_REPEATER_HPP

