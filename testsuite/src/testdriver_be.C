/*
 * Copyright (c) 1996-2009 Barton P. Miller
 * 
 * We provide the Paradyn Parallel Performance Tools (below
 * described as "Paradyn") on an AS IS basis, and do not warrant its
 * validity or performance.  We reserve the right to update, modify,
 * or discontinue this software at any time.  We shall have no
 * obligation to supply such updates or modifications or any other
 * form of support to you.
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

#include "ParameterDict.h"
#include "MutateeStart.h"
#include "CmdLine.h"
#include "test_info_new.h"
#include "test_lib.h"
#include "remotetest.h"

#include <vector>
#include <string>

using namespace std;

#if defined(cap_launchmon)
#include "lmon_api/lmon_be.h"

void init_lmon(int *argc, char ***argv)
{
   lmon_rc_e rc;
   rc = LMON_be_init(LMON_VERSION, argc, argv);

   LMON_be_finalize();
}

#else
void init_lmon(int *argc, char ***argv)
{
}
#endif
int main(int argc, char *argv[])
{
   ParameterDict params;
   vector<RunGroup *> groups;

   static volatile int spin = 0;
   /*fprintf(stderr, "testdriver_be running as %d\n", getpid());
   while (!spin)
   {
      sleep(1);
      }*/
   
   init_lmon(&argc, &argv);

   parseArgs(argc, argv, params);
   getGroupList(groups, params);

   int port = params["port"]->getInt();
   string hostname = params["hostname"]->getString();

   if (port == 0 || !hostname.length()) {
      fprintf(stderr, "[%s:%u] - No connection info.  port = %d, hostname = %s\n",
              __FILE__, __LINE__, port, hostname.c_str());
      return -1;
   }

   Connection connection(hostname, port);
   if (connection.hasError()) {
      fprintf(stderr, "[%s:%u] - Error connecting to FE\n",
              __FILE__, __LINE__);
      return -1;
   }
   
   RemoteOutputDriver remote_output(&connection);
   setOutput(&remote_output);

   RemoteBE remotebe(groups, &connection);
   for (;;) {
      char *buffer = NULL;
      bool result = connection.recv_message(buffer);
      if (!result) {
         //FE hangup.
         return 0;
      }
      remotebe.dispatch(buffer);
   }
}
