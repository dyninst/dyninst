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

MC_NetworkImpl * MC_Network::network = NULL;
MC_BackEndNode * MC_Network::back_end = NULL;

/*===========================================================*/
/*             MC_Network static function DEFINITIONS        */
/*===========================================================*/
int MC_Network::new_Network(const char * _filename,
                                const char * _commnode,
                                const char * _application)
{
  MC_Network::network = new MC_NetworkImpl(_filename, _commnode, _application);

  if( MC_Network::network->fail() ){
    return -1;
  }
  else{
    return 0;
  }
}


int MC_Network::new_NetworkNoBE( const char* cfgFileName,
                                const char* commNodeExe,
                                MC_Network::LeafInfo*** leafInfo,
                                unsigned int* nLeaves )
{
    int ret = -1;

    // build the network
    MC_Network::network = new MC_NetworkImpl( cfgFileName, commNodeExe, NULL );
    if( !MC_Network::network->fail() )
    {
        if( (leafInfo != NULL) && (nLeaves != NULL) )
        {
            MC_Network::network->get_LeafInfo( leafInfo, nLeaves );
            ret = 0;
        }
        else
        {
            // TODO is this the right error?
            ret = MC_ENETWORK_FAILURE;
        }
    }

    return ret;
}



int
MC_Network::connect_Backends( void )
{
    // TODO is this the right error?
    int ret = MC_ENETWORK_FAILURE;

    if( MC_Network::network != NULL )
    {
        ret = MC_Network::network->connect_Backends();
    }
    return ret;
}





void MC_Network::delete_Network()
{
  delete MC_Network::network;
}


int
MC_Network::getConnections( int** conns, unsigned int* nConns )
{
    int ret = 0;

    if( MC_Network::network != NULL )
    {
        ret = MC_Network::network->getConnections( conns, nConns );
    }
    else
    {
        ret = MC_Network::back_end->getConnections( conns, nConns );
        assert( (ret != 0) || (*nConns == 1) );
    }

    return ret;
} 



int MC_Network::init_Backend(const char *_hostname, const char *_port,
                             const char *_phostname,
                             const char *_pport, const char *_pid)
{
    unsigned int port(atoi(_port));
    unsigned int pport(atoi(_pport));
    unsigned int pid(atoi(_pid));

    return init_Backend(_hostname, port, _phostname, pport, pid );
}

int MC_Network::init_Backend(const char *_hostname, unsigned int port,
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

  MC_Network::back_end = new MC_BackEndNode(host, port, phost, pport, pid);

  if( MC_Network::back_end->fail()){
    return -1;
  }
  else{
    return 0;
  }
}

void MC_Network::error_str(const char *s)
{
  MC_Network::network->perror(s);
}

/*================================================*/
/*             MC_Stream class DEFINITIONS        */
/*================================================*/
MC_Stream * MC_Stream::new_Stream(MC_Communicator *comm, int _filter_id)
{
  //printf(MCFL, stderr, "comm(%p) size:%d\n",
	     //comm, ((MC_CommunicatorImpl*)(comm))->get_EndPoints()->size());
  //printf(MCFL, stderr, "comm's endpoint: %p\n", ((MC_CommunicatorImpl*)(comm))->get_EndPoints());
  return new MC_StreamImpl(comm, _filter_id);
}

int MC_Stream::recv(int *tag, void **buf, MC_Stream ** stream)
{
  return MC_StreamImpl::recv(tag, buf, stream);
}


int MC_Stream::unpack(char * buf, char const *fmt_str, ...)
{
    va_list arg_list;

    va_start( arg_list, fmt_str );
    int ret = MC_StreamImpl::unpack( buf, fmt_str, arg_list );
    va_end( arg_list );
    return ret;
}


/*======================================================*/
/*             MC_Communicator class DEFINITIONS        */
/*======================================================*/
MC_Communicator * MC_Communicator::new_Communicator()
{
  return new MC_CommunicatorImpl;
}

MC_Communicator * MC_Communicator::get_BroadcastCommunicator()
{
  return MC_CommunicatorImpl::get_BroadcastCommunicator();
}

/*==================================================*/
/*             MC_EndPoint class DEFINITIONS        */
/*==================================================*/
MC_EndPoint * MC_EndPoint::new_EndPoint(int _id, const char * _hostname,
                                        unsigned short _port)
{
  return new MC_EndPointImpl(_id, _hostname, _port);
}



