#ifndef CBT_NODES_CONTEXT_T_HPP
#define CBT_NODES_CONTEXT_T_HPP
#include "cbt/behavior.hpp"
#include <memory>
namespace cbt
{
namespace detail
{
template<typename T, typename F>
struct context_type
{
	auto operator()() noexcept -> Status;
	static auto make(F) -> context_result<T>;
private:
	context_type(F&& f): _reset(std::move(f)) {}
	T _value;
	F _reset;
};

template<typename T, typename F>
auto context_type<T, F>::operator()() noexcept -> Status
{
	try { _reset(_value); }
	catch (...) { return Failure; }
	return Success;
}

template<typename T, typename F>
auto context_type<T, F>::make(F x) -> context_result<T>
{
	auto tree = behavior_t{ context_type{std::move(x)} };
	auto *p = std::addressof(tree.get<context_type>()._value);
	return {
		std::move(tree),
		p
	};
}

template<typename T, typename F>
using context_t = context_type<std::decay_t<T>, std::decay_t<F>>;
} // detail

template<typename T, typename F>
auto context(F&& f) -> context_result<T>
{
	constexpr bool x = std::is_invocable<F, T&>::value;
	if constexpr (x)
	{
		return detail::context_t<T, F>::make(std::forward<F>(f));
	}
	else
	{
		static_assert(x, "context called with invalid initialization function");
		static_assert(x, "F is not invocable as void(T)");
	}
}

template<typename T>
auto context() -> context_result<T>
{
	auto reset = +[](T& p){ p = T{}; };
	return detail::context_t<T, decltype(reset)>::make(reset);
}
} // cbt
#endif // CBT_NODES_CONTEXT_T_HPP
