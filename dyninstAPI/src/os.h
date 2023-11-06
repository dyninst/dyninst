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

// $Id: os.h,v 1.38 2008/06/19 19:53:31 legendre Exp $

#ifndef _OS_HDR
#define _OS_HDR

/*
 * This is an initial attempt at providing an OS abstraction for paradynd
 * I am doing this so I can compile paradynd on solaris
 *
 * This should enforce the abstract OS operations
 */ 

#if defined(i386_unknown_nt4_0) 
#include "dyninstAPI/src/pdwinnt.h"

#elif defined(os_linux)
#include "dyninstAPI/src/linux.h"

#elif defined(os_freebsd)
#include "dyninstAPI/src/freebsd.h"

#endif

#include <string>
#include <vector>

class OS {
public:
  static void osTraceMe(void);
  static void osDisconnect(void);
  static bool osKill(int);
  static void make_tempfile(char *);
  static bool execute_file(char *);
  static void unlink(char *);
  static bool executableExists(const std::string &file);
  static void get_sigaction_names(std::vector<std::string> &names);
};

// Temporary prototype for a remote debugging BPatch interface.
#include "dyninstAPI/h/BPatch.h"
bool OS_isConnected(void);
bool OS_connect(BPatch_remoteHost &remote);
bool OS_getPidList(BPatch_remoteHost &remote,
                   BPatch_Vector<unsigned int> &tlist);
bool OS_getPidInfo(BPatch_remoteHost &remote, unsigned int pid, std::string &pidStr);
bool OS_disconnect(BPatch_remoteHost &remote);

#endif
