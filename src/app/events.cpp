/*
 * This file is part of `et engine`
 * Copyright 2009-2013 by Sergey Reznik
 * Please, do not modify content without approval.
 *
 */

#include <et/app/events.h>

using namespace et;

/*
 * EventReceiver
 */
void EventReceiver::eventConnected(Event* e)
{
	_events.push_back(e); 
}

EventReceiver::~EventReceiver() 
{
	for (auto& i : _events)
		i->receiverDisconnected(this);
}

void EventReceiver::eventDisconnected(Event* e)
{
	auto i = std::find(_events.begin(), _events.end(), e);
	if (i != _events.end())
		_events.erase(i);
}

/**
 *
 * Event0
 *
 */

Event0::Event0() : _invoking(false)
{
}

Event0::~Event0()
{
	for (auto& connection : _connections)
	{
		if (connection->receiver())
		{
			if (!connection->removed())
				connection->receiver()->eventDisconnected(this);

			delete connection;
		}
	}
}

void Event0::connect(Event0& e)
{
	if (&e != this)
		_connections.push_back(&e);
}

void Event0::cleanup()
{
	auto i = remove_if(_connections.begin(), _connections.end(), shouldRemoveConnection);
	if (i != _connections.end())
		_connections.erase(i, _connections.end());
}

void Event0::invoke()
{
	cleanup();

	_invoking = true;

	auto i = _connections.begin();
	while (i != _connections.end())
	{
		(*i)->invoke();
		++i;
	}

	_invoking = false;
}

void Event0::invokeInMainRunLoop(float delay)
{
	cleanup();
	
	for (auto& i : _connections)
		i->invokeInMainRunLoop(delay);
}

void Event0::receiverDisconnected(EventReceiver* r)
{
	auto i = _connections.begin();
	while (i != _connections.end())
	{
		if (r == (*i)->receiver())
		{
			if (_invoking)
			{
				(*i)->remove();
				++i;
			}
			else
			{
				delete (*i);
				i = _connections.erase(i);
			}
		}
		else 
		{
			++i;
		}
	}
}