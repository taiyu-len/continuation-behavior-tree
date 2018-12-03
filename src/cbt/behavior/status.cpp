#include "cbt/behavior/status.hpp"
#include <iostream>
namespace cbt
{
auto operator<<(std::ostream& os, Status s) -> std::ostream&
{
	switch (s)
	{
	case Invalid: return os << "Invalid";
	case Failure: return os << "Failure";
	case Success: return os << "Success";
	default     : return os << "Unknown";
	}
}

auto operator!(Status s) noexcept -> Status
{
	switch (s)
	{
	case Success: return Failure;
	case Failure: return Success;
	default     : return s;
	}
}
} // cbt
