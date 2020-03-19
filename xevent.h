#pragma once


#include <list>
#include <vector>
#include <memory>
#include <chrono>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <unordered_map>

#include "xccret.h"

#define _COT_WAIT(x) (co_yield ((time_t)x))
#define _COT_RETURN() co_return (0LL)

#pragma region _BASE

using evcal = xccret<time_t>;

inline auto steady_time(void)
{/*
	using convert_type = ::std::chrono::time_point < ::std::chrono::steady_clock, ::std::chrono::duration<time_t, ::std::milli>>;
	auto t = ::std::chrono::steady_clock::now();*/
	return (::std::chrono::time_point_cast<::std::chrono::duration<time_t, ::std::milli>>(::std::chrono::steady_clock::now()));
}

class _base_event
{
public:
	using message_type = state_t;
public:
	_base_event() noexcept
		:thread_id(::std::this_thread::get_id()),
		message(0),
		state(0),
		time(steady_time().time_since_epoch().count())
	{}

	_base_event(message_type _Val) noexcept
		:message(_Val) {}

	
	virtual ~_base_event(void)
	{
		
	}
public:
	::std::thread::id thread_id;
	message_type message;
	state_t state;
	time_t time;
};

class _base_listener
{
public:
	_base_listener()
		:state(0){}

	virtual ~_base_listener(void)
	{
	}
public:
	evcal virtual on_event(::std::shared_ptr<_base_event> args)
	{
		_COT_RETURN();
	}

public:
	state_t state;
};

using event = ::std::shared_ptr<_base_event>;


// 单纯的计时器类
class timer_t
{
public:
	static constexpr time_t STATE_IDLE = -1;
	static constexpr time_t STATE_RUN = -2;
	static constexpr time_t STATE_ONEXPIRE = 0;
public:
	timer_t()
		:repeat(0),
		interval(0),
		expires(0),
		state(STATE_IDLE),
		listeners()
	{}

	timer_t(timer_t const &) = delete;

	timer_t(timer_t &&_Right) 
		: expires(_Right.expires),
		repeat(_Right.repeat),
		interval(_Right.interval),
		state(_Right.state),
		listeners(::std::move(_Right.listeners))
	{}

	~timer_t()
	{}
public:
	timer_t& operator=(timer_t&& _Other)
	{
		expires = _Other.expires;
		interval = _Other.interval;
		repeat = _Other.repeat;
		state = _Other.state;
		listeners = ::std::forward<::std::vector<_base_listener*>>(_Other.listeners);
	}
	
	timer_t& operator=(timer_t const&) = delete;
public:
	bool is_idle(void) const noexcept
	{
		return (state == STATE_IDLE);
	}

	bool is_running(void) const noexcept
	{
		return (state == STATE_RUN);
	}

	bool is_supend(void) const noexcept
	{
		return (state > 0);
	}

	bool is_on_expire(void) const noexcept
	{
		return (state == 0);
	}
public:
	uint32_t repeat;
	uint32_t interval;
	time_t expires;
	time_t state;
	::std::vector<_base_listener*> listeners;
};

using ref_timer_t = ::std::shared_ptr<timer_t>;
using timer = ref_timer_t;

// 计时器到期事件
class event_timer_expire : public _base_event
{
public:
	using base = _base_event;
public:
	event_timer_expire()
		:base(0x1001),
		timer(0),
		now(0)
	{}
	event_timer_expire(ref_timer_t _Ptr, time_t _Now)
		:base(0x1001),
		timer(::std::forward<ref_timer_t>(_Ptr)),
		now(_Now)
	{}
	virtual ~event_timer_expire()
	{
	}
public:
	ref_timer_t timer;
	time_t now;
};

#pragma endregion

class _XTimeDisp
{
public:
	using _Cont_ty_delay = ::std::list<class xdelay>;
	using _Cont_ty_timer = ::std::list<ref_timer_t>;

	struct _Cont_ty
	{
		_Cont_ty_delay delay;
		_Cont_ty_timer timer;

		inline void swap(_Cont_ty&& _Other) 
		{
			::std::swap(delay,_Other.delay);
			::std::swap(timer,_Other.timer);
		}

		_Cont_ty():delay(), timer(){}

		_Cont_ty(_Cont_ty&) = delete;
		inline _Cont_ty(_Cont_ty&& _Other)
			:delay(::std::forward<_Cont_ty_delay>(_Other.delay)),
			timer(::std::forward<_Cont_ty_timer>(_Other.timer))
		{}

		_Cont_ty& operator=(_Cont_ty&) = delete;
		inline _Cont_ty& operator=(_Cont_ty&& _Other)
		{
			swap(::std::forward<_Cont_ty>(_Other));
			return *this;
		}

		
	};

	struct _Ty_supend
	{
		::std::shared_ptr <timer_t> timer;
		time_t supend;

		_Ty_supend()
			:timer(),
			supend()
		{}

