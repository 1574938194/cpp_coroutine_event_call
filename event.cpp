#include "pch.h"

#include <iostream>

#include "xevent.h"

#include "xccret.h"
using namespace std;



class event_unit_damage : public _base_event
{
public:
	using base =  _base_event;
public:
	 event_unit_damage()
		:base(0x100),
		object(0),
		attacker(0)
	{}
public:
	int object;
	int attacker;
};


class event_time_pass : public _base_event
{
public:
	using base =  _base_event;
public:
	 event_time_pass()
		:base(0x102),
		now(0) {}
public:
	size_t now;
};


class listener_function_001 : public _base_listener
{
public:
	using base = _base_listener;

public:
	listener_function_001()
		:base()
	{

	}

	virtual ~listener_function_001(void)
	{
		
	}
public:
	xccret<time_t> virtual on_event(::std::shared_ptr<_base_event> args)
	{
		auto _event = ::std::reinterpret_pointer_cast<event_timer_expire>(args);
		::std::cout << _event->now << ", action WITH PARAMS :"<< _event->timer <<  std::endl;

		_COT_WAIT(3000);

		::std::cout << _event->now << ", ACTION END :" << std::endl;

		_COT_RETURN();
	}
};





using tdc = _XTimeDisp;
using edc = _XEventDisp;


::std::shared_ptr<timer_t> _timer;
edc tm;

class listener_function_002 : public _base_listener
{
public:
	using base = _base_listener;

public:
	listener_function_002()
		:base()
	{

	}

	virtual ~listener_function_002(void)
	{

	}
public:
	xccret<time_t> virtual on_event(::std::shared_ptr<_base_event> args)
	{
		auto tp1 = std::chrono::steady_clock::now();
		::std::cout << steady_time().time_since_epoch().count() << std::endl;
		_COT_WAIT(3000LL);
		::std::cout << steady_time().time_since_epoch().count() << std::endl;
		tm.supend_timer(_timer);

		_COT_WAIT(5000);

		tm.resume_timer(_timer);
		auto tp2 = std::chrono::steady_clock::now();
		std::cout << std::chrono::duration_cast<std::chrono::microseconds>(tp2 - tp1).count() << " microseconds" << std::endl;
		_COT_RETURN();
	}
};

int main()
{

	


	listener_function_001 t1;
	listener_function_002 t2;

	_timer = tm.create_timer();

	_timer->listeners.push_back(&t1);

	tm.start_timer(_timer, 1000,-1,1000);

	tm.start(0);

	auto task_timer = tm.create_timer();
	task_timer->listeners.push_back(&t2);
	tm.start_timer(task_timer, 0);



	int time = 0;
	auto tp1 = std::chrono::steady_clock::now();

	auto siz = sizeof(tp1);
	while (1)
	{
		//::std::this_thread::sleep_for(::std::chrono::milliseconds(1));
		auto tp2 = std::chrono::steady_clock::now();

		auto v = std::chrono::duration_cast<std::chrono::milliseconds>(tp1 - tp2).count();
		if (std::chrono::duration_cast<std::chrono::milliseconds>(tp2 - tp1).count() < time)
		{
			continue;
		}
		tm.update_time(time);
		time += 1;
	}

	system("pause");
	return 0;
}

#undef _COT_WAIT
#undef _COT_NORET