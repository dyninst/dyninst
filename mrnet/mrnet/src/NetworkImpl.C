/***********************************************************************
 * Copyright © 2003-2004 Dorian C. Arnold, Philip C. Roth, Barton P. Miller *
 *                  Detailed MRNet usage rights in "LICENSE" file.     *
 **********************************************************************/

/*===========================================================*/
/*             Network Class DEFINITIONS                  */
/*===========================================================*/
#include <stdio.h>
#include <algorithm>
#include <errno.h>

#include "mrnet/src/NetworkImpl.h"
#include "mrnet/src/CommunicatorImpl.h"
#include "mrnet/src/utils.h"
#include "src/config.h"

extern FILE *mrnin;
extern int mrndebug;

namespace MRN
{
int mrnparse( );

NetworkGraph *NetworkImpl::parsed_graph = NULL;
bool NetworkImpl::is_backend=false;
bool NetworkImpl::is_frontend=false;
unsigned int NetworkImpl::cur_stream_idx=0;
std::map <unsigned int, Stream *> NetworkImpl::streams;

NetworkImpl::NetworkImpl( Network * _network, const char *_filename,
                          const char *_application )
    : filename( _filename ),
      application( ( _application == NULL ) ? "" : _application ),
      front_end( NULL ), back_end( NULL )
{
    // ensure our variables are set for parsing
    parsed_graph = new NetworkGraph;

    if( parse_configfile( ) == -1 ) {
        return;
    }

    graph = parsed_graph;
    if( graph->has_cycle( ) ) {
        //not really a cycle, but graph is at least not tree.
        return;
    }

    //TLS: setup thread local storage for frontend
    //I am "FE(hostname:port)"
    char port_str[128];
    sprintf( port_str, "%d", graph->get_Root( )->get_Port( ) );
    std::string name( "FE(" );
    name += graph->get_Root( )->get_HostName( );
    name += ":";
    name += port_str;
    name += ")";

    int status;
    tsd_t *local_data = new tsd_t;
    local_data->thread_id = XPlat::Thread::GetId( );
    local_data->thread_name = strdup( name.c_str( ) );
    status = tsd_key.Set( local_data );

    if( status != 0 ) {
        error( ESYSTEM, "XPlat::TLSKey::Set(): %s\n", strerror( status ) );
        return;
    }

    endpoints = graph->get_EndPoints( );
    comm_Broadcast = _network->new_Communicator( endpoints );

    //Frontend is root of the tree
    front_end = new FrontEndNode( _network, graph->get_Root( )->get_HostName( ),
                                  graph->get_Root( )->get_Port( ) );
    SerialGraph sg = graph->get_SerialGraph(  );
    sg.print(  );

    // save the serialized graph string in a variable on the stack,
    // so that we don't build a packet with a pointer into a temporary
    std::string sg_str = sg.get_ByteArray(  );
    const char* mrn_commnode_path = getenv( "MRN_COMM_PATH" );
    if( mrn_commnode_path == NULL ) {
        mrn_commnode_path = COMMNODE_EXE;
    }
    Packet packet( 0, PROT_NEW_SUBTREE, "%s%s%s", sg_str.c_str( ),
                   mrn_commnode_path, application.c_str( ) );
    if( front_end->proc_newSubTree( packet ) == -1 ) {
        _fail = true;
        return;
    }

    is_frontend = true;
    return;
}

NetworkImpl::NetworkImpl( Network *_network,
                          const char *_hostname, unsigned int port,
                          const char *_phostname, unsigned int pport,
                          unsigned int pid )
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

    tsd_t *local_data = new tsd_t;
    local_data->thread_id = XPlat::Thread::GetId(  );
    local_data->thread_name = strdup( name.c_str(  ) );
    if( ( status = tsd_key.Set( local_data ) ) != 0 ) {
        //TODO: add event to notify upstream
        error(ESYSTEM, "XPlat::TLSKey::Set(): %s\n", strerror( status ) );
        mrn_printf( 1, MCFL, stderr, "XPlat::TLSKey::Set(): %s\n",
                    strerror( status ) );
    }

