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

const int MIN_OUTPUT_LEVEL=0;
const int MAX_OUTPUT_LEVEL=5;
int CUR_OUTPUT_LEVEL=1;


const Port UnknownPort = (Port)-1;
const Rank UnknownRank = (Rank)-1;

void set_OutputLevel(int l){
    if(l <=MIN_OUTPUT_LEVEL){
        CUR_OUTPUT_LEVEL = MIN_OUTPUT_LEVEL;
    }
    else if(l >= MAX_OUTPUT_LEVEL){
        CUR_OUTPUT_LEVEL = MAX_OUTPUT_LEVEL;
    }
    else{
        CUR_OUTPUT_LEVEL = l;
    }
}

/*===========================================================*/
/*             Network static function DEFINITIONS        */
/*===========================================================*/
Network::Network( const char *ifilename, const char *iapplication,
                  const char **iargv )
{
    _network_impl = new NetworkImpl( this, ifilename, iapplication, iargv );
}

Network::Network( const char *cfgFileName, Network::LeafInfo *** leafInfo,
                  unsigned int *nLeaves )
{
    // build the network

    _network_impl = new NetworkImpl( this, cfgFileName, (const char*)NULL,
                               (const char**)NULL);
    if( !_network_impl->fail( ) ) {
        if( (leafInfo != NULL) && (nLeaves != NULL) )
        {
            _network_impl->get_LeafInfo( leafInfo, nLeaves );
        }
        else
        {
            // indicate the error
            _network_impl->error( MRN_EINTERNAL, "invalid argument" );
        }
    }
}

Network::Network( const char* _configBuffer, 
                    bool /* unused */,
                    const char* _application, const char **argv )
{
    _network_impl = new NetworkImpl( this, _configBuffer, true, _application, argv );
}

Network::Network( const char* _configBuffer, bool /* unused */,
                    Network::LeafInfo *** leafInfo, unsigned int *nLeaves )
{
    // build the network
    _network_impl = new NetworkImpl( this, _configBuffer, true, NULL, NULL );
    if( !_network_impl->fail( ) ) {
        if( ( leafInfo != NULL ) && ( nLeaves != NULL ) ) {
            _network_impl->get_LeafInfo( leafInfo, nLeaves );
        }
        else {
            // TODO is this the right error?
            _network_impl->error( MRN_EINTERNAL, "invalid argument" );
        }
    }
}


Network::~Network(  )
{
    delete _network_impl;
}

int Network::connect_Backends( void )
{
    // TODO is this the right error?
    int ret = MRN_ENETWORK_FAILURE;

    if( _network_impl != NULL ) {
        ret = _network_impl->connect_Backends(  );
    }
    return ret;
}

int Network::getConnections( int **conns, unsigned int *nConns )
{
    return _network_impl->getConnections( conns, nConns );
}


// back-end constructors
Network::Network( const char *_phostname, Port pport, Rank myrank )
{
    _network_impl = new NetworkImpl( this, _phostname, pport, myrank );
}

void Network::print_error( const char *s )
{
    assert(_network_impl);
    _network_impl->perror( s );
}

int Network::recv(bool iblocking)
{
    assert( _network_impl );

    int ret = _network_impl->recv( iblocking );

    return ret;
}

int Network::recv(int *otag, Packet **opacket, Stream ** ostream,
                  bool iblocking)
{
    assert( _network_impl );
    mrn_dbg(2, mrn_printf(FLF, stderr, "Call to MRN::recv(&tag, &buf, &stream, %s)\n",
               ( iblocking ? "blocking" : "non-blocking") ));
    int ret = _network_impl->recv( otag, opacket, ostream, iblocking );
    mrn_dbg(2, mrn_printf(FLF, stderr, "MRN::recv(&tag, &buf, &stream) => %d %d\n",
               *otag, ret ));

    return ret;
}

EndPoint * Network::get_EndPoint(const char* hostname,
                                 short unsigned int port )
{
    assert(_network_impl);

    mrn_dbg(2, mrn_printf(FLF, stderr, "Call to MRN::get_EndPoint(%s, %h)\n",
               hostname, port));
    EndPoint * ret = _network_impl->get_EndPoint( hostname, port );
    mrn_dbg(2, mrn_printf(FLF, stderr, "MRN::get_EndPoint(%s, %h) => %p\n",
               hostname, port, ret));

    return ret;
}

