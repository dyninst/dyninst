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

#include "dyninstAPI/src/mapped_object.h"
#include "dyninstAPI/src/dynamiclinking.h"
#include "dyninstAPI/src/process.h"
#include "dyninstAPI/src/dyn_lwp.h"
#include "dyninstAPI/src/symtab.h"
#include "common/h/debugOstream.h"
#include "dyninstAPI/src/dyn_thread.h"

#include <link.h>
#include <libelf.h>
#include <sys/types.h>
#include <sys/procfs.h>
#include <sys/ioctl.h>
#include <dirent.h>
#include <sys/stat.h>


// ---------------------------------------------------------------
// The link_map and r_debug structures from <link.h> decide their
// size at runtime.  This makes it impossible for a 64-bit mutator
// to use these structures for a 32-bit mutatee.
// 
// So we must provide our own 32-bit equivalent.  If the link_map
// or r_debug from <link.h> ever change, so must our structures
// below.
// 
// struct link_map and r_debug: provided by <link.h>
// struct link_map_dyn32 and r_debug_dyn32: Forced 32-bit versions
// class  link_map_x and r_debug_x: Size independant class.
//
// These definitions should probably be moved to an h file.

struct link_map_dyn32
{
  /* These first few members are part of the protocol with the debugger.
     This is the same format used in SVR4.  */

    Elf32_Addr l_addr;          /* Base address shared object is loaded at.  */

    //char *l_name;               /* Absolute file name object was found in.  */
    uint32_t l_name;

    //Elf32_Dyn *l_ld;            /* Dynamic section of the shared object.  */
    uint32_t l_ld;

    //struct link_map_32 *l_next, *l_prev; /* Chain of loaded objects.  */
    uint32_t l_next, l_prev;
};

class link_map_x {
public:
    link_map_x(process *proc_) : proc(proc_), loaded_name(false) {}
    virtual ~link_map_x() {}
    virtual size_t size() = 0;
    virtual uint64_t l_addr() = 0;
    virtual char *l_name() = 0;
    virtual void *l_ld() = 0;
    virtual bool is_last() = 0;
    virtual bool load_next() = 0;

    bool is_valid() { return valid; }

protected:
    process *proc;
    char link_name[256];
    bool loaded_name;
    bool valid;
};

class link_map_32 : public link_map_x {
public:
    link_map_32(process *proc_, void *addr_)
	: link_map_x(proc_) { valid = load_link(addr_); }

    size_t size() { return sizeof(link_elm); }
    uint64_t l_addr() { return link_elm.l_addr; }
    char *l_name() {
	if (loaded_name) return link_name;

	for (unsigned int i = 0; i < sizeof(link_name); ++i) {
	    if (!proc->readDataSpace((caddr_t)((Address)link_elm.l_name + i),
				     sizeof(char), (caddr_t)(link_name + i), true)) {
                fprintf(stderr, "%s[%d]:  readDataSpace\n", FILE__, __LINE__);
		valid = false;
		link_name[0] = '\0';
                assert(0);
		return link_name;
	    }
	    if (link_name[i] == '\0') break;
	}
	link_name[sizeof(link_name) - 1] = '\0';
	loaded_name = true;

	return link_name;
    }
    void *l_ld() { return reinterpret_cast<void *>(link_elm.l_ld); }

    bool is_last() { return (link_elm.l_next == 0); }
    bool load_next() {
	if (is_last()) {
            return false;
        }
	if (load_link(reinterpret_cast<void *>(link_elm.l_next))) {
	    loaded_name = false;
	    return true;
	}
	return false;
    }

private:
    bool load_link(void *addr) {
	bool ret =  proc->readDataSpace((caddr_t)addr, sizeof(link_elm),
				   (caddr_t)&link_elm, true);
        if (!ret)
             assert(0);
        return ret;
    }
    link_map_dyn32 link_elm;
};

class link_map_64 : public link_map_x {
public:
    link_map_64(process *proc_, void *addr_)
	: link_map_x(proc_) { valid = load_link(addr_); }

