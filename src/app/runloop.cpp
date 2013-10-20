/*
 * This file is part of `et engine`
 * Copyright 2009-2013 by Sergey Reznik
 * Please, do not modify content without approval.
 *
 */

#include <algorithm>
#include <et/app/runloop.h>
#include <et/tasks/tasks.h>

using namespace et;

RunLoop::RunLoop() :
	_actualTimeMSec(0), _time(0.0f), _activityTimeMSec(0), _started(false), _active(true)
{
	attachTimerPool(TimerPool::Pointer::create(this));
}

void RunLoop::update(uint64_t t)
{
	updateTime(t);

	if (_active) 
	{
		_taskPool.update(_time);
		for (auto& tp : _timerPools)
			tp->update(_time);
	}
}

void RunLoop::addTask(Task* t, float delay)
{
	_taskPool.addTask(t, delay);
}

void RunLoop::attachTimerPool(TimerPool::Pointer pool)
{
	if (std::find(_timerPools.begin(), _timerPools.end(), pool) == _timerPools.end())
	{
		pool->setOwner(this);
		_timerPools.push_back(pool);
	}
}

void RunLoop::detachTimerPool(TimerPool::Pointer pool)
{
	auto i = _timerPools.begin();
	while (i != _timerPools.end())
	{
		auto& p = *i;
		if (p == pool)
		{
			pool->setOwner(0);
			_timerPools.erase(i);
			break;
		}
		else 
		{
			++i;
		}
	}
}

void RunLoop::detachAllTimerPools()
{
	for (auto& i : _timerPools)
		i->setOwner(nullptr);
	
	_timerPools.clear();
}

void RunLoop::pause()
{
	_active = false;
}

void RunLoop::resume()
{
	if (_active) return;

	_active = true;
	_activityTimeMSec = _actualTimeMSec;
}

void RunLoop::updateTime(uint64_t t)
{
	_actualTimeMSec = t;
	
	if (!_started)
	{
		_started = true;
		_activityTimeMSec = _actualTimeMSec;
	}

	if (_active) 
	{
		_time += static_cast<float>(t - _activityTimeMSec) / 1000.0f;
		_activityTimeMSec = t;
	}
}