Communicator * Network::get_BroadcastCommunicator(void)
{
    assert(_network_impl);
    return _network_impl->get_BroadcastCommunicator();
}

Communicator * Network::new_Communicator()
{
    return new Communicator(this);
}

Communicator * Network::new_Communicator( Communicator& comm )
{
    assert(_network_impl);
    return new Communicator( this, comm );
}

Communicator * Network::new_Communicator( std::vector <EndPoint *> & endpoints )
{
    // TODO: technically, assert is correct, but commented as temp. fix for
    // fact that in the calling sequence Network() calls NetworkImpl() calls
    // new_Communicator() Network, object has not yet set network var

    // assert(_network_impl);
    return new Communicator( this, endpoints );
}

EndPoint * Network::new_EndPoint(Rank rank, const char * hostname, Port port)
{
    mrn_dbg(2, mrn_printf(FLF, stderr, "Call to MRN::new_EndPoint(%d, %s, %d)\n",
               rank, hostname, port));
    EndPoint * ret = new EndPoint(rank, hostname, port);
    mrn_dbg(2, mrn_printf(FLF, stderr, "MRN::new_EndPoint(%d, %s, %d) => %p\n",
               rank, hostname, port, ret));

    return ret;
}

Stream * Network::new_Stream( Communicator *comm, 
                                int us_filter_id,
                                int sync_id, 
                                int ds_filter_id)
{
    assert(_network_impl);

    mrn_dbg(2, mrn_printf(FLF, stderr, "Call to MRN::new_Stream(%p, %d, %d, %d)\n",
               comm, us_filter_id, sync_id, ds_filter_id));

    Stream * new_stream = new Stream( this, comm, us_filter_id,
                                      sync_id, ds_filter_id );
    _network_impl->set_StreamById( new_stream->get_Id(), new_stream );

    mrn_dbg(2, mrn_printf(FLF, stderr, "MRN::new_Stream(%p, %d, %d, %d) => %p (id:%d)\n",
               comm, us_filter_id, sync_id, ds_filter_id,
               new_stream, new_stream->get_Id() ));

    return new_stream;
}

Stream * Network::new_Stream( int stream_id, int *backends,
                              int num_backends,
                              int us_filter_id,
                              int sync_id,
                              int ds_filter_id)
{
    assert(_network_impl);
    mrn_dbg(2, mrn_printf(FLF, stderr, "Call to MRN::new_Stream(id: %d, %d, %d, %d)\n",
               stream_id, us_filter_id, sync_id, ds_filter_id));

    Stream * new_stream = new Stream( this, stream_id, backends,
                                      num_backends, us_filter_id,
                                      sync_id, ds_filter_id );

    _network_impl->set_StreamById( new_stream->get_Id(), new_stream );

    mrn_dbg(2, mrn_printf(FLF, stderr, "MRN::new_Stream(id: %d, %d, %d, %d) => %p (id:%d)\n",
               stream_id, us_filter_id, sync_id, ds_filter_id,
               new_stream, new_stream->get_Id() ));

    return new_stream;
}

Stream* Network::get_Stream(int stream_id)
{
    mrn_dbg(2, mrn_printf(FLF, stderr, "Call to MRN::get_Stream(%d)\n", stream_id));

    Stream * stream = _network_impl->get_StreamById( stream_id );

    mrn_dbg(2, mrn_printf(FLF, stderr, "MRN::get_Stream(%d) => %p\n",
               stream_id, stream));

    return stream;
}

int Network::get_SocketFd(){
    assert(_network_impl);

    //mrn_dbg(2, mrn_printf(FLF, stderr, "Call to MRN::get_SocketFd()\n"));
    int ret = _network_impl->get_SocketFd();
    //mrn_dbg(2, mrn_printf(FLF, stderr, "Call to MRN::get_SocketFd() => %d\n", ret));

    return ret;
}

int Network::get_SocketFd(int **array, unsigned int *array_size){
    assert(_network_impl);

    //mrn_dbg(2, mrn_printf(FLF, stderr, "Call to MRN::get_SocketFd()\n"));
    int ret = _network_impl->get_SocketFd(array, array_size);
    //mrn_dbg(2, mrn_printf(FLF, stderr, "Call to MRN::get_SocketFd() => %d\n", ret));

    return ret;
}

