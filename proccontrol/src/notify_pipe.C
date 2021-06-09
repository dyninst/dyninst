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

#include "int_process.h"
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>

int_notify::unix_details::unix_details() :
   pipe_in(-1),
   pipe_out(-1)
{
}

void int_notify::unix_details::writeToPipe()
{
   if (pipe_out == -1) 
      return;

   char c = 'e';
   ssize_t result = write(pipe_out, &c, 1);
   if (result == -1) {
      int error = errno;
      ProcControlAPI::globalSetLastError(err_internal, "Could not write to notification pipe\n");
      perr_printf("Error writing to notification pipe: %s\n", strerror(error));
      return;
   }
   pthrd_printf("Wrote to notification pipe %d\n", pipe_out);
}

void int_notify::unix_details::readFromPipe()
{
   if (pipe_out == -1)
      return;

   char c;
   ssize_t result;
   int error;
   do {
      result = read(pipe_in, &c, 1);
      error = errno;
   } while (result == -1 && error == EINTR);
   if (result == -1) {
      if (error == EAGAIN) {
         pthrd_printf("Notification pipe had no data available\n");
         return;
      }
      ProcControlAPI::globalSetLastError(err_internal, "Could not read from notification pipe\n");
      perr_printf("Error reading from notification pipe: %s\n", strerror(error));
   }
   assert(result == 1 && c == 'e');
   pthrd_printf("Cleared notification pipe %d\n", pipe_in);
}

void int_notify::unix_details::clearEvent()
{
   readFromPipe();
}

int_notify::unix_details::wait_object_t int_notify::unix_details::getWaitObject()
{
   return pipe_in;
}

bool int_notify::unix_details::internalsValid() {
   return (pipe_in != -1 && pipe_out != -1);
}

void int_notify::unix_details::noteEvent() {
   writeToPipe();
}

bool int_notify::unix_details::createInternals()
{
   if (pipe_in != -1 || pipe_out != -1)
      return true;

   int fds[2];
   int result = pipe(fds);
   if (result == -1) {
      int error = errno;
      ProcControlAPI::globalSetLastError(err_internal, "Error creating notification pipe\n");
      perr_printf("Error creating notification pipe: %s\n", strerror(error));
      return false;
   }
   assert(fds[0] != -1);
   assert(fds[1] != -1);

   result = fcntl(fds[0], F_SETFL, O_NONBLOCK);
   if (result == -1) {
      int error = errno;
      ProcControlAPI::globalSetLastError(err_internal, "Error setting properties of notification pipe\n");
      perr_printf("Error calling fcntl for O_NONBLOCK on %d: %s\n", fds[0], strerror(error));
      return false;
   }
   pipe_in = fds[0];
   pipe_out = fds[1];


   pthrd_printf("Created notification pipe: in = %d, out = %d\n", pipe_in, pipe_out);
   return true;
}

