/***********************************************************************
 * Copyright © 2003-2004 Dorian C. Arnold, Philip C. Roth, Barton P. Miller *
 *                  Detailed MRNet usage rights in "LICENSE" file.     *
 **********************************************************************/

#if !defined(eventimpl_h)
#define eventimpl_h 1

#include <string>
#include "mrnet/MRNet.h"
#include "mrnet/src/utils.h"

namespace MRN
{

class EventImpl: public Event{
 private:
    EventType type;
    std::string hostname;
    unsigned short port;
    std::string description;
    static std::list<EventImpl *> events;
    static std::list<EventImpl *> remote_events;

 public:
    static bool have_Event(){
        return !( events.empty() );
    }
    static bool have_RemoteEvent(){
        return !( remote_events.empty() );
    }

    static void add_Event(EventImpl &event){
        events.push_back( new EventImpl( event ) );
        remote_events.push_back( new EventImpl( event ) );
    }

    static EventImpl * get_NextEvent() {
        EventImpl * ret = *( events.begin() );
        events.pop_front();
        return ret;
    }

    static EventImpl * get_NextRemoteEvent() {
        EventImpl * ret = *( remote_events.begin() );
        remote_events.pop_front();
        return ret;
    }

    static unsigned int get_NumRemoteEvents() {
        return remote_events.size();
    }

    static unsigned int get_NumEvents() {
        return events.size();
    }

    EventImpl( EventType t, std::string desc="", std::string h=LocalHostName,
           unsigned short p=LocalPort )
        :type(t), hostname(h), port(p), description(desc) {}

    virtual ~EventImpl(){}

    virtual EventType get_Type( ){
        return type;
    }

    virtual const std::string & get_HostName( ){
        return hostname;
    }

    virtual unsigned short get_Port( ){
        return port;
    }

    virtual const std::string & get_Description( ){
        return description;
    }
};

} /* namespace MRN */
#endif /* eventimpl_h */
