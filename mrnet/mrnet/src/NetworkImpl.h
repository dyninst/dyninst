#if !defined(__networkimpl_h)
#define __networkimpl_h 1

#include <vector>
#include <string>


#include "mrnet/src/Errors.h"
#include "mrnet/src/Message.h"
#include "mrnet/src/NetworkGraph.h"
#include "mrnet/src/BackEndNode.h"
#include "mrnet/src/FrontEndNode.h"

#include "mrnet/h/MR_Network.h"
using namespace MRN;

class NetworkImpl: public Error {
  friend class NetworkGraph;
  friend class CommunicatorImpl;
  friend class StreamImpl;


public:
    class LeafInfoImpl : public Network::LeafInfo
    {
    private:
        char* host;
        unsigned short id;
        unsigned short rank;
        char* phost;
        unsigned short pport;
        unsigned short prank;

    public:
        LeafInfoImpl( unsigned short _id,
                        const char* _host,
                        unsigned short _rank,
                        const char* _phost,
                        unsigned short _pport,
                        unsigned short _prank )
          : host( new char[strlen(_host)+1] ),
            id( _id ),
            rank( _rank ),
            phost( new char[strlen(_phost)+1] ),
            pport( _pport ),
            prank( _prank )
        {
            strcpy( host, _host );
            strcpy( phost, _phost );
        }

        virtual ~LeafInfoImpl( void )
        {
            delete[] host;
            delete[] phost;
        }

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
  std::vector <EndPoint *> * endpoints; //BackEnds addressed by communicator
  static BackEndNode * back_end;
  FrontEndNode *front_end;

  EndPoint * get_EndPoint(const char * _hostname, unsigned short _port);
  int parse_configfile();

 public:
  NetworkGraph * graph;  /* heirarchical DAG of tree nodes */
  static void error_str(const char *);
  static int recv( bool blocking=true );
  static int send(Packet *);

  NetworkImpl(const char * _filename, const char * _application);
  ~NetworkImpl();

    int get_LeafInfo( Network::LeafInfo*** linfo, unsigned int* nLeaves );
    int connect_Backends( void );

    int getConnections( int** conns, unsigned int* nConns )
        {
            return front_end->getConnections( conns, nConns );
        }
};

#endif /* __networkimpl_h */
