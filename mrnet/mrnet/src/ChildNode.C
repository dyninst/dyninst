/***********************************************************************
 * Copyright © 2003-2004 Dorian C. Arnold, Philip C. Roth, Barton P. Miller *
 *                  Detailed MRNet usage rights in "LICENSE" file.     *
 **********************************************************************/

#include <stdio.h>

#include "mrnet/src/ChildNode.h"
#include "mrnet/src/utils.h"

namespace MRN
{

/*===================================================*/
/*  ChildNode CLASS METHOD DEFINITIONS            */
/*===================================================*/
ChildNode::ChildNode(bool _threaded, std::string _host, unsigned short _port)
    :hostname(_host), port(_port), threaded(_threaded)
{
}

ChildNode::~ChildNode(void)
{
}

int ChildNode::send_Events( )
{
    int status=0;
    mrn_printf( 3, MCFL, stderr, "Entering send_Event() ... \n" );

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
                mrn_printf( 1, MCFL, stderr, "send/flush failed\n" );
                status = -1;
            }
        }
        else {
            mrn_printf( 1, MCFL, stderr, "new packet() failed\n" );
            status = -1;
        }
    }

    mrn_printf( 3, MCFL, stderr, "send_Event() succeeded\n" );
    return status;
}

int ChildNode::getConnections( int** conns, unsigned int* nConns )
{
    int ret = 0;

    if( (conns != NULL) && (nConns != NULL) ) {
        *nConns = 1;
        *conns = new int[*nConns];
        (*conns)[0] = upstream_node->get_sockfd();
    }
    else {
        ret = -1;
    }
    return ret;
}

void ChildNode::error( EventType t, const char *fmt, ... )
{
    static char buf[1024];

    va_list arglist;

    _fail=true;
    va_start( arglist, fmt );
    vsprintf( buf, fmt, arglist );
    va_end( arglist );

    Event * event = Event::new_Event( t, buf );

    //First add event to queue
    Event::add_Event( *event );

    //Then invoke send_Events() to send upstream
    send_Events();

    delete event;
}

} // namespace MRN
