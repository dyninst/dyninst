#include "mrnet/src/Types.h"


#include <fstream>
#include <vector>
#include <algorithm>
#include <sys/types.h>
#include <ctype.h>
#include <limits.h>
#include <stdarg.h>
#include <stdio.h>
#include "mrnet/h/MRNet.h"
#include "mrnet/src/utils.h"
#include "src/config.h"
#include "mrnet/src/NetworkImpl.h"
#include "mrnet/src/BackEndNode.h"
#include "mrnet/src/StreamImpl.h"
#include "mrnet/src/CommunicatorImpl.h"
#include "mrnet/src/EndPointImpl.h"

#include "mrnet/h/MRNetC.h"

namespace MRN
{

NetworkImpl *Network::network = NULL;
BackEndNode *Network::back_end = NULL;

/*===========================================================*/
/*             Network static function DEFINITIONS        */
/*===========================================================*/
int Network::new_Network( const char *_filename, const char *_application )
{
    Network::network = new NetworkImpl( _filename, _application );

    if( Network::network->fail( ) ){
        return -1;
    }

    StreamImpl::streams = new std::map < unsigned int, StreamImpl * >;
    StreamImpl::set_ForceNetworkRecv(  );
    return 0;
}

int Network::new_NetworkNoBE( const char *cfgFileName,
                              Network::LeafInfo *** leafInfo,
                              unsigned int *nLeaves )
{
    int ret = -1;

    // build the network
    Network::network = new NetworkImpl( cfgFileName, NULL );
    if( !Network::network->fail(  ) ) {
        if( ( leafInfo != NULL ) && ( nLeaves != NULL ) ) {
            Network::network->get_LeafInfo( leafInfo, nLeaves );
            ret = 0;
        }
        else {
            // TODO is this the right error?
            ret = MRN_ENETWORK_FAILURE;
        }
    }

    return ret;
}

int Network::connect_Backends( void )
{
    // TODO is this the right error?
    int ret = MRN_ENETWORK_FAILURE;

    if( Network::network != NULL ) {
        ret = Network::network->connect_Backends(  );
    }
    return ret;
}

void Network::delete_Network(  )
{
    delete Network::network;
}

int Network::getConnections( int **conns, unsigned int *nConns )
{
    int ret = 0;

    if( Network::network != NULL ) {
        ret = Network::network->getConnections( conns, nConns );
    }
    else {
        ret = Network::back_end->getConnections( conns, nConns );
        assert( ( ret != 0 ) || ( *nConns == 1 ) );
    }

    return ret;
}

int Network::init_Backend( const char *_hostname, const char *_port,
                           const char *_phostname,
                           const char *_pport, const char *_pid )
{
    unsigned int port( atoi( _port ) );
    unsigned int pport( atoi( _pport ) );
    unsigned int pid( atoi( _pid ) );

    return init_Backend( _hostname, port, _phostname, pport, pid );
}

int Network::init_Backend( const char *_hostname, unsigned int port,
                           const char *_phostname,
                           unsigned int pport, unsigned int pid )
{
    std::string host( _hostname );
    std::string phost( _phostname );

    //TLS: setup thread local storage for frontend
    //I am "BE(host:port)"
    std::string prettyHost;
    getHostName( prettyHost, host );
    char port_str[16];
    sprintf( port_str, "%u", port );
    std::string name( "BE(" );
    name += prettyHost;
    name += ":";
    name += port_str;
    name += ")";

    int status;
    if( ( status = pthread_key_create( &tsd_key, NULL ) ) != 0 ) {
        mrn_printf( 1, MCFL, stderr, "pthread_key_create(): %s\n",
                    strerror( status ) );
        exit( -1 );
    }
    tsd_t *local_data = new tsd_t;
    local_data->thread_id = pthread_self(  );
    local_data->thread_name = strdup( name.c_str(  ) );
    if( ( status = pthread_setspecific( tsd_key, local_data ) ) != 0 ) {
        mrn_printf( 1, MCFL, stderr, "pthread_key_create(): %s\n",
                    strerror( status ) );
        exit( -1 );
    }

    StreamImpl::streams = new std::map < unsigned int, StreamImpl * >;
    Network::back_end = new BackEndNode( host, port, phost, pport, pid );

    if( Network::back_end->fail(  ) ) {
        return -1;
    }

    return 0;
}

void Network::error_str( const char *s )
{
    Network::network->perror( s );
}

/*================================================*/
/*             Stream class DEFINITIONS        */
/*================================================*/
Stream *Stream::new_Stream( Communicator * comm, int us_filter_id,
                            int sync_id, int ds_filter_id )
{
    return new StreamImpl( comm, sync_id, ds_filter_id, us_filter_id );
}

int Stream::recv( int *tag, void **buf, Stream ** stream, bool blocking )
{
    return StreamImpl::recv( tag, buf, stream, blocking );
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

int Stream::load_FilterFunc( const char *so_file, const char *func,
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
    Packet *packet = new Packet( 0, MRN_NEW_FILTER_PROT, "%uhd %s %s %c",
                                 fid, so_file, func, is_trans_filter );
    Network::network->front_end->send_PacketDownStream( packet, true );

    return fid;
}

Stream*
Stream::get_Stream( unsigned int id )
{
    return StreamImpl::get_Stream( id );
}


/*======================================================*/
/*             Communicator class DEFINITIONS        */
/*======================================================*/
Communicator *Communicator::new_Communicator(  )
{
    return new CommunicatorImpl;
}

Communicator *Communicator::get_BroadcastCommunicator(  )
{
    return CommunicatorImpl::get_BroadcastCommunicator(  );
}

/*==================================================*/
/*             EndPoint class DEFINITIONS        */
/*==================================================*/
EndPoint *EndPoint::new_EndPoint( int _id, const char *_hostname,
                                  unsigned short _port )
{
    return new EndPointImpl( _id, _hostname, _port );
}

}  // namespace MRN
