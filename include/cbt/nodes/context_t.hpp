#ifndef CBT_NODES_CONTEXT_T_HPP
#define CBT_NODES_CONTEXT_T_HPP
#include "cbt/behavior.hpp"
#include <memory>
namespace cbt
{
template<typename T, typename F>
struct _context_type
{
	auto operator()() noexcept -> Status;
	static auto make(T*& out, F) -> behavior_t;
private:
	_context_type(F&& f): _reset(std::move(f)) {}
	T _value;
	F _reset;
};
template<typename T, typename F>
using _context_t = _context_type<std::decay_t<T>, std::decay_t<F>>;

template<typename T, typename F>
auto context(T*& out, F&& f) -> behavior_t
{
	constexpr bool x = std::is_invocable<F, T&>::value;
	static_assert(x, "context called with invalid initialization function");
	static_assert(x, "F is not invocable as void(T)");
	if constexpr (x)
	{
		return _context_t<T, F>::make(out, std::forward<F>(f));
	} else return behavior_t{/* never called */};
}

template<typename T>
auto context(T*& out) -> behavior_t
{
	auto reset = +[](T& p){ p = T{}; };
	return _context_t<T, decltype(reset)>::make(out, reset);
}

template<typename T, typename F>
auto _context_type<T, F>::operator()() noexcept -> Status
{
	try { _reset(_value); }
	catch (...) { return Failure; }
	return Success;
}

template<typename T, typename F>
auto _context_type<T, F>::make(T*& out, F x) -> behavior_t
{
	auto tree = behavior_t{ _context_type{std::move(x)} };
	auto *p = std::addressof(tree.get<_context_type>());
	out = std::addressof(p->_value);
	return std::move(tree);
}
} // cbt
#endif // CBT_NODES_CONTEXT_T_HPP
