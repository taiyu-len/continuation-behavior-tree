#ifndef CBT_BEHAVIOR_HPP
#define CBT_BEHAVIOR_HPP
#include <memory>
#include "cbt/behavior/status.hpp"
#include "cbt/behavior/continuation.hpp"
#include "cbt/behavior/continues.hpp"

namespace cbt
{
/*
 * a behavior_t can be constructed with any object that can be
 * - move constructed;
 * - constructable via an object with call signature void(continuation);
 *   - the continuation must be a valid function
 *   - the continuation may only be called once, after which the reference is
 *     expeceted to be invalid.
 *   - the reference must live as long as the behavior tree does
 * - constructable via an object with call signature Status()
 *   - for leaves that dont move the continuation any where
 *
 *****************************************************************************/
class behavior_t
{
	struct concept_t;
	template<typename T,
		// for nodes that pass continuations into children
		bool = std::is_invocable_r<continues, T, continuation>::value,
		// for nodes that pass the continuation elsewhere
		bool = std::is_invocable_r<void, T, continuation>::value,
		// for leaves that simply return a status
		bool = std::is_invocable_r<Status, T>::value>
	struct model_t;

public:
	template<typename T>
	behavior_t(T&& x);

	behavior_t(behavior_t &&) noexcept = default;
	behavior_t(behavior_t const&) = delete;
	behavior_t& operator=(behavior_t&& x) noexcept = default;
	behavior_t& operator=(behavior_t const&) = delete;

	// user facing call operator
	void run(continuation c) const noexcept;

protected:
	friend continues;
	auto step(continuation c) const noexcept -> continues;
	// pointer to the saved object.
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
	{ _data(std::move(c)); return {}; }

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
: _object(std::make_unique<model_t<T>>(std::forward<T>(x))) {}
} // cbt
#endif // CBT_BEHAVIOR_T_HPP
