/***********************************************************************
 * Copyright © 2003-2004 Dorian C. Arnold, Philip C. Roth, Barton P. Miller *
 *                  Detailed MRNet usage rights in "LICENSE" file.     *
 **********************************************************************/

#include <fstream>
#include <vector>
#include <algorithm>
#include <sys/types.h>
#include <ctype.h>
#include <limits.h>
#include <stdarg.h>
#include <stdio.h>

#include "mrnet/MRNet.h"
#include "mrnet/src/Types.h"
#include "mrnet/src/utils.h"
#include "src/config.h"
#include "mrnet/src/NetworkImpl.h"
#include "mrnet/src/BackEndNode.h"
#include "mrnet/src/StreamImpl.h"
#include "mrnet/src/CommunicatorImpl.h"
#include "mrnet/src/EndPointImpl.h"
#include "mrnet/src/EventImpl.h"

namespace MRN
{

/*===========================================================*/
/*             Network static function DEFINITIONS        */
/*===========================================================*/
Network::Network( const char *_filename, const char *_application )
{
    network = new NetworkImpl( this, _filename, _application );
}

Network::Network( const char *cfgFileName, Network::LeafInfo *** leafInfo,
                  unsigned int *nLeaves )
{
    // build the network
    network = new NetworkImpl( this, cfgFileName, NULL );
    if( !network->fail( ) ) {
        if( ( leafInfo != NULL ) && ( nLeaves != NULL ) ) {
            network->get_LeafInfo( leafInfo, nLeaves );
        }
        else {
            // TODO is this the right error?
        }
    }
}

Network::~Network(  )
{
    delete network;
}

int Network::connect_Backends( void )
{
    // TODO is this the right error?
    int ret = MRN_ENETWORK_FAILURE;

    if( network != NULL ) {
        ret = network->connect_Backends(  );
    }
    return ret;
}

int Network::getConnections( int **conns, unsigned int *nConns )
{
    return network->getConnections( conns, nConns );
}

Network::Network( const char *_hostname, const char *_port,
                  const char *_phostname,
                  const char *_pport, const char *_pid )
{
    unsigned int port( atoi( _port ) );
    unsigned int pport( atoi( _pport ) );
    unsigned int pid( atoi( _pid ) );

    network = new NetworkImpl( this, _hostname, port, _phostname, pport, pid );
}

Network::Network( const char *_hostname, unsigned int port,
                      const char *_phostname,
                      unsigned int pport, unsigned int pid )
{
    network = new NetworkImpl( this, _hostname, port, _phostname, pport, pid );
}

void Network::error_str( const char *s )
{
    assert(network);
    network->perror( s );
}


int Network::recv(bool blocking)
{
    assert( network );
    return network->recv( blocking );
}

int Network::recv(int *tag, void **buf, Stream ** stream, bool blocking)
{
    assert( network );
    return network->recv( tag, buf, stream, blocking );
}

EndPoint * Network::get_EndPoint(const char* hostname,
                                 short unsigned int port )
{
    assert(network);
    return network->get_EndPoint( hostname, port );
}

Communicator * Network::get_BroadcastCommunicator(void)
{
    assert(network);
    return network->get_BroadcastCommunicator();
}

Communicator * Network::new_Communicator()
{
    return new Communicator(this);
}

Communicator * Network::new_Communicator( Communicator& comm )
{
    assert(network);
    return new Communicator( this, comm );
}

Communicator * Network::new_Communicator( std::vector <EndPoint *> & endpoints )
{
    // TODO: technically, assert is correct, but commented as temp. fix for
    // fact that in the calling sequence Network() calls NetworkImpl() calls
    // new_Communicator() Network, object has not yet set network var

    // assert(network);
    return new Communicator( this, endpoints );
}

EndPoint * Network::new_EndPoint(int id, const char * hostname,
                                 unsigned short port)
{
    return new EndPoint(id, hostname, port);
}

Stream * Network::new_Stream( Communicator *comm, int ds_filter_id,
                              int sync_id, int us_filter_id)
{
    assert(network);
    Stream * new_stream = new Stream( this, comm, us_filter_id,
                                      sync_id, ds_filter_id );

    network->streams[ new_stream->get_Id() ] = new_stream;
    mrn_printf(3, MCFL, stderr, "DCA new_stream() created stream %d (%p)\n", new_stream->get_Id(), new_stream);
    return new_stream;
}

Stream * Network::new_Stream( int stream_id, int *backends,
                              int num_backends, int us_filter_id,
                              int sync_id, int ds_filter_id)
{
    assert(network);
    Stream * new_stream = new Stream( this, stream_id, backends,
                                      num_backends, us_filter_id,
                                      sync_id, ds_filter_id );

    network->streams[ new_stream->get_Id() ] = new_stream;
    //next_stream_id++;
    return new_stream;
}

Stream* Network::get_Stream(int stream_id)
{
    Stream *stream = network->streams[stream_id];
    if(stream){
        return stream;
    }
    else{
        network->streams.erase(stream_id);
        return NULL;
    }
}

int Network::load_FilterFunc( const char *so_file, const char *func,
                             bool is_trans_filter )
{
    int fid = Filter::load_FilterFunc( so_file, func,
                                       is_trans_filter );
    
    if( fid == -1 ) {
        mrn_printf( 1, MCFL, stderr,
                    "Filter::load_FilterFunc() failed.\n" );
        return -1;
    }

    //Filter registered locally, now propagate to tree
    Packet packet( 0, PROT_NEW_FILTER, "%uhd %s %s %c",
                   fid, so_file, func, is_trans_filter );
    network->front_end->send_PacketDownStream( packet, true );

    return fid;
}

bool Network::is_FrontEnd()
{
    assert(network);
    return network->is_FrontEnd();
}

bool Network::is_BackEnd()
{
    assert(network);
    return network->is_BackEnd();
}

bool Network::good()
{
    assert(network);
    return network->good();
}

bool Network::fail()
{
    assert(network);
    return network->fail();
}

FrontEndNode * Network::get_FrontEndNode( void )
{
    assert( network );
    return network->get_FrontEndNode();
}

BackEndNode * Network::get_BackEndNode( void )
{
    assert( network );
    return network->get_BackEndNode();
}

/*================================================*/
/*             Stream class DEFINITIONS        */
/*================================================*/
Stream::Stream(Network *network, Communicator *comm, int us_filter_id,
       int sync_id, int ds_filter_id)
    : stream( new StreamImpl( network, comm, us_filter_id, sync_id,
                               ds_filter_id ) )
{
    mrn_printf(3, MCFL, stderr, "DCA Stream::Stream() has stream %p has streamimp %p\n", this, stream);
}

Stream::Stream( Network *network, int stream_id, int *backends,
                int num_backends, int ds_filter_id,
                int sync_id , int us_filter_id)
    : stream( new StreamImpl( network, stream_id, backends, num_backends,
                              ds_filter_id, sync_id, us_filter_id ) )
{
    mrn_printf(3, MCFL, stderr, "DCA Stream::Stream() has stream %p has streamimp %p\n", this, stream);
}

int Stream::unpack( void *buf, char const *fmt_str, ... )
{
    va_list arg_list;

    va_start( arg_list, fmt_str );
    int ret = StreamImpl::unpack( buf, fmt_str, arg_list );
    va_end( arg_list );
    return ret;
}

void Stream::set_BlockingTimeOut( int timeout )
{
    RemoteNode::set_BlockingTimeOut( timeout );
}

int Stream::get_BlockingTimeOut(  )
{
    return RemoteNode::get_BlockingTimeOut(  );
}

int Stream::send(int tag, const char * format_str, ...)const
{
    mrn_printf(3, MCFL, stderr, "DCA stream::recv() has stream %p streamimpl\n", this, stream);
    assert( stream );

    int status;
    va_list arg_list;

    va_start(arg_list, format_str);
    status = stream->send_aux( tag, format_str, arg_list );
    va_end(arg_list);

    return status;
}

int Stream::flush()const
{
    assert( stream );
    return stream->flush();
}

int Stream::recv( int *tag, void **buf, bool blocking )
{
    mrn_printf(3, MCFL, stderr, "DCA stream::recv() has stream %p streamimpl\n", this, stream);
    assert( stream );
    return stream->recv( tag, buf, blocking );
}

unsigned int Stream::get_NumEndPoints()const
{
    assert( stream );
    return stream->get_NumEndPoints();
}

unsigned int Stream::get_Id()const
{
    assert( stream );
    return stream->get_Id();
}

/*======================================================*/
/*             Communicator class DEFINITIONS        */
/*======================================================*/
Communicator::Communicator( Network * network )
    : communicator( new CommunicatorImpl( network ) )
{
}

Communicator::Communicator( Network * network, Communicator & comm )
    :communicator( new CommunicatorImpl( network, comm ) )
{
}

Communicator::Communicator( Network * network,
                            std::vector<EndPoint *>& endpoints )
    :communicator( new CommunicatorImpl( network, endpoints ) )
{
}

int Communicator::add_EndPoint(const char * hostname, unsigned short port)
{
    assert(communicator);
    return communicator->add_EndPoint( hostname, port );
}

void Communicator::add_EndPoint(EndPoint *endpoint )
{
    assert(communicator);
    communicator->add_EndPoint( endpoint );
}

unsigned int Communicator::size( void ) const
{
    assert(communicator);
    return communicator->size( );
}

const char * Communicator::get_HostName( int i ) const
{
    assert(communicator);
    return communicator->get_HostName( i );
}

unsigned short Communicator::get_Port( int i ) const
{
    assert(communicator);
    return communicator->get_Port( i );
}

unsigned int Communicator::get_Id( int i ) const
{
    assert(communicator);
    return communicator->get_Id( i );
}

const std::vector<EndPoint *> & Communicator::get_EndPoints( void ) const
{
    assert(communicator);
    return communicator->get_EndPoints( );
}

/*============================================*/
/*             EndPoint class DEFINITIONS        */
/*============================================*/
EndPoint::EndPoint(int id, const char * hostname, unsigned short port)
    : endpoint(new EndPointImpl( id, hostname, port ) )
{
}

EndPoint::~EndPoint()
{
    assert(endpoint);
    delete endpoint;
}

bool EndPoint::compare(const char * hostname, unsigned short port)const
{
    assert(endpoint);
    return endpoint->compare( hostname, port );
}

const char * EndPoint::get_HostName()const
{
    assert(endpoint);
    return endpoint->get_HostName();
}

unsigned short EndPoint::get_Port()const
{
    assert(endpoint);
    return endpoint->get_Port();
}

unsigned int EndPoint::get_Id()const
{
    assert(endpoint);
    return endpoint->get_Id();
}


/*============================================*/
/*             Event class DEFINITIONS        */
/*============================================*/
Event * Event::new_Event( EventType t, std::string desc,
                          std::string h, unsigned short p){
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

}  // namespace MRN
