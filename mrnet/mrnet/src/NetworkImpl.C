/*===========================================================*/
/*             MC_Network Class DEFINITIONS                  */
/*===========================================================*/
#include <stdio.h>

#include "mrnet/src/NetworkImpl.h"
#include "mrnet/src/CommunicatorImpl.h"
#include "mrnet/src/utils.h"

std::list <MC_NetworkNode *>* hostlist = NULL;
std::list <MC_NetworkNode *>* potential_root = NULL;
MC_NetworkGraph* parsed_graph = NULL;

MC_NetworkImpl::MC_NetworkImpl(const char * _filename,
                                const char * _commnode,
                                const char * _application)
  :filename(_filename),
   commnode(_commnode),
   application( (_application == NULL) ? "" : _application ),
   endpoints( NULL ),
   front_end( NULL )
{
    // ensure our variables are set for parsing
    parsed_graph = new MC_NetworkGraph;
    hostlist = new std::list<MC_NetworkNode*>;
    potential_root = new std::list<MC_NetworkNode*>;

  if( parse_configfile() == -1){
    return;
  }

  graph = parsed_graph;  
  if ( graph->has_cycle() ){
    //not really a cycle, but graph is not at least tree.
    mc_errno = MC_ENETWORK_CYCLE;
    _fail=true;
    return;
  }

  //TLS: setup thread local storage for frontend
  //I am "FE(hostname:port)"
  char port_str[128];
  sprintf(port_str, "%d", graph->get_Root()->get_Port());
  std::string name("FE(");
  name += graph->get_Root()->get_HostName();
  name += ":";
  name += port_str;
  name += ")";

  int status;
  if( (status = pthread_key_create(&tsd_key, NULL)) != 0){
    fprintf(stderr, "pthread_key_create(): %s\n", strerror(status)); 
    assert(0);
  }
  tsd_t * local_data = new tsd_t;
  local_data->thread_id = pthread_self();
  local_data->thread_name = strdup(name.c_str());
  if( (status = pthread_setspecific(tsd_key, (const void *)local_data)) != 0){
    fprintf(stderr, "pthread_key_create(): %s\n", strerror(status)); 
    exit(-1);
  }

  MC_NetworkImpl::endpoints = graph->get_EndPoints();
  MC_CommunicatorImpl::create_BroadcastCommunicator(MC_NetworkImpl::endpoints);

  //Frontend is root of the tree
  front_end = new MC_FrontEndNode(graph->get_Root()->get_HostName(),
                        graph->get_Root()->get_Port());
  MC_SerialGraph sg = graph->get_SerialGraph();
  sg.print();

  // save the serialized graph string in a variable on the stack,
  // so that we don't build a packet with a pointer into a temporary
  std::string sg_str = sg.get_ByteArray();
  MC_Packet *packet = new MC_Packet(MC_NEW_SUBTREE_PROT, "%s%s%s",
                                    sg_str.c_str(),
                                    application.c_str(),
                                    commnode.c_str() );
  if( front_end->proc_newSubTree( packet )
      == -1){
    mc_errno = MC_ENETWORK_FAILURE;
    _fail=true;
    return;
  }

  return;
}

extern FILE * mrnin;
int mrnparse();
extern int mrndebug;

int MC_NetworkImpl::parse_configfile()
{
  // mrndebug=1;
  mrnin = fopen(filename.c_str(), "r");
  if( mrnin == NULL){
    mc_errno = MC_EBADCONFIG_IO;
    _fail = true;
    return -1;
  }

  if( mrnparse() != 0 ){
    mc_errno = MC_EBADCONFIG_FMT;
    _fail = true;
    return -1;
  }

  return 0;
}

MC_NetworkImpl::~MC_NetworkImpl()
{
  delete graph;
  delete endpoints;
  delete front_end;
}

MC_EndPoint * MC_NetworkImpl::get_EndPoint(const char * _hostname,
                                           unsigned short _port)
{
  unsigned int i;

  for(i=0; i<MC_NetworkImpl::endpoints->size(); i++){
    if((*MC_NetworkImpl::endpoints)[i]->compare(_hostname, _port) == true){
      return (*MC_NetworkImpl::endpoints)[i];
    }
  }

  return NULL;
}

int MC_NetworkImpl::recv(void)
{
  if(MC_Network::network){
    mc_printf(MCFL, stderr, "In net.recv(). Calling frontend.recv()\n");
    return MC_Network::network->front_end->recv();
  }
  else{
    mc_printf(MCFL, stderr, "In net.recv(). Calling backend.recv()\n");
    return MC_Network::back_end->recv();
  }
}


int MC_NetworkImpl::send(MC_Packet *packet)
{
  if(MC_Network::network){
    return MC_Network::network->front_end->send(packet);
  }
  else{
    return MC_Network::back_end->send(packet);
  }
}



static
bool
leafInfoCompare( const MC_Network::LeafInfo* a,
                    const MC_Network::LeafInfo* b )
{
    assert( a != NULL );
    assert( b != NULL );
    return (a->get_Id() < b->get_Id());
}