    size_t size() { return sizeof(link_elm); }
    uint64_t l_addr() { return link_elm.l_addr; }
    char *l_name() {
	if (loaded_name) return link_name;

	for (unsigned int i = 0; i < sizeof(link_name); ++i) {
	    if (!proc->readDataSpace((caddr_t)((Address)link_elm.l_name + i),
				     sizeof(char), ((caddr_t)link_name + i), true)) {
                fprintf(stderr, "%s[%d]:  readDataSpace\n", __FILE__, __LINE__);
		valid = false;
		link_name[0] = '\0';
		return link_name;
	    }
	    if (link_name[i] == '\0') break;
	}
	link_name[sizeof(link_name) - 1] = '\0';
	loaded_name = true;

	return link_name;
    }
    void *l_ld() { return reinterpret_cast<void *>(link_elm.l_ld); }

    bool is_last() { return (link_elm.l_next == 0); }
    bool load_next() {
	if (is_last()) return false;

	if (load_link(link_elm.l_next)) {
	    loaded_name = false;
	    return true;
	}
	return false;
    }

private:
    bool load_link(void *addr) {
	return proc->readDataSpace((caddr_t)addr, sizeof(link_elm),
				   (caddr_t)&link_elm, true);
    }
    link_map link_elm;
};

struct r_debug_dyn32
{
    int r_version;              /* Version number for this protocol.  */

    //struct link_map_32 *r_map;     /* Head of the chain of loaded objects.  */
    uint32_t r_map;

    /* This is the address of a function internal to the run-time linker,
       that will always be called when the linker begins to map in a
       library or unmap it, and again when the mapping change is complete.
       The debugger can set a breakpoint at this address if it wants to
       notice shared object mapping changes.  */
    Elf32_Addr r_brk;
    enum
    {
        /* This state value describes the mapping change taking place when
	   the `r_brk' address is called.  */
        RT_CONSISTENT,          /* Mapping change is complete.  */
        RT_ADD,                 /* Beginning to add a new object.  */
        RT_DELETE               /* Beginning to remove an object mapping.  */
    } r_state;

    Elf32_Addr r_ldbase;        /* Base address the linker is loaded at.  */
};

class r_debug_x {
public:
    r_debug_x(process *proc_) : proc(proc_) {}
    virtual ~r_debug_x() {}
    virtual link_map_x *r_map() = 0;
    virtual void *r_brk() = 0;
    virtual int r_state() = 0;

    bool is_valid() { return valid; }

protected:
    process *proc;
    bool valid;
};

class r_debug_32 : public r_debug_x {
public:
    r_debug_32(process *proc_, Address addr) : r_debug_x(proc_) {
	valid = proc->readDataSpace((caddr_t)addr, sizeof(debug_elm),
				    (caddr_t)&debug_elm, true);
    }
    link_map_x *r_map() {
	return new link_map_32(proc, reinterpret_cast<void *>(debug_elm.r_map));
    }
    void *r_brk() { return reinterpret_cast<void *>(debug_elm.r_brk); }
    int r_state() { return (int)debug_elm.r_state; }

private:
    r_debug_dyn32 debug_elm;
};

class r_debug_64 : public r_debug_x {
public:
    r_debug_64(process *proc_, Address addr) : r_debug_x(proc_) {
	valid = proc->readDataSpace((caddr_t)addr, sizeof(debug_elm),
				    (caddr_t)&debug_elm, true);
    }
    link_map_x *r_map() {
	return new link_map_64(proc, reinterpret_cast<void *>(debug_elm.r_map));
    }
    void *r_brk() { return reinterpret_cast<void *>(debug_elm.r_brk); }
    int r_state() { return (int)debug_elm.r_state; }

private:
    r_debug debug_elm;
};

//
// End 32/64-bit helper structures.
// ---------------------------------------------------------------

/* 16 is a good choice for testing long_fgets. */
#define LONG_FGETS_GRANULARITY 256

/* Utility function; functions as fgets, except it returns a malloc()d string
   of sufficient length to hold the whole line.  Returns NULL when fgets() does. */
