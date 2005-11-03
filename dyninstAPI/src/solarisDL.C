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

// $Id: solarisDL.C,v 1.44 2005/11/03 05:21:07 jaw Exp $

#include "dyninstAPI/src/mapped_object.h"
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
    Symbol dyn_sym;
    if( ! proc->getSymbolInfo(dyn_str, dyn_sym)) {
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

    P_close(ld_fd);

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

    const char *shnames = NULL;

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
        : proc_(p), type_(t), breakAddr_(b), loadinst_(NULL) {


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
    codeGen gen(instruction::size());
    instruction::generateTrap(gen);
    proc_->writeDataSpace((caddr_t)breakAddr_,
                          gen.used(),
                          gen.start_ptr());
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
bool dynamic_linking::processLinkMaps(pdvector<fileDescriptor> &descs) {
   r_debug debug_elm;
   if(!proc->readDataSpace((caddr_t)(r_debug_addr),
                        sizeof(r_debug),(caddr_t)&(debug_elm),true)) {
      // bperr("read d_ptr_addr failed r_debug_addr = 0x%lx\n",r_debug_addr);
      return 0;
   }

   // get each link_map object
   Link_map *next_link_map = debug_elm.r_map;
   Address next_addr = (Address)next_link_map; 
   
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

      // create a mapped_object and add it to the list
      // kludge: ignore the entry if it has the same name as the
      // executable file...this seems to be the first link-map entry
      // VG(09/25/01): also ignore if address is 65536 or name is (unknown)
      if(obj_name != proc->getAOut()->fileName() && 
         obj_name != proc->getAOut()->fullName() &&
         link_elm.l_addr != 65536 &&
         obj_name != "(unknown)"
         //strncmp(obj_name.c_str(), "(unknown)", 10)
         ) {
          
          fileDescriptor desc = fileDescriptor(obj_name, link_elm.l_addr,
                                               link_elm.l_addr,
                                               true);
          descs.push_back(desc);          
      }
      
      next_addr = (Address)link_elm.l_next;
   }

   return true;
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

// handleIfDueToSharedObjectMapping: returns true if the trap was caused
// by a change to the link maps,  If it is, and if the linkmaps state is
// safe, it processes the linkmaps to find out what has changed...if it
// is not safe it sets the type of change currently going on (specified by
// the value of r_debug.r_state in link.h
// The added or removed shared objects are returned in changed_objects
// the change_type value is set to indicate if the objects have been added 
// or removed
bool dynamic_linking::handleIfDueToSharedObjectMapping(pdvector<mapped_object*> &changed_objects,
                                                       u_int &change_type) {

   struct dyn_saved_regs regs;

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
          return false;
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
         if(change_type == 0) change_type = SHAREDOBJECT_ADDED;
         findChangeToLinkMaps(change_type, changed_objects);
      } 
      
      // Don't need to reset PC
      if (!force_library_load) {
          assert(brk_lwp);
          
          // Get the registers for this lwp
          brk_lwp->getRegisters(&regs);
#if defined(arch_sparc)
          // change the pc so that it will look like the retl instr 
          // completed: set PC to o7 in current frame
          // we can do this because this retl doesn't correspond to 
          // an instrumentation point, so we don't have to worry about 
          // missing any instrumentation code by making it look like the
          // retl has already happend
          
          // first get the value of the stackpointer
          Address o7reg = regs.theIntRegs[R_O7];
          o7reg += 2*instruction::size();
          if(!(brk_lwp->changePC(o7reg, NULL))) {
              // bperr("error in changePC handleIfDueToSharedObjectMapping\n");
              return false;
          }
#else //x86
      // set the pc to the "ret" instruction
          Address next_pc = regs[R_PC] + instruction::size();
          if (!brk_lwp->changePC(next_pc))
              return false;
#endif
      }
      if (changed_objects.size() == 0) change_type = 0;
      return true;

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



