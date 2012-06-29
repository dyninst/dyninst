/*
 * Copyright (c) 1996-2011 Barton P. Miller
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

#include "common/h/std_namesp.h"
#include <iomanip>
#include <string>
#include "dyninstAPI/src/symtab.h"
#include "common/h/headers.h"
#include "dyninstAPI/src/os.h"
#include "dyninstAPI/src/addressSpace.h"
#include "common/h/stats.h"
#include "common/h/Types.h"
#include "dyninstAPI/src/debug.h"
#include "dyninstAPI/src/instPoint.h"
#include "common/h/ntHeaders.h"
#include "dyninstAPI/src/mapped_object.h"
#include "dyninstAPI/src/emit-x86.h"
#include "common/h/arch.h"
#include "dyninstAPI/src/inst-x86.h"
#include "dyninstAPI/src/registerSpace.h"
#include "symtab.h"
#include "MemoryEmulator/memEmulator.h"
#include <boost/tuple/tuple.hpp>

#include "dyninstAPI/src/ast.h"

#include "dyninstAPI/src/function.h"
#include "dynProcess.h"

/* XXX This is only needed for emulating signals. */
#include "BPatch_thread.h"
#include "BPatch_process.h"
#include "nt_signal_emul.h"
#include "dyninstAPI/src/PCEventMuxer.h"

// prototypes of functions used in this file

void InitSymbolHandler( HANDLE hPCProcess );
void ReleaseSymbolHandler( HANDLE hPCProcess );
extern bool isValidAddress(AddressSpace *proc, Address where);

void printSysError(unsigned errNo) {
    char buf[1000];
    bool result = FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, NULL, errNo, 
		  MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		  buf, 1000, NULL);
    if (!result) {
        fprintf(stderr, "Couldn't print error message\n");
        printSysError(GetLastError());
    }
    fprintf(stderr, "*** System error [%d]: %s\n", errNo, buf);
    fflush(stderr);
}


// check if a file handle is for kernel32.dll
static bool kludge_isKernel32Dll(HANDLE fileHandle, std::string &kernel32Name) {
    static DWORD IndxHigh, IndxLow;
    static bool firstTime = true;
    BY_HANDLE_FILE_INFORMATION info;
    static std::string kernel32Name_;

    if (firstTime) {
       HANDLE kernel32H;
       firstTime = false;
       char sysRootDir[MAX_PATH+1];
       if (GetSystemDirectory(sysRootDir, MAX_PATH) == 0)
          assert(0);
       kernel32Name_ = std::string(sysRootDir) + "\\kernel32.dll";
       kernel32H = CreateFile(kernel32Name_.c_str(), GENERIC_READ, 
                              FILE_SHARE_READ, NULL, OPEN_EXISTING, NULL, NULL);
       assert(kernel32H);
       if (!GetFileInformationByHandle(kernel32H, &info)) {
          printSysError(GetLastError());
          assert(0);
       }
       IndxHigh = info.nFileIndexHigh;
       IndxLow = info.nFileIndexLow;
       CloseHandle(kernel32H);
    }

    if (!GetFileInformationByHandle(fileHandle, &info))
       return false;

    if (info.nFileIndexHigh==IndxHigh && info.nFileIndexLow==IndxLow) {
      kernel32Name = kernel32Name_;
      return true;
    }
    return false;
}

/* 
   Loading libDyninstRT.dll

   We load libDyninstRT.dll dynamically, by inserting code into the
   application to call LoadLibraryA. We don't use the inferior RPC
   mechanism from class PCProcess because it already assumes that
   libdyninst is loaded (it uses the inferior heap).
   Instead, we use a simple inferior call mechanism defined below
   to insert the code to call LoadLibraryA("libdyninstRT.dll").
 */
#if 0
Address loadDyninstDll(PCProcess *p, char Buffer[LOAD_DYNINST_BUF_SIZE]) {
    return 0;
}
#endif
// osTraceMe is not needed in Windows NT
void OS::osTraceMe(void) {}

bool PCProcess::dumpImage(std::string outFile)
{
  fprintf(stderr, "%s[%d]:  Sorry, dumpImage() not implemented for windows yet\n", FILE__, __LINE__);
  fprintf(stderr, "\t cannot create '%s' as requested\n", outFile.c_str());
  return false;
}


static void hasIndex(PCProcess *, unsigned, void *data, void *result) 
{
    *((int *) data) = (int) result;
}




bool CALLBACK printMods(PCSTR name, DWORD64 addr, PVOID unused) {
    fprintf(stderr, " %s @ %llx\n", name, addr);
    return true;
}