		_Ty_supend(_Ty_supend const&) = delete;
		_Ty_supend(_Ty_supend&& _Other)
			:timer(::std::forward<::std::shared_ptr <timer_t>>(_Other.timer)),
			supend(_Other.supend)
		{}
		_Ty_supend(::std::shared_ptr <timer_t > & _timer,time_t offset)
			:timer(_timer),
			supend(offset)
		{}


		_Ty_supend& operator=(_Ty_supend const&) = delete;
		_Ty_supend& operator=(_Ty_supend&& _Other)
		{
			timer = ::std::forward<::std::shared_ptr <timer_t>>(_Other.timer);
			supend = _Other.supend;
		}

		bool operator==(const _Ty_supend& _Other)
		{
			return (timer == _Other.timer);
		}

		bool operator!=(const _Ty_supend& _Other)
		{
			return (timer != _Other.timer);
		}

		bool operator==(const ::std::shared_ptr <timer_t > & _Other)
		{
			return (timer == _Other);
		}

		bool operator!=(const ::std::shared_ptr <timer_t > & _Other)
		{
			return (timer != _Other);
		}
	};

	using _Cont_ty_supend = ::std::list<_Ty_supend>;

	static constexpr int _TVN_BITS = 4;
	static constexpr int _TVR_BITS = 6;
	static constexpr int _TVN_SIZE = 1 << _TVN_BITS;
	static constexpr int _TVR_SIZE = 1 << _TVR_BITS;
	static constexpr int _TVN_MASK = _TVN_SIZE - 1;
	static constexpr int _TVR_MASK = _TVR_SIZE - 1;

public:
	_XTimeDisp()
		:_tq_mutex(),
		tvec(),
		_sign_mutex(),
		_run_thread(),
		_sign(),
		_thread_state(0)
	{

	}

	~_XTimeDisp()
	{

	}
public:
	static constexpr time_t _INDEX(time_t expires, time_t n)
	{
		return ((expires >> (_TVR_BITS + n * _TVN_BITS)) & _TVN_MASK);
	}

	static constexpr time_t _OFFSET(int n)
	{
		return (_TVR_SIZE + n * _TVN_SIZE);
	}
public:
	time_t _UOffset(time_t expires)
	{
		time_t index = expires - _Check_time;

		unsigned int _VecIndex = 0;
		if (index < _TVR_SIZE)		// tvec_1
		{
			return (expires & _TVR_MASK);
		}
		else if (index < (1 << (_TVR_BITS + 1 * _TVN_BITS)))	// tvec_2
		{
			return (_OFFSET(0) + _INDEX(expires, 0));
		}
		else if (index <= 0)	// 异常处理，视为即将到期的计时器
		{
			return (_Check_time & _TVR_MASK);
		}
		else
		{
			if (index > 0xFFFFFFFFUL)
			{
				index = 0xFFFFFFFFUL;
				expires = index + _Check_time;
			}
			return (_OFFSET(1) + _INDEX(expires, 1));
		}
	}

	inline void put_delay(xdelay&& cc)
	{
		tvec[_UOffset(cc.expires)].delay.push_back(::std::forward<xdelay>(cc));
	}

	inline void put_timer(ref_timer_t& _timer)
	{
		tvec[_UOffset(_timer->expires)].timer.push_back(_timer);
	}

	inline void put_timer(ref_timer_t&& _timer)
	{
		tvec[_UOffset(_timer->expires)].timer.push_back(::std::forward<ref_timer_t>(_timer));
	}

	// 开始一个计时器
	inline void start_timer(ref_timer_t& _timer,time_t offset, int repeat = 0, int interval = 0)
	{
		::std::lock_guard<::std::recursive_mutex> lock(this->_tq_mutex);
		auto expires = this->_Check_time + offset;
		_timer->expires = expires;
		_timer->interval = interval;
		_timer->repeat = repeat;
		_timer->state = timer_t::STATE_RUN;
		put_timer(_timer);
	}
	
	// 暂停一个计时器
	bool supend_timer(ref_timer_t& _timer)
	{
		if (!_timer->is_running())
		{
			return false;
		}
		::std::unique_lock<::std::recursive_mutex> lock(this->_tq_mutex);
		auto index = _UOffset(_timer->expires);
		auto &list = tvec[index].timer;
		auto it = ::std::find(list.begin(), list.end(), _timer);

		if (it == list.end())
		{
			return false;
		}
		list.erase(it);
		_timer->state = _timer->expires - _Check_time;
		return true;
	}
	
	// 恢复一个计时器
	bool resume_timer(ref_timer_t& _timer)
	{
		if (!_timer->is_supend())
		{
			return false;
		}
		
		::std::lock_guard<::std::recursive_mutex> lock_2(this->_tq_mutex);
		_timer->expires = _Check_time + _timer->state;
		_timer->state = timer_t::STATE_RUN;
		put_timer(_timer);
		return true;
	}

	int cascade(int offset, int index)
	{
		_Cont_ty& list = tvec[offset + index];
		_Cont_ty empty;
		::std::swap(empty, list);

		for (auto it = empty.delay.begin(); it != empty.delay.end(); ++it)
		{
			this->put_delay(::std::move(*it));
		}

		for (auto it = empty.timer.begin(); it != empty.timer.end(); ++it)
		{
			this->put_timer(::std::move(*it));
		}
		return index;
	}

