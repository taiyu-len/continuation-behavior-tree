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
 * - invocable as continues(continuation)
 *   for behaviors that continues::down as well as doing the above
 * - invocable as void(continuation)
 *   for behaviors that continues::elsewhere, (continuing at a later point)
 * - invocable as status()
 *   for behaviors that continues::up the tree (continuing with a status)
 * - invocable as bool()
 *   for function that simply true (success) or false (failure)
 *****************************************************************************/
class behavior
{
public:
	template<typename T>
	behavior(T&& x);

	behavior() noexcept = default;
	behavior(behavior &&) noexcept = default;
	behavior(behavior const&) = delete;
	behavior& operator=(behavior&& x) noexcept = default;
	behavior& operator=(behavior const&) = delete;

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

	template<typename T, int>
	struct model_t;
	std::unique_ptr<struct concept_t> _object;
};
} // cbt
#include "cbt/behavior_t.hpp"
#endif // CBT_BEHAVIOR_HPP