static bool decodeAccessViolation_defensive(EventRecord &ev, bool &wait_until_active)
{
    bool ret = false;
    wait_until_active = true;
    ev.address = (eventAddress_t) ev.info.u.Exception.ExceptionRecord.ExceptionAddress;
    Address violationAddr = 
        ev.info.u.Exception.ExceptionRecord.ExceptionInformation[1];
    mapped_object *obj = NULL;

    switch(ev.info.u.Exception.ExceptionRecord.ExceptionInformation[0]) {
    case 0: // bad read
        if (dyn_debug_malware) {
            Address origAddr = ev.address;
            vector<func_instance *> funcs;
            baseTramp *bt = NULL;
            ev.proc->getAddrInfo(ev.address, origAddr, funcs, bt);
            mal_printf("bad read in pdwinnt.C %lx[%lx]=>%lx [%d]\n",
                       ev.address, origAddr, violationAddr,__LINE__);
            // detach so we can see what's going on 
            pdvector<pdvector<Frame> >  stacks;
            if (!ev.proc->walkStacks(stacks)) {
                mal_printf("%s[%d]:  walkStacks failed\n", FILE__, __LINE__);
                return false;
            }
            for (unsigned i = 0; i < stacks.size(); ++i) {
                pdvector<Frame> &stack = stacks[i];
                for (unsigned int j = 0; j < stack.size(); ++j) {
                    Address origPC = 0;
                    vector<func_instance*> dontcare1;
                    baseTramp *dontcare2 = NULL;
                    ev.proc->getAddrInfo(stack[j].getPC(), origPC, dontcare1, dontcare2);
                    mal_printf("frame %d: %lx[%lx]\n", j, stack[j].getPC(), origPC);
                }
            }
            dyn_saved_regs regs;
            ev.lwp->getRegisters(&regs,false);
            fprintf(stderr,"REGISTER STATE:\neax=%lx \necx=%lx \nedx=%lx \nebx=%lx \nesp=%lx \nebp=%lx \nesi=%lx "
                   "\nedi=%lx\n",regs.cont.Eax, regs.cont.Ecx, regs.cont.Edx, 
                   regs.cont.Ebx, regs.cont.Esp, regs.cont.Ebp, 
                   regs.cont.Esi, regs.cont.Edi);
        }
        break;

    case 1: {// bad write 
        Address origAddr = ev.address;
        vector<func_instance *> writefuncs;
        baseTramp *bt = NULL;
        bool success = ev.proc->getAddrInfo(ev.address, origAddr, writefuncs, bt);
        if (dyn_debug_malware) {
            Address origAddr = ev.address;
			Address shadowAddr = 0;
			bool valid = false;
			boost::tie(valid, shadowAddr) = ev.proc->getMemEm()->translateBackwards(violationAddr);

			cerr << "Overwrite insn @ " << hex << origAddr << dec << endl;
            vector<func_instance *> writefuncs;
            baseTramp *bti = NULL;
            bool success = ev.proc->getAddrInfo(ev.address, origAddr, writefuncs, bti);
            if (success) {
                fprintf(stderr,"---%s[%d] overwrite insn at %lx[%lx] in "
                        "function\"%s\" [%lx], writing to %lx (%lx) \n",
                        FILE__,__LINE__, ev.address, origAddr,
						writefuncs.empty() ? "<NO FUNC>" : writefuncs[0]->get_name().c_str(), 
						writefuncs.empty() ? 0 : writefuncs[0]->addr(), 
                        violationAddr, shadowAddr);
            } else { 
                fprintf(stderr,"---%s[%d] overwrite insn at %lx, not "
                        "contained in any range, writing to %lx \n",
                        __FILE__,__LINE__, ev.address, violationAddr);
            }
            dyn_saved_regs regs;
            ev.lwp->getRegisters(&regs,false);
            fprintf(stderr,"REGISTER STATE:\neax=%lx \necx=%lx \nedx=%lx \nebx=%lx \nesp=%lx \nebp=%lx \nesi=%lx "
                   "\nedi=%lx\n",regs.cont.Eax, regs.cont.Ecx, regs.cont.Edx, 
                   regs.cont.Ebx, regs.cont.Esp, regs.cont.Ebp, 
                   regs.cont.Esi, regs.cont.Edi);
        }

        // ignore memory access violations originating in kernel32.dll 
        // (if not originating from an instrumented instruction)
        mapped_object *obj = ev.proc->findObject(origAddr);
        assert(obj);
        if ( BPatch_defensiveMode != obj->hybridMode() ) 
        {
            wait_until_active = false;
            ret = true;
            ev.type = evtIgnore;
            ev.lwp->changeMemoryProtections(
                violationAddr - (violationAddr % ev.proc->getMemoryPageSize()), 
                ev.proc->getMemoryPageSize(), 
                PAGE_EXECUTE_READWRITE, 
                false);
            break;
        }
        // it's a write to a page containing write-protected code if region
        // permissions don't match the current permissions of the written page
        obj = ev.proc->findObject(violationAddr);
        if (!obj && ev.proc->isMemoryEmulated()) {
            bool valid=false;
            Address orig=0;
            boost::tie(valid,orig) = ev.proc->getMemEm()->translateBackwards(violationAddr);
            if (valid) {
                violationAddr = orig;
                obj = ev.proc->findObject(violationAddr);
            }
        }
        if (obj) {
            using namespace SymtabAPI;
            Region *reg = obj->parse_img()->getObject()->
                findEnclosingRegion(violationAddr - obj->codeBase());
            pdvector<CallbackBase *> callbacks;
            if (reg && (reg->getRegionPermissions() == Region::RP_RW ||
                        reg->getRegionPermissions() == Region::RP_RWX  ) &&
                getCBManager()->dispenseCallbacksMatching
                    (evtCodeOverwrite, callbacks)) //checks for CBs, doesn't call them
                {
                    ev.info2 = reg;
                    ev.type = evtCodeOverwrite;
                    ret = true;
                    wait_until_active = false;
                }
            callbacks.clear();
        }
        else {
            fprintf(stderr,"%s[%d] WARNING, possible bug, write insn at "
                    "%lx wrote to %lx\n",
                    __FILE__,__LINE__,ev.address, violationAddr);
            // detach so we can see what's going on 
            //ev.proc->detachPCProcess(true);
        }
        break;
    }
    case 8: // no execute permissions
        fprintf(stderr, "ERROR: executing code that lacks executable "
                "permissions in pdwinnt.C at %lx, evt.addr=%lx [%d]\n",
                ev.address, violationAddr,__LINE__);
        ev.proc->detachPCProcess(true);
        assert(0);
        break;
    default:
        if (dyn_debug_malware) {
            Address origAddr = ev.address;
            vector<func_instance *> funcs;
            baseTramp *bti = NULL;
            ev.proc->getAddrInfo(ev.address, origAddr, funcs, bti);
            mal_printf("weird exception in pdwinnt.C illegal instruction or "
                       "access violation w/ code (%lx) %lx[%lx]=>%lx [%d]\n",
                       ev.info.u.Exception.ExceptionRecord.ExceptionInformation[0],
                       ev.address, origAddr, violationAddr,__LINE__);
        }
        ev.proc->detachPCProcess(true);
        assert(0);
    }
    if (evtCodeOverwrite != ev.type && ev.proc->isMemoryEmulated()) {
        // see if we were executing in defensive code whose memory access 
        // would have been emulated
        Address origAddr = ev.address;
        vector<func_instance *> writefuncs;
        baseTramp *bti = NULL;
        bool success = ev.proc->getAddrInfo(ev.address, origAddr, writefuncs, bti);
        mapped_object *faultObj = NULL;
        if (success) {
            faultObj = ev.proc->findObject(origAddr);
        }
        if (!faultObj || BPatch_defensiveMode == faultObj->hybridMode()) {
            // KEVINTODO: we're emulating the instruction, pop saved regs off 
            // of the stack and into the appropriate registers,
            // signalHandlerEntry will have to fix up the saved 
            // context information on the stack 
            assert(1 || "stack imbalance and bad reg values resulting from incomplete memory emulation of instruction that caused a fault");
        }
    }
    return ret;
}
#endif

// already setup on this FD.
// disconnect from controlling terminal 
void OS::osDisconnect(void) {
}

#if 0
bool PCProcess::setMemoryAccessRights (Address start, Address size, int rights)
{
    //mal_printf("setMemoryAccessRights to %x [%lx %lx]\n", rights, start, start+size);
    // get lwp from which we can call changeMemoryProtections
    dyn_lwp *stoppedlwp = query_for_stopped_lwp();
    assert( stoppedlwp );
    if (PAGE_EXECUTE_READWRITE == rights || PAGE_READWRITE == rights) {
        mapped_object *obj = findObject(start);
        int page_size = getMemoryPageSize();
        for (Address cur = start; cur < (start + size); cur += page_size) {
            obj->removeProtectedPage(start -(start % page_size));
        }
    }
    stoppedlwp->changeMemoryProtections(start, size, rights, true);
    return true;
}
#endif
bool PCProcess::getMemoryAccessRights(Address start, Address size, int rights)
{
   assert(0 && "Unimplemented!");
   return 0;
}

#if 0
int dyn_lwp::changeMemoryProtections
(Address addr, Offset size, unsigned rights, bool setShadow)
{
    unsigned oldRights=0;
    unsigned pageSize = proc()->getMemoryPageSize();

	Address pageBase = addr - (addr % pageSize);
	size += (addr % pageSize);

	// Temporary: set on a page-by-page basis to work around problems
	// with memory deallocation
	for (Address idx = pageBase; idx < pageBase + size; idx += pageSize) {
      //mal_printf("setting rights to %lx for [%lx %lx)\n", rights, idx , idx + pageSize);
		if (!VirtualProtectEx((HANDLE)getProcessHandle(), (LPVOID)(idx), 
			(SIZE_T)pageSize, (DWORD)rights, (PDWORD)&oldRights)) 
		{
			fprintf(stderr, "ERROR: failed to set access rights for page %lx, error code %d "
				"%s[%d]\n", addr, GetLastError(), FILE__, __LINE__);
			MEMORY_BASIC_INFORMATION meminfo;
			SIZE_T size = VirtualQueryEx(getPCProcessHandle(), (LPCVOID) (addr), &meminfo, sizeof(MEMORY_BASIC_INFORMATION));
			fprintf(stderr, "ERROR DUMP: baseAddr 0x%lx, AllocationBase 0x%lx, AllocationProtect 0x%lx, RegionSize 0x%lx, State 0x%lx, Protect 0x%lx, Type 0x%lx\n",
				meminfo.BaseAddress, meminfo.AllocationBase, meminfo.AllocationProtect, meminfo.RegionSize, meminfo.State, meminfo.Protect, meminfo.Type);
		}
		else if (proc()->isMemoryEmulated() && setShadow) {
			Address shadowAddr = 0;
			unsigned shadowRights=0;
			bool valid = false;
			boost::tie(valid, shadowAddr) = proc()->getMemEm()->translate(idx);
			if (!valid) {
				//fprintf(stderr, "WARNING: set access rights on page %lx that has "
				//	"no shadow %s[%d]\n",addr,FILE__,__LINE__);
			}
			else 
			{
				if (!VirtualProtectEx((HANDLE)getPCProcessHandle(), (LPVOID)(shadowAddr), 
					(SIZE_T)pageSize, (DWORD)rights, (PDWORD)&shadowRights)) 
				{
					fprintf(stderr, "ERROR: set access rights found shadow page %lx "
						"for page %lx but failed to set its rights %s[%d]\n",
						shadowAddr, addr, FILE__, __LINE__);
				}

				if (shadowRights != oldRights) {
					//mal_printf("WARNING: shadow page[%lx] rights %x did not match orig-page"
					//           "[%lx] rights %x\n",shadowAddr,shadowRights, addr, oldRights);
				}
			}
		}
	}
	return oldRights;
}
#endif


