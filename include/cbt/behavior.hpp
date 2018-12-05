#ifndef CBT_BEHAVIOR_HPP
#define CBT_BEHAVIOR_HPP
#include <memory>
#include "cbt/behavior/status.hpp"
#include "cbt/behavior/continuation.hpp"
#include "cbt/behavior/continues.hpp"

namespace cbt
{
/*
 * A behavior can be constructed with any type that satisfies one of
 * - invocable as Status()
 *   for behaviors that continues::up the tree (continuing with a status)
 * - invocable as void(continuation)
 *   for behaviors that continues::elsewhere, (continuing at a later point)
 * - invocable as continues(continuation)
 *   for behaviors that continues::down as well as doing the above
 *****************************************************************************/
class behavior_t
{
public:
	template<typename T>
	behavior_t(T&& x);

	behavior_t(behavior_t &&) noexcept = default;
	behavior_t(behavior_t const&) = delete;
	behavior_t& operator=(behavior_t&& x) noexcept = default;
	behavior_t& operator=(behavior_t const&) = delete;

	void run(continuation c) const noexcept;

	// get _data type from stored object.
	template<typename T>
	auto get() const -> T&;
private:
	friend continues;
	auto step(continuation c) const noexcept -> continues;

	struct concept_t;

	template<typename T>
	struct model_base;

	template<typename T, bool, bool, bool>
	struct model_t;
	std::unique_ptr<struct concept_t> _object;
};
} // cbt
#include "cbt/behavior_t.hpp"
#endif // CBT_BEHAVIOR_HPP
