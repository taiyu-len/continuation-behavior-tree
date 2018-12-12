#include "cbt/behavior/status.hpp"
#include <iostream>
namespace cbt
{
auto operator<<(std::ostream& os, status s) -> std::ostream&
{
	switch (s) {
	case status::success: return os << "success";
	case status::aborted: return os << "aborted";
	case status::failure: return os << "failure";
	default: return os << "unknown";
	}
}

auto operator!(status s) noexcept -> status
{
	switch (s) {
	case status::success: return status::failure;
	case status::failure: return status::success;
	default: return s;
	}
}

auto operator&&(status x, status y) noexcept -> status
{
	if (x == status::aborted || y == status::aborted)
	{
		return status::aborted;
	}
	return (x != status::success) ? x : y;
}

auto operator||(status x, status y) noexcept -> status
{
	if (x == status::aborted || y == status::aborted)
	{
		return status::aborted;
	}
	return (x == status::success) ? x : y;
}
} // cbt
