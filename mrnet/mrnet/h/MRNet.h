/***********************************************************************
 * Copyright © 2003-2004 Dorian C. Arnold, Philip C. Roth, Barton P. Miller *
 *                  Detailed MRNet usage rights in "LICENSE" file.     *
 **********************************************************************/

#if !defined(mrnet_h)
#define mrnet_h 1

#if !defined(NULL)
#  define NULL (0)
#endif // NULL

#include <string>
#include <vector>
#include "mrnet/src/FilterDefinitions.h"
namespace MRN
{
const int FIRST_CTL_TAG=100;
const int FIRST_APPL_TAG=200;

class EndPointImpl;
class EndPoint{
    friend class Network;

 private:
    EndPoint(int _id, const char * _hostname, unsigned short _port);
    EndPointImpl * endpoint;

 public:
    ~EndPoint();
    bool compare(const char * _hostname, unsigned short _port)const;
    const char * get_HostName()const;
    unsigned short get_Port()const;
    unsigned int get_Id()const;
};

class CommunicatorImpl;
class Network;
class Communicator{
    friend class Network;

 private:
    CommunicatorImpl * communicator;
    Communicator( Network * );
    Communicator( Network *, Communicator &);
    Communicator( Network *, std::vector<EndPoint *>& );

 public:

    ~Communicator();

    int add_EndPoint(const char * hostname, unsigned short port);
    void add_EndPoint(EndPoint *);
    unsigned int size( void ) const;
    const char * get_HostName(int) const; 
    unsigned short get_Port(int) const;
    unsigned int get_Id(int) const;
    const std::vector<EndPoint *> & get_EndPoints( void ) const;
};

class StreamImpl;
class Stream{
    friend class Network;
    friend class FrontEndNode;
    friend class BackEndNode;
    friend class NetworkImpl;
 private:
    StreamImpl * stream;
    Stream(Network *, Communicator *, int us_filter_id=TFILTER_NULL,
           int _sync_id=SFILTER_WAITFORALL, int _ds_filter_id=TFILTER_NULL);
    Stream( Network *, int stream_id, int *backends = 0,
            int num_backends = -1, int ds_filter_id=TFILTER_NULL,
            int sync_id = SFILTER_DONTWAIT, int us_filter_id=TFILTER_NULL );
    Packet get_IncomingPacket();
    void add_IncomingPacket( Packet& );

 public:
    static int unpack(void * buf, const char * format_str, ...);

    void set_BlockingTimeOut(int timeout);
    int get_BlockingTimeOut( );
    int send(int tag, const char * format_str, ...)const;
    int flush()const;
    int recv(int *tag, void **buf, bool blocking=true);
    unsigned int get_NumEndPoints()const;
    Communicator* get_Communicator()const;
    unsigned int get_Id( void ) const;
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

class NetworkImpl;
class FrontEndNode;
class BackEndNode;
class Packet;
class Network{
    friend class StreamImpl;

 private:
    FrontEndNode * get_FrontEndNode( void );
    BackEndNode * get_BackEndNode( void );
    NetworkImpl * network;
    static unsigned int next_stream_id;

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

    Network(const char * _filename, const char * _backend);
    Network(const char * _filename, LeafInfo*** leafInfo,
            unsigned int* nLeaves );
    Network(const char *hostname, const char *port,
            const char *phostname, const char *pport,
            const char *pid);
    Network(const char *hostname, unsigned int backend_id,
            const char *phostname, unsigned int pport,
            unsigned int pid);
    ~Network();

    int connect_Backends( void );
    int getConnections( int** conns, unsigned int* nConns );

    int send(Packet &);
    int recv(bool blocking=true);
    int recv(int *tag, void **buf, Stream ** stream, bool blocking=true);
    int load_FilterFunc( const char * so_file, const char * func,
                                bool is_trans_filter=true );
    Communicator * get_BroadcastCommunicator( void );

    EndPoint * get_EndPoint(const char*, short unsigned int);
    Communicator * new_Communicator( void );
    Communicator * new_Communicator( Communicator& );
    Communicator * new_Communicator( std::vector <EndPoint *> & );

    static EndPoint * new_EndPoint(int id, const char * hostname, unsigned short port);

    Stream * new_Stream( Communicator *,
                         int us_filter_id=TFILTER_NULL,
                         int sync_id=SFILTER_WAITFORALL,
                         int ds_filter_id=TFILTER_NULL );

    Stream * new_Stream( int stream_id, int *backends=NULL,
                         int num_backends=-1,
                         int us_filter_id=TFILTER_NULL,
                         int sync_id=SFILTER_DONTWAIT,
                         int ds_filter_id=TFILTER_NULL );
    Stream *get_Stream( int stream_id );

    bool is_FrontEnd( void );
    bool is_BackEnd( void );
    bool fail( void );
    bool good( void );
    void error_str( const char * );
};

} /* namespace MRN */

#endif /* mrnet_h */
