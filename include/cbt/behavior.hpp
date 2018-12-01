#ifndef CBT_BEHAVIOR_HPP
#define CBT_BEHAVIOR_HPP
#include <memory>
#include "cbt/status.hpp"
#include "cbt/continuation.hpp"

namespace cbt
{
/*
 * a behavior_t can be constructed with any object that can be
 * - move constructed;
 * - callable via x(continuation);
 *   - the continuation is a valid function
 *   - the continuation may only be called once, after which the reference is
 *     expeceted to be invalid.
 *   - as long as the behavior tree itself exists, the reference is valid until
 *     that
 * - or callable via x() => Status
 *   - for leaves that dont pass continuation any where
 *
 *****************************************************************************/
class behavior_t
{
	struct concept_t;
	template<typename T, bool> struct model;

	template<typename T> using model_t = model<
		std::decay_t<T>,
		takes_continuation<T>::value>;
public:
	using ptr_t = std::unique_ptr<struct concept_t>;

	template<typename T>
	behavior_t(T&& x);
	behavior_t(behavior_t &&) = default;
	behavior_t& operator=(behavior_t&& x) = default;

	// calls the stored object with the given continuation
	// - the first version saves the continuation before calling
	// - the second uses the saved continuation to call with
	void operator()(continuation_type c) const;
	void operator()() const;

protected:
	// pointer to the saved object.
	ptr_t _object;
};

struct behavior_t::concept_t
{
	virtual ~concept_t() = default;
	virtual void start(continuation) = 0;
	// default continuation is no-op.
	continuation_type _continue = [](Status){};
};

template<typename T>
struct behavior_t::model<T, true> : concept_t
{
	model(T x): _data(std::move(x)) {};
	void start(continuation c) override { _data(std::move(c)); }
	T _data;
};

template<typename T>
struct behavior_t::model<T, false> : concept_t
{
	model(T x): _data(std::move(x)) {};
	void start(continuation c) override { c(_data()); }
	T _data;
};

template<typename T>
behavior_t::behavior_t(T&& x)
: _object(std::make_unique<model_t<T>>(std::forward<T>(x))) {}
} // cbt
#endif // CBT_BEHAVIOR_T_HPP
