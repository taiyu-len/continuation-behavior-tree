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
	struct concept_t;
	template<typename T,
		bool = std::is_invocable_r<continues, T, continuation>::value,
		bool = std::is_invocable_r<void, T, continuation>::value,
		bool = std::is_invocable_r<Status, T>::value>
	struct model_t;
	template<typename T> using model = model_t<std::decay_t<T>>;

public:
	template<typename T>
	behavior_t(T&& x);

	behavior_t(behavior_t &&) noexcept = default;
	behavior_t(behavior_t const&) = delete;
	behavior_t& operator=(behavior_t&& x) noexcept = default;
	behavior_t& operator=(behavior_t const&) = delete;

	void run(continuation c) const noexcept;

protected:
	friend continues;
	auto step(continuation c) const noexcept -> continues;
	std::unique_ptr<struct concept_t> _object;
};

struct behavior_t::concept_t
{
	virtual ~concept_t() noexcept = default;
	virtual auto start(continuation) noexcept -> continues = 0;
};

template<typename T, bool _>
struct behavior_t::model_t<T, true, _, false> : concept_t
{
	model_t(T x) noexcept: _data(std::move(x)) {};
	auto start(continuation c) noexcept -> continues override
	{ return _data(std::move(c)); }

	T _data;
};

template<typename T>
struct behavior_t::model_t<T, false, true, false> : concept_t
{
	model_t(T x) noexcept: _data(std::move(x)) {};
	auto start(continuation c) noexcept -> continues override
	{ _data(std::move(c)); return continues::elsewhere(); }

	T _data;
};

template<typename T>
struct behavior_t::model_t<T, false, false, true> : concept_t
{
	model_t(T x) noexcept: _data(std::move(x)) {};
	auto start(continuation c) noexcept -> continues override
	{ return continues::up(std::move(c), _data()); }

	T _data;
};

template<typename T, bool x, bool y, bool z>
struct behavior_t::model_t
{
	static_assert(sizeof(T) < 0, "behavior constructed with invalid type");
	static_assert(!x   , "is invocable as continues(contiuation)");
	static_assert(!y||x, "is invocable as void(contiuation)");
	static_assert(!z   , "is invocable as Status()");
};

template<typename T>
behavior_t::behavior_t(T&& x)
: _object(std::make_unique<model<T>>(std::forward<T>(x))) {}
} // cbt
#endif // CBT_BEHAVIOR_T_HPP
