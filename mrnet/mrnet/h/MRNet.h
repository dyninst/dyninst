#if !defined(mc_network_h)
#define mc_network_h 1

#if !defined(NULL)
#  define NULL (0)
#endif // NULL

#include <string>
#include "mrnet/src/FilterDefinitions.h"
namespace MRN
{
class NetworkImpl;
class BackEndNode;

const int FIRST_CTL_TAG=100;
const int FIRST_APPL_TAG=200;

class Network{
    friend class NetworkImpl;

 private:
    Network(){} //explicitly disallow creation without "new_Network()"

 public:
    class LeafInfo {
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
    friend class EndPointImpl;

 private:
    EndPoint(){} //explicitly disallow creation without "new_EndPoint()"

 public:
    static EndPoint * new_EndPoint(int _id, const char * _hostname,
                                   unsigned short _port);
    virtual bool compare(const char * _hostname, unsigned short _port)const=NULL;
    virtual const char * get_HostName()const=NULL;
    virtual unsigned short get_Port()const=NULL;
    virtual unsigned int get_Id()const=NULL;
};

class Communicator{
    friend class CommunicatorImpl;

 private:
    Communicator(){} //explicitly disallow creation without "new_Communicator()"

 public:
    static Communicator * new_Communicator();
    static Communicator * new_Communicator(Communicator &);
    static Communicator * get_BroadcastCommunicator();

    virtual int add_EndPoint(const char * hostname, unsigned short port)=NULL;
    virtual void add_EndPoint(EndPoint *)=NULL;

    virtual unsigned int size() const=NULL;
    virtual const char * get_HostName(int) const=NULL; 
    virtual unsigned short get_Port(int) const=NULL;
    virtual unsigned int get_Id(int) const=NULL;
};

class Stream{
    friend class StreamImpl;
 private:
    Stream(){} //explicitly disallow creation without "new_Stream()"

 public:
    static Stream * new_Stream(Communicator *, int us_filter_id=TFILTER_NULL,
                               int _sync_id=SFILTER_WAITFORALL,
                               int _ds_filter_id=TFILTER_NULL);
    static int recv(int *tag, void **buf, Stream ** stream, bool blocking=true);
    static int unpack(void * buf, const char * format_str, ...);
    static void set_BlockingTimeOut(int timeout);
    static int get_BlockingTimeOut( );
    static int load_FilterFunc( const char * so_file, const char * func,
                                bool is_trans_filter=true );
    static Stream* get_Stream( unsigned int id );

    virtual int send(int tag, const char * format_str, ...)const=NULL;
    virtual int flush()const=NULL;
    virtual int recv(int *tag, void **buf, bool blocking=true)=NULL;
    virtual unsigned int get_NumEndPoints()const=NULL;
    virtual Communicator* get_Communicator()const=NULL;
    virtual unsigned int get_Id( void ) const=NULL;
};

typedef enum {
    EBADCONFIG,
    ESYSTEM,
    EPACKING,
    EFMTSTR,
    EPROTOCOL,
    UNKNOWN_EVENT
} EventType;

extern std::string LocalHostName;
extern unsigned short LocalPort;

class Event{
    friend class EventImpl;
 private:
    Event(){} //explicitly disallow creation without "new_Event()"

 public:
    static Event * new_Event( EventType t, std::string desc="",
                              std::string h=LocalHostName,
                              unsigned short p=LocalPort );
    static bool have_Event();
    static bool have_RemoteEvent();
    static void add_Event( Event & );
    static Event * get_NextEvent();
    static Event * get_NextRemoteEvent();
    static unsigned int get_NumEvents();
    static unsigned int get_NumRemoteEvents();

    virtual EventType get_Type( )=NULL;
    virtual const std::string & get_HostName( )=NULL;
    virtual unsigned short get_Port( )=NULL;
    virtual const std::string & get_Description( )=NULL;
};

} /* namespace MRN */

#endif /* mrnet_H */
