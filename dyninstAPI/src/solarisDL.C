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

// $Id: solarisDL.C,v 1.19 2001/10/30 21:02:46 gaburici Exp $

#include "dyninstAPI/src/sharedobject.h"
#include "dyninstAPI/src/solarisDL.h"
#include "dyninstAPI/src/process.h"
#include "dyninstAPI/src/symtab.h"
#include "common/h/debugOstream.h"

extern debug_ostream sharedobj_cerr;

#include <link.h>
#include <libelf.h>
#include <sys/types.h>
#include <sys/procfs.h>
#include <sys/auxv.h>

// get_ld_base_addr: This routine returns the base address of ld.so.1
// it returns true on success, and false on error
bool dynamic_linking::get_ld_base_addr(Address &addr, int proc_fd){

    // get number of auxv_t structs
    int auxvnum=-1;
    if(ioctl(proc_fd, PIOCNAUXV, &auxvnum) != 0) { return false;}    
    if(auxvnum < 1) { return false; }

    // get the array of auxv_t structs
    auxv_t *auxv_elms = new auxv_t[auxvnum];
    // auxv_t auxv_elms[auxvnum];
#ifdef PURE_BUILD
    // explicitly initialize "auxv_elms" struct (to pacify Purify)
    // (at least initialize those components which we actually use)
    for(int i=0; i < auxvnum; i++) {
        auxv_elms[i].a_type=0;
        auxv_elms[i].a_un.a_ptr=(void*)0;
    }
#endif

    if(ioctl(proc_fd, PIOCAUXV, auxv_elms) != 0) { return false; }

    // find the base address of ld.so.1
    for(int i=0; i < auxvnum; i++){
        if(auxv_elms[i].a_type == AT_BASE) {
	    addr = (Address)(auxv_elms[i].a_un.a_ptr);
	    // cout << "ld.so.1 base addr = " << addr << "num " 
	    // << auxvnum << endl;
	    delete [] auxv_elms;
	    return true;
    } }
    delete [] auxv_elms;
    return false;
}