	void tick(time_t _Now)
	{
		::std::unique_lock<::std::recursive_mutex> _lock(_tq_mutex);

		while (_Check_time <= _Now)
		{
			int index = _Check_time & _TVR_MASK;
			if (!index &&		// tv1
				!cascade(_OFFSET(0), _INDEX(_Check_time, 0)))	// tv2
			{
				cascade(_OFFSET(1), _INDEX(_Check_time, 1));	// tv3
			}

			++_Check_time;
			_Cont_ty& list = tvec[index];
			_Cont_ty empty;
			::std::swap(empty, list);

			// 协程片段
			for (auto it = empty.delay.begin(); it != empty.delay.end(); ++it)
			{
				auto ret = it->execute();

				if (ret != 0)
				{
					it->expires += ret;
					put_delay(::std::move(*it));
				}
			}

			// 计时器队列
			for (auto it = empty.timer.begin(); it != empty.timer.end(); ++it)
			{
				if (!(*it)->listeners.empty())
				{
					::std::shared_ptr<event_timer_expire> _event(new event_timer_expire(*it,_Now));
					for (auto itr : (*it)->listeners)
					{
						auto cc = itr->on_event(_event);
						auto ret = cc.resume();

						// 判断是否需要加入延时队列
						if (ret != 0)
						{
							put_delay(xdelay(_Now + ret, ::std::move(cc)));
						}
					}
					// 判断是否循环计时器
					if ((*it)->repeat > 0)
					{
						--(*it)->repeat;
						(*it)->expires += (*it)->interval;
						put_timer(*it);
					}
					else if ((*it)->repeat <0)
					{
						(*it)->expires += (*it)->interval;
						put_timer(*it);
					}
				}
			}
		}

	}

	void run()
	{
		while (_thread_state == 1)
		{
			tick(_Check_time);
			::std::unique_lock<::std::mutex> _nofity(_sign_mutex);
			_sign.wait(_nofity);
		}
	}

	void nofity(time_t now)
	{
		_Check_time = now;
		_sign.notify_one();

	}

	void start(time_t time)
	{
		if (_thread_state)
		{
			return;
		}
		_Check_time = time;
		_thread_state = 1;
		_run_thread = ::std::thread(&_XTimeDisp::run, this);
	}

	void stop()
	{
		_thread_state = 0;
		_sign.notify_one();

		if (_run_thread.joinable())
		{
			_run_thread.join();
		}
	}
public:
	time_t _Check_time;
	union
	{
		class
		{
		public:
			_Cont_ty tvec_1[_TVR_SIZE];
			_Cont_ty tvec_2[_TVN_SIZE];
			_Cont_ty tvec_3[_TVN_SIZE];
		};
		_Cont_ty tvec[_TVR_SIZE + 2 * _TVN_SIZE];
	};
	::std::recursive_mutex _tq_mutex;


	::std::mutex _sign_mutex;
	::std::condition_variable _sign;
	::std::thread _run_thread;
	int _thread_state;
};

class _XEventDisp
{
public:
	_XEventDisp()
		:listeners(),
		_timer_wheel()
	{

	}
	~_XEventDisp()
	{

	}
public:
	void send(int code, ::std::shared_ptr<_base_event>& args)
	{
		::std::lock_guard<::std::recursive_mutex> lock(_lis_mutex);
		auto it = listeners.find(code);
		if (it == listeners.end())
		{
			return;
		}

		for (auto &itr : it->second)
		{
			auto cc = itr->on_event(args);

			auto ret = cc.resume();

			if (ret != 0)
			{
				_timer_wheel.put_delay(xdelay(ret, ::std::move(cc)));
			}

		}
	}

	void add(time_t code,_base_listener* list)
	{
		::std::lock_guard<::std::recursive_mutex> lock(_lis_mutex);
		auto it = listeners.find(code);
		if (it == listeners.end())
		{
			auto insert = listeners.insert(::std::make_pair(code, ::std::vector<_base_listener*>()));
			if (!insert.second)
			{
				return;
			}
			it = insert.first;
		}

		it->second.push_back(list);
	}

	void start(time_t now)
	{
		_timer_wheel.start(now);
	}

	void update_time(time_t now)
	{
		_timer_wheel.nofity(now);
	}

	[[nodiscard]] inline auto start_timer(ref_timer_t& timer,time_t offset, int repeat = 0,int interval = 0)
	{
		return (_timer_wheel.start_timer(timer,offset,repeat,interval));
	}

	[[nodiscard]] inline auto create_timer(void)
	{
		return (ref_timer_t(new timer_t()));
	}

	inline bool supend_timer(ref_timer_t& timer)
	{
		return (_timer_wheel.supend_timer(timer));
	}

	inline bool resume_timer(ref_timer_t& timer)
	{
		return (_timer_wheel.resume_timer(timer));
	}
public:
	::std::unordered_map<time_t, ::std::vector<_base_listener*>> listeners;
	::std::recursive_mutex _lis_mutex;
	_XTimeDisp _timer_wheel;
};