char * long_fgets( FILE * file ) {
	char initialBuffer[ LONG_FGETS_GRANULARITY ];
	char * status = fgets( initialBuffer, LONG_FGETS_GRANULARITY, file );
	if( status == NULL ) { return NULL; }
	if( initialBuffer[ strlen( initialBuffer ) - 2 ] == '\n' ) {
		return strdup( initialBuffer );
		}
	
	char * longline = NULL;
	char * oldline = strdup( initialBuffer );
	assert( oldline != NULL );
	for( int i = 2; oldline[ strlen( oldline ) - 1 ] != '\n'; ++i ) {
		longline = (char *)malloc( sizeof( char ) * LONG_FGETS_GRANULARITY * i );
		assert( longline != NULL );
		strcpy( longline, oldline );
		char * nextline = &( longline[ (LONG_FGETS_GRANULARITY - 1) * (i - 1) ] );
		free( oldline );
		
		status = fgets( nextline, LONG_FGETS_GRANULARITY, file );
		if( status == NULL ) { return NULL; }
		
		oldline = longline;
		}
	
	return oldline;
	} /* end long_fgets() */

/* get_ld_info() returns true if it filled in the base address of the
   ld.so library and its path, and false if it could not. */
bool dynamic_linking::get_ld_info( Address & addr, unsigned &size, char ** path)
{
   map_entries *maps = NULL;
   unsigned maps_size = 0;
   bool result = false;
   Address first = 0x0, last = 0x0;
   *path = NULL;

   addr = 0x0;
   maps = getLinuxMaps(proc->getPid(), maps_size);
   if (!maps)
      goto done;

   for (unsigned i=0; i<maps_size; i++) {
      //Check inode
      bool has_inode = (maps[i].inode != 0);


      //Get the file part of the filename. eg /lib/ld-2.3.4.so -> ld-2.3.4.so
      char *filename = "";
      if (has_inode) {
         filename = strrchr(maps[i].path, '/');
         if (!filename)
            filename = maps[i].path;
         else
            filename++;
      }
      //Check for format match of ld*.so*
      bool matches_name = (strncmp("ld", filename, 2)==0 && 
                           strstr(filename, ".so"));

      if (!matches_name) {
	continue;
      }

      if (!*path)
         *path = strdup(maps[i].path);      
      if (!first)
         first = maps[i].start;
      last = maps[i].end;
   }

   if (!first) {
      result = false;
      goto done;
   }

   assert(*path);
   assert(last);
   addr = first;
   size = last - first;
   result = true;
   
 done:
   if (maps)
      free(maps);

   return result;
} /* end get_ld_info() */


#if defined(arch_ia64)

////////////// IA-64 breakpoint routines

sharedLibHook::sharedLibHook(process *p, sharedLibHookType t, Address b) 
        : proc_(p), type_(t), breakAddr_(b), loadinst_(NULL) {

	InsnAddr iAddr = InsnAddr::generateFromAlignedDataAddress( breakAddr_, proc_ );
	/* Save the original instructions. */
	iAddr.saveMyBundleTo( (uint8_t *)saved_ );

	IA64_bundle trapBundle = generateTrapBundle();
	iAddr.replaceBundleWith( trapBundle );
}


sharedLibHook::~sharedLibHook() {
    if (!proc_->isAttached() || proc_->execing())
        return;

    InsnAddr iAddr = InsnAddr::generateFromAlignedDataAddress( breakAddr_, proc_ );
    iAddr.writeMyBundleFrom((uint8_t *)saved_);
}


#else

/////////////// x86-linux breakpoint insertion routines
sharedLibHook::sharedLibHook(process *p, sharedLibHookType t, Address b) 
        : proc_(p), type_(t), breakAddr_(b), loadinst_(NULL) {

    // Before putting in the breakpoint, save what is currently at the
    // location that will be overwritten.
    if (!proc_->readDataSpace((void *)breakAddr_, SLH_SAVE_BUFFER_SIZE,
                         (void *)saved_, true))
                fprintf(stderr, "%s[%d]:  readDataSpace\n", __FILE__, __LINE__);

    codeGen gen(1);
    instruction::generateTrap(gen);
    proc_->writeDataSpace((void*)breakAddr_, gen.used(), gen.start_ptr());
    
}

