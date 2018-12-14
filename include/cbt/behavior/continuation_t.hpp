#ifndef CBT_BEHAVIOR_CONTINUATION_T_HPP
#define CBT_BEHAVIOR_CONTINUATION_T_HPP
#include <memory>
#include <type_traits>
namespace cbt
{
template<auto M, typename T>
auto continuation::mem_fn(T& x) -> continuation
{
	constexpr bool a = std::is_invocable_r<continues, decltype(M), T*, status>::value;
	if constexpr (a)
	{
		auto c = continuation{};
		c._that = std::addressof(x);
		c._func = [](void* that, status s)
		{ return (static_cast<T*>(that)->*M)(s); };
		return c;
	}
	else
	{
		static_assert(x, "continuation constructed from invalid member function");
	}
}
} // namespace cbt
#endif // CBT_BEHAVIOR_CONTINUATION_T_HPP
