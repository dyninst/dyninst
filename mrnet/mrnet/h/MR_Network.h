#if !defined(mc_network_h)
#define mc_network_h 1

#if !defined(NULL)
#  define NULL (0)
#endif // NULL

#include "mrnet/src/FilterDefinitions.h"
namespace MRN
{
class NetworkImpl;
class BackEndNode;

const int FIRST_CTL_TAG=100;
const int FIRST_APPL_TAG=200;

class Network{
 public:
    class LeafInfo
    {
    public:
        virtual const char* get_Host( void ) const = NULL;
        virtual unsigned short get_Rank( void ) const   = NULL;
        virtual unsigned short get_Id( void ) const   = NULL;
        virtual const char* get_ParHost( void ) const   = NULL;
        virtual unsigned short get_ParPort( void ) const   = NULL;
        virtual unsigned short get_ParRank( void ) const   = NULL;
    };

  static int new_Network(const char * _filename, const char * _backend);
  static int new_NetworkNoBE(const char * _filename,
			     LeafInfo*** leafInfo,
			     unsigned int* nLeaves );
  static int connect_Backends( void );
  static void delete_Network();

  static int init_Backend(const char *hostname, const char *port,
                          const char *phostname, const char *pport,
                          const char *pid);
  static int init_Backend(const char *hostname, unsigned int backend_id,
                          const char *phostname, unsigned int pport,
                          unsigned int pid);
  static int getConnections( int** conns, unsigned int* nConns );
  static void error_str(const char *);

  static NetworkImpl * network;
  static BackEndNode * back_end;
};

class EndPoint{
 public:
  static EndPoint * new_EndPoint(int _id, const char * _hostname,
                                    unsigned short _port);
  virtual bool compare(const char * _hostname, unsigned short _port) =NULL;
  virtual const char * get_HostName() =NULL;
  virtual unsigned short get_Port() =NULL;
  virtual unsigned int get_Id() =NULL;
};

class Communicator{
 public:
  static Communicator * new_Communicator();
  static Communicator * new_Communicator(Communicator &);
  static Communicator * get_BroadcastCommunicator();

  virtual int add_EndPoint(const char * hostname, unsigned short port)=NULL;
  virtual int add_EndPoint(EndPoint *)=NULL;
  virtual unsigned int size() const =NULL;
  virtual const char * get_HostName(int) const =NULL; 
  virtual unsigned short get_Port(int) const =NULL;
  virtual unsigned int get_Id(int) const =NULL;
};

class Stream{
 public:
  static Stream * new_Stream(Communicator *, int us_filter_id=AGGR_NULL,
			     int _sync_id=SYNC_WAITFORALL,
                 int _ds_filter_id=AGGR_NULL);
  static int recv(int *tag, void **buf, Stream ** stream, bool blocking=true);
  static int unpack(void * buf, const char * format_str, ...);
  static void set_BlockingTimeOut(int timeout);
  static int get_BlockingTimeOut( );

  virtual int send(int tag, const char * format_str, ...)=NULL;
  virtual int flush()=NULL;
  virtual int recv(int *tag, void **buf, bool blocking=true)=NULL;
  virtual unsigned int get_NumEndPoints()=NULL;
  virtual Communicator* get_Communicator()=NULL;

  //static Stream * get_Stream(int stream_id);
  //static Stream * new_Stream(int stream_id, int * backends=NULL,
  // int num_backends=-1, int filter_id=-1)=NULL;
};

} /* namespace MRN */

#endif /* mrnet_H */