    back_end = new BackEndNode( _network, host, port, phost, pport, pid );

    if( back_end->fail() ){
        error( ESYSTEM, "Failed to initialize via BackEndNode()\n" );
    }
    is_backend = true;
}

int NetworkImpl::parse_configfile(  )
{
    // mrndebug=1;
    mrnin = fopen( filename.c_str(  ), "r" );
    if( mrnin == NULL ) {
        mrn_printf( 1, MCFL, stderr, "fopen() failed: %s: %s",
                    filename.c_str(), strerror( errno ) );
        error( EBADCONFIG, "fopen() failed: %s: %s", filename.c_str(),
               strerror( errno ) );
        return -1;
    }

    if( mrnparse( ) != 0 ) {
        mrn_printf( 1, MCFL, stderr, "mrnparse() failed: %s: Parse Error",
                    filename.c_str() );
        error( EBADCONFIG, "mrnparse() failed: %s: Parse Error",
               filename.c_str() );
        return -1;
    }

    return 0;
}

NetworkImpl::~NetworkImpl(  ) {
    delete graph;
    delete front_end;
    is_frontend = false;
    is_backend = false;
}

EndPoint * NetworkImpl::get_EndPoint( const char *_hostname,
                                      unsigned short _port )
{
    unsigned int i;

    for( i = 0; i < NetworkImpl::endpoints.size(  ); i++ ) {
        if( endpoints[i]->compare( _hostname, _port ) == true ) {
            return endpoints [i];
        }
    }

    return NULL;
}

Communicator * NetworkImpl::get_BroadcastCommunicator(void)
{
    return comm_Broadcast;
}

int NetworkImpl::recv( bool blocking )
{
    if( is_FrontEnd() ) {
        mrn_printf( 3, MCFL, stderr, "In NetImpl::recv(). "
                    "Calling FrontEnd::recv()\n" );
        return front_end->recv( blocking );
    }
    else if( is_BackEnd() ){
        mrn_printf( 3, MCFL, stderr, "In NetImpl::recv(). "
                    "Calling BackEnd::recv()\n" );
        return back_end->recv( blocking );
    }
    assert(0); // shouldn't call recv when backend/front not init'd
    return 0;
}


int NetworkImpl::send( Packet& packet )
{
    if( is_FrontEnd() ) {
        return front_end->send( packet );
    }
    else if ( is_BackEnd() ) {
        return back_end->send( packet );
    }
    assert(0); // shouldn't call send when backend/front not init'd
    return 0;
}

bool NetworkImpl::is_FrontEnd( )
{
    return is_frontend;
}

bool NetworkImpl::is_BackEnd( )
{
    return is_backend;
}


static
bool
leafInfoCompare( const Network::LeafInfo * a,
                 const Network::LeafInfo * b )
{
    assert( a != NULL );
    assert( b != NULL );
    return ( a->get_Id(  ) < b->get_Id(  ) );
}


