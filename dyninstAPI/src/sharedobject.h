/*
 * Copyright (c) 1996-2002 Barton P. Miller
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

// $Id: sharedobject.h,v 1.38 2003/10/21 17:22:28 bernat Exp $

#if !defined(_shared_object_h)
#define _shared_object_h

#include "common/h/String.h"
#include "common/h/Types.h"
#include "dyninstAPI/src/symtab.h"
#include "dyninstAPI/src/symtab.h"
#include "dyninstAPI/src/Object.h"
#ifndef BPATCH_LIBRARY
#include "paradynd/src/mdld.h"
#endif

/*
 * A class for link map information about a shared object that is mmapped 
 * by the dynamic linker into the applications address space at runtime. 
 */
#define 	SHAREDOBJECT_NOCHANGE	0
#define 	SHAREDOBJECT_ADDED	1
#define 	SHAREDOBJECT_REMOVED	2

class shared_object {

public:
    shared_object();
    shared_object(pdstring &n, Address b, bool p,bool m, bool i, image *d);
    shared_object(fileDescriptor *f, bool p, bool m, bool i, image *d);
    ~shared_object(){ objs_image = 0;}

    fileDescriptor *getFileDesc() { return desc; }
    const pdstring &getName(){ return name; }
    const pdstring &getShortName() { return short_name; }
    Address getBaseAddress() { return base_addr; }
    bool  isProcessed() { return(processed); }
    bool  isMapped() { return(mapped); }
    const image  *getImage() { return(objs_image); }

#ifndef BPATCH_LIBRARY
    bool includeFunctions(){ return(include_funcs); }
    void changeIncludeFuncs(bool flag){ include_funcs = flag; } 
    pdmodule *findModule(pdstring m_name, bool check_excluded);
 
#else
    pdmodule *findModule(pdstring m_name);
#endif

    const pdvector<pd_Function *> *getAllFunctions();
    void  unMapped(){ mapped = false; }
    void  setBaseAddress(Address new_ba){ base_addr = new_ba; }

#if defined(sparc_sun_solaris2_4) || defined(i386_unknown_linux2_0)
	bool isinText(Address addr){ 
		return objs_image->getObject().isinText(addr, base_addr);
	}
	void openedWithdlopen() { dlopenUsed = true; }; 
	bool isopenedWithdlopen() { return dlopenUsed; };
#endif

    bool  getSymbolInfo(const pdstring &n,Symbol &info) {
       if(objs_image) {
          return (objs_image->symbol_info(n,info));
       }
       return false;
    }

    // from a string that is a complete path name to a function in a module
    // (ie. "/usr/lib/libc.so.1/write") return a string with the function
    // part removed.  return 0 on error
    char *getModulePart(pdstring &full_path_name) ;

#ifndef BPATCH_LIBRARY
    // get only the functions not excluded by the mdl options exclude_lib
    // or exclude_funcs
    //    pdvector<pd_Function *> *getSomeFunctions();
    pdvector<pd_Function *> *getIncludedFunctions();
#endif

    // Get list of ALL modules, not just included ones.
    const pdvector<pdmodule *> *getModules() {
       if(objs_image) {
#ifndef BPATCH_LIBRARY
          return (&(objs_image->getAllModules()));
#else
          return (&(objs_image->getModules()));
#endif
       }
       return 0;
    }

    bool  addImage(image *i){ 
       if(!processed && (objs_image == 0)) {
	    objs_image = i;
	    processed = true;
       return true;
       }
       else {
          return false;
       }
    }
    bool removeImage(){ return true;}

    pd_Function *findOnlyOneFunction(const pdstring &funcname);
    pdvector<pd_Function *> *findFuncVectorByPretty(const pdstring &funcname);
 
    pd_Function *findFuncByAddress(const Address &address) {
        if (objs_image) {
            return objs_image->findFuncByOffset(address - base_addr);
        }
        else
            return NULL;
    }
    
#if defined(sparc_sun_solaris2_4) || defined(i386_unknown_linux2_0)
	//this marks the shared object as dirty, mutated
	//so it needs saved back to disk during saveworld 

	void setDirty(){ dirty_=true;}
	bool isDirty() { return dirty_; }


	//ccw 24 jul 2003
	//This marks the shared library as one that contains functions
	//that are called by instrumentation.  These functions, and hence
	//this shared library, MUST be reloaded in the same place.  The
	//shared library is not necessarily mutated itself, so it may not
	//need to be saved (as dirty_ would imply).
	void setDirtyCalled() { dirtyCalled_ = true; }
	bool isDirtyCalled() { return dirtyCalled_; }
#endif

    //
    //     PRIVATE DATA MEMBERS
    //				
private:
    fileDescriptor *desc; // full file descriptor

    pdstring  name;	// full file name of the shared object
    pdstring  short_name; // name of shared object as it should be identified
			//  in mdl, e.g. as used for "exclude"....
    Address   base_addr;// base address of where the shared object is mapped

    bool    processed;  // if true, daemon has processed the shared obj. file
    bool    mapped;     // if true, the application has the shared obj. mapped
			// shared objects can be unmapped as the appl. runs

 

    void set_short_name();
    bool dirty_; // marks the shared object as dirty 
	bool dirtyCalled_;//see comment for setDirtyCalled
  

#ifndef BPATCH_LIBRARY
    bool include_funcs; // if true include the the functions from this shared
			// object in the set of all instrumentable functions
			// (this is for foci not refined on the Code heirarchy)
			// - Conceptually assumes that shared object has 1
			// and only 1 module.
    pdvector<pd_Function *> *included_funcs; // all functions not excluded by 
				       // exclude_func option
#endif
    image  *objs_image; // pointer to image if processed is true 
    bool dlopenUsed; //mark this shared object as opened by dlopen

};

#endif
