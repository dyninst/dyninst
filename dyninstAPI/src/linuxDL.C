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
#include "dyninstAPI/src/linuxDL.h"
#include "dyninstAPI/src/process.h"
#include "dyninstAPI/src/symtab.h"
#include "common/h/debugOstream.h"

extern debug_ostream sharedobj_cerr;

#include <link.h>
#include <libelf.h>
#include <sys/types.h>
#include <sys/procfs.h>
#include <sys/ioctl.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/reg.h>

static int scandir_select_ld( const struct dirent * entry ) {
  // Select files which could be ld.so, they must start with "ld", and must
  // contain the string ".so".  Hope that's good enough...
  return !strncmp( "ld", entry->d_name, 2 ) && strstr( entry->d_name, ".so" );
}

const char ldlibpath[] = "/lib";

// get_ld_base_addr: This routine returns the base address of ld.so.x
// and a path to the particular ld.so.x this process is using
// it returns true on success, and false on error
bool dynamic_linking::get_ld_info(u_int &addr, char **path, process *proc ){

  // Allow the user to specify the directory to search for the ld library
  // within, otherwise, follow the standard /lib (from the filesystem spec)
  // Environment variable = LD_PATH
  char *ldpath;
  ldpath = getenv( "LD_PATH" );
  if( !ldpath ) {
    ldpath = new char[ strlen(ldlibpath)+1 ];
    strcpy( ldpath, ldlibpath );
  }

  // Allow the user to specify the exact file which is the ld library for
  // this process.  Note that this function will fail if this is specified
  // incorrectly.
  // Environment variable = LD_SPECIFY
  char *ldspec = NULL;
  ldspec = getenv( "LD_SPECIFY" );

  *path = NULL;
  char buf[200], lbuf[200];
  sprintf( buf, "/proc/%d/maps", proc->getPid() );

  FILE *maps = P_fopen( buf, "r" );
  if( !maps ) {
#ifdef PDYN_DEBUG
    perror( "dynamic_linking::get_ld_info -> P_fopen( maps )" );
#endif
    return false;
  }

  dirent **dirents = NULL;
  int num_dents = 0;

  // Check to see if there is a file name for the mapped region
  // at the end of the line.  ASSUMES that the first line is in
  // fact a mapped file; doesn't check for inode == 0.  It should
  // just be the main code mapping for the program.
  bool proc_maps_path = false;
  if( !P_fgets( lbuf, 199, maps ) ) { // Read in a line
#ifdef PDYN_DEBUG
    perror( "dynamic_linking::get_ld_info -> fgets" );
#endif
    fclose( maps );
    return false;
  }
  if( 1 == sscanf( lbuf, "%*x-%*x %*s %*x %*x:%*x %*d %s", buf ) ) {
    proc_maps_path = true;
#ifdef DL_DEBUG
    printf( "Determined that filename does exist in /proc/*/maps, must be >= 2.1.x !!\n" );
#endif
  }
  if( -1 == fseek( maps, 0L, SEEK_SET ) ) {
#ifdef PDYN_DEBUG
    perror( "dynamic_linking::get_ld_info -> fseek( maps, 0 )" );
#endif
    fclose( maps );
    return false;
  }

  if( ldspec ) {
    // Find the inode number for the specified file
    struct stat t_stat;
    if( stat( ldspec, &t_stat ) ) {
      perror( "dynamic_linking::get_ld_info -> stat(...)" );
      fprintf( stderr, "Error calling stat(...) on file in LD_SPECIFY, ignoring\n" );
      ldspec = NULL;
    } else {
      // Pack the inode and file name into a dirent and the path into ldpath
      num_dents = 1;
      dirents = new dirent*;
      *dirents = new dirent;
      (*dirents)->d_ino = t_stat.st_ino;
      // Split the file/path name into path and file names
      char *ldspec2 = strrchr( ldspec, '/' );
      *(ldspec2++) = '\0';
      strncpy( (*dirents)->d_name, ldspec2, 255 );
      if( ldpath ) delete ldpath;
      ldpath = new char[ strlen( ldspec ) + 1 ];
      strcpy( ldpath, ldspec );
    }
  }
  if( !proc_maps_path && !ldspec ) {
    // Scan /lib for files conforming to the criteria in scandir_select_ld
    // above, and sort them via the given (in dirent.h) alphasort function
    num_dents = scandir( ldpath, &dirents, scandir_select_ld, alphasort );
    if( num_dents == -1 ) {
#ifdef PDYN_DEBUG
      sprintf( lbuf, "dynamic_linking::get_ld_info -> scandir( %s )", ldpath );
      perror( lbuf );
#endif
      fclose( maps );
      return false;
    } else if ( !num_dents ) {
#ifdef PDYN_DEBUG
      fprintf( stderr, "ERROR -- Didn't find any ld entries in %s\n", ldpath );
#endif
      fclose( maps );
      return false;
    }
#ifdef DL_DEBUG
    printf( "Found %d possible ld entries in %s\n", num_dents, ldpath );
#endif
  }

  // Scan through the /proc/*/maps file, checking for the first mapping which
  // corresponds with ld.so  If proc_maps_path is true, simply check if the 
  // beginning of the file name at the end of the line is "ld", otherwise, we
  // need to check each file mapping (inode != 0) against each found dirent
  // in the previous step
  char *token;
  int errcode = 0;
  u_int t_addr;
  dev_t t_device;
  ino_t t_inode = 0, t_last_inode = 0;
  while( !feof( maps ) && !(*path) ) {
    // Format of lines in /proc/*/maps
    //  original (without pathname):
    //    40000000-40009000 r-xp 00000000 03:02 232624
    //    mapped address range :  permissions : offset in file : device number : inode
    //  new uname -r >= 2.1.? (with pathname):
    //    40000000-40009000 r-xp 00000000 08:01 28674      /lib/ld-2.0.7.so
    //    as above : path to mapped file

    if( !P_fgets( lbuf, 199, maps ) )  // Read in a line
      { errcode = 1; break; }

    if( !( token = strtok( lbuf, " " ) ) )  // Get the address range
      { errcode = 2; break; }
    if( 1 != sscanf( token, "%x-%*x", &t_addr ) )  // Read the range start address
      { errcode = 2; break; }

    if( !( token = strtok( NULL, " " ) ) )  // Get the permissions
      { errcode = 2; break; }
    if( !( token = strtok( NULL, " " ) ) )  // Get the file offset
      { errcode = 2; break; }

    if( !( token = strtok( NULL, " " ) ) )  // Get the device
      { errcode = 2; break; }
    {
      int dev_major, dev_minor;
      if( 2 != sscanf( token, "%x:%x", &dev_major, &dev_minor ) )
	{ errcode = 2; break; }
      t_device = ( ( dev_major & 0xff ) << 8 ) | dev_minor;
    }

    if( !( token = strtok( NULL, " \n" ) ) )  // Get the inode
      { errcode = 2; break; }
    t_inode = atoi( token );

    // Check this mapping's path name for "ld" and ".so", and succeed if found
    if( proc_maps_path && t_inode != 0 && t_inode != t_last_inode ) {
      if( !( token = strtok( NULL, " \n" ) ) )
	{ errcode = 3; break; }
#ifdef DL_DEBUG
      printf( "/proc/*/maps line: addr = %#.8x, device = %#.4x, inode = %d, file = %s\n", t_addr, (int)t_device, t_inode, token );
#endif
      char *fname = strrchr( token, '/' ) + 1;
      if( fname && !strncmp( "ld", fname, 2 ) && strstr( fname, ".so" ) ) {
	addr = t_addr;
	*path = new char[ strlen( token ) + 1 ];
	strcpy( *path, token );
#ifdef DL_DEBUG
	printf( "Found ld = %s\n", *path );
#endif
      }
    }
    // Scan through the found dirents for this inode and succeed if found
    else if( t_inode != 0 && t_inode != t_last_inode ) {
#ifdef DL_DEBUG
      printf( "/proc/*/maps line: addr = %#.8x, device = %#.4x, inode = %d\n", t_addr, (int)t_device, t_inode );
#endif
      for( int i=0; i < num_dents; i++ )
	if( (ino_t)(dirents[i]->d_ino) == t_inode ) {
	  struct stat t_stat;
	  addr = t_addr;
	  *path = new char[ strlen( ldpath ) + strlen( dirents[i]->d_name ) + 2 ];
	  strcpy( *path, ldpath );
	  strcat( *path, "/" );
	  strcat( *path, dirents[i]->d_name );
	  if( stat( *path, &t_stat ) ) {
#ifdef DL_DEBUG
	    sprintf( lbuf, "dynamic_linking::get_ld_info -> stat(\"%s\",)", *path );
	    perror( lbuf );
#endif
	    delete *path;
	    *path = NULL;
	  } else {
#ifdef DL_DEBUG
	    printf( "Found ld = %s\n", *path );
#endif
	  }
	}
    }

    t_last_inode = t_inode;
  }

  switch( errcode ) {
  case 1:
    perror( "dynamic_linking::get_ld_info -> fgets (unexpected)" );
    break;
  case 2:
    fprintf( stderr, "ERROR -- parsing /proc/*/maps, unexpected format?\n" );
    break;
  case 3:
    fprintf( stderr, "ERROR -- unexpected missing file name\n" );
  default:;
  }

  if( num_dents > 0 && dirents ) {
    for( int i=0; i < num_dents; i++ )
      if( dirents[i] )
	delete dirents[i];
    delete dirents;
  }

  fclose( maps );
  if( *path )
    return true;
  else
    return false;
}

