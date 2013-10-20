/*
 * This file is part of `et engine`
 * Copyright 2009-2013 by Sergey Reznik
 * Please, do not modify content without approval.
 *
 */

#include <pthread.h>
#include <unistd.h>
#include <et/threading/atomiccounter.h>
#include <et/threading/thread.h>

namespace et
{
	class ThreadPrivate
	{
	public:
		ThreadPrivate();
		~ThreadPrivate();
		
		static void* threadProc(void* context);
		
	public:
		pthread_attr_t attrib;
		pthread_t thread;
		pthread_mutex_t suspendMutex;
		pthread_cond_t suspend;
		ThreadId threadId;
		AtomicBool running;
		AtomicBool suspended;
	};
}

using namespace et;

ThreadPrivate::ThreadPrivate() :
	thread(nullptr), threadId(0)
{
	attrib = { };
	suspendMutex = { };
	suspend = { };
	
	pthread_attr_init(&attrib);
	pthread_mutex_init(&suspendMutex, 0);
	pthread_cond_init(&suspend, 0);
	
	pthread_attr_setdetachstate(&attrib, PTHREAD_CREATE_JOINABLE);
}

ThreadPrivate::~ThreadPrivate()
{
	pthread_mutex_destroy(&suspendMutex);
	pthread_cond_destroy(&suspend);
	pthread_attr_destroy(&attrib);
}

void* ThreadPrivate::threadProc(void* context)
{
	Thread* thread = static_cast<Thread*>(context);
	thread->_private->threadId = reinterpret_cast<ThreadId>(pthread_self());
	return reinterpret_cast<void*>(thread->main());
}

Thread::Thread() :
	_private(new ThreadPrivate)
{
}

Thread::Thread(bool start) :
	_private(new ThreadPrivate)
{
	if (start)
		run();
}

Thread::~Thread()
{
	delete _private;
}

void Thread::run()
{
	if (_private->running) return;
	
	_private->running = true;
	pthread_create(&_private->thread, &_private->attrib, ThreadPrivate::threadProc, this);
}

void Thread::suspend()
{
	if (_private->suspended) return;
	
	_private->suspended = true;
	
	pthread_mutex_lock(&_private->suspendMutex);
	pthread_cond_wait(&_private->suspend, &_private->suspendMutex);
	pthread_mutex_unlock(&_private->suspendMutex);
}

void Thread::resume()
{
	if (_private->suspended)
	{
		_private->suspended = false;
		pthread_mutex_lock(&_private->suspendMutex);
		pthread_cond_signal(&_private->suspend);
		pthread_mutex_unlock(&_private->suspendMutex);
	}
}

void Thread::stop()
{
	if (_private->running)
	{
		_private->running = false;
		resume();
	}
}

void Thread::waitForTermination()
{
	pthread_join(_private->thread, nullptr);
}

void Thread::terminate(int result)
{
	if (_private->running)
	{
		pthread_detach(_private->thread);
		pthread_exit(reinterpret_cast<void*>(result));
	}
}

ThreadResult Thread::main()
{
	return 0;
}

bool Thread::running() const
{
	return _private->running;
}

bool Thread::suspended() const
{
	return _private->suspended;
}

ThreadId Thread::id() const
{
	return _private->threadId;
}

void Thread::sleep(float seconds)
{
	usleep(static_cast<useconds_t>(seconds * 1000000.0f));
}

void Thread::sleepMSec(uint64_t msec)
{
	usleep(static_cast<useconds_t>(msec) * 1000);
}