#ifndef CBT_BEHAVIOR_CONTINUATION_HPP
#define CBT_BEHAVIOR_CONTINUATION_HPP
#include "util/unique_function.hpp"
#include "cbt/behavior/status.hpp"
namespace cbt
{
struct continues;
using  continuation_type = util::unique_function<continues(Status)>;

/* A linear function reference to a continuation_type function pointer.
 * must be called exactly once, or moved into another continuation.
 *
 * may be reconstructed multiple times from a single continuation_type
 */
struct continuation
{
	continuation() = default;
	~continuation() noexcept;
	continuation(continuation_type const&) noexcept;
	continuation(continuation&&) noexcept;
	auto operator=(continuation_type const&) noexcept -> continuation&;
	auto operator=(continuation&&) noexcept -> continuation&;

	// call operator for external continuation of user code.
	void operator()(Status) noexcept;

	explicit operator bool() const noexcept;
	friend bool operator==(std::nullptr_t, continuation const&) noexcept;
	friend bool operator==(continuation const&, std::nullptr_t) noexcept;
	friend bool operator==(continuation const&, continuation const&) noexcept;
	friend bool operator!=(std::nullptr_t, continuation const&) noexcept;
	friend bool operator!=(continuation const&, std::nullptr_t) noexcept;
	friend bool operator!=(continuation const&, continuation const&) noexcept;
	friend void swap(continuation&, continuation&) noexcept;

private:
	friend continues;
	auto step(Status) noexcept -> continues;

	continuation_type const* _ref = nullptr;
};

} // cbt
#endif // CBT_BEHAVIOR_CONTINUATION_HPP
