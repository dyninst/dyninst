/*
 * Copyright (c) 1996 Barton P. Miller
 * 
 * We provide the Paradyn Parallel Performance Tools (below
 * described as Paradyn") on an AS IS basis, and do not warrant its
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

#if !defined(solaris_dl_hdr)
#define solaris_dl_hdr

#include "util/h/Vector.h"
#include "dyninstAPI/src/sharedobject.h"
class process;

//
// All platform specific dynamic linking info. is in this class
// each version of this class must have the following funcitons:
// getSharedObjects, addSharedObject, isDynamic
//
class dynamic_linking {

public:

    dynamic_linking(){ dynlinked = false; r_debug_addr = 0;} 
    ~dynamic_linking(){}
    
    // getProcessSharedObjects: This routine is called after attaching to
    // an already running process p, or when a process reaches the breakpoint at
    // the entry point of main().  It gets all shared objects that have been
    // mapped into the process's address space
    // returns 0 on error or if there are no shared objects
    vector< shared_object *> *getSharedObjects(process *p);

    // addASharedObject: This routine is called whenever a new shared object
    // has been loaded by the run-time linker
    // It processes the image, creates new resources
    // Currently, this is not implemented, because we are not handleing
    // adding shared objects after the executable starts executing main()
    shared_object *addSharedObject(process *){ return 0;}

    // returns true if the executable is dynamically linked 
    bool isDynamic(){ return(dynlinked);}


private:
   bool  dynlinked;
   u_int r_debug_addr;

   // get_ld_base_addr: This routine returns the base address of ld.so.1
   // it returns true on success, and false on error
   bool dynamic_linking::get_ld_base_addr(u_int &addr, int proc_fd);

   // find_r_debug: this routine finds the symbol table for ld.so.1, and
   // parses it to find the address of symbol r_debug
   // it returns false on error
   bool find_r_debug(u_int ld_fd,u_int ld_base_addr);

   // processLinkMaps: This routine is called by getSharedObjects to
   // process all shared objects that have been mapped into the process's
   // address space.  This routine reads the link maps from the application
   // process to find the shared object file base mappings. returns 0 on error
   vector<shared_object *> *processLinkMaps(process *p);

};

#endif


