/****************************************************************************
 * Copyright © 2003-2005 Dorian C. Arnold, Philip C. Roth, Barton P. Miller *
 *                  Detailed MRNet usage rights in "LICENSE" file.          *
 ****************************************************************************/

#ifndef utils_h
#define utils_h 1

#if defined(os_windows)
#include <winsock2.h>
#else
#include <sys/socket.h>
#include <arpa/inet.h>
#endif // defined(os_windows)

#include <vector>
#include <string>

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

#include "xplat/TLSKey.h"
#include "xplat/Thread.h"

#include "mrnet/MRNet.h"

namespace MRN
{

int connectHost( int *sock_in, const std::string & hostname, Port port );

int bindPort( int *sock_in, Port *port_in );
int getSocketConnection( int bound_socket );

#if READY
int getSocketPeer( int connected_socket,
                   std::string & hostname, Port *port );
#endif // READY

int getPortFromSocket( int sock, Port *port );

int getHostName( std::string & out_hostname, const std::string & in_hostname = "" );    // e.g. "foo"
int getDomainName( std::string & out_hostname, const std::string & in_hostname = "" );  // e.g. "bar.net"
int getNetworkName( std::string & out_hostname, const std::string & in_hostname = "" ); // e.g. "foo.bar.net"
int getNetworkAddr( std::string & ipaddr_str, const std::string hostname = "" );    // "127.0.0.1"

struct ltstr
{
    bool operator(  ) ( std::string s1, std::string s2 ) const
    {
        return ( s1 < s2 );
    }
};

extern XPlat::TLSKey tsd_key;

class tsd_t {
 public:
    XPlat::Thread::Id thread_id;
    const char *thread_name;
};

#if defined(DEBUG_MRNET)
#  define _fprintf(X) fprintf X ;
#  define _perror(X) perror(X);
#else
#  define _fprintf(X)  ;
#  define _perror(X) ;
#endif                          // defined(DEBUG_MRNET)

#define mrn_dbg( x, y ) \
do{ \
  if( CUR_OUTPUT_LEVEL >= x ){           \
    y; \
  } \
}while(0);


//FLF is used to call mrn_printf(FLF, ...)
#if !defined( __GNUC__ )
#define CURRENT_FUNCTION ((const char*)0)
#define FLF  __FILE__,__LINE__,""
#else
#define FLF  __FILE__,__LINE__,__FUNCTION__
#endif

int mrn_printf( const char *file, int line, const char * func, FILE * fp,
                const char *format, ... );

}                               // namespace MRN
#endif                          /* utils_h */