int Network::load_FilterFunc( const char *so_file, const char *func,
                             bool is_trans_filter )
{
    mrn_dbg(2, mrn_printf(FLF, stderr, "Call to MRN::load_FilterFunc(%s, %s, %s)\n",
               so_file, func, ( is_trans_filter ? "tfilter" : "sfilter" )));

    int fid = Filter::load_FilterFunc( so_file, func,
                                       is_trans_filter );
    
    if( fid != -1 ){
        //Filter registered locally, now propagate to tree
        //TODO: ensure that filter is loaded down the entire tree
        Packet packet( 0, PROT_NEW_FILTER, "%uhd %s %s %c",
                       fid, so_file, func, is_trans_filter );
        _network_impl->front_end->send_PacketDownStream( packet, true );
    }

    mrn_dbg(2, mrn_printf(FLF, stderr, "MRN::load_FilterFunc(%s, %s, %s) => %d\n",
               so_file, func, ( is_trans_filter ? "tfilter" : "sfilter" ),
               fid));
    return fid;
}

bool Network::is_FrontEnd()
{
    assert(_network_impl);
    return _network_impl->is_FrontEnd();
}

bool Network::is_BackEnd()
{
    assert(_network_impl);
    return _network_impl->is_BackEnd();
}

bool Network::good()
{
    assert(_network_impl);
    return _network_impl->good();
}

bool Network::fail()
{
    assert(_network_impl);
    return _network_impl->fail();
}

FrontEndNode * Network::get_FrontEndNode( void )
{
    assert( _network_impl );
    return _network_impl->get_FrontEndNode();
}

BackEndNode * Network::get_BackEndNode( void )
{
    assert( _network_impl );
    return _network_impl->get_BackEndNode();
}

/*================================================*/
/*             Stream class DEFINITIONS        */
/*================================================*/
Stream::Stream(Network * inetwork,
               Communicator *icomm,
               int iupstream_filter_id,
               int isync_filter_id,
               int idownstream_filter_id)
    : _stream_impl( new StreamImpl( inetwork, icomm, iupstream_filter_id,
                              isync_filter_id, idownstream_filter_id) )
{
}

Stream::Stream( Network *inetwork,
                int istream_id,
                int *ibackends,
                int inum_backends,
                int iupstream_filter_id,
                int isync_filter_id ,
                int idownstream_filter_id)
    : _stream_impl( new StreamImpl( inetwork, istream_id, ibackends,
                                    inum_backends, iupstream_filter_id,
                                    isync_filter_id, idownstream_filter_id ) )
{
}

Stream::~Stream()
{
    delete _stream_impl;
}

int Stream::unpack( Packet *ipacket, char const *ifmt_str, ... )
{
    va_list arg_list;

    mrn_dbg(2, mrn_printf(FLF, stderr, "Call to Stream::unpack(%p, \"%s\")\n",
               ipacket, ifmt_str));

    va_start( arg_list, ifmt_str );
    int ret = StreamImpl::unpack( ipacket, ifmt_str, arg_list );
    va_end( arg_list );

    mrn_dbg(2, mrn_printf(FLF, stderr, "Stream::unpack(%p, \"%s\") => %d\n",
               ipacket, ifmt_str, ret));

    return ret;
}

void Stream::set_BlockingTimeOut( int timeout )
{
    mrn_dbg(2, mrn_printf(FLF, stderr, "Stream::set_BlockingTimeOut(%d)\n", timeout ));

    RemoteNode::set_BlockingTimeOut( timeout );
}

int Stream::get_BlockingTimeOut(  )
{
    return RemoteNode::get_BlockingTimeOut(  );
}

int Stream::send(int tag, const char * format_str, ...)const
{
    assert( _stream_impl );

    mrn_dbg(2, mrn_printf(FLF, stderr, "In Stream[%d]::send(%d, \"%s\")\n",
               _stream_impl->get_Id(), tag, format_str));

    int status;
    va_list arg_list;

    va_start(arg_list, format_str);
    status = _stream_impl->send_aux( tag, format_str, arg_list );
    va_end(arg_list);

    mrn_dbg(2, mrn_printf(FLF, stderr, "Stream[%d]::send(%d, \"%s\") => %d\n",
               _stream_impl->get_Id(), tag, format_str, status));

    return status;
}

