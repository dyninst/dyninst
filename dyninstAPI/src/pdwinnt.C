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

#include "common/src/std_namesp.h"
#include <iomanip>
#include <string>
#include "common/src/headers.h"
#include "dyninstAPI/src/os.h"
#include "dyninstAPI/src/addressSpace.h"
#include "common/src/stats.h"
#include "common/src/Types.h"
#include "dyninstAPI/src/debug.h"
#include "dyninstAPI/src/instPoint.h"
#include "common/src/ntHeaders.h"
#include "dyninstAPI/src/mapped_object.h"
#include "dyninstAPI/src/emit-x86.h"
#include "common/src/arch.h"
#include "dyninstAPI/src/inst-x86.h"
#include "dyninstAPI/src/registerSpace.h"
#include "image.h"
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

#define snprintf _snprintf

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

// FIXME
#if 0
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
            PCProcess::PCMemPerm rights(true, true, true);
            PCProcess::changeMemoryProtections(
                violationAddr - (violationAddr % ev.proc->getMemoryPageSize()), 
                ev.proc->getMemoryPageSize(), 
                rights /* PAGE_EXECUTE_READWRITE */ , 
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

bool PCProcess::getMemoryAccessRights(Address addr, PCMemPerm& rights) {
    if(!pcProc_->getMemoryAccessRights(addr, rights)) {
	    mal_printf("ERROR: failed to get access rights for page %lx, %s[%d]\n",
                   addr, FILE__, __LINE__);
        return false;
    }

    return true;
}

void PCProcess::changeMemoryProtections(Address addr, size_t size,
                                        PCMemPerm rights, bool setShadow) {
    PCMemPerm oldRights;
    unsigned pageSize = getMemoryPageSize();

	Address pageBase = addr - (addr % pageSize);
	size += (addr % pageSize);

	// Temporary: set on a page-by-page basis to work around problems
	// with memory deallocation
	for (Address idx = pageBase, idx_e = pageBase + size;
         idx < idx_e; idx += pageSize) {
        mal_printf("setting rights to %s for [%lx %lx)\n",
                   rights.getPermName().c_str(), idx , idx + pageSize);
        if (!pcProc_->setMemoryAccessRights(idx, pageSize,
                                            rights, oldRights)) {
			mal_printf("ERROR: failed to set access rights "
                       "for page %lx, %s[%d]\n", addr, FILE__, __LINE__);
		} else if (isMemoryEmulated() && setShadow) {
			Address shadowAddr = 0;
			PCMemPerm shadowRights;
			bool valid = false;
			boost::tie(valid, shadowAddr) = getMemEm()->translate(idx);
			if (!valid) {
				mal_printf("WARNING: set access rights on page %lx that has "
				           "no shadow %s[%d]\n",addr,FILE__,__LINE__);
			} else {
                if(!pcProc_->setMemoryAccessRights(shadowAddr, pageSize,
                                                   rights, shadowRights)) {
                    mal_printf("ERROR: failed to set access rights "
                               "for page %lx, %s[%d]\n",
                                shadowAddr, FILE__, __LINE__);
                }

				if (shadowRights != oldRights) {
					mal_printf("WARNING: shadow page[%lx] rights %s did not "
                               "match orig-page [%lx] rights %s\n",
                               shadowAddr, shadowRights.getPermName().c_str(),
                               addr, oldRights.getPermName().c_str());
				}
			}
		}
	}
}

bool PCProcess::setMemoryAccessRights(Address start, size_t size,
                                      PCMemPerm rights) {
    // if (PAGE_EXECUTE_READWRITE == rights || PAGE_READWRITE == rights) {
    if (rights.isRWX() || rights.isRW() ) {
        mapped_object *obj = findObject(start);
        int page_size = getMemoryPageSize();
        for (Address cur = start; cur < (start + size); cur += page_size) {
            obj->removeProtectedPage(start -(start % page_size));
        }
    }
    stopProcess();
    changeMemoryProtections(start, size, rights, true);
	return true;
}


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

mapped_object* PCProcess::createObjectNoFile(Address addr)
{
    Address closestObjEnd = 0;
    for (unsigned i = 0; i < mapped_objects.size(); i++) {
        if (addr >= mapped_objects[i]->codeAbs() &&
            addr < (mapped_objects[i]->codeAbs() +
                    mapped_objects[i]->imageSize())) {
            fprintf(stderr,"createObjectNoFile called for addr %lx, "
                    "matching existing mapped_object %s %s[%d]\n", addr,
                    mapped_objects[i]->fullName().c_str(), FILE__,__LINE__);
            return mapped_objects[i];
        }
        if (addr >= (mapped_objects[i]->codeAbs() + 
                     mapped_objects[i]->imageSize()) &&  
            closestObjEnd < (mapped_objects[i]->codeAbs() + 
                             mapped_objects[i]->imageSize())) {
            closestObjEnd = mapped_objects[i]->codeAbs() + 
                            mapped_objects[i]->imageSize();
        }
    }

    Address testRead = 0;

    // WindowsAPI VirtualQueryEx rounds down to pages size,
    // so we need to round up first.
    Address ObjOffset = closestObjEnd % getMemoryPageSize();
    if (ObjOffset) {
        closestObjEnd = closestObjEnd - ObjOffset + getMemoryPageSize();
    }
    if (readDataSpace((void*)addr, getAddressWidth(), &testRead, false)) {
		// create a module for the region enclosing this address
        ProcControlAPI::Process::MemoryRegion memRegion;
        if (!pcProc_->findAllocatedRegionAround(addr, memRegion)) {
            mal_printf("ERROR: failed to find allocated region for page %lx, %s[%d]\n",
                       addr, FILE__, __LINE__);
			assert(0);
            return NULL;
        }

        mal_printf("[%lx %lx] is valid region containing %lx and corresponding "
                   "to no object, closest is object ending at %lx %s[%d]\n", 
                   memRegion.first, memRegion.second, addr, closestObjEnd, FILE__,__LINE__);

        // The size of the region returned by VirtualQueryEx is from BaseAddress
        // to the end, NOT from meminfo.AllocationBase, which is what we want.
        // BaseAddress is the start address of the page of the address parameter
        // that is sent to VirtualQueryEx as a parameter
        Address regionSize = memRegion.second - memRegion.first;

        // read region into this PCProcess
        void* rawRegion = malloc(regionSize);
		if (!readDataSpace((void *)memRegion.first, regionSize, rawRegion, true)) {
            mal_printf("Error: failed to read memory region [%lx, %lx]\n",
                       memRegion.first, memRegion.second);
			printSysError(GetLastError());
			assert(0);
            return NULL;
		}

		// set up file descriptor
        char regname[64];
        snprintf(regname, 63, "mmap_buffer_%lx_%lx", memRegion.first, memRegion.second);
        fileDescriptor desc(string(regname),
                            memRegion.first, /*  code  */
                            memRegion.first, /*  data  */
                            regionSize,       /* length */
                            rawRegion,        /* rawPtr */
                            true);            /* shared */
        mapped_object *obj = mapped_object::createMappedObject
            (desc, this, getHybridMode(), false);
        if (obj != NULL) {
            obj->setMemoryImg();
            //mapped_objects.push_back(obj);
	        addMappedObject(obj);

            obj->parse_img()->getOrCreateModule(
                obj->parse_img()->getObject()->getDefaultModule());
            return obj;
        } else {
            fprintf(stderr,"Failed to create object (that was not backed by a file) at %lx\n", memRegion.first);
            return NULL;
        }

    }

    return NULL;
}

bool OS::executableExists(const std::string &file) {
   struct stat file_stat;
   int stat_result;

   stat_result = stat(file.c_str(), &file_stat);
   if (stat_result == -1)
       stat_result = stat((file + std::string(".exe")).c_str(), &file_stat);
   return (stat_result != -1);
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

bool PCProcess::dumpCore(std::string coreFile)
{
	assert(0);
	return false;
}

bool PCProcess::hideDebugger()
{
	Dyninst::ProcControlAPI::Thread::const_ptr threadPtr_ = pcProc_->threads().getInitialThread();
	if (!threadPtr_)
		return false;
	Address tibPtr = threadPtr_->getThreadInfoBlockAddr();
    if (!tibPtr) {
        return false;
    }

    // read in address of PEB
    unsigned int pebPtr;
    if (!readDataSpace((void*)(tibPtr+48), getAddressWidth(), (void*)&pebPtr, false)) {
        fprintf(stderr, "%s[%d] Failed to read address of Process Environment "
                "Block at 0x%x, which is TIB + 0x30\n", FILE__,__LINE__,tibPtr+48);
        return false;
    }

    // patch up the processBeingDebugged flag in the PEB
    unsigned char flag;
    if (!readDataSpace((void*)(pebPtr+2), 1, (void*)&flag, true)) 
        return false;
    if (flag) {
        flag = 0;
        if (!writeDataSpace((void*)(pebPtr+2), 1, (void*)&flag)) 
            return false;
    }

    //while we're at it, clear the NtGlobalFlag
    if (!readDataSpace((void*)(pebPtr+0x68), 1, (void*)&flag, true)) 
        return false;
    if (flag) {
        flag = flag & 0x8f;
        if (!writeDataSpace((void*)(pebPtr+0x68), 1, (void*)&flag)) 
            return false;
    }

    // clear the heap flags in the PEB
    unsigned int heapBase;
    unsigned int flagWord;
    if (!readDataSpace((void*)(pebPtr+0x18), 4, (void*)&heapBase, true)) 
        return false;

    // clear the flags in the heap itself
    if (!readDataSpace((void*)(heapBase+0x0c), 4, (void*)&flagWord, true)) 
        return false;
    flagWord = flagWord & (~0x50000062);
    if (!writeDataSpace((void*)(heapBase+0x0c), 4, (void*)&flagWord)) 
        return false;
    if (!readDataSpace((void*)(heapBase+0x10), 4, (void*)&flagWord, true)) 
        return false;
    flagWord = flagWord & (~0x40000060);
    if (!writeDataSpace((void*)(heapBase+0x10), 4, (void*)&flagWord)) 
        return false;

    return true;
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
//  if(et.code() == Dyninst::ProcControlAPI::EventType::Exit &&
//     et.time() == Dyninst::ProcControlAPI::EventType::Pre)
//    return true;
  
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

bool AddressSpace::usesDataLoadAddress() const
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

	desc = fileDescriptor(filename, efi->fileBase, efi->fileBase, false);
	desc.setHandles(efi->processHandle, efi->fileHandle);

	delete efi;
	return true;
}

bool PCProcess::skipHeap(const heapDescriptor &heap)
{
	return false;
}


inferiorHeapType PCProcess::getDynamicHeapType() const
{
	return anyHeap;
}


void OS::get_sigaction_names(std::vector<std::string> &)
{
	assert(0 && "Unimplemented");
}
