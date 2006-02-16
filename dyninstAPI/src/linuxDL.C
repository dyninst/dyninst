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

static int scandir_select_ld( const struct dirent * entry ) {
  // Select files which could be ld.so, they must start with "ld", and must
  // contain the string ".so".  Hope that's good enough...
  return !strncmp( "ld", entry->d_name, 2 ) && strstr( entry->d_name, ".so" );
}


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

const char ldlibpath[] = "/lib";

/* get_ld_info() returns true if it filled in the base address of the
   ld.so library and its path, and false if it could not. */
bool dynamic_linking::get_ld_info( Address & addr, char ** path ) {
	/* Allow the user to specify the directory to search for the ld library.
	   Otherwise, use '/lib' (from the filesystem spec). */
	char * ldpath, * ldpath_env;
	ldpath_env = getenv( "LD_PATH" );
	if( ldpath_env != NULL ) {
		ldpath = strdup( ldpath_env );
		}
	else {
		ldpath = strdup( ldlibpath );
		}
	assert( ldpath != NULL );
		
	/* Allow the user to specify the exact file which is the ld library. */
	char * ldspec = getenv( "LD_SPECIFY" );
	
	int length = snprintf( NULL, 0, "/proc/%d/maps", proc->getPid() );
	char * procMaps = (char *)malloc( (length + 1) * sizeof( char ) );
	assert( procMaps != NULL );
	sprintf( procMaps, "/proc/%d/maps", proc->getPid() );
  
  	FILE * maps = P_fopen( procMaps, "r" );
  	if( ! maps ) {
  		free( procMaps ); free( ldpath ); return false;
  		}

	/* Check if the specified ld.so exists before searching /proc/maps for it. */
	int num_dents = 0;
	dirent ** dirents = NULL;
	if(	ldspec != NULL ) { 
		/* This used to be only if the file were specified /and/ there were no pathnames in /proc/maps. */
		struct stat t_stat;
		if( stat( ldspec, &t_stat ) ) {
			perror( "dynamic_linking::get_ld_info -> stat(...)" );
			bperr( "Error calling stat(...) on file in LD_SPECIFY, ignoring\n" );
			ldspec = NULL;
		} else {
			// Pack the inode and file name into a dirent and the path into ldpath
			num_dents = 1;
			dirents = new dirent *;
			* dirents = new dirent;
			(* dirents)->d_ino = t_stat.st_ino;
			
			// Split the file/path name into path and file names
			char * ldspec2 = strrchr( ldspec, '/' );
			*(ldspec2++) = '\0';
			strncpy( (*dirents)->d_name, ldspec2, 255 );
			
			if( ldpath != NULL ) { free( ldpath ); }
			ldpath = strdup( ldspec );
			assert( ldpath != NULL );
			}
		}
		
	/* FIXME: do these error cases leak dirents? */
	if( procMaps == NULL && ldspec == NULL ) {
		// Scan /lib for files conforming to the criteria in scandir_select_ld
		// above, and sort them via the given (in dirent.h) alphasort function
		num_dents = scandir( ldpath, &dirents, scandir_select_ld, alphasort );
		if( num_dents == -1 ) {
			fclose( maps );
	  		free( procMaps ); free( ldpath ); return false;
		} else if ( !num_dents ) {
			fclose( maps );
	  		free( procMaps ); free( ldpath ); return false;
	        return false;
			}
		}


	/* Scan through /proc/maps, looking for the first mapping which corresponds
	   to ld.so.  If we have pathnames, just look for 'ld'; if not, compare the 
	   file inodes against the dirents discovered above. */

	* path = NULL;
	ino_t lastInode = 0;
	while( ! feof( maps ) && (*path == NULL) ) {
		// Format of lines in /proc/*/maps
		//  original (without pathname):
		//    40000000-40009000 r-xp 00000000 03:02 232624
		//    mapped address range :  permissions : offset in file : device number : inode
		//  new uname -r >= 2.1.? (with pathname):
		//    40000000-40009000 r-xp 00000000 08:01 28674      /lib/ld-2.0.7.so
		//    as above : path to mapped file
		
		Address lowAddr, highAddr;
		char perms[5];
		Address offset;
		unsigned int devmajor, devminor;
		ino_t inode;
		/* We assume that inodes numbers are not bigger than signed ints. */
		int matches = fscanf( 	maps, "%lx-%lx %4s %lx %x:%x %d",
								&lowAddr, &highAddr, perms, &offset,
								&devmajor, &devminor, (int *) & inode );
		if( matches != 7 ) {
			fclose( maps );
	  		free( procMaps ); free( ldpath ); return false;
	        return false;
			}
		// dev_t device = ( ( devmajor & 0xff ) << 8 ) | devminor;

     	/* Does this line have a filename? */
     	char * fullPathName = long_fgets( maps );
     	if( fullPathName != NULL && fullPathName[0] != '\n' && inode != 0 && inode != lastInode ) { 
     		char * fileName = strrchr( fullPathName, '/' );
     		assert( fileName != NULL );
     		++fileName;
     		if( strncmp( "ld", fileName, 2 ) == 0 && strstr( fileName, ".so" ) != NULL ) {
     			/* Set the return values and finish up. */
     			addr = lowAddr;
     			char * strippedOfLeadingWhitespace = strchr( fullPathName, '/' );
     			assert( strippedOfLeadingWhitespace != NULL );
     			strippedOfLeadingWhitespace[ strlen( strippedOfLeadingWhitespace ) - 1 ] = '\0';
     			* path = strdup( strippedOfLeadingWhitespace );
     			assert( * path != NULL );
     			
     			if( fullPathName != NULL ) { free( fullPathName ); }
				fclose( maps );
		  		free( procMaps ); free( ldpath );
		  		return true;
     			}
     		} /* end if we have a filename */
     	else if( inode != 0 && inode != lastInode ) {
     		for( int i = 0; i < num_dents; ++i ) {
     			if( (ino_t)(dirents[i]->d_ino) == inode ) {
     				/* Set the return values. */
     				assert( dirents[i]->d_name != NULL );     			
     				int pathlength = strlen( ldpath ) + strlen( dirents[i]->d_name ) + 2;
     				* path = (char *)malloc( sizeof( char ) * pathlength );
     				assert( * path != NULL );
     				snprintf( * path, pathlength, "%s/%s", ldpath, dirents[i]->d_name );
     				addr = lowAddr;
     				
     				/* Clean up. */     			
	     			if( fullPathName != NULL ) { free( fullPathName ); }
					fclose( maps );
			  		free( procMaps ); free( ldpath );
     				return true;
     				} /* end if the inode numbers match. */
     			} /* end iteration over dentries. */
     		} /* end if we don't have a filename but we do have a new inode. */
     		
     	free( fullPathName );
     	lastInode = inode;
     	} /* end iteration over file */
	
	/* FIXME: does this leak dirents? */
	fclose( maps );
	free( procMaps ); free( ldpath );
	
	/* If we'd found what we were looking for, we returned already. */
	return false;
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
    if (!proc_->writeDataSpace((void *)breakAddr_, SLH_SAVE_BUFFER_SIZE, saved_)) {
       //  This wds fails, and has so for a long time...
       //fprintf(stderr, "%s[%d]:  WDS failed: %d bytes at %p\n", FILE__, __LINE__, SLH_SAVE_BUFFER_SIZE, breakAddr_);
   }
}
#endif

