/****************************************************************************
 * Copyright © 2003-2005 Dorian C. Arnold, Philip C. Roth, Barton P. Miller *
 *                  Detailed MRNet usage rights in "LICENSE" file.          *
 ****************************************************************************/

/*===========================================================*/
/*             Network Class DEFINITIONS                  */
/*===========================================================*/
#include <stdio.h>
#include <algorithm>
#include <errno.h>

#include "mrnet/src/NetworkImpl.h"
#include "mrnet/src/CommunicatorImpl.h"
#include "mrnet/src/StreamImpl.h"
#include "mrnet/src/utils.h"
#include "src/config.h"

#include "xplat/NetUtils.h"

extern FILE *mrnin;

namespace MRN
{
int mrnparse( );

extern int mrndebug;
extern const char* mrnBufPtr;
extern unsigned int mrnBufRemaining;


NetworkGraph *NetworkImpl::parsed_graph = NULL;
bool NetworkImpl::is_backend=false;
bool NetworkImpl::is_frontend=false;

NetworkImpl::NetworkImpl( Network * _network, const char *_filename,
                          const char *_application, const char **argv )
    : filename( _filename ),
      application( ( _application == NULL ) ? "" : _application ),
      front_end( NULL ), back_end( NULL )
{
    InitFE( _network, argv );
}


//TODO: use unused? //quiet compiler for now
NetworkImpl::NetworkImpl( Network * _network, const char *_config,
                          bool /* unused */, const char *_application,
                          const char **argv )
    : application( ( _application == NULL ) ? "" : _application ),
      front_end( NULL ), back_end( NULL )
{
    InitFE( _network, argv, _config );
}


// BE constructor
NetworkImpl::NetworkImpl( Network *_network,
                          const char *_phostname, Port pport,
                          Rank myrank )         // may be UnknownRank
{
    std::string host = XPlat::NetUtils::GetNetworkName();
    std::string phost( _phostname );
    Port port = UnknownPort;

    //TLS: setup thread local storage for frontend
    //I am "BE(host:port)"
    std::string prettyHost;
    getHostName( prettyHost, host );
    char port_str[16];
    sprintf( port_str, "%u", port );
    char rank_str[16];
    sprintf( rank_str, "%u", myrank );
    std::string name( "BE(" );
    name += prettyHost;
    name += ":";
    name += port_str;
    name += ":";
    name += rank_str;
    name += ")";
    int status;

    tsd_t *local_data = new tsd_t;
    local_data->thread_id = XPlat::Thread::GetId(  );
    local_data->thread_name = strdup( name.c_str(  ) );
    if( ( status = tsd_key.Set( local_data ) ) != 0 ) {
        //TODO: add event to notify upstream
        error(MRN_ESYSTEM, "XPlat::TLSKey::Set(): %s\n", strerror( status ) );
        mrn_dbg( 1, mrn_printf(FLF, stderr, "XPlat::TLSKey::Set(): %s\n",
                    strerror( status ) ));
    }

    back_end = new BackEndNode( _network, host, port, myrank, phost, pport );

    if( back_end->fail() ){
        error( MRN_ESYSTEM, "Failed to initialize via BackEndNode()\n" );
    }
    is_backend = true;
}

void NetworkImpl::InitFE( Network * _network, const char **argv,
                          const char* _config )
{
    // ensure our variables are set for parsing
    parsed_graph = new NetworkGraph;

    if( parse_configfile( _config ) == -1 ) {
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
        error( MRN_ESYSTEM, "XPlat::TLSKey::Set(): %s\n", strerror( status ) );
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
    unsigned int argc=0;
    for(unsigned int i=0; argv[i] != NULL; i++){
        argc++;
    }

    Packet packet( 0, PROT_NEW_SUBTREE, "%s%s%s%as", sg_str.c_str( ),
                   mrn_commnode_path, application.c_str( ), argv, argc );
    if( front_end->proc_newSubTree( packet ) == -1 ) {
        error( MRN_EINTERNAL, "");
        return;
    }

    is_frontend = true;
}

int NetworkImpl::parse_configfile( const char* cfg )
{
    int status;
    // mrndebug=1;

    if( cfg != NULL )
    {
        // set up to parse config from a buffer in memory
        mrnBufPtr = cfg;
        mrnBufRemaining = strlen( cfg );
    }
    else
    {
        // set up to parse config from teh file named by our
        // 'filename' member variable
        mrnin = fopen( filename.c_str(  ), "r" );
        if( mrnin == NULL ) {
            mrn_dbg( 1, mrn_printf(FLF, stderr, "fopen() failed: %s: %s",
                        filename.c_str(), strerror( errno ) ));
            error( MRN_EBADCONFIG_IO, "fopen() failed: %s: %s", filename.c_str(),
                   strerror( errno ) );
            return -1;
        }
    }

    status = mrnparse( );

    if( status != 0 ) {
        mrn_dbg( 1, mrn_printf(FLF, stderr, "mrnparse() failed: %s: Parse Error\n",
                    filename.c_str() ));
        error( MRN_EBADCONFIG_FMT, "mrnparse() failed: %s: Parse Error",
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

EndPoint * NetworkImpl::get_EndPoint( const char *_hostname, Port _port )
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

int NetworkImpl::recv( bool iblocking )
{
    if( is_FrontEnd() ) {
        mrn_dbg( 3, mrn_printf(FLF, stderr, "In NetImpl::recv(%s). "
                               "Calling FrontEnd::recv(%s)\n", 
                               (iblocking? "blocking" : "non-blocking"),
                               (iblocking? "blocking" : "non-blocking") ));
        return front_end->recv( iblocking );
    }
    else if( is_BackEnd() ){
        mrn_dbg( 3, mrn_printf(FLF, stderr, "In NetImpl::recv(%s). "
                               "Calling BackEnd::recv(%s)\n",
                               (iblocking? "blocking" : "non-blocking"),
                               (iblocking? "blocking" : "non-blocking") ));
        return back_end->recv( iblocking );
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
    return ( a->get_Rank(  ) < b->get_Rank(  ) );
}


int NetworkImpl::get_LeafInfo( Network::LeafInfo *** linfo,
                               unsigned int *nLeaves )
{
    int ret = -1;

    // allocate space for the output
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
                mrn_dbg( 1, mrn_printf(FLF, stderr,
                            "failed to receive leaf info from front end node\n" ));
            }

            // now - sleep?  how do we block while still processing
            // packets?  calling receive on frontend node?
            resp = front_end->get_leafInfoPacket(  );
        }

        if( resp != *Packet::NullPacket ) {
            // we got the response successfully -
            // build the return value from the response packet
            char **hosts = NULL;
            unsigned int nHosts = 0;
            int *ports = NULL;
            unsigned int nPorts = 0;
            int *ranks = NULL;
            unsigned int nRanks = 0;
            char **phosts = NULL;
            unsigned int nPHosts = 0;
            int *pports = NULL;
            unsigned int nPPorts = 0;

            int nret = resp.ExtractArgList( "%as %ad %ad %as %ad",
                                            &hosts, &nHosts,
                                            &ports, &nPorts,
                                            &ranks, &nRanks,
                                            &phosts, &nPHosts,
                                            &pports, &nPPorts );
            if( nret == 0 ) {
                if( ( nHosts == nRanks ) &&
                    ( nHosts == nPHosts ) &&
                    ( nHosts == nPPorts ) ) {

                    // build un-ordered leaf info vector
                    std::vector < Network::LeafInfo * >linfov;
                    for( unsigned int i = 0; i < nHosts; i++ ) {

                        assert( hosts[i] != NULL );
                        assert( phosts[i] != NULL );

                        Network::LeafInfo * li =
                            new NetworkImpl::LeafInfoImpl( hosts[i],
                                                           ports[i],
                                                           ranks[i],
                                                           phosts[i],
                                                           pports[i] );
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
                    mrn_dbg( 1, mrn_printf(FLF, stderr,
                                "leaf info packet corrupt: array size mismatch\n" ));
                }
            }
            else {
                mrn_dbg( 1, mrn_printf(FLF, stderr,
                            "failed to extract arrays from leaf info packet\n" ));
            }
        }
    }
    else {
        // we failed to deliver the request 
        mrn_dbg( 1, mrn_printf(FLF, stderr, "failed to deliver request\n" ));
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
                    mrn_dbg( 1, mrn_printf(FLF, stderr,
                                "unexpected backend count %u (expecting %u)\n",
                                nBackends, nBackendsExpected ));
                }
            }
            else {
                mrn_dbg( 1, mrn_printf(FLF, stderr,
                            "format mismatch in leaf connection packet\n" ));
            }
        }
        else {
            mrn_dbg( 1, mrn_printf(FLF, stderr,
                        "failed to receive leaf connection packet\n" ));
            ret = -1;
        }
#endif // READY
    }
    else {
        // we failed to deliver the request
        mrn_dbg( 1, mrn_printf(FLF, stderr, "failed to deliver request\n" ));
        ret = -1;
    }

    return ret;
}

int NetworkImpl::recv(int *otag, Packet **opacket, Stream **ostream,
                      bool iblocking)
{
    bool checked_network = false;   // have we checked sockets for input?
    Packet cur_packet=*Packet::NullPacket;

    mrn_dbg(3, mrn_printf(FLF, stderr, "In StreamImpl::recv().\n"));

    if( is_StreamsEmpty() && is_FrontEnd() ){
      //No streams exist -- bad for FE
      mrn_dbg(1, mrn_printf(FLF, stderr, "%s recv in FE when no streams "
         "exist\n", (iblocking? "Blocking" : "Non-blocking") ));
      return -1;
    }

    // check streams for input
get_packet_from_stream_label:
    if( !is_StreamsEmpty() ) {
        bool packet_found=false;
        std::map <unsigned int, Stream *>::iterator start_iter;

        start_iter = cur_stream_iter;
        do{
            Stream* cur_stream = cur_stream_iter->second;

            cur_packet = cur_stream->get_StreamImpl()->get_IncomingPacket();
            if( cur_packet != *Packet::NullPacket ){
                mrn_dbg( 3, mrn_printf(FLF, stderr, "Packet on stream[%d] %p.\n",
                            cur_stream->get_Id(), cur_stream ));
                packet_found = true;
            }

            cur_stream_iter++;
            if( cur_stream_iter == end_StreamsById() ){
                //wrap around to start of map entries
                cur_stream_iter = begin_StreamsById();
            }
        } while( (start_iter != cur_stream_iter) && !packet_found );
    }

    if( cur_packet != *Packet::NullPacket ) {
        *otag = cur_packet.get_Tag();
        *ostream = get_StreamById( cur_packet.get_StreamId() );
        *opacket = new Packet(cur_packet);
        mrn_dbg(4, mrn_printf(FLF, stderr, "cur_packet tag: %d, fmt: %s\n",
                   cur_packet.get_Tag(), cur_packet.get_FormatString() ));
        return 1;
    }
    else if( iblocking || !checked_network ) {

        // No packets are already in the stream
        // check whether there is data waiting to be read on our sockets
        int retval = recv( iblocking );
        checked_network = true;

        if( retval == -1 ){
            mrn_dbg( 1, mrn_printf(FLF, stderr, "Network::recv() failed.\n" ));
            return -1;
        }
        else if ( retval == 1 ){
            // go back if we found a packet
            mrn_dbg( 3, mrn_printf(FLF, stderr, "Network::recv() found a packet!\n" ));
            goto get_packet_from_stream_label;
        }
    }

    mrn_dbg( 3, mrn_printf(FLF, stderr, "Network::recv() No packets found.\n" ));
    return 0;
}


NetworkImpl::LeafInfoImpl::LeafInfoImpl( const char* _host,
                                            Port _port,
                                            Rank _rank,
                                            const char* _phost,
                                            Port _pport )
    : host( new char[strlen(_host)+1] ),
    port( _port ),
    rank( _rank ),
    phost( new char[strlen(_phost)+1] ),
    pport( _pport )
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
        ret = front_end->get_SocketFd( conns, nConns );
    }
    else if( is_BackEnd() ) {
        ret = back_end->get_SocketFd( );
        assert( ( ret != 0 ) || ( *nConns == 1 ) );
    }
    else{
        assert( 0 ); //either front_end or back_end should be init'd
    }

    return ret;
}