#if 0

// sets PC for stack frames other than the active stack frame
bool Frame::setPC(Address newpc) {

	if (!pcAddr_) {
		// if pcAddr isn't set it's because the stackwalk isn't getting the 
		// frames right
		fprintf(stderr,"WARNING: unable to change stack frame PC from %lx to %lx "
			"because we don't know where the PC is on the stack %s[%d]\n",
			pc_,newpc,FILE__,__LINE__);
		return false;
	}

	if (getProc()->writeDataSpace( (void*)pcAddr_, 
		getProc()->getAddressWidth(), 
		&newpc) ) 
	{
		this->pc_ = newpc;
		return true;
	}

	return false;
}
#endif

void InitSymbolHandler( HANDLE hPCProcess )
{
}

void
ReleaseSymbolHandler( HANDLE hPCProcess )
{
    if( !SymCleanup( hPCProcess ) )
    {
        // TODO how to report error?
        fprintf( stderr, "failed to release symbol handler: %x\n",
            GetLastError() );
    }

    CloseHandle(hPCProcess);
}


/*
 * stripAtSuffix
 *
 * Strips off of a string any suffix that consists of an @ sign followed by
 * decimal digits.
 *
 * str	The string to strip the suffix from.  The string is altered in place.
 */
static void stripAtSuffix(char *str)
{
    // many symbols have a name like foo@4, we must remove the @4
    // just searching for an @ is not enough,
    // as it may occur on other positions. We search for the last one
    // and check that it is followed only by digits.
    char *p = strrchr(str, '@');
    if (p) {
      char *q = p+1;
      strtoul(p+1, &q, 10);
      if (q > p+1 && *q == '\0') {
	*p = '\0';
      }
    }
}

char *cplus_demangle(char *c, int, bool includeTypes) { 
    char buf[1000];
    if (c[0]=='_') {
       // VC++ 5.0 seems to decorate C symbols differently to C++ symbols
       // and the UnDecorateSymbolName() function provided by imagehlp.lib
       // doesn't manage (or want) to undecorate them, so it has to be done
       // manually, removing a leading underscore from functions & variables
       // and the trailing "$stuff" from variables (actually "$Sstuff")
       unsigned i;
       for (i=1; i<sizeof(buf) && c[i]!='$' && c[i]!='\0'; i++)
           buf[i-1]=c[i];
       buf[i-1]='\0';
       stripAtSuffix(buf);
       if (buf[0] == '\0') return 0; // avoid null names which seem to annoy Paradyn
       return P_strdup(buf);
    } else {
       if (includeTypes) {
          if (UnDecorateSymbolName(c, buf, 1000, UNDNAME_COMPLETE| UNDNAME_NO_ACCESS_SPECIFIERS|UNDNAME_NO_MEMBER_TYPE|UNDNAME_NO_MS_KEYWORDS)) {
            //   printf("Undecorate with types: %s = %s\n", c, buf);
            stripAtSuffix(buf);
            return P_strdup(buf);
          }
       }  else if (UnDecorateSymbolName(c, buf, 1000, UNDNAME_NAME_ONLY)) {
         //     else if (UnDecorateSymbolName(c, buf, 1000, UNDNAME_COMPLETE|UNDNAME_32_BIT_DECODE)) {
         //	printf("Undecorate: %s = %s\n", c, buf);
         stripAtSuffix(buf);          
         return P_strdup(buf);
       }
    }
    return 0;
}

bool OS::osKill(int pid) {
    bool res;
    HANDLE h = OpenProcess(PROCESS_ALL_ACCESS, false, pid);
    if (h == NULL) {
    	return false;
    }
    res = TerminateProcess(h,0);
    CloseHandle(h);
    return res;
}



bool getLWPIDs(pdvector <unsigned> &LWPids)
{
  assert (0 && "Not implemented");
  return false;
}