sharedLibHook::~sharedLibHook() 
{
    if (!proc_->isAttached() || proc_->execing())
        return;

    if (!proc_->writeDataSpace((void *)breakAddr_, SLH_SAVE_BUFFER_SIZE, saved_)) {
        //  This wds fails, and has so for a long time...
        // Yeah, because the process may be, you know, GONE... -- bernat
        
        //fprintf(stderr, "%s[%d]:  WDS failed: %d bytes at %p\n", FILE__, __LINE__, SLH_SAVE_BUFFER_SIZE, breakAddr_);
    }
}
#endif

// processLinkMaps: This routine is called by getSharedObjects to  
// process all shared objects that have been mapped into the process's
// address space.  This routine reads the link maps from the application 
// process to find the shared object file base mappings. It returns 0 on error.
bool dynamic_linking::processLinkMaps(pdvector<fileDescriptor> &descs) {
   // Use proc maps file instead of r_debug.  It's more detailed, and some
   // kernels don't report correct info in r_debug

   // Apparently the r_debug stuff keeps things in some wacked out order
   // that we need.  Use proc/maps to augment r_debug instead

   unsigned maps_size = 0;
   map_entries *maps= getLinuxMaps(proc->getPid(), maps_size);
   //   pdstring aout = process::tryToFindExecutable("", proc->getPid());

   /*
   if (maps) {
      for (unsigned i = 0; i < maps_size; i++) {
         if (maps[i].prems & PREMS_EXEC &&
             strlen(maps[i].path) &&
             aout != maps[i].path) {

            descs.push_back(fileDescriptor(maps[i].path,
                                           maps[i].start,
                                           maps[i].start,
                                           true));
         }
      }
      free(maps);
      //      return true;
      descs.clear();
   }
   */

   assert(r_debug_addr); // needs to be set before we're called

   r_debug_x *debug_elm;
   if (proc->getAddressWidth() == 4)
      debug_elm = new r_debug_32(proc, r_debug_addr);
   else
      debug_elm = new r_debug_64(proc, r_debug_addr);
    
   if (!debug_elm->is_valid()) {
      startup_printf("debug element invalid!\n");
      delete debug_elm;
      return false;
   }

   // get each link_map object
   link_map_x *link_elm = debug_elm->r_map();
   if (!link_elm->is_valid()) {
      delete link_elm;
      delete debug_elm;
      startup_printf("Link element invalid!\n");
      return false;
   }

   do {
      pdstring obj_name = pdstring(link_elm->l_name());
      Address text = link_elm->l_addr();
      if (obj_name == "" && text == 0) {
         continue;
      }

      if (obj_name == "") { // Augment using maps
         for (unsigned i = 0; i < maps_size; i++) {
            if (text == maps[i].start) {
               obj_name = maps[i].path;
               break;
            }
         }
      } /* else if (text == 0) { // Augment using maps
         // r_debug will show symlinks, but maps shows the actual file
         char symlink[PATH_MAX];
         if (realpath(obj_name.c_str(), symlink) != NULL)
            obj_name = symlink;
         for (unsigned i = 0; i < maps_size; i++) {
            if (maps[i].prems & PREMS_EXEC &&
                !strcmp(symlink,maps[i].path)) {
               text = maps[i].start;
               break;
            }
         }
         }*/

      if (obj_name.c_str()[0] == '[')
         continue;
      if (!link_elm->is_valid()) {
         delete link_elm;
         delete debug_elm;
         startup_printf("Link element invalid! (2)\n");
         return 0;
      }
      descs.push_back(fileDescriptor(obj_name, 
                                     text, text,
                                     true));
   } while (link_elm->load_next());
    
   delete link_elm;
   delete debug_elm;
   return true;
}

