/*===========================================================*/
/*             StreamImpl CLASS METHOD DEFINITIONS            */
/*===========================================================*/

#include <stdarg.h>
#include "mrnet/src/StreamImpl.h"
#include "mrnet/src/NetworkImpl.h"
#include "mrnet/src/utils.h"

unsigned int StreamImpl::cur_stream_idx=0;
unsigned int StreamImpl::next_stream_id=0;
std::map <unsigned int, StreamImpl *>* StreamImpl::streams = NULL;

StreamImpl::StreamImpl(Communicator *_comm, int _filter_id)
  :filter_id(_filter_id)
{
  communicator = new CommunicatorImpl(*_comm); //copy the comm.

  stream_id = next_stream_id++;

  if( StreamImpl::streams == NULL )
  {
    StreamImpl::streams = 
        new std::map<unsigned int, StreamImpl*>;
  }
  (*StreamImpl::streams)[stream_id] = this;

  if ( Network::network ){
    const std::vector <EndPoint*>* endpoints = communicator->get_EndPoints();
    int * backends = new int[endpoints->size()];
    unsigned int i;

    mrn_printf(3, MCFL, stderr, "Adding backends to stream %d: [ ", stream_id);
    for(i=0; i<endpoints->size(); i++){
      _fprintf((stderr, "%d, ", (*endpoints)[i]->get_Id()));
      backends[i] = (*endpoints)[i]->get_Id();
    }
    _fprintf((stderr, "]\n"));

    Packet * packet = new Packet(MRN_NEW_STREAM_PROT, "%d %ad %d",
                                       stream_id, backends, endpoints->size(),
                                       filter_id);
    StreamManager * stream_mgr;
    stream_mgr = Network::network->front_end->proc_newStream(packet);
    Network::network->front_end->send_newStream(packet, stream_mgr);
  }
}

StreamImpl::StreamImpl(int _stream_id, int * backends, int num_backends,
                     int _filter_id)
  :filter_id(_filter_id), stream_id(_stream_id)
{
    if( StreamImpl::streams == NULL )
    {
        StreamImpl::streams =
            new std::map<unsigned int,StreamImpl*>;
    }
    (*StreamImpl::streams)[stream_id] = this;
}

StreamImpl::~StreamImpl()
{
}

int StreamImpl::recv(int *tag, void **ptr, Stream **stream)
{
  unsigned int start_idx;
  StreamImpl * cur_stream=NULL;
  Packet * cur_packet=NULL;

  mrn_printf(3, MCFL, stderr, "In stream.recv(). Calling network.recv()\n");

    if( streams == NULL )
    {
        streams = new 
            std::map<unsigned int, StreamImpl*>;
    }

 get_packet_from_stream_label:
  start_idx = cur_stream_idx;
  do{
    cur_stream = (*StreamImpl::streams)[cur_stream_idx];
    if(!cur_stream){
      cur_stream_idx++;
      cur_stream_idx %= streams->size();
      continue;
    }

    mrn_printf(3, MCFL, stderr, "Checking stream[%d] for packets ...", cur_stream_idx);
    if( cur_stream->IncomingPacketBuffer.size() != 0 ){
      _fprintf((stderr, "found %d packets\n",
                (int)cur_stream->IncomingPacketBuffer.size()));

      std::list<Packet *>::iterator iter = cur_stream->IncomingPacketBuffer.begin();

      cur_packet = *iter;
      *tag = cur_packet->get_Tag();
      *stream = (*StreamImpl::streams)[cur_packet->get_StreamId()];
      *ptr = (void *) cur_packet;
      cur_stream_idx++;
      cur_stream_idx %= streams->size();

      cur_stream->IncomingPacketBuffer.pop_front();
      break;
    }
    _fprintf((stderr, "No Packets found\n"));

    cur_stream_idx++;
    cur_stream_idx %= streams->size();
  } while(start_idx != cur_stream_idx);

  if(cur_packet){
    mrn_printf(3, MCFL, stderr, "cur_packet(%p) tag: %d, fmt: %s\n", cur_packet,
               cur_packet->get_Tag(), cur_packet->get_FormatString());
    mrn_printf(3, MCFL, stderr, "%d packets left\n",
              cur_stream->IncomingPacketBuffer.size());
    return 1;
  }
  else{
    mrn_printf(3, MCFL, stderr, "No packets currently on any stream\n");

	// blocking receive?
    int rret = Network::network->recv();
	if( rret == 0 )
	{
		// broken connection indicator seen - exit the recv
		mrn_printf(1, MCFL, stderr, "broken connection 0, leaving stream::recv\n" );
		return 0;
	}
	else if( rret == -1 )
	{
		// broken connection indicator seen - exit the recv
		mrn_printf(1, MCFL, stderr, "broken connection -1, leaving stream::recv\n" );
		return -1;
	}
    goto get_packet_from_stream_label;
    return 0;
  }
}


