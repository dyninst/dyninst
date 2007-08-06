/****************************************************************************
 * Copyright © 2003-2007 Dorian C. Arnold, Philip C. Roth, Barton P. Miller *
 *                  Detailed MRNet usage rights in "LICENSE" file.          *
 ****************************************************************************/

// $Id: NetUtils-unix.C,v 1.8 2007/08/06 21:18:40 mjbrim Exp $
#include "xplat/NetUtils.h"
#include <assert.h>
#include <unistd.h>
#include <errno.h>
#include <netdb.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include "config.h"

#if defined( compiler_sun )
#include <stropts.h>
#endif

#if defined(HAVE_SYS_SOCKIO_H)
#include <sys/sockio.h>
#endif // defined(HAVE_SYS_SOCKIO_H)

#if defined (SIOCGIFCONF)
#define XPLAT_SIOCGIFCONF SIOCGIFCONF
#elif defined (CSIOCGIFCONF)
#define XPLAT_SIOCGIFCONF CSIOCGIFCONF
#endif /* SIOCGIFCONF */

namespace XPlat
{


int NetUtils::FindNumberOfLocalNetworkInterfaces( void )
{
    struct ifconf ifc;
    int num_ifs=-1;
    int rq_len;
    int ifc_count_guess=5;

    //IP6 suport?
    int fd=socket(AF_INET6,SOCK_DGRAM,0);
    if ( fd<0 ){
        //NO IP6, use IP4
        fd=socket(AF_INET,SOCK_DGRAM,0);
    }

#if defined( SIOCGIFCOUNT )
    //Try SIOCIFCOUNT request first (not always implemented).
    int ioctl_ret = ioctl(fd, SIOCGIFCOUNT, (char *)&num_ifs);
    if( num_ifs >= 0 ){
        close(fd);
        return num_ifs;
    }
#endif /* SIOCGIFNUM */

    ifc.ifc_buf=NULL;
    ifc.ifc_len=0;
    //Sometimes calling ioctl() w/ 0 len buf returns needed space
    //otherwise we guess at number of interfaces needed.
    if ( (ioctl(fd,XPLAT_SIOCGIFCONF,&ifc)<0) || (ifc.ifc_len == 0 ) ) {
        rq_len = ifc_count_guess * sizeof(struct ifreq);
    }
    else{
        rq_len = ifc.ifc_len;
    }

    //Get interfaces from kernel.
    //If buf is too small, ioctl() doesn't always fail, sometimes it truncates
    //Call ioctl() until it returns buf len less than req. len

    while( true ) {
        ifc.ifc_len = rq_len;
        ifc.ifc_buf= (char *)realloc( ifc.ifc_buf, ifc.ifc_len );

        if( ioctl( fd, XPLAT_SIOCGIFCONF, &ifc ) < 0 ) {
            perror( "ioctl(SIOCGIFCONF)" );
            free(ifc.ifc_buf);
            close(fd);
            return -1;
        }
        if( ifc.ifc_len <= rq_len ){
            //success
            break;
        }
        else{
            //buffer truncated, double size and try again
            rq_len *= 2;
        }
    }

    num_ifs = ifc.ifc_len / sizeof(struct ifreq);

    close(fd);
    return num_ifs;
}

int NetUtils::FindLocalNetworkInterfaces
( std::vector<NetUtils::NetworkAddress> &local_addresses )
{
    int num_ifs=-1, rq_len;
    int ifc_count_guess = 5;
    struct ifconf ifc;

    int fd = socket( AF_INET6, SOCK_STREAM, 0 );
    if( fd < 0 ) {
        fd = socket( AF_INET, SOCK_STREAM, 0 );
    }

#if defined( SIOCGIFCOUNT )
    //Try SIOCIFCOUNT request first (not always implemented).
    int ioctl_ret = ioctl(fd, SIOCGIFCOUNT, (char *)&num_ifs);
#endif /* SIOCGIFNUM */

    if( num_ifs > 0 ){
        rq_len = num_ifs * sizeof(struct ifreq);
    }
    else{
        ifc.ifc_buf=NULL;
        ifc.ifc_len=0;
        //Sometimes calling ioctl() w/ 0 len buf returns needed space
        //otherwise we guess at number of interfaces needed.
        if ( (ioctl(fd,XPLAT_SIOCGIFCONF,&ifc)<0) || (ifc.ifc_len == 0 ) ) {
            rq_len = ifc_count_guess * sizeof(struct ifreq);
        }
        else{
            rq_len = ifc.ifc_len;
        }
    }

    //Get interfaces from kernel.
    //If buf is too small, ioctl() doesn't always fail, sometimes it truncates
    //Call ioctl() until it returns buf len less than req. len

    while( true ) {
        ifc.ifc_len = rq_len;
        ifc.ifc_buf= (char *)realloc( ifc.ifc_buf, ifc.ifc_len );

        if( ioctl( fd, XPLAT_SIOCGIFCONF, &ifc ) < 0 ) {
            perror( "ioctl(SIOCGIFCONF)" );
            free(ifc.ifc_buf);
            close(fd);
            return -1;
        }
        if( ifc.ifc_len <= rq_len ){
            //success
            break;
        }
        else{
            //buffer truncated, double size and try again
            rq_len *= 2;
        }
    }

    num_ifs = ifc.ifc_len / sizeof(struct ifreq);

    struct ifreq ifr;
    for( unsigned int i=0; i<num_ifs; i++ ){
        ifr = ifc.ifc_req[i];
            
        struct in_addr in;
        struct sockaddr_in *sinptr = ( struct sockaddr_in * )&ifr.ifr_addr;
        memcpy( &in.s_addr, ( void * )&( sinptr->sin_addr ),
                sizeof( in.s_addr ) );
        
        local_addresses.push_back( NetworkAddress( ntohl( in.s_addr ) ) );
    }

    if( ifc.ifc_buf != NULL )
        free(ifc.ifc_buf);

    return 0;
}

int NetUtils::GetLastError( void )
{
    return errno;
}

} // namespace XPlat

