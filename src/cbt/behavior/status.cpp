#include "cbt/behavior/status.hpp"
#include <doctest/doctest.h>
#include <iostream>
namespace cbt
{
TEST_CASE("status")
{
	REQUIRE((!status::success) == status::failure);
	REQUIRE((!status::failure) == status::success);
	REQUIRE((!status::aborted) == status::aborted);
	REQUIRE((!status::unknown) == status::unknown);

	REQUIRE((status::success && status::success) == status::success);
	REQUIRE((status::success && status::failure) == status::failure);
	REQUIRE((status::success && status::unknown) == status::success);
	REQUIRE((status::success && status::aborted) == status::aborted);

	REQUIRE((status::failure && status::failure) == status::failure);
	REQUIRE((status::failure && status::unknown) == status::failure);
	REQUIRE((status::failure && status::aborted) == status::aborted);

	REQUIRE((status::unknown && status::unknown) == status::unknown);
	REQUIRE((status::unknown && status::aborted) == status::aborted);

	REQUIRE((status::aborted && status::aborted) == status::aborted);

	REQUIRE((status::success || status::success) == status::success);
	REQUIRE((status::success || status::failure) == status::success);
	REQUIRE((status::success || status::unknown) == status::success);
	REQUIRE((status::success || status::aborted) == status::aborted);

	REQUIRE((status::failure || status::failure) == status::failure);
	REQUIRE((status::failure || status::unknown) == status::failure);
	REQUIRE((status::failure || status::aborted) == status::aborted);

	REQUIRE((status::unknown || status::unknown) == status::unknown);
	REQUIRE((status::unknown || status::aborted) == status::aborted);

	REQUIRE((status::aborted || status::aborted) == status::aborted);
}

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

namespace
{
// handle the aborted, and unknown cases.
auto common_case(status& x, status y) noexcept -> bool
{
	if (x == status::aborted || y == status::aborted)
	{
		x = status::aborted;
		return true;
	}
	if (x == status::unknown)
	{
		x = y;
		return true;
	}
	return y == status::unknown;
}
} // namespace

auto operator&&(status x, status y) noexcept -> status
{
	if (common_case(x, y))
	{
		return x;
	}
	return (x != status::success) ? x : y;
}

auto operator||(status x, status y) noexcept -> status
{
	if (common_case(x, y))
	{
		return x;
	}
	return (x == status::success) ? x : y;
}
} // namespace cbt
