/***********************************************************************
 * Copyright © 2003-2004 Dorian C. Arnold, Philip C. Roth, Barton P. Miller *
 *                  Detailed MRNet usage rights in "LICENSE" file.     *
 **********************************************************************/

// $Id: NetUtils-unix.C,v 1.3 2004/06/01 16:34:22 pcroth Exp $
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

#if defined(HAVE_SYS_SOCKIO_H)
#include <sys/sockio.h>
#endif // defined(HAVE_SYS_SOCKIO_H)

namespace XPlat
{

std::string
NetUtils::FindNetworkAddress( void )
{
    std::string ret;

    int sockfd, lastlen, len, firsttime, flags;
    char *buf, *cptr, *ptr, lastname[IFNAMSIZ];
    struct ifreq *ifr, ifrcopy;
    struct ifconf ifc;

    sockfd = socket( AF_INET, SOCK_STREAM, 0 );
    if( sockfd < 0 ) {
        perror( "Failed socket()" );
        return ret;
    }

    lastlen = 0;
    firsttime = 1;
    len = 3 * sizeof( struct ifreq );   // initial size guess
    while( 1 ) {
        buf = new char[len];
        assert( buf );
        ifc.ifc_len = len;
        ifc.ifc_buf = buf;
        //printf("\tCalling ioctl w/ len: %d ...\n", len);

#if defined(CSIOCGIFCONF)
        int ioreq = CSIOCGIFCONF;
#else
        int ioreq = SIOCGIFCONF;
#endif // defined(CSIOCGIFCONF)

        if( ioctl( sockfd, ioreq, &ifc ) < 0 )
            {
                perror( "Failed ioctl()" );
                delete[] buf;
                return ret;
            }
            else {
                //printf("\tComparing %d and lastlen:%d ... \n", ifc.ifc_len, lastlen);
                if( ifc.ifc_len == lastlen ) {
                    //printf("ioctl success\n");
                    break;      //success, len has not changed
                }
                lastlen = ifc.ifc_len;
            }
        if( !firsttime ) {
            firsttime = 0;
            len += 5 * sizeof( struct ifreq );  /* increment size guess */
        }
        delete[] buf;
    }

    lastname[0] = 0;
    int i;
    for( ptr = buf, i = 0; ptr < buf + ifc.ifc_len;
         i++, ptr += sizeof( ifr->ifr_name ) + len ) {
        //printf("Processing interface %d\n", i);
        ifr = ( struct ifreq * )ptr;

        len = sizeof(sockaddr);

        if( ifr->ifr_addr.sa_family != AF_INET ) {
            //printf("\tIgnoring %s (wrong family)!\n", ifr->ifr_name );
            continue;       //ignore other address families
        }

        if( ( cptr = strchr( ifr->ifr_name, ':' ) ) != NULL ) {
            *cptr = 0;      // replace colon with null 
        }
        if( strncmp( lastname, ifr->ifr_name, IFNAMSIZ ) == 0 ) {
            //printf("\tIgnoring %s (alias)!\n", ifr->ifr_name );
            continue;
        }

        ifrcopy = *ifr;
        if( ioctl( sockfd, SIOCGIFFLAGS, &ifrcopy ) < 0 ) {
            perror( "Failed ioctl()" );
            delete[] buf;
            return ret;
        }
        flags = ifrcopy.ifr_flags;
        if( ( flags & IFF_UP ) == 0 ) {
            //printf("\tIgnoring %s (Not Up!)\n", ifr->ifr_name);
            continue;
        }

        struct in_addr in;
        struct sockaddr_in *sinptr = ( struct sockaddr_in * )&ifr->ifr_addr;
        memcpy( &in.s_addr, ( void * )&( sinptr->sin_addr ),
                sizeof( in.s_addr ) );
        if( htonl( in.s_addr ) == INADDR_LOOPBACK ) {
            //printf("\tIgnoring %s (loopback!)\n", ifr->ifr_name);
            continue;
        }

        char ip_address_buf[32];
        if( inet_ntop( AF_INET, ( const void * )&in, ip_address_buf,
                       sizeof( ip_address_buf ) ) == NULL ) {
            perror( "Failed inet_ntop()" );
            delete[] buf;
            return ret;
        }
        ret = ip_address_buf;
        break;
    }
    delete[] buf;

    if( ret.length() == 0 )
    {
        // we have no interface except loopback
        ret = "127.0.0.1";
    }
    return ret;
}


int
NetUtils::GetLastError( void )
{
    return errno;
}


} // namespace XPlat

