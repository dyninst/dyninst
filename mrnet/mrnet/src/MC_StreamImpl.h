#if !defined(__mc_streamimpl_h)
#define __mc_streamimpl_h 1

#include <list>
#include <vector>

#include <map>

#include "mrnet/h/MC_Network.h"
#include "mrnet/src/MC_Message.h"
#include "mrnet/src/MC_CommunicatorImpl.h"

class MC_StreamImpl: public MC_Stream {
  friend class MC_Network;

 private:
  /* "Registered" streams */
  static std::map <unsigned int, MC_StreamImpl *> streams;
  static unsigned int cur_stream_idx, next_stream_id;
  std::list <MC_Packet *> IncomingPacketBuffer;
  unsigned short filter_id;
  unsigned short stream_id;
  MC_CommunicatorImpl * communicator;

 public:
  MC_StreamImpl(MC_Communicator *, int _filter_id);
  virtual ~MC_StreamImpl();
  MC_StreamImpl(int stream_id, int * backends=0, int num_backends=-1, int filter_id=-1);
  static int recv(int *tag, void **buf, MC_Stream ** stream);
  //static int unpack(char * buf, const char * format_str, ...);
  static MC_StreamImpl * get_Stream(int stream_id);

  virtual int send(int tag, const char * format_str, ...);
  virtual int flush();
  virtual int recv(int *tag, void **buf);
  void add_IncomingPacket(MC_Packet *);
  std::vector <MC_EndPoint *> * get_EndPoints();
};

#endif /* __mc_streamimpl_h */
