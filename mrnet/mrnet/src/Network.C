#include "mrnet/src/Types.h"


#include <fstream>
#include <vector>
#include <algorithm>
#include <sys/types.h>
#include <ctype.h>
#include <limits.h>
#include <stdarg.h>
#include <stdio.h>
#include "mrnet/h/MR_Network.h"
#include "mrnet/src/utils.h"
#include "mrnet/src/config.h"
#include "mrnet/src/NetworkImpl.h"
#include "mrnet/src/BackEndNode.h"
#include "mrnet/src/StreamImpl.h"
#include "mrnet/src/CommunicatorImpl.h"
#include "mrnet/src/EndPointImpl.h"

#include "mrnet/h/MR_NetworkC.h"
using namespace MRN;

NetworkImpl * Network::network = NULL;
BackEndNode * Network::back_end = NULL;

/*===========================================================*/
/*             Network static function DEFINITIONS        */
/*===========================================================*/
int Network::new_Network(const char * _filename,
                                const char * _commnode,
                                const char * _application)
{
  Network::network = new NetworkImpl(_filename, _commnode, _application);

  if( Network::network->fail() ){
    return -1;
  }
  else{
    return 0;
  }
}


int Network::new_NetworkNoBE( const char* cfgFileName,
                                const char* commNodeExe,
                                Network::LeafInfo*** leafInfo,
                                unsigned int* nLeaves )
{
    int ret = -1;

    // build the network
    Network::network = new NetworkImpl( cfgFileName, commNodeExe, NULL );
    if( !Network::network->fail() )
    {
        if( (leafInfo != NULL) && (nLeaves != NULL) )
        {
            Network::network->get_LeafInfo( leafInfo, nLeaves );
            ret = 0;
        }
        else
        {
            // TODO is this the right error?
            ret = MRN_ENETWORK_FAILURE;
        }
    }

    return ret;
}



int
Network::connect_Backends( void )
{
    // TODO is this the right error?
    int ret = MRN_ENETWORK_FAILURE;

    if( Network::network != NULL )
    {
        ret = Network::network->connect_Backends();
    }
    return ret;
}





void Network::delete_Network()
{
  delete Network::network;
}


int
Network::getConnections( int** conns, unsigned int* nConns )
{
    int ret = 0;

    if( Network::network != NULL )
    {
        ret = Network::network->getConnections( conns, nConns );
    }
    else
    {
        ret = Network::back_end->getConnections( conns, nConns );
        assert( (ret != 0) || (*nConns == 1) );
    }

    return ret;
} 



int Network::init_Backend(const char *_hostname, const char *_port,
                             const char *_phostname,
                             const char *_pport, const char *_pid)
{
    unsigned int port(atoi(_port));
    unsigned int pport(atoi(_pport));
    unsigned int pid(atoi(_pid));

    return init_Backend(_hostname, port, _phostname, pport, pid );
}

int Network::init_Backend(const char *_hostname, unsigned int port,
                             const char *_phostname,
                             unsigned int pport, unsigned int pid)
{
  std::string host(_hostname);
  std::string phost(_phostname);

  //TLS: setup thread local storage for frontend
  //I am "BE(host:port)"
  char port_str[16];
  sprintf( port_str, "%u", port );
  std::string name("BE(");
  name += getHostName(host);
  name += ":";
  name += port_str;
  name += ")";

  int status;
  if( (status = pthread_key_create(&tsd_key, NULL)) != 0){
    fprintf(stderr, "pthread_key_create(): %s\n", strerror(status)); 
    exit(-1);
  }
  tsd_t * local_data = new tsd_t;
  local_data->thread_id = pthread_self();
  local_data->thread_name = strdup(name.c_str());
  if( (status = pthread_setspecific(tsd_key, local_data)) != 0){
    fprintf(stderr, "pthread_key_create(): %s\n", strerror(status)); 
    exit(-1);
  }

  Network::back_end = new BackEndNode(host, port, phost, pport, pid);

  if( Network::back_end->fail()){
    return -1;
  }
  else{
    return 0;
  }
}

void Network::error_str(const char *s)
{
  Network::network->perror(s);
}

/*================================================*/
/*             Stream class DEFINITIONS        */
/*================================================*/
Stream * Stream::new_Stream(Communicator *comm, int _filter_id)
{
  //printf(3, MCFL, stderr, "comm(%p) size:%d\n",
	     //comm, ((CommunicatorImpl*)(comm))->get_EndPoints()->size());
  //printf(3, MCFL, stderr, "comm's endpoint: %p\n", ((CommunicatorImpl*)(comm))->get_EndPoints());
  return new StreamImpl(comm, _filter_id);
}

int Stream::recv(int *tag, void **buf, Stream ** stream)
{
  return StreamImpl::recv(tag, buf, stream);
}


int Stream::unpack(char * buf, char const *fmt_str, ...)
{
    va_list arg_list;

    va_start( arg_list, fmt_str );
    int ret = StreamImpl::unpack( buf, fmt_str, arg_list );
    va_end( arg_list );
    return ret;
}


/*======================================================*/
/*             Communicator class DEFINITIONS        */
/*======================================================*/
Communicator * Communicator::new_Communicator()
{
  return new CommunicatorImpl;
}

Communicator * Communicator::get_BroadcastCommunicator()
{
  return CommunicatorImpl::get_BroadcastCommunicator();
}

/*==================================================*/
/*             EndPoint class DEFINITIONS        */
/*==================================================*/
EndPoint * EndPoint::new_EndPoint(int _id, const char * _hostname,
                                        unsigned short _port)
{
  return new EndPointImpl(_id, _hostname, _port);
}