#if 0
//
// This function retrieves the name of a DLL that has been
// loaded in an inferior PCProcess.  On the desktop flavors
// of Windows, the debug event that we get for loaded DLLs
// includes the location of a pointer to the DLL's image name.
// (Note the indirection.)  On Windows CE, the event contains
// the location of the image name string, with no indirection.
//
// There are several complications to overcome when reading this string:
//
// 1.  There is no guarantee that the image name is available.
//     In this case, the location in the debug event may be NULL,
//     or the pointer in the inferior PCProcess' address space may be NULL.
// 2.  The image name string may be either ASCII or Unicode.  Most of
//     the Windows system DLLs have Unicode strings, but many user-built
//     DLLs use single-byte strings.  If the string is Unicode, we need
//     to copy it to our address space and convert it to a single-byte
//     string because the rest of Paradyn/Dyninst has no clue what to
//     do with Unicode strings.
// 3.  We don't know how long the string is.  We have a loose upper
//     bound in that we know it is not more than MAX_PATH characters.
//     Unfortunately, the call we use to read from the inferior
//     PCProcess' address space will return failure if we ask for more
//     bytes than it can actually read (so we can't just issue a read
//     for MAX_PATH characters).  Given this limitation, we have to
//     try a read and check whether the read succeeded *and* whether
//     we read the entire image name string.  If not, we have to adjust
//     the amount we read and try again.
//
std::string GetLoadedDllImageName( PCProcess* p, const DEBUG_EVENT& ev )
{
    char *msgText = NULL;
	std::string ret;
	void* pImageName = NULL;

	if( ev.u.LoadDll.lpImageName != NULL )
	{
        msgText = new char[1024];	// buffer for error messages
	    // On non-CE flavors of Windows, the address given in the debug
        // event struct is the address of a pointer to the DLL name string.

        if( !p->readDataSpace( ev.u.LoadDll.lpImageName, 4, &pImageName, false ) )
        {
            sprintf( msgText, "Failed to read DLL image name pointer: %d\n",
            GetLastError() );
            logLine( msgText );
	    }
    }
	if( pImageName != NULL )
	{
		// we have the pointer to the DLL image name -
		// now read the name

		// allocate a conservatively-sized buffer
		char* buf = new char[(MAX_PATH + 1) * sizeof(WCHAR)];
		WCHAR* wbuf = (WCHAR*)buf;

		// We don't know how long the image name actually is, but
		// we do know that they tend not to be very long.
		// Therefore, we use a scheme to try to minimize the number
		// of reads needed to get the image name.
		// We do reads within ranges, starting with [1,128] bytes,
		// then [129,256] bytes, etc. up to MAX_PATH if necessary.
		// Within each range, we do reads following a binary search
		// algorithm.  For example, for the [1,128] range, we start
		// by trying to read 128 bytes.  If that read fails, we
		// try to half the number of bytes (i.e., 64).  If that
		// read also fails, we continue to halve the read requests 
		// until we find one that succeeds.
		//
		// When a read succeeds, we still may not have gotten the
		// entire string.  So when reads start succeeding, we have to
		// check the data we got for a null-terimated string.  If we didn't
		// get the full string, we change the byte count to either
		// move into the next higher range (if we were already reading
		// the max within the current range) or we set it to a factor
		// of 1.5 of the current byte count to try a value between the
		// current succeeding read and one that had failed.
		unsigned int loRead = 1;		// range boundaries
		unsigned int hiRead = 128;
		unsigned int cbRead = 128;		// number of bytes to read
		unsigned int chunkRead = 64;	// amount to change cbRead if we fail
										// we will not halve this before we read
		bool gotString = false;
		bool doneReading = false;
		while( !doneReading )
		{
			// try the read with the current byte count
			if( p->readDataSpace( pImageName, cbRead, buf, false ) )
			{
				// the read succeeded - 
				// did we get the full string?
				if( ev.u.LoadDll.fUnicode )
				{
					unsigned int cbReadIdx = cbRead / sizeof(WCHAR);
					wbuf[cbReadIdx] = L'\0';
					WCHAR* nulp = wcschr( wbuf, L'\0' );
					assert( nulp != NULL );			// because we just NULL-terminated the string
					gotString = (nulp != &(wbuf[cbReadIdx]));
				}
				else
				{
					buf[cbRead] = '\0';
					char* nulp = strchr( buf, '\0' );
					assert( nulp != NULL );			// because we just NULL-terminated the string
					gotString = (nulp != &(buf[cbRead]));
				}

				if( gotString )
				{
					doneReading = true;
				}
				else
				{
					// we didn't get the full string
					// we need to try a larger read
					if( cbRead == hiRead )
					{
						// we were at the high end of the current range -
						// move to the next range
						loRead = hiRead + 1;
						hiRead = loRead + 128 - 1;
						chunkRead = 128;				// we will halve this before we read again
						if( loRead > (MAX_PATH * sizeof(WCHAR)) )
						{
							// we've tried every range but still failed
							doneReading = true;
						}
						else
						{
							cbRead = hiRead;
						}
					}
					else
					{
						// we were within the current range -
						// try something higher but still within the range
						cbRead = cbRead + chunkRead;
					}
				}
			}
			else
			{
				// the read failed -
				// we need to try a smaller read
				if( cbRead > loRead )
				{
					unsigned int nextRead = cbRead - chunkRead;
					if( nextRead == cbRead )
					{
						// we can't subdivide any further
						doneReading = true;
					}
					else
					{
						cbRead = nextRead;
					}
				}
				else
				{
					// there are no smaller reads to try in this range,
					// and by induction, in any range.
					doneReading = true;
				}
			}

			// update the amount that we use to change the read request
			chunkRead /= 2;
		}

		if( !gotString )
		{
			// this is a serious problem because some read 
			// should've succeeded
			sprintf( msgText, "Failed to read DLL image name - no read succeeded\n" );
			logLine( msgText );
		}
		else
		{
			if( ev.u.LoadDll.fUnicode )
			{
				// the DLL path is a Unicode string
				// we have to convert it to single-byte characters
				char* tmpbuf = new char[MAX_PATH];

				WideCharToMultiByte(CP_ACP,		// code page to use (ANSI)
									0,			// flags
									wbuf,		// Unicode string
									-1,			// length of Unicode string (-1 => null-terminated)
									tmpbuf,		// destination buffer
									MAX_PATH,	// size of destionation buffer
									NULL,		// default for unmappable chars
									NULL);		// var to set when defaulting a char

				// swap buffers so that buf points to the single-byte string
				// when we're out of this code block
				delete[] buf;
				buf = tmpbuf;
			}
			ret = buf;
		}

		delete[] buf;
	}
	else
	{
		// we were given an image name pointer, but it was NULL
		// this happens for some system DLLs, and if we attach to
		// the PCProcess instead of creating it ourselves.
		// However, it is very important for us to know about kernel32.dll,
		// so we check for it specially.
		//
		// This call only changes the string parameter if the indicated file is
		// actually kernel32.dll.
		if (kludge_isKernel32Dll(ev.u.LoadDll.hFile, ret))
            return ret;

        //I'm embarassed to be writing this.  We didn't get a name for the image, 
        // but we did get a file handle.  According to MSDN, the best way to turn
        // a file handle into a file name is to map the file into the address space
        // (using the handle), then ask the OS what file we have mapped at that location.
        // I'm sad now.
        
        void *pmap = NULL;
        HANDLE fmap = CreateFileMapping(ev.u.LoadDll.hFile, NULL, 
                                        PAGE_READONLY, 0, 1, NULL);
        if (fmap) {
            pmap = MapViewOfFile(fmap, FILE_MAP_READ, 0, 0, 1);
            if (pmap) {   
                char filename[MAX_PATH+1];
                int result = GetMappedFileName(GetCurrentProcess(), pmap, filename, MAX_PATH);
                if (result)
                    ret = std::string(filename);
                UnmapViewOfFile(pmap);
            }
            CloseHandle(fmap);
        }
	}

	if (ret.substr(0,7) == "\\Device") {
      HANDLE currentProcess = p->ProcessHandle_;
      DWORD num_modules_needed;
      int errorCheck = EnumProcessModules(currentProcess,
                                          NULL,
                                          0,
                                          &num_modules_needed);
	  num_modules_needed /= sizeof(HMODULE);
      HMODULE* loadedModules = new HMODULE[num_modules_needed];
      errorCheck = EnumProcessModules(currentProcess,
                                          loadedModules,
                                          sizeof(HMODULE)*num_modules_needed,
                                          &num_modules_needed);
      HMODULE* candidateModule = loadedModules; 
      while(candidateModule < loadedModules + num_modules_needed)
      {
         MODULEINFO candidateInfo;
         GetModuleInformation(currentProcess, *candidateModule, &candidateInfo,
                              sizeof(candidateInfo));
         if(ev.u.LoadDll.lpBaseOfDll == candidateInfo.lpBaseOfDll)
            break;
         candidateModule++;
      }
      if(candidateModule != loadedModules + num_modules_needed) 
      {
         TCHAR filename[MAX_PATH];
         if(GetModuleFileNameEx(currentProcess, *candidateModule, filename, MAX_PATH))
         {
            ret = filename;
         }
      }
      delete[] loadedModules;

	}
	// cleanup
    if (msgText)
        delete[] msgText;

	return ret;
}
#endif



bool AddressSpace::getDyninstRTLibName() {
    // Set the name of the dyninst RT lib
    if (dyninstRT_name.length() == 0) {
        // Get env variable
        if (getenv("DYNINSTAPI_RT_LIB") != NULL) {
            dyninstRT_name = getenv("DYNINSTAPI_RT_LIB");
        }
        else {
            std::string msg = std::string("Environment variable ") +
               std::string("DYNINSTAPI_RT_LIB") +
               std::string(" has not been defined");
            showErrorCallback(101, msg);
            return false;
        }
    }
    //Canonicalize name
    char *sptr = P_strdup(dyninstRT_name.c_str());
    for (unsigned i=0; i<strlen(sptr); i++)
       if (sptr[i] == '/') sptr[i] = '\\';
    dyninstRT_name = sptr;
    free(sptr);
           
    if (_access(dyninstRT_name.c_str(), 04)) {
        std::string msg = std::string("Runtime library ") + dyninstRT_name +
                       std::string(" does not exist or cannot be accessed!");
        showErrorCallback(101, msg);
        return false;
    }

    return true;
}

