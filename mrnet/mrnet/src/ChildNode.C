/****************************************************************************
 * Copyright © 2003-2005 Dorian C. Arnold, Philip C. Roth, Barton P. Miller *
 *                  Detailed MRNet usage rights in "LICENSE" file.          *
 ****************************************************************************/

#include <stdio.h>

#include "mrnet/src/ChildNode.h"
#include "mrnet/src/utils.h"

namespace MRN
{
const RemoteNode * ChildNode::upstream_node=NULL;

/*===================================================*/
/*  ChildNode CLASS METHOD DEFINITIONS            */
/*===================================================*/
ChildNode::ChildNode(std::string ihostname, Port iport, bool ithreaded )
    :hostname(ihostname), port(iport), threaded(ithreaded)
{
}

int ChildNode::send_Events( ) const
{
    int status=0;
    mrn_dbg( 3, mrn_printf(FLF, stderr, "Entering send_Event() ... \n" ));

    while ( Event::have_RemoteEvent() ){
        Event * cur_event = Event::get_NextRemoteEvent();

        Packet packet( 0, PROT_EVENT, "%d %s %s %uhd",
                       cur_event->get_Type(),
                       cur_event->get_Description().c_str(),
                       cur_event->get_HostName().c_str(),
                       cur_event->get_Port() );

        if( packet.good(  ) ) {
            if( upstream_node->send( packet ) == -1 ||
                upstream_node->flush(  ) == -1 ) {
                mrn_dbg( 1, mrn_printf(FLF, stderr, "send/flush failed\n" ));
                status = -1;
            }
        }
        else {
            mrn_dbg( 1, mrn_printf(FLF, stderr, "new packet() failed\n" ));
            status = -1;
        }
    }

    mrn_dbg( 3, mrn_printf(FLF, stderr, "send_Event() succeeded\n" ));
    return status;
}

int ChildNode::getConnections( int** conns, unsigned int* nConns ) const
{
    int ret = 0;

    if( (conns != NULL) && (nConns != NULL) ) {
        *nConns = 1;
        *conns = new int[*nConns];
        (*conns)[0] = upstream_node->get_SocketFd();
    }
    else {
        ret = -1;
    }
    return ret;
}

void ChildNode::error( ErrorCode e, const char *fmt, ... ) const
{
    char buf[1024];

    va_list arglist;

    MRN_errno = e;
    va_start( arglist, fmt );
    vsprintf( buf, fmt, arglist );
    va_end( arglist );

    //Event * event = Event::new_Event( t, buf );

    //First add event to queue
    //Event::add_Event( *event );

    //Then invoke send_Events() to send upstream
    //send_Events();

    //delete event;
}

} // namespace MRN