int
MC_NetworkImpl::get_LeafInfo( MC_Network::LeafInfo*** linfo,
                                unsigned int* nLeaves )
{
    int ret = -1;

    // these should've been checked by our caller
    assert( linfo != NULL );
    assert( nLeaves != NULL );

    // request that our leaves give us their leaf info
    MC_Packet* pkt = new MC_Packet( MC_GET_LEAF_INFO_PROT, "" );
    if( front_end->proc_getLeafInfo( pkt ) != -1 )
    {
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
        MC_Packet* resp = front_end->get_leafInfoPacket();
        while( resp == NULL )
        {
            if( front_end->recv() == -1 )
            {
                mc_printf( MCFL, stderr, "failed to receive leaf info from front end node\n" );
            }

            // now - sleep?  how do we block while still processing
            // packets?  calling receive on frontend node?
            resp = front_end->get_leafInfoPacket();
        }

        if( resp != NULL )
        {
            // we got the response successfully -
            // build the return value from the response packet
            int* ids = NULL;
            unsigned int nIds = 0;
            char** hosts = NULL;
            unsigned int nHosts = 0;
            int* ranks = NULL;
            unsigned int nRanks = 0;
            char** phosts = NULL;
            unsigned int nPHosts = 0;
            int* pports = NULL;
            unsigned int nPPorts = 0;
            int* pranks = NULL;
            unsigned int nPRanks = 0;

            int nret = resp->ExtractArgList( "%ad %as %ad %as %ad %ad",
                                                &ids, &nIds,
                                                &hosts, &nHosts,
                                                &ranks, &nRanks,
                                                &phosts, &nPHosts,
                                                &pports, &nPPorts,
                                                &pranks, &nPRanks );
            if( nret == 0 )
            {
                if( (nHosts == nRanks) &&
                    (nHosts == nPHosts) &&
                    (nHosts == nPPorts) &&
                    (nHosts == nPRanks) )
                {
                    // build un-ordered leaf info vector
                    std::vector<MC_Network::LeafInfo*> linfov;
                    for( unsigned int i = 0; i < nHosts; i++ )
                    {
                        MC_Network::LeafInfo* li = 
                            new MC_NetworkImpl::LeafInfoImpl( ids[i],
                                                                hosts[i],
                                                                ranks[i],
                                                                phosts[i],
                                                                pports[i],
                                                                pranks[i] );
                        linfov.push_back( li );
                    }

                    // sort the leaf info array by backend id
                    std::sort( linfov.begin(), linfov.end(), leafInfoCompare );

                    // copy sorted vector to output array
                    *linfo = new MC_Network::LeafInfo*[nHosts];
                    for( unsigned int i = 0; i < nHosts; i++ )
                    {
                        (*linfo)[i] = linfov[i];
                    }
                    *nLeaves = linfov.size();

                    ret = 0;
                }
                else
                {
                    mc_printf( MCFL, stderr, "leaf info packet corrupt: array size mismatch\n" );
                }
            }
            else
            {
                mc_printf( MCFL, stderr, "failed to extract arrays from leaf info packet\n" );
            }
        }
    }
    else
    {
        // we failed to deliver the request 
        mc_printf( MCFL, stderr, "failed to deliver request\n" );
    }

    return ret;
}



int
MC_NetworkImpl::connect_Backends( void )
{
    int ret = 0;

    // broadcast message to all leaves that they 
    // should accept their connections
    MC_Packet* pkt = new MC_Packet( MC_CONNECT_LEAVES_PROT, "" );
    if( front_end->proc_connectLeaves( pkt ) != -1 )
    {
#if READY
        // in an ideal world, we don't have to get a response to this
#else
        // wait for response (?)
        MC_Packet* resp = front_end->get_leavesConnectedPacket();
        while( resp == NULL )
        {
            // allow the front end node to handle packets
            front_end->recv();

            // see if we got our response packet
            resp = front_end->get_leavesConnectedPacket();
        }

        if( resp != NULL )
        {
            // verify response based on our notion of the 
            // number of leaves we have
            unsigned int nBackendsExpected = endpoints->size(); 
            unsigned int nBackends;
            int nret = resp->ExtractArgList( "%ud", &nBackends );
            if( nret == 0 )
            {
                if( nBackends != nBackendsExpected )
                {
                    mc_printf( MCFL, stderr, "unexpected backend count %u (expecting %u)\n",
                                    nBackends, nBackendsExpected );
                }
            }
            else
            {
                mc_printf( MCFL, stderr, "format mismatch in leaf connection packet\n" );
            }
        }
        else
        {
            mc_printf( MCFL, stderr, "failed to receive leaf connection packet\n" );
            ret = -1;
        } 
#endif // READY
    }
    else
    {
        // we failed to deliver the request
        mc_printf( MCFL, stderr, "failed to deliver request\n" );
        ret = -1;
    }

    return ret;
}