#if 0
// Load the dyninst library
bool PCProcess::loadDYNINSTlib()
{
    loadDyninstLibAddr = getAOut()->parse_img()->getObject()->getEntryOffset() + getAOut()->getBaseAddress();
    Address LoadLibAddr;
    int_symbol sym;
    
    dyn_lwp *lwp;
    lwp = getInitialLwp();
 /*   if (lwp->status() == running) {
       lwp->pauseLWP();
    }*/

    if (!getSymbolInfo("_LoadLibraryA@4", sym) &&
        !getSymbolInfo("_LoadLibraryA", sym) &&
        !getSymbolInfo("LoadLibraryA", sym))
        {
            printf("unable to find function LoadLibrary\n");
            assert(0);
        }
    LoadLibAddr = sym.getAddr();
    assert(LoadLibAddr);

    char ibuf[BYTES_TO_SAVE];
    memset(ibuf, '\0', BYTES_TO_SAVE);//ccw 25 aug 2000
    char *iptr = ibuf;
    strcpy(iptr, dyninstRT_name.c_str());
    
    // Code overview:
    // Dynininst library name
    //    Executable code begins here:
    // Push (address of dyninst lib name)
    // Call LoadLibrary
    // Pop (cancel push)
    // Trap
    
    // 4: give us plenty of room after the string to start instructions
    int instructionOffset = strlen(iptr) + 4;
    // Regenerate the pointer
    iptr = &(ibuf[instructionOffset]);
    
    // At this point, the buffer contains the name of the dyninst
    // RT lib. We now generate code to load this string into memory
    // via a call to LoadLibrary
    
    // push nameAddr ; 5 bytes
    *iptr++ = (char)0x68; 
    // Argument for push
    *(int *)iptr = loadDyninstLibAddr; // string at codeBase
    iptr += sizeof(int);
    
    int offsetFromBufferStart = (int)iptr - (int)ibuf;
    offsetFromBufferStart += 5; // Skip next instruction as well.
    // call LoadLibrary ; 5 bytes
    *iptr++ = (char)0xe8;
    
    // Jump offset is relative
    *(int *)iptr = LoadLibAddr - (loadDyninstLibAddr + 
                                  offsetFromBufferStart); // End of next instruction
    iptr += sizeof(int);
    
    
    // add sp, 4 (Pop)
    *iptr++ = (char)0x83; *iptr++ = (char)0xc4; *iptr++ = (char)0x04;
    
    // int3
    *iptr = (char)0xcc;
    
    int offsetToTrap = (int) iptr - (int) ibuf;

    readDataSpace((void *)loadDyninstLibAddr, BYTES_TO_SAVE, savedCodeBuffer, false);
    writeDataSpace((void *)loadDyninstLibAddr, BYTES_TO_SAVE, ibuf);
    
    flushInstructionCache_((void *)loadDyninstLibAddr, BYTES_TO_SAVE);
    
    dyninstlib_brk_addr = loadDyninstLibAddr + offsetToTrap;
    
    savedRegs = new dyn_saved_regs;

    bool status = lwp->getRegisters(savedRegs);
    assert(status == true);    

	lwp->changePC(loadDyninstLibAddr + instructionOffset, NULL);
    
    setBootstrapState(loadingRT_bs);
    return true;
}
#endif
// Cleanup after dyninst lib loaded
#if 0
bool PCProcess::loadDYNINSTlibCleanup(dyn_lwp *)
{
    // First things first: 
    assert(savedRegs != NULL);
    getInitialLwp()->restoreRegisters(*savedRegs);
    delete savedRegs;
    savedRegs = NULL;

    writeDataSpace((void *) loadDyninstLibAddr,
                   BYTES_TO_SAVE,
                   (void *)savedCodeBuffer);

    flushInstructionCache_((void *)getAOut()->codeAbs(), BYTES_TO_SAVE);

    dyninstlib_brk_addr = 0;

    return true;
}
#endif
void loadNativeDemangler() 
{
    // ensure we load line number information when we load
    // modules, and give us mangled names
    DWORD dwOpts = SymGetOptions();
    dwOpts &= ~(SYMOPT_UNDNAME);
    dwOpts |= SYMOPT_LOAD_LINES;
    dwOpts &= ~(SYMOPT_DEFERRED_LOADS);
    SymSetOptions(dwOpts);
}


void dyninst_yield()
{
    SwitchToThread();
}

void OS::make_tempfile(char *name) {
}

bool OS::execute_file(char *file) {
   STARTUPINFO s;
   PROCESS_INFORMATION proc;
   BOOL result;

   ZeroMemory(&s, sizeof(s));
   ZeroMemory(&proc, sizeof(proc));
   s.cb = sizeof(s);

   result = CreateProcess(NULL, file, NULL, NULL, FALSE, 0, NULL, NULL, 
                          &s, &proc);
   if (!result) {
      fprintf(stderr, "Couldn't create %s - Error %d\n", file, GetLastError());
      return false;
   }

   WaitForSingleObject(proc.hProcess, INFINITE);
   CloseHandle(proc.hProcess);
   CloseHandle(proc.hThread);
   return true;
}

void OS::unlink(char *file) {
   DeleteFile(file);
}

#if !defined(TF_BIT)
#define TF_BIT 0x100
#endif

#if defined (cap_dynamic_heap)
void PCProcess::inferiorMallocConstraints(Address near, Address &lo, Address &hi,
                                        inferiorHeapType /* type */ ) 
{
}
#endif

/**
 stdcall:
   * C Naming - Name prefixed by a '_', followed by the name, then an '@',
     followed by number of bytes in arguments.  
     i.e. foo(int a, double b) = _foo@12
   * C++ Naming - __stdcall
   * Args - Arguments are passed on the stack.
   * Cleanup - Callee cleans up the stack before returning
 cdecl:
   * C Naming - Name prefixed by a '_'
   * C++ Naming - __cdecl in demangled name
   * Args - Arguments are passed on the stack.
   * Cleanup - Caller cleans up the stack after the return
 fastcall:
   * C Naming - Name prefixed by a '@', followed by the func name, then 
     another '@', followed by the number of bytes in the arguments.  i.e.
     foo(double a, int b, int c, int d) = @foo@20
   * C++ Naming - __fastcall in the mangled name
   * Args - First two arguments that are less than DWORD size are passed in ECX & EDX
   * Cleanup - Callee cleans up the stack before returning
 thiscall:
   * C Naming - NA
   * C++ Naming - __thiscall in the demangled name
   * 'this' parameter is passed in ECX, others are passed in the stack
   * Cleanup Callee cleans up the stack before returning
 **/
callType func_instance::getCallingConvention() {
    const char *name = symTabName().c_str();
    const int buffer_size = 1024;
    char buffer[buffer_size];
    const char *pos;

    if (callingConv != unknown_call)
        return callingConv;

    if (!name) {
        //Umm...
        return unknown_call;
    }

    switch(name[0]) {
        case '?':
            //C++ Encoded symbol. Everything is stored in the C++ name 
            // mangling scheme
            UnDecorateSymbolName(name, buffer, buffer_size, 
                UNDNAME_NO_ARGUMENTS | UNDNAME_NO_FUNCTION_RETURNS);
            if (strstr(buffer, "__thiscall")) {
                callingConv = thiscall_call;
                return callingConv;
            }
            if (strstr(buffer, "__fastcall")) {
                callingConv = fastcall_call;
                return callingConv;
            }
            if (strstr(buffer, "__stdcall")) {
                callingConv = stdcall_call;
                return callingConv;
            }
            if (strstr(buffer, "__cdecl")) {
                callingConv = cdecl_call;
                return callingConv;
            }
            break;
        case '_':
          //Check for stdcall or cdecl
          pos = strrchr(name, '@');
          if (pos) {
            callingConv = stdcall_call;
            return callingConv;
          }
          else {
            callingConv = cdecl_call;
            return callingConv;
          }
          break;
        case '@':
          //Should be a fast call
          pos = strrchr(name, '@');
          if (pos) {
             callingConv = fastcall_call;
             return callingConv;
          }
          break;
    }

    //We have no idea what this call is.  We probably got an undecorated
    // name.  If the function doesn't clean up it's own stack (doesn't 
    // have a ret #) instruction, then it must be a cdecl call, as that's
    // the only type that doesn't clean its own stack.
    //If the function is part of a class, then it's most likely a thiscall,
    // although that could be incorrect for a static function.  
    //Otherwise let's guess that it's a stdcall.
    if (!ifunc()->cleansOwnStack()) {
        callingConv = cdecl_call;
    }
    else if (strstr(name, "::")) {
        callingConv = thiscall_call;
    }
    else {
        callingConv = stdcall_call;
    }
    return callingConv;
}

static void emitNeededCallSaves(codeGen &gen, Register reg, pdvector<Register> &extra_saves);
static void emitNeededCallRestores(codeGen &gen, pdvector<Register> &saves);

