/*
 * Copyright (c) 1996-2009 Barton P. Miller
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

#include "dyninstAPI/src/mapped_object.h"
#include "dyninstAPI/src/dynamiclinking.h"
#include "dyninstAPI/src/process.h"
#include "dyninstAPI/src/dyn_lwp.h"
#include "dyninstAPI/src/symtab.h"
#include "common/h/debugOstream.h"
#include "dyninstAPI/src/dyn_thread.h"
#include "dyninstAPI/src/function.h"

#include <link.h>
#include <libelf.h>
#include <sys/types.h>
#include <sys/procfs.h>
#include <sys/ioctl.h>
#include <dirent.h>
#include <sys/stat.h>

using namespace Dyninst::SymtabAPI;


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
   virtual uint64_t l_ld() = 0;
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
      : link_map_x(proc_) { 
      valid = load_link(addr_); 
   }

   size_t size() { 
      return sizeof(link_elm); 
   }

   uint64_t l_addr() { 
      return link_elm.l_addr; 
   }
   
   char *l_name() {
      if (loaded_name) return link_name;
      
      for (unsigned int i = 0; i < sizeof(link_name); ++i) {
         if (!proc->readDataSpace((caddr_t)((Address)link_elm.l_name + i),
                                  sizeof(char), (caddr_t)(link_name + i), true)) {
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

   uint64_t l_ld() { 
      return static_cast<uint64_t>(link_elm.l_ld); 
   }

   bool is_last() {
      return (link_elm.l_next == 0); 
   }

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
      loaded_name = false;
      return ret;
   }
   link_map_dyn32 link_elm;
};

class link_map_64 : public link_map_x {
public:
   link_map_64(process *proc_, void *addr_)
      : link_map_x(proc_) { valid = load_link(addr_); }

   size_t size() { 
      return sizeof(link_elm); 
   }

   uint64_t l_addr() { 
      return link_elm.l_addr; 
   }

   char *l_name() {
      if (loaded_name) return link_name;

      for (unsigned int i = 0; i < sizeof(link_name); ++i) {
         if (!proc->readDataSpace((caddr_t)((Address)link_elm.l_name + i),
                                  sizeof(char), ((caddr_t)link_name + i), true)) {
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

   uint64_t l_ld() { 
      return reinterpret_cast<uint64_t>(link_elm.l_ld); 
   }

   bool is_last() { 
      return (link_elm.l_next == 0); 
   }

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
   r_debug_32(process *proc_, Address addr) :
      r_debug_x(proc_) 
   {
      valid = proc->readDataSpace((caddr_t)addr, sizeof(debug_elm),
                                  (caddr_t)&debug_elm, true);
   }

   link_map_x *r_map() {
      return new link_map_32(proc, reinterpret_cast<void *>(debug_elm.r_map));
   }

   void *r_brk() { 
      return reinterpret_cast<void *>(debug_elm.r_brk);
   }

   int r_state() {
      return (int)debug_elm.r_state; 
   }

private:
   r_debug_dyn32 debug_elm;
};

class r_debug_64 : public r_debug_x {
public:
   r_debug_64(process *proc_, Address addr) : 
      r_debug_x(proc_) 
   {
      valid = proc->readDataSpace((caddr_t)addr, sizeof(debug_elm),
                                  (caddr_t)&debug_elm, true);
   }

   link_map_x *r_map() {
      return new link_map_64(proc, reinterpret_cast<void *>(debug_elm.r_map));
   }

   void *r_brk() { 
      return reinterpret_cast<void *>(debug_elm.r_brk);
   }

   int r_state() {
      return (int)debug_elm.r_state; 
   }

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

static bool isValidMemory(Address addr, int pid) {
   bool result = false;
   unsigned maps_size = 0;
   map_entries *maps = NULL;

   maps = getLinuxMaps(pid, maps_size);
   if (!maps)
      goto done;
   for (unsigned i=0; i<maps_size; i++) {
      if (maps[i].start <= addr && maps[i].end > addr) {
         result = true;
         goto done;
      }
   }

 done:
   if (maps)
      free(maps);
   return result;
}

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
      char *filename = NULL;
      bool matches_name = false;
      if (has_inode) {
         filename = strrchr(maps[i].path, '/');
         if (!filename)
            filename = maps[i].path;
         else
            filename++; 

         //Check for format match of ld*.so* (don't get /etc/ld.so.cache)
         matches_name = (strncmp("ld", filename, 2)==0 && 
                         strstr(filename, ".so") && 
                         !strstr(filename, ".cache"));
     }

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



/////////////// x86-linux breakpoint insertion routines
sharedLibHook::sharedLibHook(process *p, sharedLibHookType t, Address b) 
        : proc_(p), type_(t), breakAddr_(b), loadinst_(NULL) {

    // Before putting in the breakpoint, save what is currently at the
    // location that will be overwritten.
    if (!proc_->readDataSpace((void *)breakAddr_, SLH_SAVE_BUFFER_SIZE,
                         (void *)saved_, true))
                fprintf(stderr, "%s[%d]:  readDataSpace\n", __FILE__, __LINE__);
#if defined(arch_power)
    codeGen gen(instruction::size());
#else
    // Need a "trap size" method...
    codeGen gen(1);
#endif
    insnCodeGen::generateTrap(gen);
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

// processLinkMaps: This routine is called by getSharedObjects to  
// process all shared objects that have been mapped into the process's
// address space.  This routine reads the link maps from the application 
// process to find the shared object file base mappings. It returns 0 on error.
bool dynamic_linking::processLinkMaps(pdvector<fileDescriptor> &descs) {
   // Use proc maps file instead of r_debug.  It's more detailed, and some
   // kernels don't report correct info in r_debug

   // Apparently the r_debug stuff keeps things in some wacked out
   // order that we need (i.e. the order in which the libs are loaded)
   // Use proc/maps to augment r_debug instead

   unsigned maps_size = 0;
   map_entries *maps= getLinuxMaps(proc->getPid(), maps_size);
   //   std::string aout = process::tryToFindExecutable("", proc->getPid());

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

   startup_printf("%s[%d]: dumping maps info\n",
                  FILE__, __LINE__);
   for (unsigned i = 0; i < maps_size; i++) {
       startup_printf("\t Entry %d, name %s, addr 0x%lx\n",
                      i, maps[i].path, maps[i].start);
   }

   do {
      string obj_name = link_elm->l_name();
      Address text = static_cast<Address>(link_elm->l_addr());
      startup_printf("%s[%d]: processing element, name %s, text addr 0x%lx\n",
                     FILE__, __LINE__, obj_name.c_str(), text);
      if (obj_name == "") {
         continue;
      }

      if (obj_name == "") { // Augment using maps
         for (unsigned i = 0; i < maps_size; i++) {
            if (text == maps[i].start) {
                startup_printf("%s[%d]: augmenting empty name with maps name %s\n",
                               FILE__, __LINE__, maps[i].path);
                obj_name = maps[i].path;
                break;
            }
         }
      }
      if (obj_name[0] == '[')
         continue;
      if (obj_name == "") {
          // Should we try to parse this out of memory?
          continue;
      }
      if (!link_elm->is_valid()) {
         delete link_elm;
         delete debug_elm;
         startup_printf("Link element invalid! (2)\n");
         return 0;
      }
      startup_printf("%s[%d]: creating new file descriptor %s/0x%lx/0x%lx, ld is %lx\n",
                     FILE__, __LINE__, obj_name.c_str(), text, text, link_elm->l_ld());
      descs.push_back(fileDescriptor(obj_name, 
                                     text, text,
                                     true, 
                                     static_cast<Address>(link_elm->l_ld())));
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
      (*link_addresses).push_back(static_cast<Address>(link_elm->l_addr()));

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
    std::string dyn_str = std::string("DYNAMIC");
    int_symbol dyn_sym;
    if( ! proc->getSymbolInfo( dyn_str, dyn_sym ) ) { 
        startup_printf("[%s][%d]Failed to find DYNAMIC symbol in dyn::init, "
                       "this may not be a dynamic executable\n",__FILE__,__LINE__);
        //return false; 
    }

    /* Find the base address of ld.so.1, since the entries we get
       from its Object won't be right, otherwise. */
    
    Address ld_base, ld_base_backup = 0x0;
    unsigned ld_size;
    char *ld_path_backup = NULL;

    const char *ld_path = proc->getInterpreterName();
    ld_base = proc->getInterpreterBase();

    if( ! get_ld_info( ld_base_backup, ld_size, &ld_path_backup) ) { 
       startup_printf("Failed to get ld info, ret false from dyn::init\n");
       return false; 
    }
    if (!ld_path && ld_path_backup) {
       ld_path = ld_path_backup;
       proc->setInterpreterName(ld_path);
    }
    if (!ld_path) {
        startup_printf("[%s][%d]Secondary attempt to find the dynamic linker using /proc failed\n",__FILE__,__LINE__);
        return false;
    }
    /* Generate its Object and set r_debug_addr ("_r_debug"/STT_OBJECT) */
    // We haven't parsed libraries at this point, so do it by hand.
    Symtab *ldsoOne = new Symtab();
    string fileName = ld_path;
    ldsoOne->Symtab::openFile(ldsoOne, fileName);

    vector<Variable *> vars;

    if (!ldsoOne->findVariablesByName(vars, "_r_debug")) {
        startup_printf("Failed to find _r_debug, ret false from dyn::init\n");
        return false;
    }
    if (vars.size() != 1) {
        startup_printf("Found %d symbols for _r_debug, expecting 1, ret false from dyn::init\n", vars.size());
        return false; 
    }        

    r_debug_addr = vars[0]->getOffset();

    if (!isValidMemory(r_debug_addr + ld_base, proc->getPid())) {
       ld_base = ld_base_backup;
    }
    if (!isValidMemory(r_debug_addr + ld_base, proc->getPid())) {
       ld_base = ldsoOne->getLoadOffset();
    }
    r_debug_addr += ld_base;

    assert( r_debug_addr );

    dynlinked = true;

    return true;
}

