#ifndef CBT_BEHAVIOR_T_HPP
#define CBT_BEHAVIOR_T_HPP
#include <memory>
#include "cbt/behavior/status.hpp"
#include "cbt/behavior/continuation.hpp"
#include "cbt/behavior/continues.hpp"

namespace cbt
{
struct behavior::concept_t
{
	virtual ~concept_t() noexcept = default;
	virtual auto start(continuation) noexcept -> continues = 0;
};

template<typename T>
struct behavior::model_base : concept_t
{
	model_base(T&& data): _data(std::move(data)) {}
	T _data;
};

template<typename T, bool x, bool y, bool z>
struct behavior::model_t : model_base<T>
{
	model_t(T data): model_base<T>{std::move(data)} {};
	auto start(continuation c) noexcept -> continues override
	{
		if constexpr (x)
		{
			return this->_data(std::move(c));
		}
		else if constexpr (y)
		{
			this->_data(std::move(c));
			return continues::elsewhere();
		}
		else if constexpr (z)
		{
			return continues::up(std::move(c), this->_data());
		}
	}

};

template<typename T>
behavior::behavior(T&& object)
{
	constexpr bool x = std::is_invocable_r<continues, T, continuation>::value;
	constexpr bool y = !x && std::is_invocable_r<void, T, continuation>::value;
	constexpr bool z = std::is_invocable_r<Status, T>::value;
	constexpr bool valid = x+y+z == 1;
	static_assert(valid, "behavior constructed with invalid type");
	static_assert(valid || !x, "invocable as continues(continuation)");
	static_assert(valid || !y, "invocable as void(continuation)");
	static_assert(valid || !z, "invocable as Status()");
	static_assert(valid || x, "not invocable as continues(continuation)");
	static_assert(valid || y, "not invocable as void(continuation)");
	static_assert(valid || z, "not invocable as Status()");
	if constexpr (valid)
	{
		_object = std::make_unique<model_t<std::decay_t<T>, x, y, z>>(
			std::forward<T>(object));
	}
}

template<typename T>
auto behavior::get() const -> T&
{
	auto *ptr = static_cast<model_base<std::decay_t<T>>*>(_object.get());
	return ptr->_data;
}
} // cbt
#endif // CBT_BEHAVIOR_T_HPP