int EmitterIA32::emitCallParams(codeGen &gen, 
                              const pdvector<AstNodePtr> &operands,
                              func_instance *target, 
                              pdvector<Register> &extra_saves, 
                              bool noCost)
{
    callType call_conven = target->getCallingConvention();
    int estimatedFrameSize = 0;
    pdvector <Register> srcs;
    Register ecx_target = REG_NULL, edx_target = REG_NULL;
    Address unused = ADDR_NULL;
    const int num_operands = operands.size();

    switch (call_conven) {
        case unknown_call:
        case cdecl_call:
        case stdcall_call:
          //Push all registers onto stack
          for (unsigned u = 0; u < operands.size(); u++) {
              Register src = REG_NULL;
              Address unused = ADDR_NULL;
              if (!operands[u]->generateCode_phase2( gen, false, unused, src)) assert(0);
              assert(src != REG_NULL);
              srcs.push_back(src);
          }
          break;
    case thiscall_call:
        //Allocate the ecx register for the 'this' parameter
        if (num_operands) {
            //result = gen.rs()->allocateSpecificRegister(gen, REGNUM_ECX, false);
            //if (!result) {
            //    emitNeededCallSaves(gen, REGNUM_ECX, extra_saves);
            //}
            if (!operands[0]->generateCode_phase2(gen, 
                                                  noCost, 
                                                  unused, ecx_target)) assert(0);
        }
        srcs.push_back(Null_Register);
        //Push other registers onto the stack
        for (unsigned u = 1; u < operands.size(); u++) {
              Register src = REG_NULL;
              Address unused = ADDR_NULL;
              if (!operands[u]->generateCode_phase2( gen, false, unused, src)) assert(0);
              assert(src != REG_NULL);
              srcs.push_back(src);
        }     
        break;
    case fastcall_call:
        if (num_operands) {
            //Allocate the ecx register for the first parameter
            //ecx_target = gen.rs()->allocateSpecificRegister(gen, REGNUM_ECX, false);
            //if (!ecx_target) {
            //    emitNeededCallSaves(gen, REGNUM_ECX, extra_saves);
            //}
        }
        if (num_operands > 1) {
            //Allocate the edx register for the second parameter
            //edx_target = gen.rs()->allocateSpecificRegister(gen, REGNUM_EDX, false);
            //if (!edx_target) {
            //    emitNeededCallSaves(gen, REGNUM_EDX, extra_saves);
            //}
        }
        if (num_operands) {
            if (!operands[0]->generateCode_phase2(gen, 
                                                  noCost, 
                                                  unused, ecx_target)) assert(0);
        }
        if (num_operands > 1) {
            if (!operands[1]->generateCode_phase2(gen, 
                                                  noCost, unused, edx_target)) assert(0);
        }
        srcs.push_back(Null_Register);
        srcs.push_back(Null_Register);

        //Push other registers onto the stack
        for (unsigned u = 2; u < operands.size(); u++) {
              Register src = REG_NULL;
              Address unused = ADDR_NULL;
              if (!operands[u]->generateCode_phase2( gen, false, unused, src)) assert(0);
              assert(src != REG_NULL);
              srcs.push_back(src);
        }
        break;
    default:
        fprintf(stderr, "Internal error.  Unknown calling convention\n");
        assert(0);
    }

    // push arguments in reverse order, last argument first
    // must use int instead of unsigned to avoid nasty underflow problem:
    for (int i=srcs.size() - 1; i >= 0; i--) {
       if (srcs[i] == Null_Register) continue;
	   RealRegister r = gen.rs()->loadVirtual(srcs[i], gen);
	   ::emitPush(r, gen);
       estimatedFrameSize += 4;
       if (operands[i]->decRefCount())
          gen.rs()->freeRegister(srcs[i]);
    }

    if (ecx_target != REG_NULL) {
        //Store the parameter in ecx
		gen.rs()->loadVirtualToSpecific(ecx_target, RealRegister(REGNUM_ECX), gen);
    }

    if (edx_target != REG_NULL) {
		gen.rs()->loadVirtualToSpecific(edx_target, RealRegister(REGNUM_EDX), gen);
    }
    return estimatedFrameSize;
}

bool EmitterIA32::emitCallCleanup(codeGen &gen, func_instance *target, 
                     int frame_size, pdvector<Register> &extra_saves)
{
    callType call_conv = target->getCallingConvention();
    if ((call_conv == unknown_call || call_conv == cdecl_call) && frame_size)
    {
        //Caller clean-up
        emitOpRegImm(0, RealRegister(REGNUM_ESP), frame_size, gen); // add esp, frame_size        
    }
    gen.rs()->incStack(-1 * frame_size);

    //Restore extra registers we may have saved when storing parameters in
    // specific registers
    //emitNeededCallRestores(gen, extra_saves);
    return 0;
}

static void emitNeededCallSaves(codeGen &gen, Register regi, 
                           pdvector<Register> &extra_saves)
{
    extra_saves.push_back(regi);
    switch (regi) {
        case REGNUM_EAX:
            emitSimpleInsn(PUSHEAX, gen);
            break;
        case REGNUM_EBX:
            emitSimpleInsn(PUSHEBX, gen);
            break;
        case REGNUM_ECX:
            emitSimpleInsn(PUSHECX, gen);
            break;
        case REGNUM_EDX:
            emitSimpleInsn(PUSHEDX, gen);
            break;
        case REGNUM_EDI:
            emitSimpleInsn(PUSHEDI, gen);
            break;
    }
}

static void emitNeededCallRestores(codeGen &gen, pdvector<Register> &saves)
{
    for (unsigned i=0; i<saves.size(); i++) {
      switch (saves[i]) {
          case REGNUM_EAX:
              emitSimpleInsn(POP_EAX, gen);
              break;
          case REGNUM_EBX:
              emitSimpleInsn(POP_EBX, gen);
              break;
          case REGNUM_ECX:
              emitSimpleInsn(POP_ECX, gen);
              break;
          case REGNUM_EDX:
              emitSimpleInsn(POP_EDX, gen);
              break;
          case REGNUM_EDI:
              emitSimpleInsn(POP_EDI, gen);
              break;
      }
    }
    saves.clear();
}



#if 0
mapped_object *PCProcess::createObjectNoFile(Address addr)
{
	cerr << "createObjectNoFile " << hex << addr << dec << endl;
    Address closestObjEnd = 0;
    for (unsigned i=0; i<mapped_objects.size(); i++)
    {
        if (addr >= mapped_objects[i]->codeAbs() &&
            addr <   mapped_objects[i]->codeAbs() 
                   + mapped_objects[i]->imageSize())
        {
            fprintf(stderr,"createObjectNoFile called for addr %lx, "
                    "matching existing mapped_object %s %s[%d]\n", addr,
                    mapped_objects[i]->fullName().c_str(), FILE__,__LINE__);
            return mapped_objects[i];
        }
        if (  addr >= ( mapped_objects[i]->codeAbs() + 
                        mapped_objects[i]->imageSize() ) &&  
            closestObjEnd < ( mapped_objects[i]->codeAbs() + 
                               mapped_objects[i]->imageSize() ) ) 
        {
            closestObjEnd = mapped_objects[i]->codeAbs() + 
                            mapped_objects[i]->imageSize();
        }
    }

    Address testRead = 0;

    // VirtualQueryEx rounds down to pages size, so we need to round up first.
    if (proc()->proc() && closestObjEnd % proc()->proc()->getMemoryPageSize())
    {
        closestObjEnd = closestObjEnd 
            - (closestObjEnd % proc()->proc()->getMemoryPageSize()) 
            + proc()->proc()->getMemoryPageSize();
    }
    if (proc()->proc() && readDataSpace((void*)addr, proc()->getAddressWidth(),
                                        &testRead, false)) 
    {
		// create a module for the region enclosing this address
        MEMORY_BASIC_INFORMATION meminfo;
        memset(&meminfo,0, sizeof(MEMORY_BASIC_INFORMATION) );
        SIZE_T size = VirtualQueryEx(proc()->ProcessHandle_,
                                     (LPCVOID)addr, &meminfo, 
                                     sizeof(MEMORY_BASIC_INFORMATION));
        assert(meminfo.State == MEM_COMMIT);
		cerr << "VirtualQuery reports baseAddr " << hex << meminfo.BaseAddress << ", allocBase " << meminfo.AllocationBase << ", size " << meminfo.RegionSize << ", state " << meminfo.State << dec << endl;

        Address objStart = (Address) meminfo.AllocationBase;
        Address probeAddr = (Address) meminfo.BaseAddress +  (Address) meminfo.RegionSize;
        Address objEnd = probeAddr;
        MEMORY_BASIC_INFORMATION probe;
        memset(&probe, 0, sizeof(MEMORY_BASIC_INFORMATION));
        do {
            objEnd = probeAddr;
            SIZE_T size2 = VirtualQueryEx(proc()->ProcessHandle_,
                                          (LPCVOID) ((Address)meminfo.BaseAddress + meminfo.RegionSize),
                                          &probe,
                                          sizeof(MEMORY_BASIC_INFORMATION));
			cerr << "VirtualQuery reports baseAddr " << hex << probe.BaseAddress << ", allocBase " << probe.AllocationBase << ", size " << probe.RegionSize << ", state " << probe.State << dec << endl;

			probeAddr = (Address) probe.BaseAddress + (Address) probe.RegionSize;
        } while ((probe.AllocationBase == meminfo.AllocationBase) && // we're in the same allocation unit...
			(objEnd != probeAddr)); // we're making forward progress


        // The size of the region returned by VirtualQueryEx is from BaseAddress
        // to the end, NOT from meminfo.AllocationBase, which is what we want.
        // BaseAddress is the start address of the page of the address parameter
        // that is sent to VirtualQueryEx as a parameter
        Address regionSize = objEnd - objStart;
        mal_printf("[%lx %lx] is valid region containing %lx and corresponding "
               "to no object, closest is object ending at %lx %s[%d]\n", 
               objStart, 
               objEnd,
               addr, closestObjEnd, FILE__,__LINE__);
        // read region into this PCProcess
        unsigned char* rawRegion = (unsigned char*) 
            ::LocalAlloc(LMEM_FIXED, regionSize);
		if (!proc()->readDataSpace((void *)objStart,
								   regionSize,
								   rawRegion, true))
		{
			cerr << "Error: failed to read memory region [" << hex << objStart << "," << objStart + regionSize << "]" << dec << endl;
			printSysError(GetLastError());
			assert(0);
		}
		// set up file descriptor
        char regname[64];
        snprintf(regname,63,"mmap_buffer_%lx_%lx",
                    objStart, objEnd);
        fileDescriptor desc(string(regname), 
                            0, 
                            (HANDLE)0, 
                            (HANDLE)0, 
                            true, 
                            (Address)objStart,
                            (Address)regionSize,
                            rawRegion);
        mapped_object *obj = mapped_object::createMappedObject
            (desc,this,proc()->getHybridMode(),false);
        if (obj != NULL) {
            obj->setMemoryImg();
            //mapped_objects.push_back(obj);
	    addMappedObject(obj);

            obj->parse_img()->getOrCreateModule(
                obj->parse_img()->getObject()->getDefaultModule());
            return obj;
        } else {
           fprintf(stderr,"Failed to create object (that was not backed by a file) at %lx\n", objStart);
        }
    }
    return NULL;
}
#endif



