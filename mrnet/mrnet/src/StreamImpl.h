#if !defined(__streamimpl_h)
#define __streamimpl_h 1

#include <list>
#include <vector>

#include <map>

#include "mrnet/h/MR_Network.h"
using namespace MRN;
#include "mrnet/src/Message.h"
#include "mrnet/src/CommunicatorImpl.h"

class StreamImpl: public Stream {
  friend class Network;

 private:
  /* "Registered" streams */
  static std::map<unsigned int, StreamImpl*>* streams;
  static unsigned int cur_stream_idx, next_stream_id;
  static bool force_network_recv;

  std::list <Packet *> IncomingPacketBuffer;
  unsigned short filter_id;
  unsigned short stream_id;
  CommunicatorImpl * communicator;

 public:
  StreamImpl(Communicator *, int _filter_id);
  virtual ~StreamImpl();
  StreamImpl(int stream_id, int * backends=0, int num_backends=-1, int filter_id=-1);
  static int recv(int *tag, void **buf, Stream ** stream, bool blocking=true);
  static StreamImpl * get_Stream(int stream_id);
  static void set_ForceNetworkRecv( bool force=true );

  virtual int send(int tag, const char * format_str, ...);
  virtual int flush();
  virtual int recv(int *tag, void **buf, bool blocking=true);
  void add_IncomingPacket(Packet *);
  const std::vector <EndPoint *> * get_EndPoints() const;


    int send_aux(int tag, const char * format_str, va_list arg_list ); 
    static int unpack(char* buf, const char* fmt, va_list arg_list );
};

#endif /* __streamimpl_h */
