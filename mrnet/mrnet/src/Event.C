/****************************************************************************
 * Copyright © 2003-2008 Dorian C. Arnold, Philip C. Roth, Barton P. Miller *
 *                  Detailed MRNet usage rights in "LICENSE" file.          *
 ****************************************************************************/

#include <list>

#include "mrnet/MRNet.h"

namespace MRN {

std::list<EventImpl *> EventImpl::events;
std::list<EventImpl *> EventImpl::remote_events;

Event * Event::new_Event( EventType t, Rank r, std::string desc )
{
    return new EventImpl( t, desc, h, p );
}

bool Event::have_Event()
{
    return EventImpl::have_Event();
}

bool Event::have_RemoteEvent()
{
    return EventImpl::have_RemoteEvent();
}

void Event::add_Event( Event & event ){
    EventImpl::add_Event( (EventImpl&)event );
}

Event * Event::get_NextEvent()
{
    return EventImpl::get_NextEvent();
}

Event * Event::get_NextRemoteEvent()
{
    return EventImpl::get_NextRemoteEvent();
}

unsigned int Event::get_NumEvents()
{
    return EventImpl::get_NumEvents();
}

unsigned int Event::get_NumRemoteEvents()
{
    return EventImpl::get_NumRemoteEvents();
}
}
