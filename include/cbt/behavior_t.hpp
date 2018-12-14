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

template<typename T, int type>
struct behavior::model_t : model_base<T>
{
	model_t(T data): model_base<T>{std::move(data)} {};
	auto start(continuation c) noexcept -> continues override
	{
		if constexpr (type == 1)
		{
			return this->_data(std::move(c));
		}
		else if constexpr (type == 2)
		{
			this->_data(std::move(c));
			return continues::elsewhere();
		}
		else if constexpr (type == 3)
		{
			return continues::up(std::move(c), this->_data());
		}
		else if constexpr (type == 4)
		{
			auto result = this->_data()
				? status::success : status::failure;
			return continues::up(std::move(c), result);
		}
	}

};

template<typename T>
behavior::behavior(T&& x)
{
	constexpr int kind =
		std::is_invocable_r_v<continues, T, continuation> ? 1 :
		std::is_invocable_r_v<void     , T, continuation> ? 2 :
		std::is_invocable_r_v<status   , T              > ? 3 :
		std::is_invocable_r_v<bool     , T              > ? 4 : -1;
	if constexpr (kind > 0)
	{
		using M = model_t<std::decay_t<T>, kind>;
		_object = std::make_unique<M>(std::forward<T>(x));
	}
	static_assert(kind>0, "behavior constructed with invalid type");
	static_assert(kind>0, "not invocable as continues(continuation)");
	static_assert(kind>0, "not invocable as void(continuation)");
	static_assert(kind>0, "not invocable as status()");
	static_assert(kind>0, "not invocable as bool()");
}

template<typename T>
auto behavior::get() const -> T&
{
	auto *ptr = static_cast<model_base<std::decay_t<T>>*>(_object.get());
	return ptr->_data;
}
} // namespace cbt
#endif // CBT_BEHAVIOR_T_HPP
