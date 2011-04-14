/*
 * Copyright (c) 1996-2011 Barton P. Miller
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

// $Id: dynamiclinking.h,v 1.20 2008/02/23 02:09:05 jaw Exp $

#if !defined(dynamic_linking_h)
#define dynamic_linking_h

#include "common/h/Vector.h"
#include "common/h/Types.h"
#include <string>

class process;
class dyn_lwp;
class instMapping;
class mapped_object;
class fileDescriptor;
class EventRecord;


// Encapsulate a shared object event/mapping pair

enum sharedLibHookType {
    SLH_INSERT_PRE, // executed before a shared lib is added
    SLH_INSERT_POST, // Executed after a shared lib is added
    SLH_REMOVE_PRE, // Executed before a shared lib is removed
    SLH_REMOVE_POST, // Executed after a shared lib is removed
    SLH_UNKNOWN
};

// 16 on ia-64, smaller on other platforms (2*sizeof instruction)
#define SLH_SAVE_BUFFER_SIZE 16

class sharedLibHook {
  public:
    // Insert a hook into the shared library process
    sharedLibHook(process *p, sharedLibHookType t, Address b);
    sharedLibHook(process *p, sharedLibHookType t, instMapping *inst);
    // Remove the hook
    ~sharedLibHook();

    // Fork constructor
    sharedLibHook(const sharedLibHook *pSLH,
                  process *child);
        
    bool reachedBreakAddr(Address b) const;
    sharedLibHookType eventType() const { return type_; };
    
  private:
    process *proc_;
    sharedLibHookType type_;
    unsigned char saved_[SLH_SAVE_BUFFER_SIZE];
    Address breakAddr_;
    instMapping *loadinst_;
};

// Class which encapsulates hooking to dlopen and dlclose. All platform specific
// code must be in the private section.
class dynamic_linking {

public:

    // Due to process-specific initialization this can be found in the .C files
    dynamic_linking(process *p);
    
    // Fork constructor
    dynamic_linking(const dynamic_linking *pDL, process *child);
    ~dynamic_linking();

    // Initialize the dynamic_linking structure (normally grabbing addresses)
    // Initialization that must be done before we can get a list of loaded shared objects
    bool initialize();
    
    // getProcessSharedObjects: This routine is called after attaching to
    // an already running process p, or when a process reaches the breakpoint at
    // the entry point of main().  It gets all shared objects that have been
    // mapped into the process's address space
    // returns 0 on error or if there are no shared objects
    bool getSharedObjects(pdvector<mapped_object *> &);

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
    // This function normally only runs if we're at the dlopen/dlclose break address
    bool handleIfDueToSharedObjectMapping(EventRecord &ev,
                                          pdvector<mapped_object *> &changed_objects,
                                          pdvector<bool> &is_new_object);
    bool decodeIfDueToSharedObjectMapping(EventRecord &, unsigned int &change_type);
    bool getChangedObjects(EventRecord &,pdvector<mapped_object *> &changed_objects);
                           

    // Force running handleIfDue... above
    void set_force_library_check(){ force_library_load = true; }
    void unset_force_library_check(){ force_library_load = false; }
    
    Address get_dlopen_addr() const { return dlopen_addr; }

    // External access to our list of traps (used by linux)
    sharedLibHook *reachedLibHook(Address);
    dyn_lwp *findLwpAtLibHook(process *p, sharedLibHook **);
    
    // Does whatever is necessary to detect dlopen/dlclose events
    bool installTracing();
    bool uninstallTracing();
    
	Address getlowestSObaseaddr(){ return lowestSObaseaddr; } 
	
  private:

    // Entirely cross-platform
    process *proc;
    
    bool  dynlinked;

    Address dlopen_addr; // Address of dlopen, used for calling it directly

    pdvector<sharedLibHook *> sharedLibHooks_;

    bool instru_based;
    
    // Whether to ignore the PC check in handleIfDue...
    bool force_library_load;

    // Used for saveTheWorld
	Address lowestSObaseaddr; 
    
	void setlowestSObaseaddr(Address baseaddr){ 
		if(lowestSObaseaddr ==0 || baseaddr < lowestSObaseaddr){
			lowestSObaseaddr = baseaddr;
		}
	}

    // more saveTheWorld
	bool dlopenUsed; 
    
    // processLinkMaps: This routine is called by getSharedObjects to
    // process all shared objects that have been mapped into the process's
    // address space.  This routine reads the link maps from the application
    // process to find the shared object file base mappings. returns 0 on error
    bool processLinkMaps(pdvector<fileDescriptor> &);
    
    // findChangeToLinkMaps: This routine returns a shared objects
    // that have been deleted or added to the link maps as indicated by
    // change_type.  If an error occurs it sets error_occured to true.
    bool findChangeToLinkMaps(pdvector<mapped_object *> &,
                              pdvector<bool> &);

    // getNewSharedObjects: returns a vector of mapped_object one element for 
    // newly mapped shared object. 
    // Returns NULL if there is an error. 
    // And the returned vector needs to be cleaned up.
    bool getNewSharedObjects(pdvector<mapped_object *> &);

    //////////////// PLATFORM SPECIFIC
    // Could this be wrapped into a sub-object?
#if defined(os_linux)
    // Location of link map in process address space
    Address r_debug_addr;

    Address r_brk_target_addr;	// Unused on x86 (could be changed), this is
    // where the IA-64 inserts a return instruction

    unsigned previous_r_state;
    unsigned current_r_state;

    // Gets necessary info from the linker
    bool get_ld_info(Address &addr, unsigned &size, char **path);
    
    // getLinkMapAddrs: returns a vector of addresses corresponding to all 
    // base addresses in the link maps.  Returns 0 on error.
    pdvector<Address> *getLinkMapAddrs();
#endif // os-linux

#if defined(os_solaris)
       Address r_debug_addr;
       u_int r_state;  // either 0 (RT_CONSISTENT), 1 (RT_ADD), or 2 (RT_DELETE)

       // Return /proc/.../object name for ld.so.1
       bool get_ld_name(char *ld_name, Address ld_base, int map_fd, int pid);
       
       // get_ld_base_addr: This routine returns the base address of ld.so.1
       // it returns true on success, and false on error
       bool get_ld_base_addr(Address &addr, int auxv_fd);

          // find_function: this routine finds the symbol table for ld.so.1, and
       // parses it to find the address of f_name
       // fills in f_name_addr with the address of f_name
       // it returns false on error
       // f_name_addr can't be passed in by reference since it makes calling it
       // with a NULL parameter unpleasant.
       bool findFunctionIn_ld_so_1(std::string f_name, int ld_fd, 
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
       
       // getLinkMapAddrs: returns a vector of addresses corresponding to all 
       // base addresses in the link maps.  Returns 0 on error.
       pdvector<Address> *getLinkMapAddrs();
#endif
#if defined(os_irix)
       pdvector<pdElfObjInfo *> getRldMap();
       pdvector<pdElfObjInfo *> rld_map;
       pdElfObjInfo *libc_obj;
#endif
       
    
    
};

#endif