int Stream::flush()const
{
    mrn_dbg(2, mrn_printf(FLF, stderr, "Call to Stream::flush\n"));
    assert( _stream_impl );

    return _stream_impl->flush();
}

int Stream::recv( int *otag, Packet **opacket, bool iblocking )
{
    mrn_dbg(2, mrn_printf(FLF, stderr, "Call to Stream::recv(&tag, &buf, %s)\n",
               ( iblocking ? "blocking": "non-blocking" ) ));
    assert( _stream_impl );
    int ret = _stream_impl->recv( otag, opacket, iblocking );

    mrn_dbg(2, mrn_printf(FLF, stderr, "Stream::recv(&tag, &buf, %s) => %d, %p, %d\n",
               ( iblocking ? "blocking": "non-blocking" ), *otag, *opacket, ret ));
    return ret;
}

unsigned int Stream::get_NumEndPoints()const
{
    assert( _stream_impl );
    return _stream_impl->get_NumEndPoints();
}

Communicator* Stream::get_Communicator()const
{
    assert ( _stream_impl );
    return _stream_impl->get_Communicator();
}

unsigned int Stream::get_Id()const
{
    assert( _stream_impl );
    return _stream_impl->get_Id();
}

/*======================================================*/
/*             Communicator class DEFINITIONS        */
/*======================================================*/
Communicator::Communicator( Network * inetwork )
    : _communicator_impl( new CommunicatorImpl( inetwork ) )
{
}

Communicator::Communicator( Network * inetwork, Communicator & icomm )
    :_communicator_impl( new CommunicatorImpl( inetwork, icomm ) )
{
}

Communicator::Communicator( Network * inetwork,
                            std::vector<EndPoint *>& iendpoints )
    :_communicator_impl( new CommunicatorImpl( inetwork, iendpoints ) )
{
}

Communicator::~Communicator( )
{
    assert(_communicator_impl);
    delete _communicator_impl;
}

int Communicator::add_EndPoint(const char * ihostname, Port iport)
{
    assert(_communicator_impl);
    return _communicator_impl->add_EndPoint( ihostname, iport );
}

void Communicator::add_EndPoint(EndPoint *iendpoint )
{
    assert(_communicator_impl);
    _communicator_impl->add_EndPoint( iendpoint );
}

unsigned int Communicator::size( void ) const
{
    assert(_communicator_impl);
    return _communicator_impl->size( );
}

const char * Communicator::get_HostName( int iidx ) const
{
    assert(_communicator_impl);
    return _communicator_impl->get_HostName( iidx );
}

Port Communicator::get_Port( int iidx ) const
{
    assert(_communicator_impl);
    return _communicator_impl->get_Port( iidx );
}

Rank Communicator::get_Rank( int iidx ) const
{
    assert(_communicator_impl);
    return _communicator_impl->get_Rank( iidx );
}

const std::vector<EndPoint *> & Communicator::get_EndPoints( void ) const
{
    assert(_communicator_impl);
    return _communicator_impl->get_EndPoints( );
}

/*============================================*/
/*             EndPoint class DEFINITIONS        */
/*============================================*/
EndPoint::EndPoint(Rank irank, const char * ihostname, Port iport)
    : _endpoint_impl(new EndPointImpl( irank, ihostname, iport ) )
{
}

EndPoint::~EndPoint()
{
    assert(_endpoint_impl);
    delete _endpoint_impl;
}

bool EndPoint::compare(const char * ihostname, Port iport)const
{
    assert(_endpoint_impl);
    return _endpoint_impl->compare( ihostname, iport );
}

const char * EndPoint::get_HostName()const
{
    assert(_endpoint_impl);
    return _endpoint_impl->get_HostName();
}

Port EndPoint::get_Port()const
{
    assert(_endpoint_impl);
    return _endpoint_impl->get_Port();
}

Rank EndPoint::get_Rank()const
{
    assert(_endpoint_impl);
    return _endpoint_impl->get_Rank();
}


/*============================================*/
/*             Event class DEFINITIONS        */
/*============================================*/
Event * Event::new_Event( EventType t, std::string desc,
                          std::string h, Port p){
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
