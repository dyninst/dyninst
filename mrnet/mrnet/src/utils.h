/****************************************************************************
 * Copyright © 2003-2007 Dorian C. Arnold, Philip C. Roth, Barton P. Miller *
 *                  Detailed MRNet usage rights in "LICENSE" file.          *
 ****************************************************************************/

#ifndef utils_h
#define utils_h 1

#include "xplat/Types.h"

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <math.h>
#include <sys/types.h>

#ifndef os_windows

# include <signal.h>
# include <sys/time.h>

#else

# include <io.h>
# include <sys/timeb.h>

# define srand48 srand
# define drand48 (double)rand
# define snprintf _snprintf
# define sleep(x) Sleep(1000*(DWORD)x)

inline int gettimeofday( struct timeval *tv, struct timezone *tz )
{
    struct _timeb now;
    _ftime( &now );
    if( tv != NULL ) {
        tv->tv_sec = now.time;
        tv->tv_usec = now.millitm * 1000;
    }
    return 0;
}

#endif // ifndef(os_windows)

#include <vector>
#include <string>

#include "xplat/TLSKey.h"
#include "xplat/Thread.h"

#include "mrnet/Types.h"

using namespace MRN;
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

extern int CUR_OUTPUT_LEVEL; 
#define mrn_dbg( x, y ) \
do{ \
    if( MRN::CUR_OUTPUT_LEVEL >= x ){           \
        y;                                      \
    }                                           \
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

#define mrn_dbg_func_end()                    \
do { \
    mrn_dbg(3, MRN::mrn_printf(FLF, stderr, "Function exit\n"));    \
} while(0);

#define mrn_dbg_func_begin()                    \
do { \
    mrn_dbg(3, MRN::mrn_printf(FLF, stderr, "Function start ...\n")); \
} while(0);

/* struct timeval/double conversion */
double tv2dbl( struct timeval tv);
struct timeval dbl2tv(double d) ;

class Timer{
 public:
    struct timeval _start_tv, _stop_tv;
    double  _start_d, _stop_d;
    
    Timer( void );
    void start( void );
    void stop( void );
    void stop( double d );
    double get_latency_secs( void );
    double get_latency_msecs( void );
    double get_latency_usecs( void );
    double get_offset_msecs( void );

 private:
    static double offset;
    static bool first_time;
};

Rank getrank();
void setrank( Rank ir );
}                               // namespace MRN
#endif                          /* utils_h */
