/***********************************************************************
 * Copyright © 2003 Dorian C. Arnold, Philip C. Roth, Barton P. Miller *
 *                  Detailed MRNet usage rights in "LICENSE" file.     *
 **********************************************************************/

#include <stdio.h>
#include <stdarg.h>


#include "mrnet/src/Errors.h"

namespace MRN
{

struct ErrorDefs errors[]= {
    { MRN_ENONE, MRN_INFO, MRN_IGNORE, "No Error"},
    { MRN_EBADCONFIG_IO, MRN_CRIT, MRN_ABORT, "Config File Input Error"},
    { MRN_EBADCONFIG_FMT, MRN_CRIT, MRN_ABORT, "Config File Format Error"},
    { MRN_ENETWORK_CYCLE, MRN_CRIT, MRN_ABORT,
      "Config File Error: Network cycle exists"},
    { MRN_ENETWORK_NOTCONNECTED, MRN_CRIT, MRN_ABORT,
      "Config File Error: Network not fully connected"},
    { MRN_ENETWORK_FAILURE, MRN_CRIT, MRN_ABORT, "Network Failure"}, 
    { MRN_EOUTOFMEMORY, MRN_CRIT, MRN_ABORT, "Out of Memory"}, 
    { MRN_EFMTSTR_MISMATCH, MRN_ERR, MRN_ALERT, "Format string mismatch"}, 
    { MRN_ECREATPROCFAILURE, MRN_ERR, MRN_ALERT, "Cannot create process"}, 
    { MRN_ECANNOTBINDPORT, MRN_ERR, MRN_ABORT, "Cannot bind to port"}, 
    { MRN_ESOCKETCONNECT, MRN_ERR, MRN_ABORT, "Cannot connect to socket"}, 
    { MRN_EPACKING, MRN_CRIT, MRN_ABORT, "PDR encoding/decoding failure"}
};


void Error::error( EventType t, const char * fmt, ... )
{
    static char buf[1024];

    va_list arglist;

    _fail=true;
    va_start( arglist, fmt );
    vsprintf( buf, fmt, arglist );
    va_end( arglist );

    Event * event = Event::new_Event( t, buf );
    Event::add_Event( *event );
    delete event;
}

} // namespace MRN
