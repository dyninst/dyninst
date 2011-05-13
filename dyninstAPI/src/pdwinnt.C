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
#include <psapi.h>
#include <windows.h>
#include "dyninstAPI/src/mapped_object.h"
#include "dyninstAPI/src/emit-x86.h"
#include "common/h/arch.h"
#include "dyninstAPI/src/inst-x86.h"
#include "dyninstAPI/src/registerSpace.h"
#include "symtab.h"
#include "pcProcess.h"

#include "dyninstAPI/src/ast.h"

#include "dyninstAPI/src/function.h"

/* XXX This is only needed for emulating signals. */
#include "BPatch_thread.h"
#include "BPatch_process.h"
#include "nt_signal_emul.h"

// prototypes of functions used in this file

void InitSymbolHandler( HANDLE hProcess );
void ReleaseSymbolHandler( HANDLE hProcess );
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



bool CALLBACK printMods(PCSTR name, DWORD64 addr, PVOID unused) {
    fprintf(stderr, " %s @ %llx\n", name, addr);
    return true;
}


// already setup on this FD.
// disconnect from controlling terminal 
void OS::osDisconnect(void) {
}



void InitSymbolHandler( HANDLE hProcess )
{
}

void
ReleaseSymbolHandler( HANDLE hProcess )
{
    if( !SymCleanup( hProcess ) )
    {
        // TODO how to report error?
        fprintf( stderr, "failed to release symbol handler: %x\n",
            GetLastError() );
    }

    CloseHandle(hProcess);
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
callType int_function::getCallingConvention() {
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
                              int_function *target, 
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

bool EmitterIA32::emitCallCleanup(codeGen &gen, int_function *target, 
                     int frame_size, pdvector<Register> &extra_saves)
{
    callType call_conv = target->getCallingConvention();
    if ((call_conv == unknown_call || call_conv == cdecl_call) && frame_size)
    {
        //Caller clean-up
	// This effectively adds frame_size to %esp, without affecting %eflags
	emitLEA(RealRegister(REGNUM_ESP), RealRegister(Null_Register), 0,
		frame_size, RealRegister(REGNUM_ESP), gen);
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



bool OS::executableExists(const std::string &file) {
   struct stat file_stat;
   int stat_result;

   stat_result = stat(file.c_str(), &file_stat);
   if (stat_result == -1)
       stat_result = stat((file + std::string(".exe")).c_str(), &file_stat);
   return (stat_result != -1);
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

mapped_object *PCProcess::createObjectNoFile(Address addr)
{
	assert(!"not implemented");
    return NULL;
}


bool PCProcess::dumpCore(std::string coreFile)
{
	assert(!"not implemented");
	return false;
}

bool PCProcess::hideDebugger()
{
	assert(!"not implemented");
	return false;
}

int PCProcess::setMemoryAccessRights(Dyninst::Address start, Dyninst::Address size, int rights)
{
	assert(!"not implemented");
	return 0;
}

bool Frame::setPC(Dyninst::Address newpc)
{
	assert(!"not implemented");
	return false;
}

unsigned long PCProcess::setAOutLoadAddress(fileDescriptor &desc)
{
	assert(!"not implemented");
	return 0;
}

PCEventHandler::CallbackBreakpointCase PCEventHandler::getCallbackBreakpointCase(Dyninst::ProcControlAPI::EventType et)
{
	assert(!"not implemented");
	return PCEventHandler::NoCallbackOrBreakpoint;
}

bool PCEventHandler::isKillSignal(int signal)
{
	assert(!"not implemented");
	return false;
}

bool PCEventHandler::isCrashSignal(int signal)
{
	assert(!"not implemented");
	return false;
}

bool PCEventHandler::shouldStopForSignal(int signal)
{
	assert(!"not implemented");
	return false;
}

bool PCEventHandler::isValidRTSignal(int signal, PCEventHandler::RTBreakpointVal breakpointVal,
									 Dyninst::Address arg1, int status)
{
	assert(!"not implemented");
	return false;
}

bool PCProcess::usesDataLoadAddress() const
{
	assert(!"not implemented");
	return false;
}

bool PCProcess::setEnvPreload(std::vector<std::string> &envp, std::string fileName)
{
	assert(!"not implemented");
	return false;
}

void PCProcess::redirectFds(int stdin_fd, int stdout_fd, int stderr_fd, std::map<int,int> &result)
{
	assert(!"not implemented");
}

std::string PCProcess::createExecPath(const std::string &file, const std::string &dir)
{
	assert(!"not implemented");
	return "";
}

bool PCProcess::multithread_capable(bool ignoreIfMtNotSet)
{
	assert(!"not implemented");
	return false;
}

bool PCProcess::copyDanglingMemory(PCProcess *parent)
{
	assert(!"not implemented");
	return false;
}

bool PCProcess::instrumentMTFuncs()
{
	assert(!"not implemented");
	return false;
}

bool PCProcess::hasPassedMain()
{
	assert(!"not implemented");
	return false;
}

bool PCProcess::getExecFileDescriptor(std::string filename, bool waitForTrap, fileDescriptor &desc)
{
	assert(!"not implemented");
	return false;
}

bool PCProcess::skipHeap(const heapDescriptor &heap)
{
	assert(!"not implemented");
	return false;
}

bool PCProcess::postRTLoadCleanup()
{
	assert(!"not implemented");
	return false;
}

unsigned long PCProcess::findFunctionToHijack()
{
	assert(!"not implemented");
	return 0;
}

AstNodePtr PCProcess::createLoadRTAST()
{
	assert(!"not implemented");
	return AstNodePtr();
}

void PCProcess::inferiorMallocConstraints(Dyninst::Address, Dyninst::Address &lo, Dyninst::Address &hi, inferiorHeapType)
{
	assert(!"not implemented");
}

inferiorHeapType PCProcess::getDynamicHeapType() const
{
	assert(!"not implemented");
	return anyHeap;
}

bool PCProcess::startDebugger()
{
	assert(!"not implemented");
	return false;
}