int StreamImpl::send_aux(int tag, char const * fmt, va_list arg_list )
{
  Packet* packet = new Packet(stream_id, tag, fmt, arg_list);
  if(packet->fail()){
    mrn_printf(1, MCFL, stderr, "new packet() fail\n");
    return -1;
  }
  mrn_printf(3, MCFL, stderr, "new packet() succeeded. Calling frontend.send()\n");
  int status = Network::network->send(packet);
  return status;
}


int StreamImpl::send(int tag, char const * fmt, ...)
{
  int status;
  va_list arg_list;

  mrn_printf(3, MCFL, stderr, "In stream[%d].send(). Calling new packet()\n", stream_id);

  va_start(arg_list, fmt);
  status = send_aux( tag, fmt, arg_list );
  va_end(arg_list);

  mrn_printf(3, MCFL, stderr, "network.send() %s",
             (status==-1 ? "failed\n" : "succeeded\n"));
  return status;
}

int StreamImpl::flush()
{
  if(Network::network){
    return Network::network->front_end->flush(stream_id);
  }

  return Network::back_end->flush();
}


int StreamImpl::recv(int *tag, void ** ptr)
{
  Packet * cur_packet=NULL;

  mrn_printf(3, MCFL, stderr, "In stream.recv(). Calling network.recv()\n");

  if( IncomingPacketBuffer.size() == 0){
      Network::network->recv();
  }
  else{
    mrn_printf(3, MCFL, stderr, "stream has packets\n");
    std::list<Packet *>::iterator iter = IncomingPacketBuffer.begin();

    cur_packet = *iter;
    *tag = cur_packet->get_Tag();
    *ptr = (void *) cur_packet;
  }

  cur_stream_idx++;
  cur_stream_idx %= streams->size();

  int ret = 0;
  if(cur_packet){
    mrn_printf(3, MCFL, stderr, "cur_packet's tag: %d\n", cur_packet->get_Tag());
    IncomingPacketBuffer.remove(cur_packet);
    ret = 1;
  }
  else{
    mrn_printf(3, MCFL, stderr, "No packets currently on stream\n");
  }

  return ret;
}

StreamImpl * StreamImpl::get_Stream(int stream_id)
{
    assert( streams != NULL );
  StreamImpl *stream = (*streams)[stream_id];
  if(stream){
    return stream;
  }
  else{
    streams->erase(stream_id);
    return NULL;
  }
}

void StreamImpl::add_IncomingPacket(Packet *packet)
{
  IncomingPacketBuffer.push_back(packet);
}

const std::vector <EndPoint *> * StreamImpl::get_EndPoints() const
{
  return communicator->get_EndPoints();
}

int
StreamImpl::unpack( char* buf, const char* fmt_str, va_list arg_list )
{
  Packet * packet = (Packet *)buf;
  int ret = packet->ExtractVaList(fmt_str, arg_list); 

  return ret;
}
