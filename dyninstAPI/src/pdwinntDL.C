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

// $Id: pdwinntDL.C,v 1.12 2008/02/23 02:09:10 jaw Exp $

#include "dynamiclinking.h"
#include "process.h"
#include "signalhandler.h"
#include "dyn_lwp.h"
#include "mapped_object.h"
#include <windows.h>
#include <string>

// Since Windows handles library loads for us, there is nothing to do here
// Write in stubs to make the platform-indep code happy

extern std::string GetLoadedDllImageName( process* p, const DEBUG_EVENT& ev );
extern void printSysError(unsigned errNo);

sharedLibHook::sharedLibHook(process *p, sharedLibHookType t, Address b) 
        : proc_(p), type_(t), breakAddr_(b), loadinst_(NULL) {}

sharedLibHook::~sharedLibHook() {}

bool dynamic_linking::initialize() {
   dynlinked = true;
   return true;
}


bool dynamic_linking::installTracing()
{
    return true;
}

bool dynamic_linking::decodeIfDueToSharedObjectMapping(EventRecord &, unsigned int &)
{
    // This can be called by a platform indep. layer that wants
    // to get the list of new libraries loaded. 
    return false;
}
bool dynamic_linking::getChangedObjects(EventRecord &, pdvector<mapped_object *> &)
{
    // This can be called by a platform indep. layer that wants
    // to get the list of new libraries loaded. 
    return true;
}
bool dynamic_linking::handleIfDueToSharedObjectMapping(EventRecord &ev, 
                                 pdvector<mapped_object*> &changed_objects,
                                 pdvector<bool> &is_new_object)
{
   if (!ev.lwp)
       //Return early if we're in the call from loadDyninstLib.
       // Windows can handle this without the special case call.
       return true;

   process *proc = ev.proc;
   handleT procHandle = ev.lwp->getProcessHandle();

   if (ev.type == evtLoadLibrary) {
     std::string imageName = GetLoadedDllImageName( proc, ev.info );

	 parsing_printf("%s[%d]: load dll %s: hFile=%x, base=%x, debugOff=%x, debugSz=%d lpname=%x, %d\n",
         __FILE__, __LINE__,
         imageName.c_str(),
         ev.info.u.LoadDll.hFile, ev.info.u.LoadDll.lpBaseOfDll,
         ev.info.u.LoadDll.dwDebugInfoFileOffset,
         ev.info.u.LoadDll.nDebugInfoSize,
         imageName.c_str(),
         ev.info.u.LoadDll.fUnicode,
         GetFileSize(ev.info.u.LoadDll.hFile,NULL));
     startup_printf("Loaded dll: %s\n", imageName.c_str());

	 if (!imageName.length())
		 return true;
     DWORD64 iresult = SymLoadModule64(procHandle, ev.info.u.LoadDll.hFile, 
                                 (PSTR) imageName.c_str(), NULL,
                                 (DWORD64) ev.info.u.LoadDll.lpBaseOfDll, 0);
     if (!iresult) {
       printSysError(GetLastError());
	   fprintf(stderr, "[%s:%u] - Couldn't SymLoadModule64\n", FILE__, __LINE__);
	   return true;
     }

     fileDescriptor desc(imageName.c_str(), 
                         (Address)ev.info.u.LoadDll.lpBaseOfDll,
                         (HANDLE)procHandle,
                         ev.info.u.LoadDll.hFile, true, 
                         (Address)ev.info.u.LoadDll.lpBaseOfDll);   
     // discover structure of new DLL, and incorporate into our
     // list of known DLLs
     mapped_object *newobj = mapped_object::createMappedObject(desc, proc);
     changed_objects.push_back(newobj);
     is_new_object.push_back(true);
     ev.what = SHAREDOBJECT_ADDED;
	 return true;
   }
   /**
    * Handle the library unload case.
	**/
    Address base = (Address) ev.info.u.UnloadDll.lpBaseOfDll;
    bool result = SymUnloadModule64(procHandle, base);
	if (!result) {
       printSysError(GetLastError());
	   fprintf(stderr, "[%s:%u] - Couldn't SymUnloadModule64\n", FILE__, __LINE__);
	}
        
	mapped_object *oldobj = NULL;
        const pdvector<mapped_object *> &objs = proc->mappedObjects();
	for (unsigned i=0; i<objs.size(); i++) {
            if (objs[i]->codeBase() == base) {
                oldobj = objs[i];
                break;
            }
	}
	if (!oldobj) 
            return true;
    changed_objects.push_back(oldobj);
    is_new_object.push_back(false);
    ev.what = SHAREDOBJECT_REMOVED;
    return true;
}

bool dynamic_linking::processLinkMaps(pdvector<fileDescriptor> &) {
    // Empty list so nothing happens

    return true;
}
