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

#include "dyninstAPI/src/sharedobject.h"
#include "dyninstAPI/src/solarisDL.h"
#include "dyninstAPI/src/process.h"
#include "dyninstAPI/src/symtab.h"
#include "util/h/debugOstream.h"

extern debug_ostream sharedobj_cerr;

#include <link.h>
#include <libelf.h>
#include <sys/types.h>
#include <sys/procfs.h>
#include <sys/auxv.h>

// get_ld_base_addr: This routine returns the base address of ld.so.1
// it returns true on success, and false on error
bool dynamic_linking::get_ld_base_addr(u_int &addr, int proc_fd){

    // get number of auxv_t structs
    int auxvnum;
    if(ioctl(proc_fd, PIOCNAUXV, &auxvnum) != 0) { return false;}    
    if(auxvnum < 1) { return false; }

    // get the array of auxv_t structs
    auxv_t *auxv_elms = new auxv_t[auxvnum];
    // auxv_t auxv_elms[auxvnum];
    if(ioctl(proc_fd, PIOCAUXV, auxv_elms) != 0) { return false; }

    // find the base address of ld.so.1
    for(int i=0; i < auxvnum; i++){
        if(auxv_elms[i].a_type == AT_BASE) {
	    addr = (u_int)(auxv_elms[i].a_un.a_ptr);
	    // cout << "ld.so.1 base addr = " << addr << "num " 
	    // << auxvnum << endl;
	    delete auxv_elms;
	    return true;
    } }
    delete auxv_elms;
    return false;
}

// find_r_debug: this routine finds the symbol table for ld.so.1, and 
// parses it to find the address of symbol r_debug
// it returns false on error
bool dynamic_linking::find_r_debug(u_int ld_fd,u_int ld_base_addr){

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
    string r_debug_name = string("r_debug");
    Elf32_Sym*  syms   = (Elf32_Sym *) symdatap->d_buf;
    const char* strs   = (const char *) strdatap->d_buf;

    r_debug_addr = 0;
    for(u_int i=0; i < nsyms; i++){
	if (syms[i].st_shndx != SHN_UNDEF) {
	    if(ELF32_ST_TYPE(syms[i].st_info) == STT_OBJECT){
		string name = string(&strs[syms[i].st_name]);
		if(name == r_debug_name){
		    r_debug_addr = syms[i].st_value + ld_base_addr; 
		    break;
		} 
    } } }
    elf_end(elfp);
    if(!r_debug_addr){ return false; }
    return true;
}

// processLinkMaps: This routine is called by getSharedObjects to  
// process all shared objects that have been mapped into the process's
// address space.  This routine reads the link maps from the application 
// process to find the shared object file base mappings. It returns 0 on error.
vector<shared_object *> *dynamic_linking::processLinkMaps(process *p) {

    r_debug debug_elm;
    if(!p->readDataSpace((caddr_t)(r_debug_addr),
        sizeof(r_debug),(caddr_t)&(debug_elm),true)) {
        // printf("read d_ptr_addr failed r_debug_addr = 0x%x\n",r_debug_addr);
        return 0;
    }

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

	sharedobj_cerr << "dynamicLinking::processLinkMaps(): file name of next shared obj=" << obj_name << endl;

	// create a shared_object and add it to the list
	// kludge: ignore the entry if it has the same name as the
	// executable file...this seems to be the first link-map entry
	if(obj_name != p->getImage()->file() && 
	   obj_name != p->getImage()->name() &&
	   obj_name != p->getArgv0()) {
	   sharedobj_cerr << "file name doesn't match that of image, so not ignoring it...firsttime=" << first_time << endl;

	   // kludge for when an exec occurs...the first element
	   // in the link maps is the file name of the parent process
	   // so in this case, we ignore the first entry
	   if((!(p->wasExeced())) || (p->wasExeced() && !first_time)){ 
	        // printf("adding so: %s",blah.string_of());
                shared_object *newobj = new shared_object(obj_name,
			link_elm.l_addr,false,true,true,0);
	        *shared_objects += newobj;
	    }
	}
	else
	   sharedobj_cerr << "file name matches that of image, so ignoring...firsttime=" << first_time << endl;

	first_time = false;
	next_addr = (u_int)link_elm.l_next;
    }
    p->setDynamicLinking();
    dynlinked = true;
    return shared_objects;
    shared_objects = 0;
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
vector< shared_object *> *dynamic_linking::getSharedObjects(process *p){

    // step 1: figure out if this is a dynamic executable
    string dyn_str = string("DYNAMIC");
    internalSym *dyn_sym = p->findInternalSymbol(dyn_str,true);
    if(!dyn_sym){ return 0;}
    int proc_fd = p->getProcFileDescriptor();
    if(!proc_fd){ return 0;}

    // step 2: find the base address and file descriptor of ld.so.1
    u_int ld_base = 0;
    int ld_fd = -1;
    if(!(this->get_ld_base_addr(ld_base,proc_fd))) { return 0;}
    if((ld_fd = ioctl(proc_fd, PIOCOPENM, (caddr_t *)&ld_base)) == -1) { 
	return 0;
    } 

    // step 3: get its symbol table and find r_debug
    if (!(this->find_r_debug(ld_fd,ld_base))) { return 0; }
    close(ld_fd);

    // step 4: get link-maps and process them
    return (this->processLinkMaps(p));
}

