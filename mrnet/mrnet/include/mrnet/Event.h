/****************************************************************************
 * Copyright © 2003-2008 Dorian C. Arnold, Philip C. Roth, Barton P. Miller *
 *                  Detailed MRNet usage rights in "LICENSE" file.          *
 ***************************************************************************/

#if !defined( __event_h )
#define __event_h  1

#include <string>

#include "mrnet/Types.h"

namespace MRN {

typedef enum {
    EBADCONFIG,
    ESYSTEM,
    EPACKING,
    EFMTSTR,
    EPROTOCOL,
    UNKNOWN_EVENT
} EventType;

class Event{
 private:
    Event(){} //explicitly disallow creation without "new_Event()"

 public:
    virtual ~Event(){};
    static Event * new_Event( EventType t, Rank irank, std::string desc="" );
    static bool have_Event();
    static bool have_RemoteEvent();
    static void add_Event( Event & );
    static Event * get_NextEvent();
    static Event * get_NextRemoteEvent();
    static unsigned int get_NumEvents();
    static unsigned int get_NumRemoteEvents();

    virtual EventType get_Type( )=0;
    virtual const std::string & get_HostName( )=0;
    virtual Port get_Port( )=0;
    virtual const std::string & get_Description( )=0;
};

} /* namespace MRN */

#endif /* __event_h */
