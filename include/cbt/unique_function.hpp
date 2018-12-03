#pragma once
#include <memory>
#include <utility>
#include <cassert>
// https://github.com/arcanis/tco
// rewritten version of the my::function from this.

namespace cbt
{
template<typename T>
class unique_function;

template<typename R, typename...Args>
class unique_function<R(Args...)>
{
	using function_t = R(Args...);
	struct concept_t;
	template<typename T, bool = std::is_invocable_r<R, T, Args...>::value>
	struct model_t;
	template<typename T>
	using model = model_t<std::decay_t<T>>;
public:
	unique_function() noexcept = default;
	unique_function(std::nullptr_t) noexcept {};
	unique_function(unique_function &&) noexcept = default;
	unique_function& operator=(unique_function &&) noexcept = default;
	unique_function& operator=(std::nullptr_t) noexcept { _ptr.reset(); };

	template<typename T>
	unique_function(T&& x)
	: _ptr(std::make_unique<model<T>>(std::forward<T>(x))) {};

	R operator()(Args... args) const
	{
		assert(_ptr != nullptr);
		return _ptr->operator()(std::forward<Args>(args) ...);
	};

	operator bool() const noexcept
	{ return bool(_ptr); }
	friend void swap(unique_function& x, unique_function& y)
	{ std::swap(x._ptr, y._ptr); }
	friend bool operator==(unique_function const& x, unique_function const& y)
	{ return x._ptr == y._ptr; }
	friend bool operator!=(unique_function const& x, unique_function const& y)
	{ return !(x == y); }

private:
	struct concept_t
	{
		virtual ~concept_t() noexcept = default;
		virtual R operator()(Args... args) = 0;
	};

	template<typename T>
	struct model_t<T, true> : concept_t
	{
		model_t(T&& x): _data(std::move(x)) {};
		R operator()(Args... args) override { return _data(args...); }
		T _data;
	};
	template<typename T>
	struct model_t<T, false>
	{
		static_assert(sizeof(T)<0,
			"unique_function constructed with invalid function object");
	};
	std::unique_ptr<concept_t> _ptr {};
};
}
