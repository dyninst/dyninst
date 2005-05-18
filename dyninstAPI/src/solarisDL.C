/*
 * Copyright (c) 1996-2004 Barton P. Miller
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

// $Id: solarisDL.C,v 1.40 2005/05/18 20:14:40 rchen Exp $

#include "dyninstAPI/src/sharedobject.h"
#include "dyninstAPI/src/dynamiclinking.h"
#include "dyninstAPI/src/dyn_lwp.h"
#include "dyninstAPI/src/process.h"
#include "dyninstAPI/src/symtab.h"
#include "common/h/debugOstream.h"

#include <link.h>
#include <libelf.h>
#include <sys/types.h>
#include <procfs.h>
#include <sys/auxv.h>


// initialize: perform initialization tasks on a platform-specific level
bool dynamic_linking::initialize() {
    // First, initialize all platform-specific stuff
    r_debug_addr = 0;
    r_state = 0;
    // First, find if we're a dynamic executable
    pdstring dyn_str = pdstring("DYNAMIC");
    internalSym dyn_sym;
	if( ! proc->findInternalSymbol( dyn_str, true, dyn_sym ) ) { 
        bperr( "Failed to find string DYNAMIC\n");
        return false; 
    }

    // step 2: find the base address and file descriptor of ld.so.1
    // Three-step process.
    // 1) Examine aux vector for address of the loader
    Address ld_base = 0;
    dyn_lwp *replwp = proc->getRepresentativeLWP();
    if(!(this->get_ld_base_addr(ld_base, replwp->auxv_fd()))) {
        return false;
    }
    
    // 2) Examine virtual address map for the name of the object (inode style)
    char ld_name[128+PRMAPSZ];
    if(!(this->get_ld_name(ld_name, ld_base, replwp->map_fd(),
                           proc->getPid()))) { 
        return false;
    }
    
    // 3) Open that file
    int ld_fd = -1;    
    ld_fd = open(ld_name, O_RDONLY, 0);
    if(ld_fd == -1) {
        perror("LD.SO");
        return false;
    } 

    // step 3: get its symbol table and find r_debug
    if (!(this->find_r_debug(ld_fd,ld_base))) { 
        return false;
    }

    if (!(this->find_dlopen(ld_fd,ld_base))) {
        bperr( "WARNING: we didn't find dlopen in ld.so.1\n");
    }

    close(ld_fd);

   proc->setDynamicLinking();
   dynlinked = true;

    return true;
}


// get_ld_base_addr: This routine returns the base address of ld.so.1
// it returns true on success, and false on error
bool dynamic_linking::get_ld_base_addr(Address &addr, int auxv_fd){

    auxv_t auxv_elm;
    lseek(auxv_fd, 0, SEEK_SET);
    while(read(auxv_fd, &auxv_elm, sizeof(auxv_elm)) == sizeof(auxv_elm)) {
        if (auxv_elm.a_type == AT_BASE) {
            addr = (Address)auxv_elm.a_un.a_ptr;
            lseek(auxv_fd, 0, SEEK_SET);
            return true;
        }
        
    }
    return false;
}

// get_ld_name: returns name (in /proc/pid/object/ format) of ld.so.1
// it returns true on success, and false on error
bool dynamic_linking::get_ld_name(char *ld_name, Address ld_base, int map_fd, int pid)
{
    prmap_t map_elm;
    lseek(map_fd, 0, SEEK_SET);
    while(read(map_fd, &map_elm, sizeof(map_elm)) == sizeof(map_elm)) {
        if (map_elm.pr_vaddr == ld_base) {
            sprintf(ld_name, "/proc/%d/object/%s", 
                    pid, map_elm.pr_mapname);
            lseek(map_fd, 0, SEEK_SET);
            return true;
        }
        
    }
    return false;
}

// findFunctionIn_ld_so_1: this routine finds the symbol table for ld.so.1 and 
// parses it to find the address of symbol r_debug
// it returns false on error
bool dynamic_linking::findFunctionIn_ld_so_1(pdstring f_name, int ld_fd, 
					     Address ld_base_addr, 
					     Address *f_addr, int st_type)
{
    bool result = false;
    Elf_X elf;
    Elf_X_Shdr shstrscn;
    Elf_X_Data shstrdata;
    Elf_X_Shdr symscn;
    Elf_X_Shdr strscn;
    Elf_X_Data symdata;
    Elf_X_Data strdata;

    const char *shnames;

    lseek(ld_fd, 0, SEEK_SET);
    elf = Elf_X(ld_fd, ELF_C_READ);
    if (elf.isValid()) shstrscn = elf.get_shdr( elf.e_shstrndx() );
    if (shstrscn.isValid()) shstrdata = shstrscn.get_data();
    if (shstrdata.isValid()) shnames = shstrdata.get_string();

    if (elf.isValid() && shstrscn.isValid() && shstrdata.isValid()) {
	for (int i = 0; i < elf.e_shnum(); ++i) {
	    Elf_X_Shdr shdr = elf.get_shdr(i);
	    if (!shdr.isValid()) return false;
	    const char* name = (const char *) &shnames[shdr.sh_name()];

	    if (P_strcmp(name, ".symtab") == 0) {
		symscn = shdr;

	    } else if (P_strcmp(name, ".strtab") == 0) {
		strscn = shdr;
	    }
	}

	if (strscn.isValid()) symdata = symscn.get_data();
	if (symscn.isValid()) strdata = strscn.get_data();
	if (symdata.isValid() && strdata.isValid()) {
	    Elf_X_Sym syms = symdata.get_sym();
	    const char* strs = strdata.get_string();

	    if (f_addr != NULL) *f_addr = 0;

	    for (u_int i = 0; i < syms.count(); ++i) {
		if (syms.st_shndx(i) != SHN_UNDEF) {
		    if (syms.ST_TYPE(i) == st_type) {
			pdstring name = pdstring(&strs[ syms.st_name(i) ]);
			if (name == f_name) {
			    if (f_addr != NULL) {
				*f_addr = syms.st_value(i) + ld_base_addr;
			    }
			    result = true;
			    break;
			}
		    }
		}
	    }
	}
    }
    elf.end();

    return result;
}

// find_r_debug: this routine finds the symbol table for ld.so.1, and 
// parses it to find the address of symbol r_debug
// it returns false on error
bool dynamic_linking::find_r_debug(int ld_fd, Address ld_base_addr){
  return findFunctionIn_ld_so_1("r_debug", ld_fd, ld_base_addr, &r_debug_addr,
        STT_OBJECT);
}

// find_dlopen: this routine finds the symbol table for ld.so.1, and 
// parses it to find the address of symbol dlopen
// it returns false on error
bool dynamic_linking::find_dlopen(int ld_fd, Address ld_base_addr){
  return findFunctionIn_ld_so_1("_dlopen", ld_fd, ld_base_addr, &dlopen_addr,
        STT_FUNC);
}

// set_r_brk_point: this routine instruments the code pointed to by
// the r_debug.r_brk (the linkmap update routine).  Currently this code  
// corresponds to no function in the symbol table and consists of only
// 2 instructions on sparc-solaris (retl nop).  Also, the library that 
// contains these instrs is the runtime linker which is not parsed like
// other shared objects, so adding instrumentation here is ugly. 

sharedLibHook::sharedLibHook(process *p, sharedLibHookType t, Address b) 
        : proc_(p), type_(t), breakAddr_(b) {


    // Before putting in the breakpoint, save what is currently at the
    // location that will be overwritten.
    proc_->readDataSpace((void *)breakAddr_, SLH_SAVE_BUFFER_SIZE,
                         (void *)saved_, true);
#if defined(arch_sparc)
    // because there is no pdFunction, instPoint, or image associated with
    // this instrumentation point we can't use all the normal routines to
    // insert base tramp and minitrap code here.  Instead we replace the 
    // retl instruction  with a trap instruction.   Then in paradynd in  
    // the routine that handles the sigtrap for this case we will simulate
    // the retl instruction by changing the value of the %o7 register
    // so that it looks like the retl instruction has executed
    instruction trap_insn;
    trap_insn.raw = BREAK_POINT_INSN;
    proc_->writeDataSpace((caddr_t)breakAddr_,
                         sizeof(instruction),&trap_insn.raw);
#else // x86
    instruction trap_insn((const unsigned char*)"\017\013\0220\0220",
        ILLEGAL, 4);
    proc_->writeDataSpace((void *)breakAddr_, 4, trap_insn.ptr());
    //proc->SetIllForTrap();
#endif
}

sharedLibHook::~sharedLibHook() {
    proc_->writeDataSpace((void *)breakAddr_, SLH_SAVE_BUFFER_SIZE, saved_);
}


// processLinkMaps: This routine is called by getSharedObjects to  
// process all shared objects that have been mapped into the process's
// address space.  This routine reads the link maps from the application 
// process to find the shared object file base mappings. It returns 0 on error.
pdvector<shared_object *> *dynamic_linking::processLinkMaps() {
   r_debug debug_elm;
   if(!proc->readDataSpace((caddr_t)(r_debug_addr),
                        sizeof(r_debug),(caddr_t)&(debug_elm),true)) {
      // bperr("read d_ptr_addr failed r_debug_addr = 0x%lx\n",r_debug_addr);
      return 0;
   }

   // get each link_map object
   bool first_time = true;
   Link_map *next_link_map = debug_elm.r_map;
   Address next_addr = (Address)next_link_map; 
   pdvector<shared_object*> *shared_objects = new pdvector<shared_object*>;
   while(next_addr != 0){
      Link_map link_elm;
      if(!proc->readDataSpace((caddr_t)(next_addr),
                           sizeof(Link_map),(caddr_t)&(link_elm),true)) {
         logLine("read next_link_map failed\n");
         return 0;
      }
      // get file name
      char f_name[256]; // assume no file names greater than 256 chars
      // check to see if reading 256 chars will go out of bounds
      // of data segment
      u_int f_amount = 256;
      bool done = false;
      for(u_int i=0; (i<256) && (!done); i++){
         if(!proc->readDataSpace((caddr_t)((u_int)(link_elm.l_name)+i),
                              sizeof(char),(caddr_t)(&(f_name[i])),true)){
         }
         if(f_name[i] == '\0'){
            done = true;
            f_amount = i+1;
         }
      }
      f_name[f_amount-1] = '\0';
      pdstring obj_name = pdstring(f_name);

      parsing_cerr << 
         "dynamicLinking::processLinkMaps(): file name of next shared obj="
                     << obj_name << endl;

      // create a shared_object and add it to the list
      // kludge: ignore the entry if it has the same name as the
      // executable file...this seems to be the first link-map entry
      // VG(09/25/01): also ignore if address is 65536 or name is (unknown)
      if(obj_name != proc->getImage()->file() && 
         obj_name != proc->getImage()->name() &&
         link_elm.l_addr != 65536 &&
         obj_name != "(unknown)"
         //strncmp(obj_name.c_str(), "(unknown)", 10)
         ) {
         parsing_cerr << 
            "file name doesn't match image, so not ignoring it...firsttime=" 
                        << (int)first_time << endl;

         // kludge for when an exec occurs...the first element
         // in the link maps is the file name of the parent process
         // so in this case, we ignore the first entry
         if((!(proc->wasExeced())) || (proc->wasExeced() && !first_time)) { 
            shared_object *newobj = new shared_object(obj_name,
                                                      link_elm.l_addr,false,true,true,0, proc);
            (*shared_objects).push_back(newobj);
#if defined(BPATCH_LIBRARY)
#if defined(sparc_sun_solaris2_4)
            setlowestSObaseaddr(link_elm.l_addr);
#endif
#endif

         }
      }
      else {
         parsing_cerr << 
            "file name matches that of image, so ignoring...firsttime=" 
                        << (int)first_time << endl;
      }

      first_time = false;
      next_addr = (Address)link_elm.l_next;
   }
   return shared_objects;
   shared_objects = 0;
}


// getLinkMapAddrs: returns a vector of addresses corresponding to all 
// base addresses in the link maps.  Returns 0 on error.
pdvector<Address> *dynamic_linking::getLinkMapAddrs() {

    r_debug debug_elm;
    if(!proc->readDataSpace((caddr_t)(r_debug_addr),
        sizeof(r_debug),(caddr_t)&(debug_elm),true)) {
        // bperr("read d_ptr_addr failed r_debug_addr = 0x%lx\n",r_debug_addr);
        return 0;
    }

    bool first_time = true;
    Link_map *next_link_map = debug_elm.r_map;
    Address next_addr = (Address)next_link_map; 
    pdvector<Address> *link_addresses = new pdvector<Address>;
    while(next_addr != 0) {
	Link_map link_elm;
        if(!proc->readDataSpace((caddr_t)(next_addr),
            sizeof(Link_map),(caddr_t)&(link_elm),true)) {
            logLine("read next_link_map failed\n");
	    return 0;
        }
	// kludge: ignore the first entry
	if(!first_time) { 
	    (*link_addresses).push_back(link_elm.l_addr); 
	}
	else {
	    // bperr("first link map addr 0x%x\n",link_elm.l_addr);
	}

	first_time = false;
	next_addr = (Address)link_elm.l_next;
    }
    return link_addresses;
    link_addresses = 0;
}

// getNewSharedObjects: returns a vector of shared_object one element for 
// newly mapped shared object. 
// Returns NULL if there is an error. 
// And the returned vector needs to be cleaned up.
pdvector<shared_object *> *dynamic_linking::getNewSharedObjects()
{
   r_debug debug_elm;
   if(!proc->readDataSpace((caddr_t)(r_debug_addr),
                        sizeof(r_debug),(caddr_t)&(debug_elm),true)) {
      // bperr("read d_ptr_addr failed r_debug_addr = 0x%lx\n",r_debug_addr);
     return NULL;
   }

   // get each link_map object
   Link_map *next_link_map = debug_elm.r_map;
   Address next_addr = (Address)next_link_map; 

   pdvector<shared_object*> *cur_objs = proc->sharedObjects();
   pdvector<shared_object*> *new_objs = new pdvector<shared_object*>;

   // Explicitly skip the first entry - == to the a.out.
   // You know, when we go to a unified list of objects this skip can
   // be removed....

   Link_map link_elm;
   if(!proc->readDataSpace((caddr_t)(next_addr),
			   sizeof(Link_map),(caddr_t)&(link_elm),true)) {
     logLine("read next_link_map failed\n");
     delete new_objs;
     return NULL;
   }
   next_addr = (Address)link_elm.l_next;

   while(next_addr != 0){

     if(!proc->readDataSpace((caddr_t)(next_addr),
			     sizeof(Link_map),(caddr_t)&(link_elm),true)) {
       logLine("read next_link_map failed\n");
       delete new_objs;
       return NULL;
     }

     // check to see if this is a new shared object 
     bool found = false;
     for(u_int i=0; i < cur_objs->size(); i++){
       shared_object *cur_obj = (*cur_objs)[i];
       // Base address might be 0. If that's true, check by name.
       // Otherwise check by base addr
       if (cur_obj->getBaseAddress() != 0) {
	 if (cur_obj->getBaseAddress() == link_elm.l_addr) {
	   // Already got this one...
	   found = true;
	   break;
	 }
       }
       else {
	 // Not handled yet
	 assert(0 && "Shared object with 0 base address, need to fix!");
       }
     }
     if (!found) {  
       // this is a new shared object, create a shrared_object for it 
       char f_name[256];// assume no file names greater than 256 chars
       // check to see if reading 256 chars will go out of bounds
       // of data segment
       u_int f_amount = 256;
       bool done = false;
       for(u_int i=0; (i<256) && (!done); i++){
	 if(!proc->readDataSpace((caddr_t)((Address)(link_elm.l_name)+i),
				 sizeof(char),(caddr_t)(&(f_name[i])),true)){
	 }
	 if(f_name[i] == '\0'){
	   done = true;
	   f_amount = i+1;
	 }
       }
       f_name[f_amount-1] = '\0';
       pdstring obj_name = pdstring(f_name);
       shared_object *newobj =
	 new shared_object(obj_name, link_elm.l_addr, false,true,true,0, proc);
       (*new_objs).push_back(newobj);
     }

     next_addr = (Address)link_elm.l_next;
   }

   return new_objs;
}


// getSharedObjects: This routine is called after attaching to
// an already running process p, or when a process reaches the breakpoint at
// the entry point of main().  It gets all shared objects that have been
// mapped into the process's address space, and returns 0 on error or if 
// there are no shared objects.
// The assumptions are that the dynamic linker has already run, and that
// a /proc file descriptor has been opened for the application process.
// This is a very kludgy way to get this stuff, but it is the only way to
// do it until verision 2.6 of Solaris is released.
// TODO: this should also set a breakpoint in the r_brk routine that will
// catch future changes to the linkmaps (from dlopen and dlclose)
// dlopen events should result in a call to addSharedObject
// dlclose events should result in a call to removeASharedObject
pdvector< shared_object *> *dynamic_linking::getSharedObjects() {
    assert(r_debug_addr);
    
    pdvector<shared_object *> *result = this->processLinkMaps();

    return result;
}


// findChangeToLinkMaps: This routine returns a vector of shared objects
// that have been deleted or added to the link maps as indicated by
// change_type.  If an error occurs it sets error_occured to true.
pdvector <shared_object *> *dynamic_linking::findChangeToLinkMaps(u_int change_type,
								  bool &error_occured) {
   // get list of current shared objects
   pdvector<shared_object *> *curr_list = proc->sharedObjects();
   if((change_type == 2) && !curr_list) {
      error_occured = true;
      return 0;
   }

   // if change_type is add then figure out what has been added
   if(change_type == 1){
      // create a vector of addresses of the current set of shared objects
     pdvector <shared_object *> *new_shared_objs = getNewSharedObjects();
     if (new_shared_objs == NULL)
       error_occured = true;
     return new_shared_objs; 
   }
   // if change_type is remove then figure out what has been removed
   else if((change_type == 2) && curr_list) {
      // create a list of base addresses from the linkmaps and
      // compare them to the addr in vector of shared object to see
      // what has been removed
      pdvector<Address> *addr_list = getLinkMapAddrs();
      if(addr_list) {
         pdvector <shared_object *> *remove_list = new pdvector<shared_object*>;
         // find all shared objects that have been removed
         for(u_int i=0; i < curr_list->size(); i++){
            Address curr_addr = ((*curr_list)[i])->getBaseAddress(); 
            bool found = false;
            for(u_int j=0; j < addr_list->size(); j++){
               if(curr_addr == (*addr_list)[j]){
                  found = true;
                  break;
               }
            }
            if(!found) {
               (*remove_list).push_back((*curr_list)[i]);
            }
         }
         delete addr_list;
         return remove_list; 
         remove_list = 0;
      }
   }
   error_occured = true;
   return 0;
}




// handleIfDueToSharedObjectMapping: returns true if the trap was caused
// by a change to the link maps,  If it is, and if the linkmaps state is
// safe, it processes the linkmaps to find out what has changed...if it
// is not safe it sets the type of change currently going on (specified by
// the value of r_debug.r_state in link.h
// The added or removed shared objects are returned in changed_objects
// the change_type value is set to indicate if the objects have been added 
// or removed
bool dynamic_linking::handleIfDueToSharedObjectMapping(pdvector<shared_object*>  **changed_objects,
				u_int &change_type,
				bool &error_occured){ 

   struct dyn_saved_regs regs;

   error_occured = false;

   // multi-threaded: possible one of many threads hit the breakpoint

   pdvector<Frame> activeFrames;
   if (!proc->getAllActiveFrames(activeFrames)) {
      return false;
   }

   dyn_lwp *brk_lwp = NULL;
   sharedLibHook *hook = NULL;
   for (unsigned frame_iter = 0; frame_iter < activeFrames.size();frame_iter++)
   {
       hook = reachedLibHook(activeFrames[frame_iter].getPC());
       if (hook) {
           brk_lwp = activeFrames[frame_iter].getLWP();
           break;
       }
   }

   if (brk_lwp || force_library_load) {
       // find out what has changed in the link map
      // and process it
      r_debug debug_elm;
      if(!proc->readDataSpace((caddr_t)(r_debug_addr),
                              sizeof(r_debug),(caddr_t)&(debug_elm),true)) {
          // bperr("read failed r_debug_addr = 0x%x\n",r_debug_addr);
          error_occured = true;
          return true;
      }
      
      // if the state of the link maps is consistent then we can read
      // the link maps, otherwise just set the r_state value
      change_type = r_state;   // previous state of link maps 
      r_state = debug_elm.r_state;  // new state of link maps
      if( debug_elm.r_state == 0) {
         // figure out how link maps have changed, and then create
         // a list of either all the removed shared objects if this
         // was a dlclose or the added shared objects if this was a dlopen
      
         // kludge: the state of the first add can get screwed up
         // so if both change_type and r_state are 0 set change_type to 1
         if(change_type == 0) change_type = 1;
         *changed_objects = findChangeToLinkMaps(change_type,
                                                 error_occured);
      
#if defined(BPATCH_LIBRARY)
#if defined(sparc_sun_solaris2_4)
         if(change_type == 1) { // RT_ADD
            for(int index = 0; index < (*changed_objects)->size(); index++){
               shared_object *chobj = (*(*changed_objects))[index];
               char *tmpStr = new char[1+strlen(chobj->getName().c_str())];
               strcpy(tmpStr, chobj->getName().c_str());
               if( !strstr(tmpStr, "libdyninstAPI_RT.so") && 
                   !strstr(tmpStr, "libelf.so")){
                  //bperr(" dlopen: %s \n", tmpStr);
                  chobj->openedWithdlopen();
               }
               setlowestSObaseaddr(chobj->getBaseAddress());
               delete [] tmpStr;	
            }
         }
#endif
#endif
      } 

      // Don't need to reset PC
      if (force_library_load) return true;
      else {
          assert(brk_lwp);
          
          // Get the registers for this lwp
          brk_lwp->getRegisters(&regs);
#if (arch_sparc)
          // change the pc so that it will look like the retl instr 
          // completed: set PC to o7 in current frame
          // we can do this because this retl doesn't correspond to 
          // an instrumentation point, so we don't have to worry about 
          // missing any instrumentation code by making it look like the
          // retl has already happend
          
          // first get the value of the stackpointer
          Address o7reg = regs.theIntRegs[R_O7];
          o7reg += 2*sizeof(instruction);
          if(!(brk_lwp->changePC(o7reg, NULL))) {
              // bperr("error in changePC handleIfDueToSharedObjectMapping\n");
              error_occured = true;
              return true;
          }
#else //x86
      // set the pc to the "ret" instruction
          Address next_pc = regs[R_PC] + sizeof(instruction);
          if (!brk_lwp->changePC(next_pc))
              error_occured = true;
#endif
          return true;
      }

  }
  return false;
}


// This function performs all initialization necessary to catch shared object
// loads and unloads (symbol lookups and trap setting)
bool dynamic_linking::installTracing()
{
    assert(r_debug_addr);
    
    r_debug debug_elm;
    if(!proc->readDataSpace((caddr_t)(r_debug_addr),
                            sizeof(r_debug),(caddr_t)&(debug_elm),true)) {
        // bperr("read d_ptr_addr failed r_debug_addr = 0x%lx\n",r_debug_addr);
        return 0;
    }

    sharedLibHook *sharedHook = new sharedLibHook(proc, SLH_UNKNOWN, // not used
                                                  debug_elm.r_brk);
    sharedLibHooks_.push_back(sharedHook);

    return true;
}



