/***********************************************************************
 * Copyright © 2003-2004 Dorian C. Arnold, Philip C. Roth, Barton P. Miller *
 *                  Detailed MRNet usage rights in "LICENSE" file.     *
 **********************************************************************/

#ifndef utils_h
#define utils_h 1

#include <vector>
#include <string>

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

#if defined(WIN32)
#include <winsock2.h>
#include <windows.h>
#else
#include <sys/socket.h>
#include <arpa/inet.h>
#endif // defined(WIN32)

#include "xplat/TLSKey.h"
#include "xplat/Thread.h"

namespace MRN
{

extern std::string LocalHostName;
extern unsigned short LocalPort;

int connectHost( int *sock_in,
                 const std::string & hostname, unsigned short port );

int bindPort( int *sock_in, unsigned short *port_in );
int getSocketConnection( int bound_socket );

#if READY
int getSocketPeer( int connected_socket,
                   std::string & hostname, unsigned short *port );
#endif // READY

int getPortFromSocket( int sock, unsigned short *port );

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

#define MCFL  __FILE__,__LINE__ //used to call mrn_printf(MCFL, ...)
int mrn_printf( int level, const char *file, int line, FILE * fp,
                const char *format, ... );

}                               // namespace MRN
#endif                          /* utils_h */
