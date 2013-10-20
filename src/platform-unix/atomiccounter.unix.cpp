/*
 * This file is part of `et engine`
 * Copyright 2009-2013 by Sergey Reznik
 * Please, do not modify content without approval.
 *
 */


#include <et/core/et.h>

#if (ET_PLATFORM_ANDROID)
#	include <sys/atomics.h>
#else
#	include <libkern/OSAtomic.h>
#endif

#include <et/threading/atomiccounter.h>

using namespace et;

AtomicCounter::AtomicCounter() : _counter(0)
{
}

AtomicCounterType AtomicCounter::retain()
{
#if (ET_PLATFORM_ANDROID)
	return __atomic_dec(&_counter);
#else
	return OSAtomicIncrement32(&_counter);
#endif
}

AtomicCounterType AtomicCounter::release()
{
#if (ET_PLATFORM_ANDROID)
	return __atomic_dec(&_counter);
#else
	return OSAtomicDecrement32(&_counter);
#endif
}

static const AtomicCounterType validMask = static_cast<AtomicCounterType>(0xfffffffc);

AtomicBool::AtomicBool() :
	_value(0) { }

bool AtomicBool::operator = (bool b)
{
	assert((_value & validMask) == 0);
	OSAtomicCompareAndSwap32Barrier(_value, AtomicCounterType(b), &_value);
	return (_value != 0);
}

bool AtomicBool::operator == (bool b)
	{ assert((_value & validMask) == 0); return b == (_value != 0); }

bool AtomicBool::operator == (const AtomicBool& r)
	{ assert((_value & validMask) == 0); return (r._value != 0) == (_value != 0); }

bool AtomicBool::operator != (bool b)
	{ assert((_value & validMask) == 0); return b != (_value != 0); }

bool AtomicBool::operator != (const AtomicBool& r)
	{ assert((_value & validMask) == 0); return (r._value != 0) != (_value != 0); }

AtomicBool::operator bool() const
	{ assert((_value & validMask) == 0); return (_value != 0); }