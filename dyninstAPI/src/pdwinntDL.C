/*
 * See the dyninst/COPYRIGHT file for copyright information.
 * 
 * We provide the Paradyn Tools (below described as "Paradyn")
 * on an AS IS basis, and do not warrant its validity or performance.
 * We reserve the right to update, modify, or discontinue this
 * software at any time.  We shall have no obligation to supply such
 * updates or modifications or any other form of support to you.
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

#include "mapped_object.h"
#include <windows.h>
#include <string>
#include "dynProcess.h"

// Since Windows handles library loads for us, there is nothing to do here
// Write in stubs to make the platform-indep code happy

extern std::string GetLoadedDllImageName( PCProcess* p, const DEBUG_EVENT& ev );
extern void printSysError(unsigned errNo);

sharedLibHook::sharedLibHook(PCProcess *p, sharedLibHookType t, Address b) 
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
bool dynamic_linking::getChangedObjects(EventRecord &, std::vector<mapped_object *> &)
{
    // This can be called by a platform indep. layer that wants
    // to get the list of new libraries loaded. 
    return true;
}
bool dynamic_linking::handleIfDueToSharedObjectMapping(EventRecord &ev, 
                                 std::vector<mapped_object*> &changed_objects,
                                 std::vector<bool> &is_new_object)
{
   if (!ev.lwp)
       //Return early if we're in the call from loadDyninstLib.
       // Windows can handle this without the special case call.
       return true;

   PCProcess *proc = ev.proc;
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
	   fprintf(stderr, "[%s:%d] - Couldn't SymLoadModule64\n", FILE__, __LINE__);
	   return true;
     }

     fileDescriptor desc(imageName.c_str(), 
                         (Address)ev.info.u.LoadDll.lpBaseOfDll,
                         (HANDLE)procHandle,
                         ev.info.u.LoadDll.hFile, true, 
                         (Address)ev.info.u.LoadDll.lpBaseOfDll);   

     BPatch_hybridMode mode = proc->getHybridMode();
     bool parseGaps = true;
     if (BPatch_defensiveMode == mode && !mapped_object::isSystemLib(imageName))
         parseGaps = false;
     //else //KEVINTODO: re-instate the else once isSystemLib is working
         mode = BPatch_normalMode;

     mapped_object *newobj = 
         mapped_object::createMappedObject(desc, proc, mode, parseGaps);
     if (!newobj) {
         fprintf(stderr, "[%s:%d] - Couldn't parse loaded module %s\n",
                 FILE__,__LINE__, imageName.c_str());
         return true;
     }
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
	   fprintf(stderr, "[%s:%d] - Couldn't SymUnloadModule64\n", FILE__, __LINE__);
	}
        
	mapped_object *oldobj = NULL;
        const std::vector<mapped_object *> &objs = proc->mappedObjects();
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

bool dynamic_linking::processLinkMaps(std::vector<fileDescriptor> &) {
    // Empty list so nothing happens

    return true;
}
