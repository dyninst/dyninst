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

// $Id: aixDL.C,v 1.10 2001/06/13 19:50:08 hollings Exp $

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
  static bool did_ptrace_multi = false;
  // We hope that we don't get more than 1024 libraries loaded.
  ptr = (struct ld_info *) malloc (1024*sizeof(*ptr));
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
    perror("PT_LDINFO");
    fprintf(stderr, "For process %d\n", pid);
    statusLine("Unable to get loader info about process, application aborted");
    showErrorCallback(43, "Unable to get loader info about process, application aborted");
    return 0;
  }

  // turn on 'multiprocess debugging', which allows ptracing of both the
  // parent and child after a fork.  In particular, both parent & child will
  // TRAP after a fork.  Also, a process will TRAP after an exec (after the
  // new image has loaded but before it has started to execute at all).
  // Note that turning on multiprocess debugging enables two new values to be
  // returned by wait(): W_SEWTED and W_SFWTED, which indicate stops during
  // execution of exec and fork, respectively.
  // Should do this in loadSharedObjects
  // Note: this can also get called when we incrementally find a shared object.
  // So? :)
  if (!did_ptrace_multi) {
    ptrace(PT_MULTI, pid, 0, 1, 0);
    did_ptrace_multi = true;
  }

  if (!ptr->ldinfo_next)
    {
      // Non-shared object, return 0
      return 0;
    }

  // Skip the first element, which appears to be the executable file.
  ptr = (struct ld_info *)(ptr->ldinfo_next + (char *)ptr);

  // We want to fill in this vector.
  vector<shared_object *> *result = new(vector<shared_object *>);

  // So we have this list of ldinfo structures. This will include the executable and
  // all shared objects it has loaded. Parse it.
  do
    {
      // Gripe if the shared object we have loaded has been deleted
      // If this is so, the file descriptor has been set to -1
      // This can be a problem, since we expect to find symbol table
      // information from the file.
      if (fstat (ptr->ldinfo_fd, &ld_stat) < 0) {
	// We were given a file handle that is invalid. We get this
	// behavior if there's a library that's been loaded into memory,
	// but it's disk equivalent is no longer there. Examples:
	// loaded a library and deleted it, loaded a library and then
	// copied in a fresh copy of that library, i.e. the nightly build.
	// Solution: take the pathname we have and construct a new
	// handle.
	ptr->ldinfo_fd = open(ptr->ldinfo_filename, O_RDONLY);
	if (ptr->ldinfo_fd == -1) {
	  // Sucks to be us
	  //fprintf(stderr, "aixDL.C:getSharedObjects: library %s has disappeared!\n", ptr->ldinfo_filename);
	  perror("aixDL.C:getSharedObjects");
	  // Set the memory offsets to -1 so we don't try to parse it.
	  ptr->ldinfo_textorg = (void *)-1;
	  ptr->ldinfo_dataorg = (void *)-1;
	}
      }

      string obj_name = string(ptr->ldinfo_filename);
      string member = string(ptr->ldinfo_filename + 
			     (strlen(ptr->ldinfo_filename) + 1));
      Address text_org =(Address) ptr->ldinfo_textorg;
      Address data_org =(Address) ptr->ldinfo_dataorg;
#ifdef DEBUG
      fprintf(stderr, "%s:%s (%x/%x)\n",
	      obj_name.string_of(), member.string_of(),
	      text_org, data_org);
#endif /* DEBUG */
      // I believe that we need to pass this as a pointer so that
      // subclassing will work
      fileDescriptor_AIX *fda = new fileDescriptor_AIX(obj_name, member,
						       text_org, data_org,
						       pid);
      shared_object *newobj = new shared_object(fda,
						false,true,true,0);
      (*result).push_back(newobj);      

      // Close the file descriptor we're given
      close(ptr->ldinfo_fd);
    }
  while ((ptr->ldinfo_next) &&
	 // Pointer arithmetic -- the ldinfo_next data is all relative.
	 (ptr = (struct ld_info *)(ptr->ldinfo_next + (char *)ptr))); 

  p->setDynamicLinking();
  dynlinked = true;

  free(ptr);
  return result;

}