// findFunctionIn_ld_so_1: this routine finds the symbol table for ld.so.1 and 
// parses it to find the address of symbol r_debug
// it returns false on error
bool dynamic_linking::findFunctionIn_ld_so_1(string f_name, u_int ld_fd, u_int ld_base_addr, u_int *f_addr, int st_type){

    Elf *elfp = 0;
    if ((elfp = elf_begin(ld_fd, ELF_C_READ, 0)) == 0)
      {perror("elf_begin"); return false;}
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
bool dynamic_linking::find_r_debug(u_int ld_fd,u_int ld_base_addr){
  return findFunctionIn_ld_so_1("_r_debug", ld_fd, ld_base_addr, &r_debug_addr, STT_OBJECT);
}

// find_dlopen: this routine finds the symbol table for ld.so.1, and 
// parses it to find the address of symbol _dl_map_object, which is
// called by glibc's _dl_open
// it returns false on error
bool dynamic_linking::find_dlopen(u_int ld_fd,u_int ld_base_addr){
  return findFunctionIn_ld_so_1("_dl_map_object", ld_fd, ld_base_addr, &dlopen_addr, STT_FUNC);
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

#ifdef LD_DEBUG
    fprintf( stderr, "Installing r_brk illegal instruction\n" );
#endif

#if defined(BPATCH_LIBRARY)
    // Before putting in the breakpoint, save what is currently at the
    // location that will be overwritten.
    if (!proc->readDataSpace((void *)r_brk_addr, R_BRK_SAVE_BYTES,
			     (void *)r_brk_save, true)) {
	return false;
    }
#endif

    instruction trap_insn((const unsigned char*)"\017\013\0220\0220", ILLEGAL, 4);
    if (!proc->writeDataSpace((void *)r_brk_addr, 4, trap_insn.ptr()))
        return false;
    //proc->SetIllForTrap();

    brkpoint_set = true;
    return true;
}

#if defined(BPATCH_LIBRARY)
bool dynamic_linking::unset_r_brk_point(process *proc) {
#ifdef LD_DEBUG
    fprintf( stderr, "*** Removing r_brk illegal instruction\n" );
#endif
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
#ifdef PDYN_DEBUG
        printf("read d_ptr_addr failed r_debug_addr = 0x%x\n",r_debug_addr);
#endif
        return 0;
    }

    r_brk_addr = debug_elm.r_brk;

    // get each link_map object
    bool first_time = true;
    link_map *next_link_map = debug_elm.r_map;
    Address next_addr = (Address)next_link_map; 
    vector<shared_object*> *shared_objects = new vector<shared_object*>;
    while(next_addr != 0){
	link_map link_elm;
        if(!p->readDataSpace((caddr_t)(next_addr),
            sizeof(link_map),(caddr_t)&(link_elm),true)) {
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
	if(obj_name != p->getImage()->file() && 
	   obj_name != p->getImage()->name() &&
	   obj_name != p->getArgv0()) {
	   sharedobj_cerr << 
	       "file name doesn't match image, so not ignoring it...firsttime=" 
	       << (int)first_time << endl;

	   // kludge for when an exec occurs...the first element
	   // in the link maps is the file name of the parent process
	   // so in this case, we ignore the first entry
	   if((!(p->wasExeced())) || (p->wasExeced() && !first_time)){ 
                shared_object *newobj = new shared_object(obj_name,
			link_elm.l_addr,false,true,true,0);
	        (*shared_objects).push_back(newobj);
#if defined(BPATCH_LIBRARY)
#if defined(i386_unknown_linux2_0)
		setlowestSObaseaddr(link_elm.l_addr);
#endif
#endif

	    }
	}
	else {
	   sharedobj_cerr << 
	       "file name matches that of image, so ignoring...firsttime=" 
	       << (int)first_time << endl;
        }

	first_time = false;
	next_addr = (u_int)link_elm.l_next;
    }
    p->setDynamicLinking();
    dynlinked = true;
    return shared_objects;
    shared_objects = 0;
}


// getLinkMapAddrs: returns a vector of addresses corresponding to all 
// base addresses in the link maps.  Returns 0 on error.
vector<u_int> *dynamic_linking::getLinkMapAddrs(process *p) {

    r_debug debug_elm;
    if(!p->readDataSpace((caddr_t)(r_debug_addr),
        sizeof(r_debug),(caddr_t)&(debug_elm),true)) {
#ifdef PDYN_DEBUG
        printf("read d_ptr_addr failed r_debug_addr = 0x%x\n",r_debug_addr);
#endif
        return 0;
    }

    bool first_time = true;
    link_map *next_link_map = debug_elm.r_map;
    Address next_addr = (Address)next_link_map; 
    vector<u_int> *link_addresses = new vector<u_int>;
    while(next_addr != 0) {
	link_map link_elm;
        if(!p->readDataSpace((caddr_t)(next_addr),
            sizeof(link_map),(caddr_t)&(link_elm),true)) {
            logLine("read next_link_map failed\n");
	    return 0;
        }
	// kludge: ignore the first entry
	if(!first_time) { 
	    (*link_addresses).push_back(link_elm.l_addr); 
	}
	else {
#ifdef PDYN_DEBUG
	  printf("first link map addr 0x%x\n",link_elm.l_addr);
#endif
	}

	first_time = false;
	next_addr = (u_int)link_elm.l_next;
    }
    return link_addresses;
    link_addresses = 0;
}

// getNewSharedObjects: returns a vector of shared_object one element for
// newly mapped shared object.  old_addrs contains the addresses of the
// currently mapped shared objects. Sets error_occured to true, and 
// returns 0 on error.
vector<shared_object *> *dynamic_linking::getNewSharedObjects(process *p,
						vector<u_int> *old_addrs,
						bool &error_occured){

    r_debug debug_elm;
    if(!p->readDataSpace((caddr_t)(r_debug_addr),
        sizeof(r_debug),(caddr_t)&(debug_elm),true)) {
#ifdef PDYN_DEBUG
        printf("read d_ptr_addr failed r_debug_addr = 0x%x\n",r_debug_addr);
#endif
	error_occured = true;
        return 0;
    }

    // get each link_map object
    bool first_time = true;
    link_map *next_link_map = debug_elm.r_map;
    Address next_addr = (Address)next_link_map; 
    vector<shared_object*> *new_shared_objects = new vector<shared_object*>;
    while(next_addr != 0){
	link_map link_elm;
        if(!p->readDataSpace((caddr_t)(next_addr),
            sizeof(link_map),(caddr_t)&(link_elm),true)) {
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
                shared_object *newobj = new shared_object(obj_name,
			link_elm.l_addr,false,true,true,0);
		(*new_shared_objects).push_back(newobj);
            }
	}
	first_time = false;
	next_addr = (u_int)link_elm.l_next;
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

#ifdef LD_DEBUG
    fprintf( stderr, "Welcome to dynamic_linking::getSharedObjects\n" );
#endif
    // step 1: figure out if this is a dynamic executable
    string dyn_str = string("DYNAMIC");
    internalSym dyn_sym;
    bool flag = p->findInternalSymbol(dyn_str,true, dyn_sym);
    if(!flag){ return 0;}
    int proc_fd = p->getProcFileDescriptor();
    if(!proc_fd){ return 0;}

    // step 2: find the base address and file descriptor of ld.so.1
    u_int ld_base = 0;
    int ld_fd = -1;
    char *ld_path;
    if(!(this->get_ld_info(ld_base,&ld_path,p))) { return 0;}
    if( (ld_fd = P_open( ld_path, O_RDONLY, 0 ) ) == -1 ) { return 0; }
#ifdef LD_DEBUG
    printf( "1st load of %s (%d)\n", ld_path, ld_fd );
#endif

    // step 3: get its symbol table and find r_debug
    if (!(this->find_r_debug(ld_fd,ld_base))) {
      close(ld_fd);
#ifdef PDYN_DEBUG
      fprintf( stderr, "Error in finding r_debug\n" );
#endif
      return 0;
    }
    close(ld_fd);

    // step 4: get link-maps and process them
    vector<shared_object *> *result = this->processLinkMaps(p);

    // step 5: set brkpoint in r_brk to catch dlopen and dlclose events
    if(!(this->set_r_brk_point(p))){ 
	// printf("error after step5 in getSharedObjects\n");
    }

    // additional step: find dlopen - naim
    if( (ld_fd = P_open( ld_path, O_RDONLY, 0 ) ) == -1 ) { return 0; }
#ifdef LD_DEBUG
    printf( "2nd load of %s (%d)\n", ld_path, ld_fd );
#endif
    if (!(this->find_dlopen(ld_fd,ld_base))) {
      logLine("WARNING: we didn't find dlopen in ld.so\n");
    }
    close(ld_fd);

    fflush(stdout);
    delete ld_path;
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
	vector<u_int> *addr_list =  new vector<u_int>;
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
	vector<u_int> *addr_list = getLinkMapAddrs(p);
	if(addr_list) {
	    vector <shared_object *> *remove_list = new vector<shared_object*>;
	    // find all shared objects that have been removed
	    for(u_int i=0; i < curr_list->size(); i++){
		u_int curr_addr = ((*curr_list)[i])->getBaseAddress(); 
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

static Address getSP(int pid) {
   Address regaddr = UESP * sizeof(int);
   int res;
   res = P_ptrace (PTRACE_PEEKUSER, pid, regaddr, 0);
   if( errno ) {
     perror( "getSP" );
     exit(-1);
     return 0; // Shut up the compiler
   } else {
     assert(res);
     return (Address)res;
   }   
}

static bool changeSP(int pid, Address loc) {
  Address regaddr = UESP * sizeof(int);
  if (0 != P_ptrace (PTRACE_POKEUSER, pid, regaddr, loc )) {
    perror( "process::changeSP - PTRACE_POKEUSER" );
    return false;
  }

  return true;
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
  error_occured = false;
  u_int pc = getPC( proc->getPid() );

  // is the trap instr at r_brk_addr?
  if(pc == (u_int)r_brk_addr){ 
#ifdef LD_DEBUG
    fprintf( stderr, "r_brk occurred\n" );
#endif
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
    if( r_state == 0 ){
#ifdef LD_DEBUG
      fprintf( stderr, "Change in link maps, now consistent\n" );
#endif
      // figure out how link maps have changed, and then create
      // a list of either all the removed shared objects if this
      // was a dlclose or the added shared objects if this was a dlopen

      // kludge: the state of the first add can get screwed up
      // so if both change_type and r_state are 0 set change_type to 1
      if( change_type == 0 ) change_type = 1;
      *changed_objects = findChangeToLinkMaps(proc, change_type,
					      error_occured);
#if defined(BPATCH_LIBRARY)
#if defined(i386_unknown_linux2_0)
		if(change_type == 1) { // RT_ADD
			for(int index = 0; index < (*changed_objects)->size(); index++){
				char *tmpStr = new char[ 1+strlen((*(*changed_objects))[index]->getName().string_of())];
				strcpy(tmpStr,(*(*changed_objects))[index]->getName().string_of());
				if( !strstr(tmpStr, "libdyninstAPI_RT.so") && 
			    		!strstr(tmpStr, "libelf.so")){
					//printf(" dlopen: %s \n", tmpStr);
					(*(*changed_objects))[index]->openedWithdlopen();
				}
				setlowestSObaseaddr((*(*changed_objects))[index]->getBaseAddress());
				delete [] tmpStr;	
			}	
		}
#endif
#endif

    } 

    // Return from the function.  We used to do this by setting the program
    // counter to the end of the function, but we don't necessarily know
    // how long it is, so now we emulate a ret instruction by changing the
    // PC to the return value from the stack and incrementing the stack
    // pointer.
    Address sp = getSP(proc->getPid());

    Address ret_addr;
    if(!proc->readDataSpace((caddr_t)sp, sizeof(Address),
			    (caddr_t)&ret_addr, true)) {
      // printf("read failed sp = 0x%x\n", sp);
      error_occured = true;
      return true;
    }

    if (!changeSP(proc->getPid(), sp + sizeof(Address))) {
      error_occured = true;
      return true;
    }

    if (!proc->changePC(ret_addr))
      error_occured = true;

    return true;
  }

  return false; 
}



