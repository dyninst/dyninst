/*
 * Copyright (c) 1996 Barton P. Miller
 * 
 * We provide the Paradyn Parallel Performance Tools (below
 * described as "Paradyn") on an AS IS basis, and do not warrant its
 * validity or performance.  We reserve the right to update, modify,
 * or discontinue this software at any time.  We shall have no
 * obligation to supply such updates or modifications or any other
 * form of support to you.
 * 
 * This license is for research uses.  For such uses, there is no
 * charge. We define "research use" to mean you may freely use it
 * inside your organization for whatever purposes you see fit. But you
 * may not re-distribute Paradyn or parts of Paradyn, in any form
 * source or binary (including derivatives), electronic or otherwise,
 * to any other organization or entity without our permission.
 * 
 * (for other uses, please contact us at paradyn@cs.wisc.edu)
 * 
 * All warranties, including without limitation, any warranty of
 * merchantability or fitness for a particular purpose, are hereby
 * excluded.
 * 
 * By your use of Paradyn, you understand and agree that we (or any
 * other person or entity with proprietary rights in Paradyn) are
 * under no obligation to provide either maintenance services,
 * update services, notices of latent defects, or correction of
 * defects for Paradyn.
 * 
 * Even if advised of the possibility of such damages, under no
 * circumstances shall we (or any other person or entity with
 * proprietary rights in the software licensed hereunder) be liable
 * to you or any third party for direct, indirect, or consequential
 * damages of any character regardless of type of action, including,
 * without limitation, loss of profits, loss of use, loss of good
 * will, or computer failure or malfunction.  You agree to indemnify
 * us (and any other person or entity with proprietary rights in the
 * software licensed hereunder) for any and all liability it may
 * incur to third parties resulting from your use of Paradyn.
 */

// $Id: aixDL.C,v 1.1 2000/11/16 01:33:15 bernat Exp $

#include "dyninstAPI/src/sharedobject.h"
#include "dyninstAPI/src/aixDL.h"
#include "dyninstAPI/src/process.h"
#include "dyninstAPI/src/symtab.h"
#include "dyninstAPI/src/arch-power.h"
#include "dyninstAPI/src/inst-power.h"
#include "common/h/debugOstream.h"
#include <sys/ptrace.h>
#include <sys/ldr.h>

/* Parse a binary to extract all shared objects it
   contains, and create shared object objects for each
   of them */

vector< shared_object *> *dynamic_linking::getSharedObjects(process *p)
{
  // First things first, get a list of all loader info structures.
  int pid;
  int ret;
  struct ld_info *ptr;
  struct stat ld_stat;
  // Credit where credit is due: it works in GDB 5.0, so it should
  // work here. Right? Of course it will. NOT!
  // alloca() so we don't have to worry about deallocating it. 
  // hacky.
  ptr = (struct ld_info *) alloca (1024*sizeof(*ptr));
  pid = p->getPid();

  /* It seems that AIX has some timing problems and
     when the user stack grows, the kernel doesn't update the stack info in time
     and ptrace calls step on user stack. This is the reason why call sleep 
     here, giving the kernel some time to update its internals. */
  usleep (36000);

  ret = 0;
  ret = ptrace(PT_LDINFO, pid, 
	       (int *) ptr, 1024 * sizeof(*ptr), (int *)ptr);
  
  if (ret != 0) {
    cerr << "Ptrace PT_LDINFO failed" << endl;
    statusLine("Unable to get loader info about process, application aborted");
    showErrorCallback(43, "Unable to get loader info about process, application aborted");
    return false;
  }

  if (!ptr->ldinfo_next)
    {
      // Non-shared object, return 0
      cerr << "No shared objects found" << endl;
      return 0;
    }

  // Skip the first element, which appears to be the executable file.
  // We could also find and skip the entry with a text_org value
  // of 0x10000000, since that can _only_ be the executable file.
  ptr = (struct ld_info *)(ptr->ldinfo_next + (char *)ptr);

  // So we want to fill in this vector.
  vector<shared_object *> *result = new(vector<shared_object *>);

  // So we have this list of ldinfo structures. This will include the executable and
  // all shared objects it has loaded. Parse it.
  do
    {
      // Gripe if the shared object we have loaded has been deleted
      // If this is so, the file descriptor has been set to -1
      // This can be a problem, since we expect to find symbol table
      // information from the file.
      if (fstat (ptr->ldinfo_fd, &ld_stat) < 0)
	cerr << "File " << ptr->ldinfo_filename << " has disappeared!" << endl;
      
      // Futility: the interface to the Object class defines a "name"
      // and an "address", while we want to pass two names (library
      // and object file), and two addresses (text offset and data
      // offset). I think the best way to handle this is to pass 
      // enough through to be unique, and repeat the ptrace call in
      // the object constructor to get the rest of the needed info.
      // GACK.
      
      string obj_name = string(ptr->ldinfo_filename);
      string member = string(ptr->ldinfo_filename + 
			     (strlen(ptr->ldinfo_filename) + 1));
      Address text_org =(Address) ptr->ldinfo_textorg;
      Address data_org =(Address) ptr->ldinfo_dataorg;

      // I believe that we need to pass this as a pointer so that
      // subclassing will work
      fileDescriptor_AIX *fda = new fileDescriptor_AIX(obj_name, member,
						       text_org, data_org,
						       pid);

      // pass in file (library) name and text relocation
      // address. Object constructor will use this to look up
      // archive member name and data relocation address.

      shared_object *newobj = new shared_object(fda,
						false,true,true,0);
      *result += newobj;      
    }
  while ((ptr->ldinfo_next) &&
	 // Pointer arithmetic -- the ldinfo_next data is all relative.
	 (ptr = (struct ld_info *)(ptr->ldinfo_next + (char *)ptr))); 

  p->setDynamicLinking();
  dynlinked = true;

  return result;

}

bool dynamic_linking::handleIfDueToSharedObjectMapping(process *, vector<shared_object *> **,
				      u_int &, bool &){ return false;}

