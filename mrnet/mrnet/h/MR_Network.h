#if !defined(mc_network_h)
#define mc_network_h 1

#ifndef NULL
#  define NULL (0)
#endif // NULL

class MC_NetworkImpl;
class MC_BackEndNode;




class MC_Network{
 public:
    class LeafInfo
    {
    public:
        virtual const char* get_Host( void ) const = NULL;
        virtual unsigned short  get_Port( void ) const = NULL;
    };

  static int new_Network(const char * _filename, 
                            const char * _commnode,
                            const char * _application);
  static int new_Network(const char * _filename,
                            const char * _commnode,
                            MC_Network::LeafInfo*** leaves,
                            unsigned int* numLeaves);
  static int connect_Backends( void );
  static void delete_Network();

  static int init_Backend(const char *hostname, const char *port,
                          const char *phostname, const char *pport,
                          const char *pid);
  static void error_str(const char *);

  static MC_NetworkImpl * network;
  static MC_BackEndNode * back_end;
};

class MC_EndPoint{
 public:
  static MC_EndPoint * new_EndPoint(int _id, const char * _hostname,
                                    unsigned short _port);
  virtual bool compare(const char * _hostname, unsigned short _port) =NULL;
  virtual const char * get_HostName() =NULL;
  virtual unsigned short get_Port() =NULL;
  virtual unsigned int get_Id() =NULL;
};

class MC_Communicator{
 public:
  static MC_Communicator * new_Communicator();
  static MC_Communicator * new_Communicator(MC_Communicator &);
  static MC_Communicator * get_BroadcastCommunicator();

  virtual int add_EndPoint(const char * hostname, unsigned short port)=NULL;
  virtual int add_EndPoint(MC_EndPoint *)=NULL;
  virtual int size()=NULL;
  virtual const char * get_HostName(int)=NULL; 
  virtual unsigned short get_Port(int)=NULL;
  virtual unsigned int get_Id(int)=NULL;
};

class MC_Stream{
 public:
  static MC_Stream * new_Stream(MC_Communicator *, int _filter_id);
  static int recv(int *tag, void **buf, MC_Stream ** stream);
  static int unpack(char * buf, const char * format_str, ...);

  virtual int send(int tag, const char * format_str, ...)=NULL;
  virtual int flush()=NULL;
  virtual int recv(int *tag, void **buf)=NULL;

  //static MC_Stream * get_Stream(int stream_id);
  //static MC_Stream * new_Stream(int stream_id, int * backends=NULL,
  // int num_backends=-1, int filter_id=-1)=NULL;
};

#endif /* mrnet_H */
