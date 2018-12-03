#ifndef CBT_BEHAVIOR_CONTINUATION_HPP
#define CBT_BEHAVIOR_CONTINUATION_HPP
#include "cbt/unique_function.hpp"
#include "cbt/behavior/status.hpp"
namespace cbt
{
struct continues;
using  continuation_type = unique_function<continues(Status)>;

/* A linear function reference to a continuation_type function pointer.
 * must be called exactly once, or moved into another continuation.
 *
 * may be reconstructed multiple times from a single continuation_type
 */
struct continuation
{
	continuation() = default;
	continuation(continuation_type const& x) noexcept;
	continuation(continuation&& x) noexcept;
	auto operator=(continuation_type const& x) noexcept -> continuation&;
	auto operator=(continuation&& x) noexcept -> continuation&;

	// call operator for external continuation of user code.
	void operator()(Status);

	operator bool() const noexcept;
	friend bool operator==(std::nullptr_t, continuation const&) noexcept;
	friend bool operator==(continuation const&, std::nullptr_t) noexcept;
	friend bool operator==(continuation const&, continuation const&) noexcept;
	friend bool operator!=(std::nullptr_t, continuation const&) noexcept;
	friend bool operator!=(continuation const&, std::nullptr_t) noexcept;
	friend bool operator!=(continuation const&, continuation const&) noexcept;
	friend void swap(continuation&, continuation&) noexcept;

private:
	friend continues;
	auto step(Status) -> continues;

	continuation_type const* _ref = nullptr;
};

} // cbt
#endif // CBT_BEHAVIOR_CONTINUATION_HPP
