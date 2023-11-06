/*
 * See the dyninst/COPYRIGHT file for copyright information.
 * 
 * We provide the Paradyn Tools (below described as "Paradyn")
 * on an AS IS basis, and do not warrant its validity or performance.
 * We reserve the right to update, modify, or discontinue this
 * software at any time.  We shall have no obligation to supply such
 * updates or modifications or any other form of support to you.
 * 
 * By your use of Paradyn, you understand and agree that we (or any
 * other person or entity with proprietary rights in Paradyn) are
 * under no obligation to provide either maintenance services,
 * update services, notices of latent defects, or correction of
 * defects for Paradyn.
 * 
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 * 
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */


// $Id: headers.h,v 1.30 2008/06/30 19:40:18 legendre Exp $

#ifndef KLUDGES_H
#define KLUDGES_H

#include <sys/types.h>
#include <stddef.h>
#include <string.h>

#ifndef FILE__
#define FILE__ strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILE__
#endif

/*
 * Kludges to handle broken system includes and such...
 */

#if defined(os_linux)
#include "common/src/linuxHeaders.h"

#elif defined(os_freebsd)
#include "common/src/freebsdHeaders.h"

#elif defined(os_windows)
#include "common/src/ntHeaders.h"

#endif  /* architecture specific */

typedef enum {
   RRVsuccess,
   RRVnoData,
   RRVinsufficientData,
   RRVreadError,
   RRVerror
} readReturnValue_t;


#if 0
template<class T>
readReturnValue_t P_socketRead(PDSOCKET fd, T &it, size_t sz)
{
   ssize_t bytes_read = 0;
#if defined (os_windows)
   bytes_read = recv( fd, (char *)&it, sz, 0 );
#else
try_again:
   bytes_read = read(fd, &it, sz);
#endif

   if ( (ssize_t)PDSOCKET_ERROR == bytes_read ) {
#if defined (os_windows)
      if (errno != 0) {
         fprintf(stderr, "%s[%d]:  read failed: %s:%d\n", FILE__, __LINE__,
               strerror(errno), errno);
         return REreadError;
      }
#else
      if (errno == EAGAIN || errno == EINTR)
         goto try_again;

      fprintf(stderr, "%s[%d]:  read failed: %s:%d\n", FILE__, __LINE__,
            strerror(errno), errno);
      return REreadError;
#endif
   }

   if (0 == bytes_read) {
      //  fd closed on other end (most likely)
      //bperr("%s[%d]:  cannot read, fd is closed\n", FILE__, __LINE__);
      return REnoData;
   }
#if defined (os_windows)
   if ((PDSOCKET_ERROR == bytes_read) && (errno == 0)) {
      //  fd closed on other end (most likely)
      //bperr("%s[%d]:  cannot read, fd is closed\n", FILE__, __LINE__);
      return REnoData;
   }
#endif

   if (bytes_read != sz) {
      fprintf(stderr, "%s[%d]:  read wrong number of bytes! %d, not %d\n",
            FILE__, __LINE__, bytes_read, sz);
      fprintf(stderr, "FIXME:  Need better logic to handle incomplete reads\n");
      return REinsufficientData;
   }

   return REsuccess;

}
#endif

#if !defined(os_windows)
template <class T>
readReturnValue_t P_socketRead(PDSOCKET fd, T &it, ssize_t sz)
{
   ssize_t bytes_read = 0;
try_again:
   bytes_read = read(fd, (char *)&it, sz);

   if ( (ssize_t)-1 == bytes_read ) {
      if (errno == EAGAIN || errno == EINTR) 
         goto try_again;

      fprintf(stderr, "%s[%d]:  read failed: %s:%d\n", FILE__, __LINE__,
            strerror(errno), errno);
      return RRVreadError;
   }

   if (0 == bytes_read) {
      //  fd closed on other end (most likely)
      //bperr("%s[%d]:  cannot read, fd is closed\n", FILE__, __LINE__);
      return RRVnoData;
   }
   if (bytes_read != sz) {
      //bperr("%s[%d]:  read wrong number of bytes! %d, not %d\n", 
      //      FILE__, __LINE__, bytes_read, sz);
      //bperr("FIXME:  Need better logic to handle incomplete reads\n");
      return RRVinsufficientData;
   }

   return RRVsuccess;
}
#else

#define ssize_t int
template <class T>
readReturnValue_t P_socketRead(PDSOCKET fd, T &it, ssize_t sz)
{
   ssize_t bytes_read = 0;

   bytes_read = recv( fd, (char *)&it, sz, 0 );

   if ( PDSOCKET_ERROR == bytes_read && errno != 0 ) {
      fprintf(stderr, "%s[%d]:  read failed: %s:%d\n", FILE__, __LINE__,
            strerror(errno), errno);
      return RRVreadError;
   }

   if (0 == bytes_read || (PDSOCKET_ERROR == bytes_read && errno == 0)) {
      //  fd closed on other end (most likely)
      //bperr("%s[%d]:  cannot read, fd is closed\n", FILE__, __LINE__);
      return RRVnoData;
   }

   if (bytes_read != sz) {
      //bperr("%s[%d]:  read wrong number of bytes!\n", FILE__, __LINE__);
      //bperr("FIXME:  Need better logic to handle incomplete reads\n");
      return RRVinsufficientData;
   }

   return RRVsuccess;
}
#endif /* !defined(os_windows) */

template<class T>
readReturnValue_t P_socketRead(PDSOCKET fd, T &it)
{
   return P_socketRead<T>(fd, it, sizeof(T));
}

#endif /* KLUDGES_H */
