#ifndef CBT_STATUS_HPP
#define CBT_STATUS_HPP
#include <iosfwd>
namespace cbt
{
enum Status
{
	Invalid = -1,
	Failure = +0,
	Success = +1
};

auto operator<<(std::ostream& os, Status s) -> std::ostream&;
auto operator!(Status) noexcept -> Status;

} // cbt
#endif // CBT_STATUS_HPP
