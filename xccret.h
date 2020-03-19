#pragma once

#include <experimental/coroutine>
#include <cstddef>
#include <chrono>


// 协程返回值封装类型，保存有协程句柄，通过该对象控制具体协程行为
template<class _Ty>
class xccret
{
public:

	class promise_type
	{
	public:
		using value_type = typename _Ty;
	public:
		auto get_return_object()
		{
			return xccret(*this);
		}

		std::experimental::suspend_always initial_suspend() noexcept
		{
			return std::experimental::suspend_always{};
		}

		std::experimental::suspend_always final_suspend() noexcept
		{
			// suspend it to save the coroutine handle 
			return std::experimental::suspend_always{};
		}

		auto yield_value(value_type _Value)
		{
			_CurrentValue = _Value;
			return std::experimental::suspend_always{};
		}

		auto return_value(value_type _Value)
		{
			_CurrentValue = _Value;
			return std::experimental::suspend_always{}; // dont suspend it
		}
		
		void unhandled_exception() noexcept 
		{
			_Eptr = ::std::current_exception();
		}

		void _Rethrow_if_exception() 
		{
			if (_Eptr) {
				::std::rethrow_exception(_Eptr);
			}
		}

		//void return_void() {}

		value_type operator *(void) const noexcept
		{
			return _CurrentValue;
		}

	public:
		value_type _CurrentValue;
		::std::exception_ptr _Eptr;
	};

	using value_type = typename promise_type::value_type;
public:

	explicit xccret(promise_type &_Prom)
		: _Coro(::std::experimental::coroutine_handle<promise_type>::from_promise(_Prom)),
		_Value(*_Prom)
	{
		
	}

	xccret() = default;

	xccret(xccret const &) = delete;


	xccret(xccret &&_Right) : _Coro(_Right._Coro), _Value(_Right._Value)
	{
		_Right._Coro = nullptr;
		_Right._Value = 0;
	}

	~xccret()
	{
		if (_Coro) {
			_Coro.destroy();
		}
	}
public:
	_NODISCARD value_type resume()
	{
		if (_Coro) {
			_Coro.resume();
			_Value = *_Coro.promise();
			if (_Coro.done() || (_Value == 0))
			{
				_Coro.destroy();
				_Coro = 0;
				_Value = 0;
				return _Value;
			}
		}
		return _Value;
	}

public:
	xccret &operator=(xccret const &) = delete;

	xccret &operator=(xccret &&_Right)
	{
		if (this != _STD addressof(_Right)) {
			_Coro = _Right._Coro;
			_Right._Coro = nullptr;
			_Value = _Right._Value;
			_Right._Value = 0;
		}
		return *this;
	}

	operator bool(void)
	{
		return (_Coro != 0);
	}
private:
public:
	::std::experimental::coroutine_handle<promise_type> _Coro = nullptr;
	value_type _Value = 0;
};

using time_t = signed long long;
using state_t = unsigned long long;
using uint32_t = unsigned int;
using int32_t = int;


// 协程延迟动作类型
class xdelay
{
public:
	xdelay()
		:expires(0),
		handle()
	{
	}
	xdelay(size_t time, xccret<time_t>&& cc)
		:expires(time),
		handle(::std::forward<xccret<time_t>>(cc))
	{

	}

	xdelay(xdelay const &) = delete;


	xdelay(xdelay &&_Right) : expires(_Right.expires), handle(::std::move(_Right.handle))
	{
		_Right.expires = 0;
		_Right.handle = xccret<time_t>();
	}
	~xdelay()
	{
		handle.~xccret();
	}
public:
	inline auto execute(void)
	{
		return handle.resume();
	}
public:
	xccret<time_t> handle;
	time_t expires;
};
