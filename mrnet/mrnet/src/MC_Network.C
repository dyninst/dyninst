#include "common/h/Types.h"


#include <fstream>
#include <sys/types.h>
#include <ctype.h>
#include <limits.h>
#include <stdarg.h>
#include <stdio.h>
#include "mrnet/h/MC_Network.h"
#include "mrnet/src/utils.h"
#include "mrnet/src/config.h"
#include "common/src/list.C"
#include "common/src/Dictionary.C"


MC_Network * MC_Network::network = NULL;
MC_BackEndNode * MC_Network::back_end = NULL;
unsigned int MC_Stream::cur_stream_idx=0;
unsigned int MC_Stream::next_stream_id=0;
MC_Communicator * MC_Communicator::comm_Broadcast=NULL;
dictionary_hash <unsigned int, MC_Stream *> MC_Stream::streams(uintHash);

/*===========================================================*/
/*             MC_Network static function DEFINITIONS        */
/*===========================================================*/
int MC_Network::new_Network(const char * _filename, const char * _application)
{
  MC_Network::network = new MC_Network(_filename, _application);

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

int MC_Network::init_Backend(const char *_hostname, const char *_port)
{
  string host(_hostname); unsigned short port(atoi(_port));
  MC_Network::back_end = new MC_BackEndNode(host, port);

  if( MC_Network::back_end->fail()){
    return -1;
  }
  else{
    return 0;
  }
}

int MC_Network::recv()
{
  if(MC_Network::network){
    return MC_Network::network->front_end->recv();
  }
  else{
    return MC_Network::back_end->recv();
  }
}

int MC_Network::send(MC_Packet *packet)
{
  if(MC_Network::network){
    return MC_Network::network->front_end->send(packet);
  }
  else{
    return MC_Network::back_end->send(packet);
  }
}

void MC_Network::error_str(const char *s)
{
  MC_Network::network->perror(s);
}

/*===========================================================*/
/*             MC_Network Class DEFINITIONS                  */
/*===========================================================*/
MC_Network::MC_Network(const char * _filename, const char * _application)
  :filename(_filename), application(_application)
{
  if( parse_configfile() == -1){
    return;
  }
  
  if ( graph->has_cycle() ){
    //Network has a cycle
    mc_errno = MC_ENETWORK_CYCLE;
    _fail=true;
    return;
  }

  if ( !graph->fully_connected() ){
   //All nodes not reachable from root
    mc_errno = MC_ENETWORK_NOTCONNECTED;
    _fail=true;
    return;
  }

  MC_Network::endpoints = graph->get_EndPoints();
  MC_Communicator::create_BroadcastCommunicator(MC_Network::endpoints);

  MC_Network::front_end = new MC_FrontEndNode;

  graph->get_SerialGraph().print();
  if( MC_Network::front_end->send_newSubTree( graph->get_SerialGraph() ) == -1){
    mc_errno = MC_ENETWORK_FAILURE;
    _fail=true;
    return;
  }

  string app(application);
  vector <string> args;

  if( MC_Network::front_end->send_newApplication(app, args) == -1){
    mc_errno = MC_ENETWORK_FAILURE;
    _fail=true;
    return;
  }

  return;
}

extern FILE * yyin;
int yyparse(void);
int MC_Network::parse_configfile()
{
  yyin = fopen(filename.c_str(), "r");
  if( yyin == NULL){
    mc_errno = MC_EBADCONFIG_IO;
    _fail = true;
    return -1;
  }

  if( yyparse() != 0 ){
    mc_errno = MC_EBADCONFIG_FMT;
    _fail = true;
    return -1;
  }
}

MC_Network::~MC_Network()
{
  delete graph;
  delete endpoints;
  delete front_end;
}

MC_EndPoint * MC_Network::get_EndPoint(string _hostname, unsigned short _port)
{
  unsigned int i;

  for(i=0; i<MC_Network::endpoints->size(); i++){
    if((*MC_Network::endpoints)[i]->compare(_hostname, _port) == true){
      return (*MC_Network::endpoints)[i];
    }
  }

  return NULL;
}

/*===========================================================*/
/*             MC_Stream CLASS METHOD DEFINITIONS            */
/*===========================================================*/
MC_Stream::MC_Stream(MC_Communicator &_comm, int _filter_id)
  :filter_id(_filter_id)
{
  communicator = new MC_Communicator(_comm); //copy so that change in comm
                                              //doesn't change stream's comm
  stream_id = next_stream_id++;
  MC_Stream::streams[stream_id] = this;
  if ( MC_Network::network ){
    MC_Network::network->front_end->send_newStream(stream_id, filter_id);
  }
}

MC_Stream::MC_Stream(int _stream_id, int * backends=0, int num_backends=-1,
                     int _filter_id=-1)
  :filter_id(_filter_id), stream_id(_stream_id)
{
  MC_Stream::streams[stream_id] = this;
}

int MC_Stream::recv(int *tag, void **ptr, MC_Stream **stream)
{
  unsigned int start_idx;
  MC_Stream * cur_stream=NULL;
  MC_Packet * cur_packet=NULL;

  mc_printf((stderr, "In stream.recv(). Calling frontend.recv()\n"));
  MC_Network::network->recv();
  mc_printf((stderr, "network.recv() returned\n"));

  start_idx = cur_stream_idx;
  do{
    cur_stream = MC_Stream::streams[cur_stream_idx];
    if(!cur_stream){
      cur_stream_idx++;
      cur_stream_idx %= streams.size();
      continue;
    }

    mc_printf((stderr, "Checking stream[%d] for packets ...", cur_stream_idx));
    if(! cur_stream->IncomingPacketBuffer.isEmpty() ){
      mc_printf((stderr, "Packets found\n"));
      List<MC_Packet *>::iterator iter = cur_stream->IncomingPacketBuffer.begin();

      cur_packet = *iter;
      *tag = cur_packet->get_Tag();
      *stream = MC_Stream::streams[cur_packet->get_StreamId()];
      *ptr = (void *) cur_packet;
      cur_stream_idx++;
      cur_stream_idx %= streams.size();
      break;
    }
    mc_printf((stderr, "No Packets found\n"));

    cur_stream_idx++;
    cur_stream_idx %= streams.size();
  } while(start_idx != cur_stream_idx);

  if(cur_packet){
    mc_printf((stderr, "cur_packet(%p) tag: %d, fmt: %s\n", cur_packet, cur_packet->get_Tag(), cur_packet->get_FormatString()));
    assert( (*stream)->IncomingPacketBuffer.remove(cur_packet) );
    return 1;
  }
  else{
    mc_printf((stderr, "No packets currently on stream\n"));
    return 0;
  }
}

int MC_Stream::unpack(char * buf, char const *fmt_str, ...)
{
  MC_Packet * packet = (MC_Packet *)buf;
  int status;
  va_list arg_list;

  mc_printf((stderr, "In stream.unpack()\n"));
  mc_printf((stderr, "packet(%p) tag: %d, fmt: %s\n", packet, packet->get_Tag(), packet->get_FormatString()));
  va_start(arg_list, fmt_str);
  status = packet->ExtractVaList(fmt_str, arg_list); 
  va_end(arg_list);

  mc_printf((stderr, "stream.unpack() %s",
             (status==-1 ? "failed\n" : "succeeded\n")));
  return status;
}

int MC_Stream::send(int tag, char const * fmt, ...)
{
  int status;
  va_list arg_list;
  MC_Packet * packet;

  mc_printf((stderr, "In stream[%d].send(). Calling new packet()\n", stream_id));

  va_start(arg_list, fmt);
  packet = new MC_Packet(stream_id, tag, fmt, arg_list);
  if(packet->fail()){
    mc_printf((stderr, "new packet() fail\n"));
    return -1;
  }
  mc_printf((stderr, "new packet() succeeded. Calling frontend.send()\n"));
  status = MC_Network::network->send(packet);
  va_end(arg_list);
  mc_printf((stderr, "network.send() %s",
             (status==-1 ? "failed\n" : "succeeded\n")));
  return status;
}

int MC_Stream::flush()
{
  if(MC_Network::network){
    return MC_Network::network->front_end->flush(stream_id);
  }
  else{
    return MC_Network::back_end->flush();
  }
}

int MC_Stream::recv(int *tag, void ** ptr)
{
  MC_Packet * cur_packet=NULL;

  mc_printf((stderr, "In stream.recv(). Calling frontend.recv()\n"));

  MC_Network::network->recv();
  mc_printf((stderr, "network.recv() returned\n"));

  if(!IncomingPacketBuffer.isEmpty() ){
    mc_printf((stderr, "stream has packets\n"));
    List<MC_Packet *>::iterator iter = IncomingPacketBuffer.begin();

    cur_packet = *iter;
    *tag = cur_packet->get_Tag();
    *ptr = (void *) cur_packet;
  }

  cur_stream_idx++;
  cur_stream_idx %= streams.size();

  if(cur_packet){
    mc_printf((stderr, "cur_packet's tag: %d\n", cur_packet->get_Tag()));
    assert( IncomingPacketBuffer.remove(cur_packet) );
    return 1;
  }
  else{
    mc_printf((stderr, "No packets currently on stream\n"));
    return 0;
  }
}

MC_Stream * MC_Stream::get_Stream(int stream_id)
{
  MC_Stream *stream = streams[stream_id];
  if(stream){
    return stream;
  }
  else{
    //remove stream_id from list since [] operator will add if not found.
    streams.get_and_remove(stream_id);
    return NULL;
  }
}

void MC_Stream::add_IncomingPacket(MC_Packet *packet)
{
  IncomingPacketBuffer.push_back(packet);
}

vector <MC_EndPoint *> * MC_Stream::get_EndPoints()
{
  return communicator->get_EndPoints();
}

/*===========================================================*/
/*      MC_Communicator CLASS METHOD DEFINITIONS             */
/*===========================================================*/

void MC_Communicator::create_BroadcastCommunicator(vector <MC_EndPoint *> * endpoints)
{
  unsigned int i;
  comm_Broadcast = new MC_Communicator();

  mc_printf((stderr, "In create_broadcastcomm()\n"));

  for(i=0; i<endpoints->size(); i++){
    mc_printf((stderr, "adding enpoint %s:%d[%d]\n",
              (*endpoints)[i]->get_HostName().c_str(),
              (*endpoints)[i]->get_Port(), (*endpoints)[i]->get_Id()));
    comm_Broadcast->add_EndPoint( (*endpoints)[i] );
  }
  return;  
}

MC_Communicator * MC_Communicator::get_BroadcastCommunicator()
{
  return comm_Broadcast;
}

MC_Communicator::MC_Communicator()
{}

int MC_Communicator::add_EndPoint(string _hostname, unsigned short _port)
{
  MC_EndPoint * new_endpoint = MC_Network::network->get_EndPoint(_hostname, _port);

  if(new_endpoint == NULL){
    return -1;
  }

  endpoints += new_endpoint;
  return 0;
}

int MC_Communicator::add_EndPoint(MC_EndPoint * new_endpoint)
{
  if(new_endpoint){
    endpoints += new_endpoint;
    return 0;
  }
  else{
    return -1;
  }
}

int MC_Communicator::size(void){
  return MC_Network::network->endpoints->size();
}

string MC_Communicator::get_HostName(int id)
{
  if( (unsigned int)id >= MC_Network::network->endpoints->size() )
    return string("");  //Needs better error code

  return (*MC_Network::network->endpoints)[id]->get_HostName();
}

unsigned short MC_Communicator::get_Port(int id)
{
  if( (unsigned int)id >= MC_Network::network->endpoints->size() )
    return 0;  //Needs better error code

  return (*MC_Network::network->endpoints)[id]->get_Port();
}

unsigned int MC_Communicator::get_Id(int id)
{
  if( (unsigned int)id >= MC_Network::network->endpoints->size() )
    return 0;  //Needs better error code

  return (*MC_Network::network->endpoints)[id]->get_Id();
}

vector <MC_EndPoint *> * MC_Communicator::get_EndPoints()
{
  return &endpoints;
}

/*===========================================================*/
/*            MC_EndPoint CLASS METHOD DEFINITIONS            */
/*===========================================================*/
MC_EndPoint::MC_EndPoint(int _id, string _hostname, unsigned short _port)
  :id(_id), hostname(_hostname), port(_port)
{
  mc_printf((stderr, "In endpoint() %s:%d [%d]\n", hostname.c_str(), port, id));
}

string MC_EndPoint::get_HostName()
{
  return hostname;
}

unsigned short MC_EndPoint::get_Port()
{
  return port;
}

unsigned int MC_EndPoint::get_Id()
{
  return id;
}

bool MC_EndPoint::compare(string _hostname, unsigned short _port)
{
  mc_printf((stderr, "endpoint.compare(): %s:%d, %s:%d\n",
             hostname.c_str(), port, _hostname.c_str(), _port));
  if( (_hostname == hostname) &&
      (_port == port) ) {
    return true;
  }
  else{
    return false;
  }
}