int NetworkImpl::get_LeafInfo( Network::LeafInfo *** linfo,
                               unsigned int *nLeaves )
{
    int ret = -1;

    // these should've been checked by our caller
    assert( linfo != NULL );
    assert( nLeaves != NULL );

    // request that our leaves give us their leaf info
    Packet pkt( 0, PROT_GET_LEAF_INFO, "" );
    if( front_end->proc_getLeafInfo( pkt ) != -1 ) {
        // Gather the response from the tree
        //
        // This is a little problematic because we must force the
        // front end node to handle packets, and we are in the 
        // same process as the front end node.
        //
        // The saving grace here is that we haven't returned yet to
        // our caller, so the tool hasn't yet had the opportunity to start
        // using the network.  The only packets flowing should be our own.
        // 
        Packet resp = front_end->get_leafInfoPacket(  );
        while( resp == *Packet::NullPacket ) {
            if( front_end->recv(  ) == -1 ) {
                mrn_printf( 1, MCFL, stderr,
                            "failed to receive leaf info from front end node\n" );
            }

            // now - sleep?  how do we block while still processing
            // packets?  calling receive on frontend node?
            resp = front_end->get_leafInfoPacket(  );
        }

        if( resp != *Packet::NullPacket ) {
            // we got the response successfully -
            // build the return value from the response packet
            int *ids = NULL;
            unsigned int nIds = 0;
            char **hosts = NULL;
            unsigned int nHosts = 0;
            int *ranks = NULL;
            unsigned int nRanks = 0;
            char **phosts = NULL;
            unsigned int nPHosts = 0;
            int *pports = NULL;
            unsigned int nPPorts = 0;
            int *pranks = NULL;
            unsigned int nPRanks = 0;

            int nret = resp.ExtractArgList( "%ad %as %ad %as %ad %ad",
                                            &ids, &nIds,
                                            &hosts, &nHosts,
                                            &ranks, &nRanks,
                                            &phosts, &nPHosts,
                                            &pports, &nPPorts,
                                            &pranks, &nPRanks );
            if( nret == 0 ) {
                if( ( nHosts == nRanks ) &&
                    ( nHosts == nPHosts ) &&
                    ( nHosts == nPPorts ) && ( nHosts == nPRanks ) ) {
                    // build un-ordered leaf info vector
                    std::vector < Network::LeafInfo * >linfov;
                    for( unsigned int i = 0; i < nHosts; i++ ) {
                        Network::LeafInfo * li =
                            new NetworkImpl::LeafInfoImpl( ids[i],
                                                           hosts[i],
                                                           ranks[i],
                                                           phosts[i],
                                                           pports[i],
                                                           pranks[i] );
                        linfov.push_back( li );
                    }

                    // sort the leaf info array by backend id
                    std::sort( linfov.begin(  ), linfov.end(  ),
                               leafInfoCompare );

                    // copy sorted vector to output array
                    *linfo = new Network::LeafInfo *[nHosts];
                    for( unsigned int h = 0; h < nHosts; h++ ) {
                        ( *linfo )[h] = linfov[h];
                    }
                    *nLeaves = linfov.size(  );

                    ret = 0;
                }
                else {
                    mrn_printf( 1, MCFL, stderr,
                                "leaf info packet corrupt: array size mismatch\n" );
                }
            }
            else {
                mrn_printf( 1, MCFL, stderr,
                            "failed to extract arrays from leaf info packet\n" );
            }
        }
    }
    else {
        // we failed to deliver the request 
        mrn_printf( 1, MCFL, stderr, "failed to deliver request\n" );
    }

    return ret;
}



int NetworkImpl::connect_Backends( void )
{
    int ret = 0;

    // broadcast message to all leaves that they 
    // should accept their connections
    Packet pkt( 0, PROT_CONNECT_LEAVES, "" );
    if( front_end->proc_connectLeaves( pkt ) != -1 ) {
#if READY
        // in an ideal world, we don't have to get a response to this
#else
        // wait for response (?)
        Packet resp = front_end->get_leavesConnectedPacket(  );
        while( resp == *Packet::NullPacket ) {
            // allow the front end node to handle packets
            front_end->recv(  );

            // see if we got our response packet
            resp = front_end->get_leavesConnectedPacket(  );
        }

        if( resp != *Packet::NullPacket ) {
            // verify response based on our notion of the 
            // number of leaves we have
            unsigned int nBackendsExpected = endpoints.size(  );
            unsigned int nBackends;
            int nret = resp.ExtractArgList( "%ud", &nBackends );
            if( nret == 0 ) {
                if( nBackends != nBackendsExpected ) {
                    mrn_printf( 1, MCFL, stderr,
                                "unexpected backend count %u (expecting %u)\n",
                                nBackends, nBackendsExpected );
                }
            }
            else {
                mrn_printf( 1, MCFL, stderr,
                            "format mismatch in leaf connection packet\n" );
            }
        }
        else {
            mrn_printf( 1, MCFL, stderr,
                        "failed to receive leaf connection packet\n" );
            ret = -1;
        }
#endif // READY
    }
    else {
        // we failed to deliver the request
        mrn_printf( 1, MCFL, stderr, "failed to deliver request\n" );
        ret = -1;
    }

    return ret;
}

