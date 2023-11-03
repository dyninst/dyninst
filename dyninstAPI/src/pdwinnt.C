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

#include <iomanip>
#include <string>
#include "common/src/headers.h"
#include "dyninstAPI/src/os.h"
#include "dyninstAPI/src/addressSpace.h"
#include "binaryEdit.h"
#include "common/src/stats.h"
#include "dyninstAPI/src/debug.h"
#include "dyninstAPI/src/instPoint.h"
#include "common/src/ntHeaders.h"
#include "dyninstAPI/src/mapped_object.h"
#include "dyninstAPI/src/emit-x86.h"
#include "common/src/arch.h"
#include "dyninstAPI/src/inst-x86.h"
#include "dyninstAPI/src/registerSpace.h"
#include "image.h"
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
    fprintf(stderr, "*** System error [%u]: %s\n", errNo, buf);
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



bool getLWPIDs(std::vector <unsigned> &LWPids)
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
	//std::cerr << "symtab name (c++): " << symTabName() << std::endl;
    //const char *name = symTabName().c_str();
    const int buffer_size = 1024;
    char buffer[buffer_size];
    int pos;

    if (callingConv != unknown_call)
        return callingConv;

    if (symTabName().empty()) {
		assert(0);
        //Umm...
        return unknown_call;
    }
    switch(symTabName()[0]) {
        case '?':
            //C++ Encoded symbol. Everything is stored in the C++ name 
            // mangling scheme
            UnDecorateSymbolName(symTabName().c_str(), buffer, buffer_size, 
                UNDNAME_NO_ARGUMENTS | UNDNAME_NO_FUNCTION_RETURNS);
			printf("undecorated name: %s\n", buffer);
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
          pos = symTabName().find('@');
          if (pos != std::string::npos) {
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
          pos = symTabName().find('@');
          if (pos != std::string::npos) {
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
    else if (symTabName().find("::") != std::string::npos) {
        callingConv = thiscall_call;
    }
    else {
        callingConv = stdcall_call;
    }
    return callingConv;
}

static void emitNeededCallSaves(codeGen &gen, Register reg, std::vector<Register> &extra_saves);
static void emitNeededCallRestores(codeGen &gen, std::vector<Register> &saves);

int EmitterIA32::emitCallParams(codeGen &gen, 
                              const std::vector<AstNodePtr> &operands,
                              func_instance *target, 
                              std::vector<Register> &extra_saves, 
                              bool noCost)
{
    callType call_conven = target->getCallingConvention();
    int estimatedFrameSize = 0;
    std::vector <Register> srcs;
    Register ecx_target = Null_Register, edx_target = Null_Register;
    Address unused = ADDR_NULL;
    const int num_operands = operands.size();

    switch (call_conven) {
        case unknown_call:
        case cdecl_call:
        case stdcall_call:
          //Push all registers onto stack
          for (unsigned u = 0; u < operands.size(); u++) {
              Register src = Null_Register;
              Address unused = ADDR_NULL;
              if (!operands[u]->generateCode_phase2( gen, false, unused, src)) assert(0);
              assert(src != Null_Register);
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
              Register src = Null_Register;
              Address unused = ADDR_NULL;
              if (!operands[u]->generateCode_phase2( gen, false, unused, src)) assert(0);
              assert(src != Null_Register);
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
              Register src = Null_Register;
              Address unused = ADDR_NULL;
              if (!operands[u]->generateCode_phase2( gen, false, unused, src)) assert(0);
              assert(src != Null_Register);
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

    if (ecx_target != Null_Register) {
        //Store the parameter in ecx
		gen.rs()->loadVirtualToSpecific(ecx_target, RealRegister(REGNUM_ECX), gen);
    }

    if (edx_target != Null_Register) {
		gen.rs()->loadVirtualToSpecific(edx_target, RealRegister(REGNUM_EDX), gen);
    }
    return estimatedFrameSize;
}

bool EmitterIA32::emitCallCleanup(codeGen &gen, func_instance *target, 
                     int frame_size, std::vector<Register> &extra_saves)
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
                           std::vector<Register> &extra_saves)
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

static void emitNeededCallRestores(codeGen &gen, std::vector<Register> &saves)
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


void OS::get_sigaction_names(std::vector<std::string> &names)
{
	//names.push_back("signal");
}

bool PCProcess::getDyninstRTLibName()
{
    startup_printf("Begin getDyninstRTLibName\n");
    bool use_abi_rt = false;
#if defined(arch_64bit)
    use_abi_rt = (getAddressWidth() == 4);
#endif

    std::vector<std::string> rt_paths;
    std::string rt_base = "dyninstAPI_RT";
    if(use_abi_rt) rt_base += "_m32";
    rt_base += ".dll";
    if(!BinaryEdit::getResolvedLibraryPath(rt_base, rt_paths) || rt_paths.empty())
    {
        startup_printf("%s[%d]: Could not find %s in search path\n", FILE__, __LINE__, rt_base.c_str());
        return false;
    }
    for(auto i = rt_paths.begin();
        i != rt_paths.end();
        ++i)
    {
        startup_printf("%s[%d]: Candidate RTLib is %s\n", FILE__, __LINE__, i->c_str());
    }
    dyninstRT_name = rt_paths[0];
    return true;
}
