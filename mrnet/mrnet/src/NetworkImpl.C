/*===========================================================*/
/*             Network Class DEFINITIONS                  */
/*===========================================================*/
#include <stdio.h>
#include <algorithm>
#include <errno.h>

#include "mrnet/src/NetworkImpl.h"
#include "mrnet/src/CommunicatorImpl.h"
#include "mrnet/src/utils.h"

extern FILE *mrnin;
extern int mrndebug;

namespace MRN
{
int mrnparse( );

NetworkGraph *NetworkImpl::parsed_graph = NULL;

NetworkImpl::NetworkImpl( const char *_filename,
                          const char *_application )
    : filename( _filename ),
      application( ( _application == NULL ) ? "" : _application ),
      front_end( NULL )
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
    if( ( status = pthread_key_create( &tsd_key, NULL ) ) != 0 ) {
        mrn_printf( 1, MCFL, stderr, "pthread_key_create(): %s\n",
                    strerror( status ) );
        error( ESYSTEM, "pthread_setspecific(): %s\n", strerror( status ) );
        return;
    }
    tsd_t *local_data = new tsd_t;
    local_data->thread_id = pthread_self( );
    local_data->thread_name = strdup( name.c_str( ) );
    status = pthread_setspecific( tsd_key, ( const void * )local_data );

    if( status != 0 ) {
        error( ESYSTEM, "pthread_setspecific(): %s\n", strerror( status ) );
        return;
    }

    NetworkImpl::endpoints = graph->get_EndPoints( );
    CommunicatorImpl::create_BroadcastCommunicator( NetworkImpl::endpoints );

    //Frontend is root of the tree
    front_end = new FrontEndNode( graph->get_Root( )->get_HostName( ),
                                  graph->get_Root( )->get_Port( ) );
    SerialGraph sg = graph->get_SerialGraph(  );
    sg.print(  );

    // save the serialized graph string in a variable on the stack,
    // so that we don't build a packet with a pointer into a temporary
    std::string sg_str = sg.get_ByteArray(  );
    Packet packet( 0, PROT_NEW_SUBTREE, "%s%s",
                   sg_str.c_str(  ), application.c_str(  ) );
    if( front_end->proc_newSubTree( packet ) == -1 ) {
        _fail = true;
        return;
    }

    return;
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

int NetworkImpl::recv( bool blocking )
{
    if( Network::network ) {
        mrn_printf( 3, MCFL, stderr, "In NetImpl::recv(). "
                    "Calling FrontEnd::recv()\n" );
        return Network::network->front_end->recv( blocking );
    }
    else {
        mrn_printf( 3, MCFL, stderr, "In NetImpl::recv(). "
                    "Calling BackEnd::recv()\n" );
        return Network::back_end->recv( blocking );
    }
}


int NetworkImpl::send( Packet& packet )
{
    if( Network::network ) {
        return Network::network->front_end->send( packet );
    }
    else {
        return Network::back_end->send( packet );
    }
}

bool NetworkImpl::is_FrontEnd(  )
{
    return ( Network::network != NULL );
}

bool NetworkImpl::is_BackEnd(  )
{
    return ( Network::network == NULL );
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
                    for( unsigned int i = 0; i < nHosts; i++ ) {
                        ( *linfo )[i] = linfov[i];
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

}                               // namespace MRN
