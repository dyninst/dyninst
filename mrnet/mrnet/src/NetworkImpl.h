#if !defined(mc_networkimpl_h)
#define mc_networkimpl_h 1

#include <vector>
#include <string>


#include "mrnet/src/Errors.h"
#include "mrnet/src/Message.h"
#include "mrnet/src/NetworkGraph.h"
#include "mrnet/src/BackEndNode.h"
#include "mrnet/src/FrontEndNode.h"

#include "mrnet/h/MR_Network.h"

class MC_NetworkImpl: public MC_Error {
  friend class MC_NetworkGraph;
  friend class MC_CommunicatorImpl;
  friend class MC_StreamImpl;

 private:

    class LeafInfoImpl : public MC_Network::LeafInfo
    {
    private:
        char* host;
        unsigned short port;

    public:
        LeafInfoImpl( const char* h, unsigned short p )
          : host( new char[strlen(h)+1] ),
            port( p )
        {
            strcpy( host, h );
        }

        virtual ~LeafInfoImpl( void )
        {
            delete[] host;
        }

        virtual const char* get_Host( void ) const      { return host; }
        virtual unsigned short get_Port( void ) const   { return port; }
    };

  std::string filename;          /* Name of topology configuration file */
  std::string commnode;          // path to comm_node executable
  std::string application;       /* Name of application to launch */
  std::vector <MC_EndPoint *> * endpoints; //BackEnds addressed by communicator
  static MC_BackEndNode * back_end;
  MC_FrontEndNode *front_end;

  MC_EndPoint * get_EndPoint(const char * _hostname, unsigned short _port);
  int parse_configfile();

 public:
  MC_NetworkGraph * graph;  /* heirarchical DAG of tree nodes */
  static void error_str(const char *);
  static int recv(void);
  static int send(MC_Packet *);

  MC_NetworkImpl(const char * _filename,
                    const char * _commnode,
                    const char * _application);
  ~MC_NetworkImpl();

    std::vector<MC_Network::LeafInfo*> get_LeafInfo( void );
    int connect_Backends( void );
};

#endif /* mrnet_H */