// getLinkMapAddrs: returns a vector of addresses corresponding to all 
// base addresses in the link maps.  Returns 0 on error.
pdvector<Address> *dynamic_linking::getLinkMapAddrs() {

    r_debug_x *debug_elm;
    if (proc->getAddressWidth() == 4)
	debug_elm = new r_debug_32(proc, r_debug_addr);
    else
	debug_elm = new r_debug_64(proc, r_debug_addr);

    if (!debug_elm->is_valid()) {
	delete debug_elm;
	return 0;
    }

    bool first_time = true;
    link_map_x *link_elm = debug_elm->r_map();
    if (!link_elm->is_valid()) {
	delete link_elm;
        delete debug_elm;
        return 0;
    }

    pdvector<Address> *link_addresses = new pdvector<Address>;
    do {
	// kludge: ignore the first entry
	if (!first_time)
	    (*link_addresses).push_back(link_elm->l_addr());

	first_time = false;
    } while (link_elm->load_next());

    delete link_elm;
    delete debug_elm;
    return link_addresses;
}
// initialize: perform initialization tasks on a platform-specific level
bool dynamic_linking::initialize() {
    r_debug_addr = 0;
    r_brk_target_addr = 0;

    // Set an initial value for use in
    // handleIfDueToSharedObjectMapping.  The value should be
    // RT_CONSISTENT if we ld-preloaded our RT lib and RT_ADD if we
    // are force-loading RT lib (the hook is not placed
    // yet). Fortunately, the code recognizes that the RT lib is added
    // even if previous_r_state was set to RT_CONSISTENT.
    previous_r_state = r_debug::RT_CONSISTENT;
    
    /* Is this a dynamic executable? */
    pdstring dyn_str = pdstring("DYNAMIC");
    Dyn_Symbol dyn_sym;
    if( ! proc->getSymbolInfo( dyn_str, dyn_sym ) ) { 
        startup_printf("Failed to find DYNAMIC symbol, returning false from dyn::init\n");
        return false; 
    }
    
    /* Find the base address of ld.so.1, since the entries we get
       from its Object won't be right, otherwise. */
    Address ld_base = 0;
    unsigned ld_size = 0;
    char * ld_path;
    if( ! get_ld_info( ld_base, ld_size, & ld_path) ) { 
        startup_printf("Failed to get ld info, ret false from dyn::init\n");
        return false; 
    }
    
    /* Generate its Object and set r_debug_addr ("_r_debug"/STT_OBJECT) */
    // We haven't parsed libraries at this point, so do it by hand.
    fileDescriptor ld_desc(ld_path, ld_base, ld_base, true);
    Dyn_Symtab *ldsoOne = new Dyn_Symtab();
    string fileName = ld_path;
    ldsoOne->Dyn_Symtab::openFile(fileName, ldsoOne);
    //Object ldsoOne( ld_desc );
    pdvector< Dyn_Symbol > rDebugSyms;
    Dyn_Symbol rDebugSym;
    vector<Dyn_Symbol *>syms;
    if(!ldsoOne->findSymbolByType(syms,"_r_debug",Dyn_Symbol::ST_UNKNOWN)){
        startup_printf("Failed to find _r_debug, ret false from dyn::init\n");
    	return false;
    }	
    for(unsigned index=0;index<syms.size();index++)
       rDebugSyms.push_back(*(syms[index]));
    if( rDebugSyms.size() == 1 ) { rDebugSym = rDebugSyms[0]; } else { 
       startup_printf("rDebugSyms size %d, expecting 1, ret false from " 
                      "dyn::init\n", rDebugSyms.size());
       return false; 
    }
    if( ! (rDebugSym.getType() == Dyn_Symbol::ST_OBJECT) ) {
       startup_printf("Unexpected type %d for rDebugSym, ret false from "
                      "dyn::init\n", rDebugSym.getType());
       return false; 
    }
    
    // Set r_debug_addr
    r_debug_addr = rDebugSym.getAddr();
    if (!ldsoOne->getLoadAddress()) {
        r_debug_addr += ld_base;
    }

    signed long adjustment = 0;
    if ((r_debug_addr < ld_base) || (r_debug_addr >= ld_base + ld_size)) {
       //On RHEL4's version of ssh, we're seeing the OS ignore the load address
       // for libraries, and force them into other locations.  Try to correct 
       // for this by noting wheter the r_debug_addr symbol falls outside
       // the addresses boundries, and then adjusting the address.
       adjustment = ld_base - ldsoOne->getLoadAddress();
    }
    r_debug_addr += adjustment;
    assert( r_debug_addr );
    
    /* Set dlopen_addr ("_dl_map_object"/STT_FUNC); apparently it's OK if this fails. */
    syms.clear();
    ldsoOne->findSymbolByType(syms,"_dl_map_object",Dyn_Symbol::ST_UNKNOWN);
    for(unsigned index=0;index<syms.size();index++)
       rDebugSyms.push_back(*(syms[index]));
    //if( ! ldsoOne.get_symbols( "_dl_map_object", rDebugSyms ) ) { ; }
    /* It's also apparently OK if this uses a garbage symbol. */
    if( rDebugSyms.size() == 1 ) { rDebugSym = rDebugSyms[0]; }
    if( ! (rDebugSym.getType() == Dyn_Symbol::ST_FUNCTION) ) { ; } 
    dlopen_addr = rDebugSym.getAddr() + ld_base;
    dlopen_addr += adjustment;
    assert( dlopen_addr );
    
    free(ld_path);
    
    dynlinked = true;

    return true;
}
bool dynamic_linking::decodeIfDueToSharedObjectMapping(EventRecord &ev,
                                                       u_int &change_type)
{
   sharedLibHook *hook;
   //dyn_lwp *lwp = ev.lwp; 
   process *proc = ev.proc;

   assert(ev.lwp);
   Frame lwp_frame = ev.lwp->getActiveFrame();
   hook = reachedLibHook(lwp_frame.getPC());
   if (!hook) {
     return false;
   }

   // find out what has changed in the link map
   r_debug_x *debug_elm;
   if (proc->getAddressWidth() == 4)
     debug_elm = new r_debug_32(proc, r_debug_addr);
   else
     debug_elm = new r_debug_64(proc, r_debug_addr);
    
   if (!debug_elm->is_valid()) {
       bperr("read failed r_debug_addr = 0x%x\n",r_debug_addr);
       delete debug_elm;
       return false;
   }

       switch(previous_r_state) {
       case r_debug::RT_CONSISTENT:
         change_type = SHAREDOBJECT_NOCHANGE;
         break;
       case r_debug::RT_ADD:
          change_type = SHAREDOBJECT_ADDED;
          break;
        case r_debug::RT_DELETE:
          change_type = SHAREDOBJECT_REMOVED;
          break;
        default:
          break;
       };

    if (debug_elm->r_state() == r_debug::RT_CONSISTENT) {
      // figure out how link maps have changed, and then create
      // a list of either all the removed shared objects if this
      // was a dlclose or the added shared objects if this was a dlopen
      
      // kludge: the state of the first add can get screwed up
      // so if both change_type and r_state are 0 set change_type to 1
      pdvector<fileDescriptor> newfds;
      bool res = didLinkMapsChange(change_type, newfds);
      if (!res) {
        return false;
      }
      return true;
    }

    return true;
}

