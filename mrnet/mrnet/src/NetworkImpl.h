/***********************************************************************
 * Copyright © 2003-2004 Dorian C. Arnold, Philip C. Roth, Barton P. Miller *
 *                  Detailed MRNet usage rights in "LICENSE" file.     *
 **********************************************************************/

#if !defined(__networkimpl_h)
#define __networkimpl_h 1

#include <vector>
#include <string>

#include "mrnet/MRNet.h"

#include "mrnet/src/Errors.h"
#include "mrnet/src/Message.h"
#include "mrnet/src/NetworkGraph.h"
#include "mrnet/src/BackEndNode.h"
#include "mrnet/src/FrontEndNode.h"


namespace MRN
{

class NetworkImpl: public Error {
    friend class Network;
 public:
    class LeafInfoImpl : public Network::LeafInfo {
    private:
        char* host;
        Port port;
        Rank rank;
        char* phost;
        Port pport;

    public:
        LeafInfoImpl( const char* _host, Port _port, Rank _rank,
                        const char* _phost, Port _pport );
        virtual ~LeafInfoImpl( void );
      
        virtual const char* get_Host( void ) const      { return host; }
        virtual Port get_Port( void ) const   { return port; }
        virtual Rank get_Rank( void ) const   { return rank; }
        virtual const char* get_ParHost( void ) const   { return phost; }
        virtual Port get_ParPort( void ) const   { return pport; }
    };

 private:
    std::string filename;          /* Name of topology configuration file */
    std::string application;       /* Name of application to launch */
    std::vector <EndPoint *> endpoints; //BackEnds addressed by communicator
    Communicator * comm_Broadcast;
    FrontEndNode *front_end;
    BackEndNode * back_end;

    //There is both is_backend and is_frontend to detect the case
    //when neither has been initialized.
    static bool is_backend;
    static bool is_frontend;
    FrontEndNode * get_FrontEndNode( void ) { return front_end; }
    BackEndNode * get_BackEndNode( void ) { return back_end; }
    
    int parse_configfile( const char* _configBuf = NULL );

    /* "Registered" streams */
    static std::map < unsigned int, Stream * >streams;
    static unsigned int cur_stream_idx;

    void InitFE( Network * _network, const char * _config = NULL );

 public:
    NetworkGraph * graph;  /* hierarchical DAG of tree nodes */
    static NetworkGraph* parsed_graph;

    static void error_str(const char *);
    static bool is_FrontEnd();
    static bool is_BackEnd();

    // FE constructors
    // The first takes a configuration from a file named by '_filename',
    // the second takes its configuration from a buffer in memory.
    NetworkImpl(Network *, const char * _filename, const char * _application);
    NetworkImpl(Network *, const char * _config, 
                    bool unused, const char * _application);

    // BE constructor
    NetworkImpl(Network *, const char *phostname, Port pport, Rank myrank );
    ~NetworkImpl();

    Communicator * get_BroadcastCommunicator(void);
    int recv(int *tag, void **ptr, Stream **stream, bool blocking=true);
    int send(Packet &);
    int recv( bool blocking=true );
    EndPoint * get_EndPoint(const char * _hostname, Port _port);

    int get_LeafInfo( Network::LeafInfo*** linfo, unsigned int* nLeaves );
    int connect_Backends( void );

    int getConnections( int** conns, unsigned int* nConns );
};


} // namespace MRN
#endif /* __networkimpl_h */