bool OS::executableExists(const std::string &file) {
   struct stat file_stat;
   int stat_result;

   stat_result = stat(file.c_str(), &file_stat);
   if (stat_result == -1)
       stat_result = stat((file + std::string(".exe")).c_str(), &file_stat);
   return (stat_result != -1);
}

#if 0
func_instance *dyn_thread::map_initial_func(func_instance *ifunc) {
    if (!ifunc || strcmp(ifunc->prettyName().c_str(), "mainCRTStartup"))
        return ifunc;

    //mainCRTStartup is not a real initial function.  Use main, if it exists.
    const pdvector<func_instance *> *mains = proc->getAOut()->findFuncVectorByPretty("main");
    if (!mains || !mains->size())
        return ifunc;
    return (*mains)[0];
}
#endif

bool PCProcess::instrumentThreadInitialFunc(func_instance *f) {
    if (!f)
        return false;

    for (unsigned i=0; i<initial_thread_functions.size(); i++) {
		if (initial_thread_functions[i] == f) {
            return true;
    }
    }
    func_instance *dummy_create = findOnlyOneFunction("DYNINST_dummy_create");
    if (!dummy_create)
    {
		return false;
    } 

    pdvector<AstNodePtr> args;
    AstNodePtr call_dummy_create = AstNode::funcCallNode(dummy_create, args);
	instPoint *entry = instPoint::funcEntry(f);
	miniTramp *mt = entry->push_front(call_dummy_create, false);
	//	relocate();
    /* PatchAPI stuffs */
    AddressSpace::patch(this);
    /* End of PatchAPI stuffs */

    if (!mt) {
      fprintf(stderr, "[%s:%d] - Couldn't instrument thread_create\n",
              __FILE__, __LINE__);
	}
    initial_thread_functions.push_back(f);
    return true;
}


bool PCProcess::hasPassedMain() 
{
   return true;
}


bool PCProcess::startDebugger()
{
   return false;
}

// Temporary remote debugger interface.
// I assume these will be removed when procControlAPI is complete.
bool OS_isConnected(void)
{
    return true;  // We're always connected to the child on this platform.
}

bool OS_connect(BPatch_remoteHost &remote)
{
    return true;  // We're always connected to the child on this platform.
}

bool OS_getPidList(BPatch_remoteHost &remote,
                   BPatch_Vector<unsigned int> &tlist)
{
    return false;  // Not implemented.
}

bool OS_getPidInfo(BPatch_remoteHost &remote,
                   unsigned int pid, std::string &pidStr)
{
    return false;  // Not implemented.
}

bool OS_disconnect(BPatch_remoteHost &remote)
{
    return true;
}
#if 0
mapped_object *PCProcess::createObjectNoFile(Address addr)
{
    Address closestObjEnd = 0;
    for (unsigned i=0; i<mapped_objects.size(); i++)
    {
        if (addr >= mapped_objects[i]->codeAbs() &&
            addr <   mapped_objects[i]->codeAbs()
                   + mapped_objects[i]->imageSize())
        {
            fprintf(stderr,"createObjectNoFile called for addr %lx, "
                    "matching existing mapped_object %s %s[%d]\n",
                    mapped_objects[i]->fullName().c_str(), FILE__,__LINE__);
            return mapped_objects[i];
        }
        if (  addr >= ( mapped_objects[i]->codeAbs() +
                        mapped_objects[i]->imageSize() ) &&
            closestObjEnd < ( mapped_objects[i]->codeAbs() +
                               mapped_objects[i]->imageSize() ) )
        {
            closestObjEnd = mapped_objects[i]->codeAbs() +
                            mapped_objects[i]->imageSize();
        }
    }

    Address testRead = 0;

    // VirtualQueryEx rounds down to pages size, so we need to round up first.
    if (proc()->proc() && closestObjEnd % proc()->proc()->getMemoryPageSize())
    {
        closestObjEnd = closestObjEnd
            - (closestObjEnd % proc()->proc()->getMemoryPageSize())
            + proc()->proc()->getMemoryPageSize();
    }
    if (proc()->proc() && readDataSpace((void*)addr, proc()->getAddressWidth(),
                                        &testRead, false))
    {
        // create a module for the region enclosing this address
        MEMORY_BASIC_INFORMATION meminfo;
        memset(&meminfo,0, sizeof(MEMORY_BASIC_INFORMATION) );
        SIZE_T size = VirtualQueryEx(proc()->ProcessHandle_,
                                     (LPCVOID)addr, &meminfo,
                                     sizeof(MEMORY_BASIC_INFORMATION));
        assert(meminfo.State == MEM_COMMIT);
        // The size of the region returned by VirtualQueryEx is from BaseAddress
        // to the end, NOT from meminfo.AllocationBase, which is what we want.
        // BaseAddress is the start address of the page of the address parameter
        // that is sent to VirtualQueryEx as a parameter
        Address regionSize = (Address)meminfo.BaseAddress
            - (Address)meminfo.AllocationBase
            + (Address)meminfo.RegionSize;
        mal_printf("[%lx %lx] is valid region containing %lx and corresponding "
               "to no object, closest is object ending at %lx %s[%d]\n",
               meminfo.AllocationBase,
               ((Address)meminfo.AllocationBase) + regionSize,
               addr, closestObjEnd, FILE__,__LINE__);
        // read region into this PCProcess
        unsigned char* rawRegion = (unsigned char*)
            ::LocalAlloc(LMEM_FIXED, meminfo.RegionSize);
        assert( proc()->readDataSpace(meminfo.AllocationBase,
                                    regionSize, rawRegion, true) );
        // set up file descriptor
        char regname[64];
        snprintf(regname,63,"mmap_buffer_%lx_%lx",
                 ((Address)meminfo.AllocationBase),
                 ((Address)meminfo.AllocationBase) + regionSize);

        fileDescriptor desc(string(regname),
                            0,
                            (HANDLE)0,
                            (HANDLE)0,
                            true,
                            (Address)meminfo.AllocationBase,
                            (Address)meminfo.RegionSize,
                            rawRegion);
        mapped_object *obj = mapped_object::createMappedObject
            (desc,this,proc()->getHybridMode(),false);
        if (obj != NULL) {
            mapped_objects.push_back(obj);
            addOrigRange(obj);
            return obj;
        }
    }
    return NULL;
}
#endif