// findFunctionIn_ld_so_1: this routine finds the symbol table for ld.so.1 and 
// parses it to find the address of symbol r_debug
// it returns false on error
bool dynamic_linking::findFunctionIn_ld_so_1(string f_name, int ld_fd, 
					     Address ld_base_addr, 
					     Address *f_addr, int st_type){

    Elf *elfp = 0;
    if ((elfp = elf_begin(ld_fd, ELF_C_READ, 0)) == 0) {return false;}
    Elf32_Ehdr *phdr = elf32_getehdr(elfp);
    if(!phdr){ elf_end(elfp); return false;}

    Elf_Scn*    shstrscnp  = 0;
    Elf_Scn*    symscnp = 0;
    Elf_Scn*    strscnp = 0;
    Elf_Data*   shstrdatap = 0;
    if ((shstrscnp = elf_getscn(elfp, phdr->e_shstrndx)) == 0) {
	elf_end(elfp); 
	return false;
    }
    if((shstrdatap = elf_getdata(shstrscnp, 0)) == 0) {
	elf_end(elfp); 
	return false;
    }
    const char* shnames = (const char *) shstrdatap->d_buf;
    Elf_Scn*    scnp    = 0;
    while ((scnp = elf_nextscn(elfp, scnp)) != 0) {
	Elf32_Shdr* shdrp = elf32_getshdr(scnp);
	if (!shdrp) { elf_end(elfp); return false; }
	const char* name = (const char *) &shnames[shdrp->sh_name];
        if (strcmp(name, ".symtab") == 0) {
            symscnp = scnp;
        }
        else if (strcmp(name, ".strtab") == 0) {
            strscnp = scnp;
        }
    }
    if (!strscnp || !symscnp) { elf_end(elfp); return false;}

    Elf_Data* symdatap = elf_getdata(symscnp, 0);
    Elf_Data* strdatap = elf_getdata(strscnp, 0);
    if (!symdatap || !strdatap) { elf_end(elfp); return false;}
    u_int nsyms = symdatap->d_size / sizeof(Elf32_Sym);
    Elf32_Sym*  syms   = (Elf32_Sym *) symdatap->d_buf;
    const char* strs   = (const char *) strdatap->d_buf;

    if (f_addr != NULL) *f_addr = 0;
    for(u_int i=0; i < nsyms; i++){
	if (syms[i].st_shndx != SHN_UNDEF) {
	    if(ELF32_ST_TYPE(syms[i].st_info) == st_type){
		string name = string(&strs[syms[i].st_name]);
		if(name == f_name){
		  if (f_addr != NULL) {
		    *f_addr = syms[i].st_value + ld_base_addr; 
		  }
		  break;
		} 
    } } }
    elf_end(elfp);
    if((f_addr != NULL) && ((*f_addr)==0)) { return false; }
    return true;
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
bool dynamic_linking::set_r_brk_point(process *proc) {

    if(brkpoint_set) return true;
    if(!r_brk_addr) return false;

#if defined(BPATCH_LIBRARY)
    // Before putting in the breakpoint, save what is currently at the
    // location that will be overwritten.
    if (!proc->readDataSpace((void *)r_brk_addr, R_BRK_SAVE_BYTES,
			     (void *)r_brk_save, true)) {
	return false;
    }
#endif

#if defined(sparc_sun_solaris2_4)
    // because there is no pdFunction, instPoint, or image associated with
    // this instrumentation point we can't use all the normal routines to
    // insert base tramp and minitrap code here.  Instead we replace the 
    // retl instruction  with a trap instruction.   Then in paradynd in  
    // the routine that handles the sigtrap for this case we will simulate
    // the retl instruction by changing the value of the %o7 register
    // so that it looks like the retl instruction has executed
    instruction trap_insn;
    trap_insn.raw = BREAK_POINT_INSN;
    if(!proc->writeDataSpace((caddr_t)r_brk_addr,
	sizeof(instruction),&trap_insn.raw)){
	return false;
    }
#else // x86
    instruction trap_insn((const unsigned char*)"\017\013\0220\0220",
        ILLEGAL, 4);
    if (!proc->writeDataSpace((void *)r_brk_addr, 4, trap_insn.ptr()))
        return false;
    //proc->SetIllForTrap();
#endif

    brkpoint_set = true;
    return true;
}

#if defined(BPATCH_LIBRARY)
// set_r_brk_point: this routine instruments the code pointed to by
// the r_debug.r_brk (the linkmap update routine).  Currently this code  
// corresponds to no function in the symbol table and consists of only
// 2 instructions on sparc-solaris (retl nop).  Also, the library that 
// contains these instrs is the runtime linker which is not parsed like
// other shared objects, so adding instrumentation here is ugly. 
// TODO: add support for this on x86
bool dynamic_linking::unset_r_brk_point(process *proc) {
    return proc->writeDataSpace((caddr_t)r_brk_addr, R_BRK_SAVE_BYTES,
				(caddr_t)r_brk_save);
}
#endif

// processLinkMaps: This routine is called by getSharedObjects to  
// process all shared objects that have been mapped into the process's
// address space.  This routine reads the link maps from the application 
// process to find the shared object file base mappings. It returns 0 on error.
vector<shared_object *> *dynamic_linking::processLinkMaps(process *p) {

    r_debug debug_elm;
    if(!p->readDataSpace((caddr_t)(r_debug_addr),
        sizeof(r_debug),(caddr_t)&(debug_elm),true)) {
        // printf("read d_ptr_addr failed r_debug_addr = 0x%lx\n",r_debug_addr);
        return 0;
    }

    r_brk_addr = debug_elm.r_brk;

    // get each link_map object
    bool first_time = true;
    Link_map *next_link_map = debug_elm.r_map;
    Address next_addr = (Address)next_link_map; 
    vector<shared_object*> *shared_objects = new vector<shared_object*>;
    while(next_addr != 0){
	Link_map link_elm;
        if(!p->readDataSpace((caddr_t)(next_addr),
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
            if(!p->readDataSpace((caddr_t)((u_int)(link_elm.l_name)+i),
		sizeof(char),(caddr_t)(&(f_name[i])),true)){
            }
	    if(f_name[i] == '\0'){
		done = true;
		f_amount = i+1;
	    }
	}
        f_name[f_amount-1] = '\0';
	string obj_name = string(f_name);

	sharedobj_cerr << 
	    "dynamicLinking::processLinkMaps(): file name of next shared obj="
	    << obj_name << endl;

	// create a shared_object and add it to the list
	// kludge: ignore the entry if it has the same name as the
	// executable file...this seems to be the first link-map entry
        // VG(09/25/01): also ignore if address is 65536 or name is (unknown)
	if(obj_name != p->getImage()->file() && 
	   obj_name != p->getImage()->name() &&
	   obj_name != p->getArgv0() &&
           link_elm.l_addr != 65536 &&
           obj_name != "(unknown)"
           //strncmp(obj_name.string_of(), "(unknown)", 10)
           ) {
	   sharedobj_cerr << 
	       "file name doesn't match image, so not ignoring it...firsttime=" 
	       << (int)first_time << endl;

	   // kludge for when an exec occurs...the first element
	   // in the link maps is the file name of the parent process
	   // so in this case, we ignore the first entry
	   if((!(p->wasExeced())) || (p->wasExeced() && !first_time)) { 
                shared_object *newobj = new shared_object(obj_name,
			link_elm.l_addr,false,true,true,0);
	        (*shared_objects).push_back(newobj);
	    }
	}
	else {
	   sharedobj_cerr << 
	       "file name matches that of image, so ignoring...firsttime=" 
	       << (int)first_time << endl;
        }

	first_time = false;
	next_addr = (Address)link_elm.l_next;
    }
    p->setDynamicLinking();
    dynlinked = true;
    return shared_objects;
    shared_objects = 0;
}


// getLinkMapAddrs: returns a vector of addresses corresponding to all 
// base addresses in the link maps.  Returns 0 on error.
vector<Address> *dynamic_linking::getLinkMapAddrs(process *p) {

    r_debug debug_elm;
    if(!p->readDataSpace((caddr_t)(r_debug_addr),
        sizeof(r_debug),(caddr_t)&(debug_elm),true)) {
        // printf("read d_ptr_addr failed r_debug_addr = 0x%lx\n",r_debug_addr);
        return 0;
    }

    bool first_time = true;
    Link_map *next_link_map = debug_elm.r_map;
    Address next_addr = (Address)next_link_map; 
    vector<Address> *link_addresses = new vector<Address>;
    while(next_addr != 0) {
	Link_map link_elm;
        if(!p->readDataSpace((caddr_t)(next_addr),
            sizeof(Link_map),(caddr_t)&(link_elm),true)) {
            logLine("read next_link_map failed\n");
	    return 0;
        }
	// kludge: ignore the first entry
	if(!first_time) { 
	    (*link_addresses).push_back(link_elm.l_addr); 
	}
	else {
	    // printf("first link map addr 0x%x\n",link_elm.l_addr);
	}

	first_time = false;
	next_addr = (Address)link_elm.l_next;
    }
    return link_addresses;
    link_addresses = 0;
}

// getNewSharedObjects: returns a vector of shared_object one element for
// newly mapped shared object.  old_addrs contains the addresses of the
// currently mapped shared objects. Sets error_occured to true, and 
// returns 0 on error.
vector<shared_object *> *dynamic_linking::getNewSharedObjects(process *p,
						vector<Address> *old_addrs,
						bool &error_occured){

    r_debug debug_elm;
    if(!p->readDataSpace((caddr_t)(r_debug_addr),
        sizeof(r_debug),(caddr_t)&(debug_elm),true)) {
        // printf("read d_ptr_addr failed r_debug_addr = 0x%lx\n",r_debug_addr);
	error_occured = true;
        return 0;
    }

    // get each link_map object
    bool first_time = true;
    Link_map *next_link_map = debug_elm.r_map;
    Address next_addr = (Address)next_link_map; 
    vector<shared_object*> *new_shared_objects = new vector<shared_object*>;
    while(next_addr != 0){
	Link_map link_elm;
        if(!p->readDataSpace((caddr_t)(next_addr),
            sizeof(Link_map),(caddr_t)&(link_elm),true)) {
            logLine("read next_link_map failed\n");
	    delete new_shared_objects;
	    error_occured = true;
	    return 0;
        }

	// kludge: ignore the entry 
	if(!first_time){
	    // check to see if this is a new shared object 
	    bool found = false;
	    for(u_int i=0; i < old_addrs->size(); i++){
		if((*old_addrs)[i] == link_elm.l_addr) {
		    found = true; 
		    break;
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
                    if(!p->readDataSpace((caddr_t)((Address)(link_elm.l_name)+i),
		        sizeof(char),(caddr_t)(&(f_name[i])),true)){
                    }
	            if(f_name[i] == '\0'){
		        done = true;
		        f_amount = i+1;
	            }
	        }
                f_name[f_amount-1] = '\0';
	        string obj_name = string(f_name);
                shared_object *newobj = new shared_object(obj_name,
			link_elm.l_addr,false,true,true,0);
		(*new_shared_objects).push_back(newobj);
            }
	}
	first_time = false;
	next_addr = (Address)link_elm.l_next;
    }
    error_occured = false;
    return new_shared_objects;
    new_shared_objects = 0;
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
vector< shared_object *> *dynamic_linking::getSharedObjects(process *p) {

    // step 1: figure out if this is a dynamic executable
    string dyn_str = string("DYNAMIC");
    internalSym dyn_sym;
    bool flag = p->findInternalSymbol(dyn_str, true, dyn_sym);
    if(!flag){ return 0;}
    int proc_fd = p->getProcFileDescriptor();
    if(!proc_fd){ return 0;}

    // step 2: find the base address and file descriptor of ld.so.1
    Address ld_base = 0;
    int ld_fd = -1;
    if(!(this->get_ld_base_addr(ld_base,proc_fd))) { return 0;}
    if((ld_fd = ioctl(proc_fd, PIOCOPENM, (caddr_t *)&ld_base)) == -1) { 
	return 0;
    } 

    // step 3: get its symbol table and find r_debug
    if (!(this->find_r_debug(ld_fd,ld_base))) { return 0; }
    close(ld_fd);

    // step 4: get link-maps and process them
    vector<shared_object *> *result = this->processLinkMaps(p);

    // step 5: set brkpoint in r_brk to catch dlopen and dlclose events
    if(!(this->set_r_brk_point(p))){ 
	// printf("error after step5 in getSharedObjects\n");
    }

    // additional step: find dlopen - naim
    if(!(this->get_ld_base_addr(ld_base,proc_fd))) { return 0;}
    if((ld_fd = ioctl(proc_fd, PIOCOPENM, (caddr_t *)&ld_base)) == -1) { 
	return 0;
    }
    if (!(this->find_dlopen(ld_fd,ld_base))) {
      logLine("WARNING: we didn't find dlopen in ld.so.1\n");
    }
    close(ld_fd);

    fflush(stdout);
    return (result);
    result = 0;
}


// findChangeToLinkMaps: This routine returns a vector of shared objects
// that have been deleted or added to the link maps as indicated by
// change_type.  If an error occurs it sets error_occured to true.
vector <shared_object *> *dynamic_linking::findChangeToLinkMaps(process *p, 
						   u_int change_type,
						   bool &error_occured) {

    // get list of current shared objects
    vector<shared_object *> *curr_list = p->sharedObjects();
    if((change_type == 2) && !curr_list) {
	error_occured = true;
	return 0;
    }

    // if change_type is add then figure out what has been added
    if(change_type == 1){
        // create a vector of addresses of the current set of shared objects
	vector<Address> *addr_list =  new vector<Address>;
	for (u_int i=0; i < curr_list->size(); i++) {
	    (*addr_list).push_back(((*curr_list)[i])->getBaseAddress());
	}
	vector <shared_object *> *new_shared_objs = 
				getNewSharedObjects(p, addr_list,error_occured);
        if(!error_occured){
	    delete addr_list;
	    return new_shared_objs; 
	    new_shared_objs = 0;
	}
    }
    // if change_type is remove then figure out what has been removed
    else if((change_type == 2) && curr_list) {
	// create a list of base addresses from the linkmaps and
	// compare them to the addr in vector of shared object to see
	// what has been removed
	vector<Address> *addr_list = getLinkMapAddrs(p);
	if(addr_list) {
	    vector <shared_object *> *remove_list = new vector<shared_object*>;
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
bool dynamic_linking::handleIfDueToSharedObjectMapping(process *proc,
				vector<shared_object*>  **changed_objects,
				u_int &change_type,
				bool &error_occured){ 

  prgregset_t regs;
#ifdef PURE_BUILD
  // explicitly initialize "regs" struct (to pacify Purify)
  // (at least initialize those components which we actually use)
  for (unsigned r=0; r<(sizeof(regs)/sizeof(regs[0])); r++) regs[r]=0;
#endif

  error_occured = false;
  int proc_fd = proc->getProcFileDescriptor(); 
  if (ioctl (proc_fd, PIOCGREG, &regs) != -1) {
    // is the trap instr at r_brk_addr?
    if(regs[R_PC] == (int)r_brk_addr){ 
	// find out what has changed in the link map
	// and process it
	r_debug debug_elm;
	if(!proc->readDataSpace((caddr_t)(r_debug_addr),
	    sizeof(r_debug),(caddr_t)&(debug_elm),true)) {
	    // printf("read failed r_debug_addr = 0x%x\n",r_debug_addr);
	    error_occured = true;
	    return true;
	}

        // if the state of the link maps is consistent then we can read
	// the link maps, otherwise just set the r_state value
	change_type = r_state;   // previous state of link maps 
	r_state = debug_elm.r_state;  // new state of link maps
        if( debug_elm.r_state == 0){
	    // figure out how link maps have changed, and then create
	    // a list of either all the removed shared objects if this
	    // was a dlclose or the added shared objects if this was a dlopen

	    // kludge: the state of the first add can get screwed up
	    // so if both change_type and r_state are 0 set change_type to 1
	    if(change_type == 0) change_type = 1;
	    *changed_objects = findChangeToLinkMaps(proc, change_type,
						  error_occured);
	} 

#if defined(sparc_sun_solaris2_4)
	// change the pc so that it will look like the retl instr 
        // completed: set PC to o7 in current frame
	// we can do this because this retl doesn't correspond to 
	// an instrumentation point, so we don't have to worry about 
	// missing any instrumentation code by making it look like the
	// retl has already happend

	// first get the value of the stackpointer
	Address o7reg = regs[R_O7];
        o7reg += 2*sizeof(instruction);
	if(!(proc->changePC(o7reg))) {
	      // printf("error in changePC handleIfDueToSharedObjectMapping\n");
	      error_occured = true;
	      return true;
        }
#else //x86
	// set the pc to the "ret" instruction
	Address next_pc = regs[R_PC] + sizeof(instruction);
	if (!proc->changePC(next_pc))
	    error_occured = true;
#endif
	return true;
    }
  }

  return false; 
}
