#if !defined(mc_network_h)
#define mc_network_h 1

class MC_NetworkImpl;
class MC_BackEndNode;
class MC_Network{
 public:
  static MC_NetworkImpl * network;
  static MC_BackEndNode * back_end;
  static int new_Network(const char * _filename, const char * _application);
  static void delete_Network();
  static int init_Backend(const char *hostname, const char *port,
                          const char *phostname, const char *pport,
                          const char *pid);
  static void error_str(const char *);
};

class MC_EndPoint{
 public:
  static MC_EndPoint * new_EndPoint(int _id, const char * _hostname,
                                    unsigned short _port);
  virtual bool compare(const char * _hostname, unsigned short _port) =0;
  virtual const char * get_HostName() =0;
  virtual unsigned short get_Port() =0;
  virtual unsigned int get_Id() =0;
};

class MC_Communicator{
 public:
  static MC_Communicator * new_Communicator();
  static MC_Communicator * new_Communicator(MC_Communicator &);
  static MC_Communicator * get_BroadcastCommunicator();

  virtual int add_EndPoint(const char * hostname, unsigned short port)=0;
  virtual int add_EndPoint(MC_EndPoint *)=0;
  virtual int size()=0;
  virtual const char * get_HostName(int)=0; 
  virtual unsigned short get_Port(int)=0;
  virtual unsigned int get_Id(int)=0;
};

class MC_Stream{
 public:
  static MC_Stream * new_Stream(MC_Communicator *, int _filter_id);
  static int recv(int *tag, void **buf, MC_Stream ** stream);
  static int unpack(char * buf, const char * format_str, ...);

  virtual int send(int tag, const char * format_str, ...)=0;
  virtual int flush()=0;
  virtual int recv(int *tag, void **buf)=0;

  //static MC_Stream * get_Stream(int stream_id);
  //static MC_Stream * new_Stream(int stream_id, int * backends=0,
  // int num_backends=-1, int filter_id=-1)=0;
};

#endif /* mrnet_H */
