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

// $Id: solarisDL.h,v 1.12 2002/02/05 17:01:39 chadd Exp $

#if !defined(solaris_dl_hdr)
#define solaris_dl_hdr

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

    dynamic_linking(): dynlinked(false),r_debug_addr(0),dlopen_addr(0),
		       r_brk_addr(0),r_state(0), brkpoint_set(false),lowestSObaseaddr(0)  {}
    ~dynamic_linking(){}
    
    // getProcessSharedObjects: This routine is called after attaching to
    // an already running process p, or when a process reaches the breakpoint at
    // the entry point of main().  It gets all shared objects that have been
    // mapped into the process's address space
    // returns 0 on error or if there are no shared objects
    vector< shared_object *> *getSharedObjects(process *p);

    // returns true if the executable is dynamically linked 
    bool isDynamic(){ return(dynlinked);}

    // handleIfDueToSharedObjectMapping: returns true if the trap was caused
    // by a change to the link maps,  If it is, and if the linkmaps state is
    // safe, it processes the linkmaps to find out what has changed...if it
    // is not safe it sets the type of change currently going on (specified by
    // the value of r_debug.r_state in link.h
    // The added or removed shared objects are returned in changed_objects, 
    // and the change_type value is set to indicate if the object has been 
    // added or removed. On error error_occured is true.
    bool handleIfDueToSharedObjectMapping(process *proc,
			vector<shared_object *> **changed_objects,
			u_int &change_type,
			bool &error_occured);
    Address get_dlopen_addr() const { return dlopen_addr; }
    Address get_r_brk_addr() const { return r_brk_addr; }
    
#if defined(BPATCH_LIBRARY) 
   // unset_r_brk_point: this routine removes the breakpoint in the code
   // pointed to by r_debug.r_brk, which was previously set by
   // set_r_brk_point.
   // XXX We may want to make this private and call it from some general
   //     cleanup routine instead letting it be called directly.
   bool unset_r_brk_point(process *proc);
#endif

	Address getlowestSObaseaddr(){ return lowestSObaseaddr; } 
private:
   bool  dynlinked;
   Address r_debug_addr;
   Address dlopen_addr;
   Address r_brk_addr;   // this routine consists of retl and nop instrs, used
		         // in handleSigChild to determine what trap occured 
   u_int r_state;  // either 0 (RT_CONSISTENT), 1 (RT_ADD), or 2 (RT_DELETE)
   bool brkpoint_set; // true if brkpoint set in r_brk
   instPoint *r_brk_instPoint; // used to instrument r_brk

	Address lowestSObaseaddr; 

	void setlowestSObaseaddr(Address baseaddr){ 
		if(lowestSObaseaddr ==0 || baseaddr < lowestSObaseaddr){
			lowestSObaseaddr = baseaddr;
		}
	}

	bool dlopenUsed; 


#if defined(BPATCH_LIBRARY)
#if defined(i386_unknown_solaris2_5)
   static const u_int R_BRK_SAVE_BYTES = 4;
#else /* Sparc */
   static const u_int R_BRK_SAVE_BYTES = sizeof(instruction);
#endif

   char r_brk_save[R_BRK_SAVE_BYTES];
#endif /* BPATCH_LIBRARY */

   // get_ld_base_addr: This routine returns the base address of ld.so.1
   // it returns true on success, and false on error
   bool dynamic_linking::get_ld_base_addr(Address &addr, int proc_fd);

   // find_function: this routine finds the symbol table for ld.so.1, and
   // parses it to find the address of f_name
   // fills in f_name_addr with the address of f_name
   // it returns false on error
   // f_name_addr can't be passed in by reference since it makes calling it
   // with a NULL parameter unpleasant.
   bool findFunctionIn_ld_so_1(string f_name, int ld_fd, 
			       Address ld_base_addr, Address *f_name_addr, 
			       int st_type);

   // find_r_debug: this routine finds the symbol table for ld.so.1, and
   // parses it to find the address of symbol r_debug
   // it returns false on error
   bool find_r_debug(int ld_fd, Address ld_base_addr);

   // find_dlopen: this routine finds the symbol table for ld.so.1, and
   // parses it to find the address of symbol r_debug
   // it returns false on error
   bool find_dlopen(int ld_fd, Address ld_base_addr);   

   // set_r_brk_point: this routine instruments the code pointed to by
   // the r_debug.r_brk (the linkmap update routine).  Currently this code
   // corresponds to no function in the symbol table and consists of only
   // 2 instructions (retl nop).  This makes instrumenting it a little 
   // bit kludgey
   bool set_r_brk_point(process *proc);

   // processLinkMaps: This routine is called by getSharedObjects to
   // process all shared objects that have been mapped into the process's
   // address space.  This routine reads the link maps from the application
   // process to find the shared object file base mappings. returns 0 on error
   vector<shared_object *> *processLinkMaps(process *p);

   // findChangeToLinkMaps: This routine returns a shared objects
   // that have been deleted or added to the link maps as indicated by
   // change_type.  If an error occurs it sets error_occured to true.
   vector<shared_object *> *findChangeToLinkMaps(process *p, u_int change_type,
				               bool &error_occured);

   // getLinkMapAddrs: returns a vector of addresses corresponding to all 
   // base addresses in the link maps.  Returns 0 on error.
   vector<Address> *getLinkMapAddrs(process *p);

   // getNewSharedObjects: returns a vector of shared_object one element for 
   // newly mapped shared object.  old_addrs contains the addresses of the
   // currently mapped shared objects. Sets error_occured to true, and 
   // returns 0 on error.
   vector<shared_object *> *getNewSharedObjects(process *p,
						vector<Address> *old_addrs,
						bool &error_occured);

};

#endif