int NetworkImpl::get_SocketFd(  )
{
    assert( is_BackEnd() );
    return back_end->get_SocketFd( );
}

int NetworkImpl::get_SocketFd( int **array, unsigned int * array_size )
{
    assert( is_FrontEnd() );
    return front_end->get_SocketFd( array, array_size );
}

void NetworkImpl::set_StreamById( unsigned int iid, Stream * istream )
{
    all_streams_mutex.Lock();
    allStreamsById[iid] = istream;
    if(allStreamsById.size() == 1 ){
        cur_stream_iter = allStreamsById.begin();
    }
    all_streams_mutex.Unlock();
}

Stream * NetworkImpl::get_StreamById( unsigned int iid )
{
    Stream * ret;
    std::map<unsigned int, Stream*>::iterator iter; 

    all_streams_mutex.Lock();

    iter = allStreamsById.find( iid );
    if(  iter != allStreamsById.end() ){
        ret = iter->second;
    }
    else{
        ret = NULL;
    }
    all_streams_mutex.Unlock();

    return ret;
}

void NetworkImpl::delete_StreamById( unsigned int iid )
{
    std::map<unsigned int, Stream*>::iterator iter; 

    all_streams_mutex.Lock();

    iter = allStreamsById.find( iid );
    if(  iter != allStreamsById.end() ){

        //if we are about to delete start_iter, set it to next elem. (w/wrap)
        if( iter == cur_stream_iter ){
            cur_stream_iter++;
            if( cur_stream_iter == NetworkImpl::allStreamsById.end() ){
                cur_stream_iter = NetworkImpl::allStreamsById.begin();
            }
        }

        //case where streams is now empty handled by setting cur_stream_iter to
        //streams.begin() when 1st stream is created.

        allStreamsById.erase( iter );
    }

    all_streams_mutex.Unlock();
}

bool NetworkImpl::is_StreamsEmpty( )
{
    bool ret;
    all_streams_mutex.Lock();
    ret = allStreamsById.empty();
    all_streams_mutex.Unlock();
    return ret;
}

std::map < unsigned int, Stream * >::iterator NetworkImpl::begin_StreamsById()
{
    std::map < unsigned int, Stream * >::iterator ret;

    all_streams_mutex.Lock();
    ret = allStreamsById.begin();
    all_streams_mutex.Unlock();

    return ret;
}

std::map < unsigned int, Stream * >::iterator NetworkImpl::end_StreamsById()
{
    std::map < unsigned int, Stream * >::iterator ret;

    all_streams_mutex.Lock();
    ret = allStreamsById.end();
    all_streams_mutex.Unlock();

    return ret;
}

unsigned int NetworkImpl::size_StreamsById( )
{
    unsigned int ret;

    all_streams_mutex.Lock();
    ret = allStreamsById.size();
    all_streams_mutex.Unlock();

    return ret;
}
}                               // namespace MRN