bool dynamic_linking::getChangedObjects(EventRecord & /* ev */, pdvector<mapped_object*> & /* changed_objects */)
{
  return false;
}

// handleIfDueToSharedObjectMapping: returns true if the trap was caused
// by a change to the link maps,  If it is, and if the linkmaps state is
// safe, it processes the linkmaps to find out what has changed...if it
// is not safe it sets the type of change currently going on (specified by
// the value of r_debug.r_state in link.h
// The added or removed shared objects are returned in changed_objects
// the change_type value is set to indicate if the objects have been added 
// or removed
bool dynamic_linking::handleIfDueToSharedObjectMapping(EventRecord &ev,
                                                       pdvector<mapped_object*> &changed_objects)
{ 
   //  pdvector<dyn_thread *>::iterator iter = proc->threads.begin();
  
  dyn_lwp *brk_lwp = ev.lwp;
  sharedLibHook *hook = NULL;

  if (brk_lwp) {
    Frame lwp_frame = brk_lwp->getActiveFrame();
    hook = reachedLibHook(lwp_frame.getPC());
  }

  // is the trap instr at at the breakpoint?
  if (force_library_load || hook != NULL) {
    // We can force this manually, even if we aren't at
    // the correct address.
    // dlclose_brk_addr would be the same if it was set, so we need to 
    // compare link maps to find what has changed
    // find out what has changed in the link map
    // and process it
    r_debug_x *debug_elm;
    if (proc->getAddressWidth() == 4)
      debug_elm = new r_debug_32(proc, r_debug_addr);
    else
      debug_elm = new r_debug_64(proc, r_debug_addr);
    
    if (!debug_elm->is_valid()) {
        bperr("read failed r_debug_addr = 0x%x\n",r_debug_addr);
        delete debug_elm;
        return false;
    }
    
    // if the state of the link maps is consistent then we can read
    // the link maps, otherwise just set the r_state value
    
    // We see two events: first a "something is changing..." then a 
    // "now consistent". We want the library list from consistent,
    // and the "what's new" from "changing".
    switch(previous_r_state) {
    case r_debug::RT_CONSISTENT:
      ev.what = SHAREDOBJECT_NOCHANGE;
      break;
    case r_debug::RT_ADD:
      ev.what = SHAREDOBJECT_ADDED;
      break;
    case r_debug::RT_DELETE:
      ev.what = SHAREDOBJECT_REMOVED;
      break;
    default:
      assert(0);
      break;
    }
    current_r_state = debug_elm->r_state();
    
    delete debug_elm; 
    if (current_r_state == r_debug::RT_CONSISTENT) {
      // figure out how link maps have changed, and then create
      // a list of either all the removed shared objects if this
      // was a dlclose or the added shared objects if this was a dlopen
      
      // kludge: the state of the first add can get screwed up
      // so if both change_type and r_state are 0 set change_type to 1
      bool res = findChangeToLinkMaps((u_int &)ev.what,
				      changed_objects);
      if (!res) return false;

#if defined(i386_unknown_linux2_0) \
 || defined(x86_64_unknown_linux2_4) /* Blind duplication - Ray */
      // SAVE THE WORLD
      if (previous_r_state == r_debug::RT_ADD) {
	for(unsigned int index = 0; index < changed_objects.size(); index++){
	  if (changed_objects[index]->fileName() == "libdyninstAPI_RT.so")
	    continue; // Don't want 
	  if (changed_objects[index]->fileName() == "libelf.so")
	    continue; // Don't want 
	  setlowestSObaseaddr(changed_objects[index]->getBaseAddress());
	}	
      }
#endif
    }
    
    // Now to clean up.
    
    if (!force_library_load) {
#if defined(arch_x86) || defined(arch_x86_64)
      // Return from the function.  We used to do this by setting the program
      // counter to the end of the function, but we don't necessarily know
      // how long it is, so now we emulate a ret instruction by changing the
      // PC to the return value from the stack and incrementing the stack
      // pointer.
      dyn_saved_regs regs;
      
      brk_lwp->getRegisters(&regs);
      
      Address sp = regs.gprs.PTRACE_REG_SP;
      
      Address ret_addr;
      if(!proc->readDataSpace((caddr_t)sp, sizeof(Address),
			      (caddr_t)&ret_addr, true)) {
	// bperr("read failed sp = 0x%x\n", sp);
	fprintf(stderr, "%s[%d]:  readDataSpace\n", __FILE__, __LINE__);
	return false;
      }
      // Fix up the stack pointer
      regs.gprs.PTRACE_REG_SP = sp + proc->getAddressWidth();
      brk_lwp->restoreRegisters(regs);
      
      if (! brk_lwp->changePC(ret_addr, NULL)) {
	return false;
      }
#else   // ia64
      dyn_lwp *lwp_to_use = NULL;
      lwp_to_use = proc->getRepresentativeLWP();
      if(lwp_to_use == NULL)
	if(process::IndependentLwpControl() && lwp_to_use == NULL)
	  lwp_to_use = proc->getInitialThread()->get_lwp();
      
      /* We've inserted a return statement at r_brk_target_addr
	 for our use here. */
      // fprintf(stderr, "Changing PC to 0x%llx\n", r_brk_target_addr);
      assert(r_brk_target_addr);
      lwp_to_use->changePC( r_brk_target_addr, NULL );
#endif  //x86
    } // non-forced, clean up for breakpoint
    previous_r_state = current_r_state;
    return true;
  } // Not a library load (!forced && pc != breakpoint)
  return false; 
}