/* Return true if the exception was caused by the hook in the linker
 * code, we'll worry about whether or not any libraries were
 * added/removed later on when we handle the exception
 */
bool dynamic_linking::decodeIfDueToSharedObjectMapping(EventRecord &ev,
                                                       unsigned int & /*change_type*/)
{
    sharedLibHook *hook;
    assert(ev.lwp);
    Frame lwp_frame = ev.lwp->getActiveFrame();
    hook = reachedLibHook(lwp_frame.getPC());
    return (bool)hook;
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
                                 pdvector<mapped_object*> &changed_objects,
                                 pdvector<bool> &is_new_object)
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
    
    // The EventRecord is used for passing information in, but not
    // back out, it's not necessary to set ev.what.

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
    // figure out how link maps have changed, and then create
    // a list of all the changes in shared object mappings
    bool res = findChangeToLinkMaps(changed_objects, is_new_object);
    if (!res) {
       return false;
    }
#if defined(i386_unknown_linux2_0)                                   \
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
      
      Address ret_addr = 0;
      if(!proc->readDataSpace((caddr_t)sp, proc->getAddressWidth(),
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
#elif defined(arch_power)
      // The debug function looks like so:

      // 0x4000c4d0 <_dl_debug_state+0>: stwu    r1,-32(r1)
      // 0x4000c4d4 <_dl_debug_state+4>: addi    r1,r1,32
      // 0x4000c4d8 <_dl_debug_state+8>: blr

      // For now, to avoid a dependence on the size of the stack frame
      // (which we can't tell) I'm going to instead fake an immediate return
      // from the function - bernat, 3JUL07

      dyn_saved_regs regs;
      if (!brk_lwp->getRegisters(&regs))
          return false;

      Address retAddr = regs.gprs.link;
      if (!brk_lwp->changePC(retAddr, NULL))
          return false;

#else   
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
#endif  
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

#if defined(arch_power) && defined(arch_64bit)
    /* 64-bit POWER architectures also use function descriptors instead of directly
       pointing at the function code.  Find the actual address of the function.
    */
    Address actualAddr;
    if (! proc->readDataSpace( (void *)breakAddr, 8, (void *)&actualAddr, true ) ) {
        bperr( "Failed to read breakAddr_.\n" );
        return 0;
    }
    breakAddr = actualAddr;
#endif

    sharedLibHook *sharedHook = new sharedLibHook(proc, SLH_UNKNOWN, // not used
						  breakAddr);
    sharedLibHooks_.push_back(sharedHook);

    return true;
}