// handleIfDueToSharedObjectMapping: returns true if the trap was caused
// by a change to the link maps
// p - process we're dealing with
// changed_objects -- set to list of new objects
// change_type -- set to 1 if added, 2 if removed
// error_occurred -- duh
// return value: true if there was a change to the link map,
// false otherwise
bool dynamic_linking::handleIfDueToSharedObjectMapping(process *p,
						       vector<shared_object *> **changed_objects,
						       u_int &change_type, 
						       bool &error_occurred) {
  // Well, this is easy, ain't it?
  // List of current shared objects
  vector <shared_object *> *curr_list = p->sharedObjects();
  // List of new shared objects (since we cache parsed objects, we
  // can go overboard safely)
  vector <shared_object *> *new_list = getSharedObjects(p);

  error_occurred = false; // Boy, we're optimistic.
  change_type = 0; // Assume no change

  // I've seen behavior where curr_list should be null, but instead has zero size
  if (!curr_list || (curr_list->size() == 0)) {
    change_type = 0;
    return false;
  }

  // Check to see if something was returned by getSharedObjects
  // They all went away? That's odd
  if (!new_list) {
    error_occurred = true;
    change_type = 2;
    return false;
  }

  if (new_list->size() == curr_list->size())
    change_type = 0;
  else if (new_list->size() > curr_list->size())
    change_type = 1; // Something added
  else
    change_type = 2; // Something removed


  *changed_objects = new(vector<shared_object *>);

  // if change_type is add, figure out what is new
  if (change_type == 1) {
    // Compare the two lists, and stick anything new on
    // the added_list vector (should only be one, but be general)
    bool found_object = false;
    for (u_int i = 0; i < new_list->size(); i++) {
      for (u_int j = 0; j < curr_list->size(); j++) {
	// Check for equality -- file descriptor equality, nothing
	// else is good enough.
	shared_object *sh1 = (*new_list)[i];
	shared_object *sh2 = (*curr_list)[j];
	fileDescriptor *fd1 = sh1->getFileDesc();
	fileDescriptor *fd2 = sh2->getFileDesc();

	if (*fd1 == *fd2) {
	  found_object = true;
	  break;
	}
      }
      // So if found_object is true, we don't care. Set it to false and loop. Otherwise,
      // add this to the new list of objects
      if (!found_object) {
	(**changed_objects).push_back(((*new_list)[i]));
      }
      else found_object = false; // reset
    }
  }
  else if (change_type == 2) {
    // Compare the two lists, and stick anything deleted on
    // the removed_list vector (should only be one, but be general)
    bool found_object = false;
    // Yes, this almost identical to the previous case. The for loops
    // are reversed, but that's it. Basically, find items in the larger
    // list that aren't in the smaller. 
    for (u_int j = 0; j < curr_list->size(); j++) {
      for (u_int i = 0; i < new_list->size(); i++) {
	// Check for equality -- file descriptor equality, nothing
	// else is good enough.
	shared_object *sh1 = (*new_list)[i];
	shared_object *sh2 = (*curr_list)[j];
	fileDescriptor *fd1 = sh1->getFileDesc();
	fileDescriptor *fd2 = sh2->getFileDesc();
	
	if (*fd1 == *fd2) {
	  found_object = true;
	  break;
	}
      }
      // So if found_object is true, we don't care. Set it to false and loop. Otherwise,
      // add this to the new list of objects
      if (!found_object) {
	(**changed_objects).push_back(((*new_list)[j]));
      }
      else found_object = false; // reset
    }
  }
  
  // Check to see that there is something in the new list
  if ((*changed_objects)->size() == 0) {
    change_type = 0; // no change after all
    delete changed_objects; // Save memory
    changed_objects = 0;
    return false;
  }
  return true;
}

