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
        unsigned short id;
        unsigned short rank;
        char* phost;
        unsigned short pport;
        unsigned short prank;

    public:
        LeafInfoImpl( unsigned short _id, const char* _host,
                      unsigned short _rank, const char* _phost,
                      unsigned short _pport, unsigned short _prank );
        virtual ~LeafInfoImpl( void );
      
        virtual const char* get_Host( void ) const      { return host; }
        virtual unsigned short get_Rank( void ) const   { return rank; }
        virtual unsigned short get_Id( void ) const   { return id; }
        virtual const char* get_ParHost( void ) const   { return phost; }
        virtual unsigned short get_ParPort( void ) const   { return pport; }
        virtual unsigned short get_ParRank( void ) const   { return prank; }
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
    
    int parse_configfile();

    /* "Registered" streams */
    static std::map < unsigned int, Stream * >streams;
    static unsigned int cur_stream_idx;

 public:
    NetworkGraph * graph;  /* hierarchical DAG of tree nodes */
    static NetworkGraph* parsed_graph;

    static void error_str(const char *);
    static bool is_FrontEnd();
    static bool is_BackEnd();

    NetworkImpl(Network *, const char * _filename, const char * _application);
    NetworkImpl(Network *, const char *hostname, unsigned int backend_id,
                const char *phostname, unsigned int pport, unsigned int pid);
    ~NetworkImpl();

    Communicator * get_BroadcastCommunicator(void);
    int recv(int *tag, void **ptr, Stream **stream, bool blocking=true);
    int send(Packet &);
    int recv( bool blocking=true );
    EndPoint * get_EndPoint(const char * _hostname, unsigned short _port);

    int get_LeafInfo( Network::LeafInfo*** linfo, unsigned int* nLeaves );
    int connect_Backends( void );

    int getConnections( int** conns, unsigned int* nConns );
};


} // namespace MRN
#endif /* __networkimpl_h */
