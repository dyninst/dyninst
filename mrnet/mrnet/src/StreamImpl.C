/*===========================================================*/
/*             MC_StreamImpl CLASS METHOD DEFINITIONS            */
/*===========================================================*/

#include <stdarg.h>
#include "mrnet/src/MC_StreamImpl.h"
#include "mrnet/src/MC_NetworkImpl.h"
#include "mrnet/src/utils.h"

unsigned int MC_StreamImpl::cur_stream_idx=0;
unsigned int MC_StreamImpl::next_stream_id=0;
std::map <unsigned int, MC_StreamImpl *> MC_StreamImpl::streams;

MC_StreamImpl::MC_StreamImpl(MC_Communicator *_comm, int _filter_id)
  :filter_id(_filter_id)
{
  communicator = new MC_CommunicatorImpl(*_comm); //copy the comm.
  //mc_printf(MCFL, stderr, "old comm(%p) endpoint:%p, new comm(%p) endpoint:%p\n",
	     //_comm, ((MC_CommunicatorImpl*)(_comm))->get_EndPoints(),
	     //communicator, communicator->get_EndPoints());

  stream_id = next_stream_id++;
  MC_StreamImpl::streams[stream_id] = this;

  if ( MC_Network::network ){
    std::vector <MC_EndPoint *> * endpoints = communicator->get_EndPoints();
    int * backends = (int *) malloc (sizeof(int) * endpoints->size());
    unsigned int i;

    mc_printf(MCFL, stderr, "Adding backends to stream %d: [ ", stream_id);
    for(i=0; i<endpoints->size(); i++){
      _fprintf((stderr, "%d, ", (*endpoints)[i]->get_Id()));
      backends[i] = (*endpoints)[i]->get_Id();
    }
    _fprintf((stderr, "]\n"));

    MC_Packet * packet = new MC_Packet(MC_NEW_STREAM_PROT, "%d %ad %d",
                                       stream_id, backends, endpoints->size(),
                                       filter_id);
    MC_Network::network->front_end->proc_newStream(packet);
  }
}

MC_StreamImpl::MC_StreamImpl(int _stream_id, int * backends, int num_backends,
                     int _filter_id)
  :filter_id(_filter_id), stream_id(_stream_id)
{
  MC_StreamImpl::streams[stream_id] = this;
}

MC_StreamImpl::~MC_StreamImpl()
{
}

int MC_StreamImpl::recv(int *tag, void **ptr, MC_Stream **stream)
{
  unsigned int start_idx;
  MC_StreamImpl * cur_stream=NULL;
  MC_Packet * cur_packet=NULL;

  mc_printf(MCFL, stderr, "In stream.recv(). Calling network.recv()\n");
  MC_Network::network->recv();

  start_idx = cur_stream_idx;
  do{
    cur_stream = MC_StreamImpl::streams[cur_stream_idx];
    if(!cur_stream){
      cur_stream_idx++;
      cur_stream_idx %= streams.size();
      continue;
    }

    mc_printf(MCFL, stderr, "Checking stream[%d] for packets ...", cur_stream_idx);
    if( cur_stream->IncomingPacketBuffer.size() != 0 ){
      mc_printf(MCFL, stderr, "found %d packets\n",
                cur_stream->IncomingPacketBuffer.size());
      std::list<MC_Packet *>::iterator iter = cur_stream->IncomingPacketBuffer.begin();

      cur_packet = *iter;
      *tag = cur_packet->get_Tag();
      *stream = MC_StreamImpl::streams[cur_packet->get_StreamId()];
      *ptr = (void *) cur_packet;
      cur_stream_idx++;
      cur_stream_idx %= streams.size();

      cur_stream->IncomingPacketBuffer.pop_front();
      break;
    }
    mc_printf(MCFL, stderr, "No Packets found\n");

    cur_stream_idx++;
    cur_stream_idx %= streams.size();
  } while(start_idx != cur_stream_idx);

  if(cur_packet){
    mc_printf(MCFL, stderr, "cur_packet(%p) tag: %d, fmt: %s\n", cur_packet,
               cur_packet->get_Tag(), cur_packet->get_FormatString());
    mc_printf(MCFL, stderr, "%d packets left\n",
              cur_stream->IncomingPacketBuffer.size());
    return 1;
  }
  else{
    mc_printf(MCFL, stderr, "No packets currently on stream\n");
    return 0;
  }
}


int MC_StreamImpl::send(int tag, char const * fmt, ...)
{
  int status;
  va_list arg_list;
  MC_Packet * packet;

  mc_printf(MCFL, stderr, "In stream[%d].send(). Calling new packet()\n", stream_id);

  va_start(arg_list, fmt);
  packet = new MC_Packet(stream_id, tag, fmt, arg_list);
  if(packet->fail()){
    mc_printf(MCFL, stderr, "new packet() fail\n");
    return -1;
  }
  mc_printf(MCFL, stderr, "new packet() succeeded. Calling frontend.send()\n");
  status = MC_Network::network->send(packet);
  va_end(arg_list);
  mc_printf(MCFL, stderr, "network.send() %s",
             (status==-1 ? "failed\n" : "succeeded\n"));
  return status;
}

int MC_StreamImpl::flush()
{
  if(MC_Network::network){
    return MC_Network::network->front_end->flush(stream_id);
  }

  return MC_Network::back_end->flush();
}

int MC_StreamImpl::recv(int *tag, void ** ptr)
{
  MC_Packet * cur_packet=NULL;

  mc_printf(MCFL, stderr, "In stream.recv(). Calling frontend.recv()\n");

  MC_Network::network->recv();
  mc_printf(MCFL, stderr, "network.recv() returned\n");

  if( IncomingPacketBuffer.size() != 0){
    mc_printf(MCFL, stderr, "stream has packets\n");
    std::list<MC_Packet *>::iterator iter = IncomingPacketBuffer.begin();

    cur_packet = *iter;
    *tag = cur_packet->get_Tag();
    *ptr = (void *) cur_packet;
  }

  cur_stream_idx++;
  cur_stream_idx %= streams.size();

  if(cur_packet){
    mc_printf(MCFL, stderr, "cur_packet's tag: %d\n", cur_packet->get_Tag());
    IncomingPacketBuffer.remove(cur_packet);
    return 1;
  }
  else{
    mc_printf(MCFL, stderr, "No packets currently on stream\n");
    return 0;
  }
}

MC_StreamImpl * MC_StreamImpl::get_Stream(int stream_id)
{
  MC_StreamImpl *stream = streams[stream_id];
  if(stream){
    return stream;
  }
  else{
    streams.erase(stream_id);
    return NULL;
  }
}

void MC_StreamImpl::add_IncomingPacket(MC_Packet *packet)
{
  IncomingPacketBuffer.push_back(packet);
}

std::vector <MC_EndPoint *> * MC_StreamImpl::get_EndPoints()
{
  return communicator->get_EndPoints();
}