int NetworkImpl::recv(int *tag, void **ptr, Stream **stream, bool blocking)
{
    bool checked_network = false;   // have we checked sockets for input?
    Packet cur_packet;

    mrn_printf(3, MCFL, stderr, "In StreamImpl::recv().\n");

    if( streams.empty() && is_FrontEnd() ){
      //No streams exist -- bad for FE
      mrn_printf(1, MCFL, stderr, "%s recv in FE when no streams "
         "exist\n", (blocking? "Blocking" : "Non-blocking") );
      return -1;
    }

    // check streams for input
get_packet_from_stream_label:
    if( !streams.empty() ) {
        unsigned int start_idx = cur_stream_idx;
        do{
            Stream* cur_stream = streams[cur_stream_idx];
            if(!cur_stream){
                //TODO: on failure, doesn't map allocate a new entry?
                cur_stream_idx++;
                cur_stream_idx %= streams.size();
                continue;
            }

            cur_packet = cur_stream->get_IncomingPacket();
            if( cur_packet != *Packet::NullPacket ){
                mrn_printf( 3, MCFL, stderr,
                            "Found a packet on stream[%d] %p ...\n",
                            cur_stream_idx, cur_stream );
                cur_stream_idx++;
                cur_stream_idx %= streams.size();
                break;
            }

            

            cur_stream_idx++;
            cur_stream_idx %= streams.size();
        } while(start_idx != cur_stream_idx);
    }

    if( cur_packet != *Packet::NullPacket ) {
        *tag = cur_packet.get_Tag();
        *stream = streams[cur_packet.get_StreamId()];
        mrn_printf( 3, MCFL, stderr, "DCA: Setting returned stream[%d] to %p. Packet = %p\n",
                    cur_packet.get_StreamId(), *stream, &cur_packet );
        *ptr = (void *) new Packet(cur_packet);
        mrn_printf(4, MCFL, stderr, "cur_packet tag: %d, fmt: %s\n",
                   cur_packet.get_Tag(), cur_packet.get_FormatString() );
        return 1;
    }
    else if( blocking || !checked_network ) {

        // No packets are already in the stream
        // check whether there is data waiting to be read on our sockets
        if( recv( blocking ) == -1 ){
            mrn_printf( 1, MCFL, stderr, "Network::recv() failed.\n" );
            return -1;
        }
        checked_network = true;

        // go back to check whether we found enough to make a complete packet
        goto get_packet_from_stream_label;
    }

    assert( !blocking );
    assert( checked_network );
    return 0;
}


NetworkImpl::LeafInfoImpl::LeafInfoImpl( unsigned short _id, const char* _host,
                                         unsigned short _rank,
                                         const char* _phost,
                                         unsigned short _pport,
                                         unsigned short _prank )
    : host( new char[strlen(_host)+1] ),
    id( _id ),
    rank( _rank ),
    phost( new char[strlen(_phost)+1] ),
    pport( _pport ),
    prank( _prank )
{
    strcpy( host, _host );
    strcpy( phost, _phost );
}

NetworkImpl::LeafInfoImpl::~LeafInfoImpl( void )
{
    delete[] host;
    delete[] phost;
}

int NetworkImpl::getConnections( int** conns, unsigned int* nConns )
{
    int ret = 0;

    if( is_FrontEnd()  ) {
        ret = front_end->getConnections( conns, nConns );
    }
    else if( is_BackEnd() ) {
        ret = back_end->getConnections( conns, nConns );
        assert( ( ret != 0 ) || ( *nConns == 1 ) );
    }
    else{
        assert( 0 ); //either front_end or back_end should be init'd
    }

    return ret;
}

}                               // namespace MRN