// This function performs all initialization necessary to catch shared object
// loads and unloads (symbol lookups and trap setting)
bool dynamic_linking::installTracing()
{
  // Need to update for IA64 as well.
#if defined(arch_x86)
  //Libc >= 2.3.3 has security features that prevent _dl_open from being
  // called from outside libc.  We'll disable those features by finding the
  // function that implements them and writing 'return 0' over the top of
  // the function.
    startup_printf("... Looking for dl_check_caller...\n");
    pdvector<int_function *> dlchecks;
    if (proc->findFuncsByMangled("_dl_check_caller",
                                 dlchecks)) {
        for (unsigned i = 0; i < dlchecks.size(); i++) {
            startup_printf("Overwriting retval for hit %d\n", i);
            dlchecks[i]->setReturnValue(0);
        }
    }
    
    // And find the address of do_dlopen and set the RT library symbol correctly
    // TODO: check libc only
    // TODO: use replaceCall to do this right...
    pdvector<int_function *> do_dlopens;
    startup_printf("... Looking for do_dlopen...\n");
    if (proc->findFuncsByMangled("do_dlopen",
                                 do_dlopens)) {
        Address do_dlopen_addr = do_dlopens[0]->getAddress();
        startup_printf("... Found do_dlopen at 0x%x\n", do_dlopen_addr);
        pdvector<int_variable *> vars;
        if (proc->findVarsByAll("DYNINST_do_dlopen", vars)) {
            assert(vars.size() == 1);
            Address tmp = vars[0]->getAddress();
            startup_printf("... writing to RT var at 0x%x\n", tmp);
            proc->writeDataSpace((void *)tmp, sizeof(Address), (void *)&do_dlopen_addr);
        }
    }
#endif
  
  assert(r_debug_addr);
    
    r_debug_x *debug_elm;
    if (proc->getAddressWidth() == 4)
	debug_elm = new r_debug_32(proc, r_debug_addr);
    else
	debug_elm = new r_debug_64(proc, r_debug_addr);

    if (!debug_elm->is_valid()) {
        bperr( "Failed data read\n");

	delete debug_elm;
	return true;
    }
    Address breakAddr = reinterpret_cast<Address>(debug_elm->r_brk());
    delete debug_elm;

#if defined(arch_ia64)
    /* The IA-64's function pointers don't actually point at the function,
       because you also need to know the GP to call it.  Hence, another
       indirection. */
    if(! proc->readDataSpace( (void *)breakAddr, 8, (void *)&breakAddr, true ) ) {
	bperr( "Failed to read breakAddr_.\n" );
	return 0;
    }

    // We don't handle the trap directly, rather inserting a sequence of instructions
    // in the runtime library that patch it up

    /* Insert a return bundle for use in the handler. */
    instruction memoryNOP( NOP_M );
    instruction returnToBZero = generateReturnTo( 0 );
    IA64_bundle returnBundle( MMBstop, memoryNOP, memoryNOP, returnToBZero );

    Dyn_Symbol r_brk_target;
    if (!proc->getSymbolInfo("R_BRK_TARGET", r_brk_target))
        assert(0);
    r_brk_target_addr = r_brk_target.getAddr(); assert( r_brk_target_addr );

    InsnAddr jAddr = InsnAddr::generateFromAlignedDataAddress( r_brk_target_addr, proc );
    jAddr.replaceBundleWith( returnBundle );
#endif

    sharedLibHook *sharedHook = new sharedLibHook(proc, SLH_UNKNOWN, // not used
						  breakAddr);
    sharedLibHooks_.push_back(sharedHook);

    return true;
}
