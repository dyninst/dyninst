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

// $Id: osfDL.h,v 1.5 2002/12/20 07:49:58 jaw Exp $

#if !defined(osf_dl_hdr)
#define osf_dl_hdr

#include "common/h/Vector.h"
#include "dyninstAPI/src/sharedobject.h"
class process;

//
// All platform specific dynamic linking info. is in this class
// each version of this class must have the following funcitons:
// getSharedObjects, isDynamic, handleIfDueToSharedObjectMapping
//
class dynamic_linking {

public:

    dynamic_linking(): dynlinked(false) {} 
    ~dynamic_linking(){}
    
    // get_ld_base_addr: This routine returns the base address of ld.so.1
    // it returns true on success, and false on error
    bool dynamic_linking::get_ld_base_addr(Address &addr, int proc_fd);

    // getSharedObjects: This routine is called before main() to get and
    // process all shared objects that have been mapped into the process's
    // address space
    // returns 0 
    pdvector <shared_object *> *getSharedObjects(process *);

    // handleIfDueToSharedObjectMapping: returns true if the trap was caused
    // by a change to the link maps  
    bool handleIfDueToSharedObjectMapping(process *, pdvector<shared_object *> **,
			       u_int &, bool &);


    // setup all of the dlopen/dlcose traps
    void setMappingHooks(process *);

    // returns true if the executable is dynamically linked 
    bool isDynamic(){ return(dynlinked);}

    Address get_dlopen_addr() const { return dlopen_addr; }

private:
    bool dynlinked;
    Address dlopenRetAddr;
    Address dlcloseRetAddr;
    Address dlopen_addr;
};

#endif