bool PCProcess::dumpCore(std::string coreFile)
{
	assert(0);
	return false;
}

bool PCProcess::hideDebugger()
{
	assert(0);
	return false;
}

bool PCProcess::setMemoryAccessRights(Dyninst::Address start, Dyninst::Address size, int rights)
{
	assert(0);
	return false;
}

unsigned long PCProcess::setAOutLoadAddress(fileDescriptor &desc)
{
	assert(0);
	return 0;
}

bool PCEventMuxer::useCallback(Dyninst::ProcControlAPI::EventType et)
{
    // This switch statement can be derived from the EventTypes and Events
    // table in the ProcControlAPI manual -- it states what Events are
    // available on each platform
    
    switch(et.code()) {
        case Dyninst::ProcControlAPI::EventType::Exit:
            switch(et.time()) {
                case Dyninst::ProcControlAPI::EventType::Pre:
                case Dyninst::ProcControlAPI::EventType::Post:
					return true;
                default:
                    break;
            }
            break;
		case Dyninst::ProcControlAPI::EventType::LWPDestroy:
            switch(et.time()) {
                case Dyninst::ProcControlAPI::EventType::Pre:
					return true;
                default:
                    break;
            }
            break;
    }

    return false;
}

bool PCEventMuxer::useBreakpoint(Dyninst::ProcControlAPI::EventType et)
{
	return false;
}

bool PCEventHandler::isKillSignal(int signal)
{
	// Kill on Windows does not generate a signal
	return false;
}
bool PCEventHandler::isCrashSignal(int signal)
{
	switch(signal)
	{
	case EXCEPTION_ACCESS_VIOLATION:
	case EXCEPTION_ARRAY_BOUNDS_EXCEEDED:
	case EXCEPTION_DATATYPE_MISALIGNMENT:
	case EXCEPTION_FLT_DIVIDE_BY_ZERO:
	case EXCEPTION_FLT_INVALID_OPERATION:
	case EXCEPTION_ILLEGAL_INSTRUCTION:
	case EXCEPTION_IN_PAGE_ERROR:
	case EXCEPTION_INT_DIVIDE_BY_ZERO:
	case EXCEPTION_NONCONTINUABLE_EXCEPTION:
	case EXCEPTION_PRIV_INSTRUCTION:
	case EXCEPTION_STACK_OVERFLOW:
		return true;
	}
	return false;
}
bool PCEventHandler::shouldStopForSignal(int signal)
{
	switch(signal)
	{
		case EXCEPTION_BREAKPOINT:
			return true;
	}
	return false;
}
bool PCEventHandler::isValidRTSignal(int signal, PCEventHandler::RTBreakpointVal breakpointVal,
									 Dyninst::Address arg1, int status)
{
	if(signal == EXCEPTION_BREAKPOINT)
	{
        if( breakpointVal == NormalRTBreakpoint ) {
            if( (status != DSE_forkExit) || (arg1 != 0) ) return true;

            proccontrol_printf("%s[%d]: child received signal %d\n",
                    FILE__, __LINE__, EXCEPTION_BREAKPOINT);
        } else if( breakpointVal == SoftRTBreakpoint ) {
            if( status == DSE_forkExit ) {
                if( arg1 == 0 ) return true;

                proccontrol_printf("%s[%d]: parent process received SIGSTOP\n",
                        FILE__, __LINE__);
            }else{
                proccontrol_printf("%s[%d]: SIGSTOP wasn't due to fork exit\n",
                        FILE__, __LINE__);
            }
        } else {
            proccontrol_printf("%s[%d]: mismatch in signal for breakpoint type\n",
                    FILE__, __LINE__);
        }
	} else {
        proccontrol_printf("%s[%d]: signal wasn't sent by RT library\n",
                FILE__, __LINE__);
    }

	return false;
}

bool PCProcess::usesDataLoadAddress() const
{
	return false;
}
bool PCProcess::setEnvPreload(std::vector<std::string> &envp, std::string fileName)
{
	// We don't LD_PRELOAD on Windows
	return true;
}

void PCProcess::redirectFds(int stdin_fd, int stdout_fd, int stderr_fd, std::map<int,int> &result)
{
	// Not implemented on existing dyninst-on-windows, just skip
	return;
}
std::string PCProcess::createExecPath(const std::string &file, const std::string &dir)
{
	return dir + file;
}

bool PCProcess::multithread_capable(bool ignoreIfMtNotSet)
{
	return true;
}

bool PCProcess::copyDanglingMemory(PCProcess *parent)
{
	assert(0);
	return false;
}

bool PCProcess::instrumentMTFuncs()
{
	// This is not needed on Windows, as we get thread events directly.
	return true;
}

bool PCProcess::getExecFileDescriptor(std::string filename, bool waitForTrap, fileDescriptor &desc)
{
	Address mainFileBase = 0;
	Dyninst::ProcControlAPI::ExecFileInfo* efi = pcProc_->getExecutableInfo();

	desc = fileDescriptor(filename.c_str(),
			(Address)(0),
			efi->processHandle,
			efi->fileHandle,
			false,
			efi->fileBase);
	delete efi;
	return true;
}

bool PCProcess::skipHeap(const heapDescriptor &heap)
{
	return false;
}

bool PCProcess::postRTLoadCleanup()
{
	return true;
}

unsigned long PCProcess::findFunctionToHijack()
{
	return 0;
}

bool PCProcess::postRTLoadRPC()
{
    Address loadDyninstLibAddr = getAOut()->parse_img()->getObject()->getEntryOffset() + getAOut()->getBaseAddress();
	Address LoadLibAddr;
    int_symbol sym;
    

    if (!getSymbolInfo("_LoadLibraryA@4", sym) &&
        !getSymbolInfo("_LoadLibraryA", sym) &&
        !getSymbolInfo("LoadLibraryA", sym))
        {
            printf("unable to find function LoadLibrary\n");
            assert(0);
        }
    LoadLibAddr = sym.getAddr();
    assert(LoadLibAddr);

    char ibuf[BYTES_TO_SAVE];
    memset(ibuf, '\0', BYTES_TO_SAVE);//ccw 25 aug 2000
    char *iptr = ibuf;
    
    // Code overview:
    // Dynininst library name
    //    Executable code begins here:
    // Push (address of dyninst lib name)
    // Call LoadLibrary
    // Pop (cancel push)
    // Trap
    
    
    // push nameAddr ; 5 bytes
    *iptr++ = (char)0x68; 
    // Argument for push
	int* relocAddr = (int*)(iptr);
    iptr += sizeof(int);
    
    int offsetFromBufferStart = (int)iptr - (int)ibuf;
    offsetFromBufferStart += 5; // Skip next instruction as well.
    // call LoadLibrary ; 5 bytes
    *iptr++ = (char)0xe8;
    
    // Jump offset is relative
    *(int *)iptr = LoadLibAddr - (loadDyninstLibAddr + 
                                  offsetFromBufferStart); // End of next instruction
    iptr += sizeof(int);
    
    
    // add sp, 4 (Pop)
    *iptr++ = (char)0x83; *iptr++ = (char)0xc4; *iptr++ = (char)0x04;
    
    // int3
    *iptr = (char)0xcc;
    
    int offsetToTrap = (int) iptr - (int) ibuf;
	strcpy(iptr+1, dyninstRT_name.c_str());
    *(int *)relocAddr = offsetToTrap + 1 + loadDyninstLibAddr; // string at end of code
	void* result;
	postIRPC(ibuf, BYTES_TO_SAVE, NULL, false, NULL, true, &result, false, false, loadDyninstLibAddr);

   return true;
}


AstNodePtr PCProcess::createLoadRTAST()
{
	assert(!"Unused on Windows");
	return AstNodePtr();
}

inferiorHeapType PCProcess::getDynamicHeapType() const
{
	return anyHeap;
}

mapped_object* PCProcess::createObjectNoFile(Dyninst::Address addr)
{
	assert(0);
	return false;
}


