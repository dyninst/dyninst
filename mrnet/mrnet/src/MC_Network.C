#include "mrnet/src/Types.h"


#include <fstream>
#include <sys/types.h>
#include <ctype.h>
#include <limits.h>
#include <stdarg.h>
#include <stdio.h>
#include "mrnet/h/MC_Network.h"
#include "mrnet/src/utils.h"
#include "mrnet/src/config.h"
#include "mrnet/src/MC_NetworkImpl.h"
#include "mrnet/src/MC_BackEndNode.h"
#include "mrnet/src/MC_StreamImpl.h"
#include "mrnet/src/MC_CommunicatorImpl.h"
#include "mrnet/src/MC_EndPointImpl.h"

MC_NetworkImpl * MC_Network::network = NULL;
MC_BackEndNode * MC_Network::back_end = NULL;

/*===========================================================*/
/*             MC_Network static function DEFINITIONS        */
/*===========================================================*/
int MC_Network::new_Network(const char * _filename, const char * _application)
{
  MC_Network::network = new MC_NetworkImpl(_filename, _application);

  if( MC_Network::network->fail() ){
    return -1;
  }
  else{
    return 0;
  }
}

void MC_Network::delete_Network()
{
  delete MC_Network::network;
}

int MC_Network::init_Backend(const char *_hostname, const char *_port,
                             const char *_phostname,
                             const char *_pport, const char *_pid)
{
  string host(_hostname);
  unsigned short port(atoi(_port));
  string phost(_phostname);
  unsigned short pport(atoi(_pport));
  unsigned short pid(atoi(_pid));

  //TLS: setup thread local storage for frontend
  //I am "BE(host:port)"
  string name("BE(");
  name += getHostName(host);
  name += ":";
  name += _port;
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
  //mc_printf(MCFL, stderr, "comm(%p) size:%d\n",
	     //comm, ((MC_CommunicatorImpl*)(comm))->get_EndPoints()->size());
  //mc_printf(MCFL, stderr, "comm's endpoint: %p\n", ((MC_CommunicatorImpl*)(comm))->get_EndPoints());
  return new MC_StreamImpl(comm, _filter_id);
}

int MC_Stream::recv(int *tag, void **buf, MC_Stream ** stream)
{
  return MC_StreamImpl::recv(tag, buf, stream);
}

int MC_Stream::unpack(char * buf, char const *fmt_str, ...)
{
  MC_Packet * packet = (MC_Packet *)buf;
  int status;
  va_list arg_list;

  mc_printf(MCFL, stderr, "In stream.unpack()\n");
  mc_printf(MCFL, stderr, "packet(%p) tag: %d, fmt: %s\n", packet, packet->get_Tag(), packet->get_FormatString());
  va_start(arg_list, fmt_str);
  status = packet->ExtractVaList(fmt_str, arg_list); 
  va_end(arg_list);

  mc_printf(MCFL, stderr, "stream.unpack() %s",
             (status==-1 ? "failed\n" : "succeeded\n"));
  return status;
}

/*======================================================*/
/*             MC_Communicator class DEFINITIONS        */
/*======================================================*/
MC_Communicator * new_Communicator()
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