// processLinkMaps: This routine is called by getSharedObjects to  
// process all shared objects that have been mapped into the process's
// address space.  This routine reads the link maps from the application 
// process to find the shared object file base mappings. It returns 0 on error.
bool dynamic_linking::processLinkMaps(pdvector<fileDescriptor> &descs) {
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
        if (obj_name == "" &&
            link_elm->l_addr() == 0) {
            continue;
        }

	if (!link_elm->is_valid()) {
	    delete link_elm;
	    delete debug_elm;
            startup_printf("Link element invalid! (2)\n");
	    return 0;
	}
        descs.push_back(fileDescriptor(obj_name, 
                                       link_elm->l_addr(), link_elm->l_addr(),
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
    // We figure the first thing we'll see is an add... this serves
    // as an initial value in handleIfDue... above
    previous_r_state = r_debug::RT_ADD;
    
    /* Is this a dynamic executable? */
    pdstring dyn_str = pdstring("DYNAMIC");
    Symbol dyn_sym;
    if( ! proc->getSymbolInfo( dyn_str, dyn_sym ) ) { 
        startup_printf("Failed to find DYNAMIC symbol, returning false from dyn::init\n");
        return false; 
    }
    
    /* Find the base address of ld.so.1, since the entries we get
       from its Object won't be right, otherwise. */
    Address ld_base = 0;
    char * ld_path;
    if( ! get_ld_info( ld_base, & ld_path) ) { 
        startup_printf("Failed to get ld info, ret false from dyn::init\n");
        return false; 
    }
    
    /* Generate its Object and set r_debug_addr ("_r_debug"/STT_OBJECT) */
    // We haven't parsed libraries at this point, so do it by hand.
    fileDescriptor ld_desc(ld_path, ld_base, ld_base, true);
    Object ldsoOne( ld_desc );
    pdvector< Symbol > rDebugSyms;
    Symbol rDebugSym;
    if( ! ldsoOne.get_symbols( "_r_debug", rDebugSyms ) ) {
        startup_printf("Failed to find _r_debug, ret false from dyn::init\n");
        return false; 
    }
    if( rDebugSyms.size() == 1 ) { rDebugSym = rDebugSyms[0]; } else { 
        startup_printf("rDebugSyms size %d, expecting 1, ret false from dyn::init\n",
                rDebugSyms.size());
        return false; 
    }
    if( ! (rDebugSym.type() == Symbol::PDST_OBJECT) ) {
        startup_printf("Unexpected type %d for rDebugSym, ret false from dyn::init\n",
                       rDebugSym.type());
        return false; 
    }
    
    // Set r_debug_addr
    r_debug_addr = rDebugSym.addr();
    if (!ldsoOne.getLoadAddress()) {
        r_debug_addr += ld_base;
    }
    assert( r_debug_addr );
    
    /* Set dlopen_addr ("_dl_map_object"/STT_FUNC); apparently it's OK if this fails. */
    if( ! ldsoOne.get_symbols( "_dl_map_object", rDebugSyms ) ) { ; }
    /* It's also apparently OK if this uses a garbage symbol. */
    if( rDebugSyms.size() == 1 ) { rDebugSym = rDebugSyms[0]; }
    if( ! (rDebugSym.type() == Symbol::PDST_FUNCTION) ) { ; } 
    dlopen_addr = rDebugSym.addr() + ld_base;
    assert( dlopen_addr );
    
    delete [] ld_path; // ccw 8 mar 2004
    
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
#if defined(BPATCH_LIBRARY)

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

    Symbol r_brk_target;
    if (!proc->getSymbolInfo("R_BRK_TARGET", r_brk_target))
        assert(0);
    r_brk_target_addr = r_brk_target.addr(); assert( r_brk_target_addr );

    InsnAddr jAddr = InsnAddr::generateFromAlignedDataAddress( r_brk_target_addr, proc );
    jAddr.replaceBundleWith( returnBundle );
#endif

    sharedLibHook *sharedHook = new sharedLibHook(proc, SLH_UNKNOWN, // not used
						  breakAddr);
    sharedLibHooks_.push_back(sharedHook);

    return true;
}
