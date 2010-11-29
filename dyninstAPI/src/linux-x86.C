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

// $Id: linux-x86.C,v 1.142 2008/08/01 17:55:12 roundy Exp $

#include <fstream>
#include <string>

#include <fcntl.h>
#include <dlfcn.h>
#include <sys/user.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <math.h> // for floor()
#include <unistd.h> // for sysconf()
#include <elf.h>
#include <libelf.h>

#include "dyninstAPI/src/miniTramp.h"
#include "dyninstAPI/src/baseTramp.h"
#include "dyninstAPI/src/symtab.h"
#include "dyninstAPI/src/function.h"
#include "dyninstAPI/src/instPoint.h"
#include "common/h/headers.h"
#include "dyninstAPI/src/os.h"
#include "common/h/stats.h"
#include "common/h/Types.h"
#include "dyninstAPI/src/debug.h"
#include "dyninstAPI/src/util.h" // getCurrWallTime
#include "common/h/pathName.h"
#include "dyninstAPI/src/inst-x86.h"
#include "dyninstAPI/src/emit-x86.h"

#include "dyninstAPI/src/mapped_object.h" 

#include "dyninstAPI/src/linux.h"

#include "dyninstAPI/src/registerSpace.h"

#include <sstream>

#include "dyninstAPI/src/ast.h"
#include "dyninstAPI/src/binaryEdit.h"
#include "dyninstAPI/src/pcThread.h"
#include "dyninstAPI/src/pcProcess.h"
#include "common/h/linuxKludges.h"

#include "instructionAPI/h/InstructionDecoder.h"
#include "instructionAPI/h/Instruction.h"

using namespace Dyninst;
using namespace Dyninst::SymtabAPI;
using namespace Dyninst::ProcControlAPI;

#define DLOPEN_MODE (RTLD_NOW | RTLD_GLOBAL)

const char *DL_OPEN_FUNC_USER = NULL;
const char DL_OPEN_FUNC_EXPORTED[] = "dlopen";
const char DL_OPEN_FUNC_NAME[] = "do_dlopen";
const char DL_OPEN_FUNC_INTERNAL[] = "_dl_open";

const char libc_version_symname[] = "__libc_version";

#if defined(arch_x86_64)
bool PCProcess::getSysCallParameters(const RegisterPool &regs, long *params, int numparams) 
{
   if (getAddressWidth() == 4) { // 32 bit mutatee
   } else { // 64 bit mutatee, have to use ifdef, otherwise it won't
      // compile on a 32 bit machine
      if (numparams > 0) {
         params[0] = regs[x86_64::rdi];
      }
      if (numparams > 1) {
         params[1] = regs[x86_64::rsi];
      }
      if (numparams > 2) {
         params[2] = regs[x86_64::rdx];
      }
      if (numparams > 3) {
         params[3] = regs[x86_64::r8];
      }
      if (numparams > 4) {
         params[4] = regs[x86_64::r9];
      }
      if (numparams > 5) {
         params[5] = regs[x86_64::r10];
      }
      for (int i=6; i < numparams; i++) {
         if (!readDataSpace((void*)regs[x86_64::rsp], getAddressWidth(), 
                  (void*)(params + i * getAddressWidth()), true)) {
            return false;
         }
      }
   }
   return true;
}

int PCProcess::getSysCallNumber(const RegisterPool &regs) {
   return regs[x86_64::orax];
}

long PCProcess::getSysCallReturnValue(const RegisterPool &regs) {
   return regs[x86_64::rax];
}

Address PCProcess::getSysCallProgramCounter(const RegisterPool &regs) {
   return regs[x86_64::rip];
}

bool PCProcess::isMmapSysCall(int callnum) {
   if (getAddressWidth() == 4) {
      startup_printf("CALLNUM=%d\n",callnum);
   }
   return callnum == SYS_mmap;
}

Offset PCProcess::getMmapLength(int, const RegisterPool &regs) {
   return (Offset) regs[x86_64::rsi];
}

Address PCProcess::getLibcStartMainParam(PCThread *trappingThread) {
   Address mainaddr = 0;
   RegisterPool regs;
   trappingThread->getRegisters(regs);
   if (getAddressWidth() == 4) { // 32 bit mutatee
      if (!readDataSpace((void*)(regs[x86_64::rsp] + getAddressWidth()),
               getAddressWidth(), (void*)&mainaddr,true)) {
         fprintf(stderr,"[%s][%d]: failed readDataSpace\n", __FILE__,__LINE__); 
      }
   } else { // 64 bit mutatee
      mainaddr = regs[x86_64::rdi];
   }
   return mainaddr;
}
// 64 bit architecture
#else 
// 32 bit architecture
bool PCProcess::getSysCallParameters(const RegisterPool &regs, 
      long *params, int numparams) {
   if (numparams > 0) {
      params[0] = regs[x86::ebx];
   }
   if (numparams > 1) {
      params[1] = regs[x86::ecx];
   }
   if (numparams > 2) {
      params[2] = regs[x86::edx];
   }
   if (numparams > 3) {
      params[3] = regs[x86::esi];
   }
   if (numparams > 4) {
      params[4] = regs[x86::edi];
   }
   if (numparams > 5) {
      params[5] = regs[x86::ebp];
   }
   for (int i=6; i < numparams; i++) {
      if (!readDataSpace((void*)regs[x86::esp], getAddressWidth(), 
               (void*)(params + i * getAddressWidth()), true)) {
         return false;
      }
   }
   return true;
}

int PCProcess::getSysCallNumber(const RegisterPool &regs) 
{
   return regs[x86::oeax];
}

long PCProcess::getSysCallReturnValue(const RegisterPool &regs) 
{
   return regs[x86::eax];
}

Address PCProcess::getSysCallProgramCounter(const RegisterPool &regs) 
{
   return regs[x86::eip];
}

bool PCProcess::isMmapSysCall(int callnum) {
   return (callnum == SYS_mmap || callnum == SYS_mmap2);
}

Offset PCProcess::getMmapLength(int callnum, const RegisterPool &regs) 
{
   if (callnum == SYS_mmap) {
      Offset length;
      readDataSpace((void*)(regs[x86::ebx] + getAddressWidth()),
            getAddressWidth(), (void*)&length, true);
      return length;
   }
   else {
      return (Offset) regs[x86::ecx];
   }
}

Address PCProcess::getLibcStartMainParam(PCThread *trappingThread) 
{
   RegisterPool regs;
   trappingThread->getRegisters(regs);
   Address mainaddr;
   if (!readDataSpace((void*)(regs[x86::esp] + getAddressWidth()),
            getAddressWidth(), (void*)&mainaddr,true)) {
      fprintf(stderr,"[%s][%d]: failed readDataSpace\n", __FILE__,__LINE__); 
   }
   return mainaddr;
} 

#endif

bool PCProcess::postRTLoadCleanup() {
    if( rtLibLoadHeap_ ) {
        if( !pcProc_->freeMemory(rtLibLoadHeap_) ) {
            startup_printf("%s[%d]: failed to free memory used for RT library load\n",
                    FILE__, __LINE__);
            return false;
        }
        rtLibLoadHeap_ = 0;
    }
    return true;
}

AstNodePtr PCProcess::createLoadRTAST() {
    pdvector<int_function *> dlopen_funcs;

    // allow user to override default dlopen func names
    // with env. var

    DL_OPEN_FUNC_USER = getenv("DYNINST_DLOPEN_FUNC");

    if( DL_OPEN_FUNC_USER ) {
        findFuncsByAll(DL_OPEN_FUNC_USER, dlopen_funcs);
    }

    bool useHiddenFunction = false;
    if( dlopen_funcs.size() == 0 ) {
        if( !findFuncsByAll(DL_OPEN_FUNC_EXPORTED, dlopen_funcs) ) {
            useHiddenFunction = true;
            if( !findFuncsByAll(DL_OPEN_FUNC_NAME, dlopen_funcs) ) {
                pdvector<int_function *> dlopen_int_funcs;
                // If we can't find the do_dlopen function (because this library
                // is stripped, for example), try searching for the internal
                // _dl_open function and find the do_dlopen function by examining
                // the functions that call it. This depends on the do_dlopen
                // function having been parsed (though its name is not known)
                // through speculative parsing.
                if(findFuncsByAll(DL_OPEN_FUNC_INTERNAL, dlopen_int_funcs)) {
                    if(dlopen_int_funcs.size() > 1) {
                        startup_printf("%s[%d] warning: found %d matches for %s\n",
                                       __FILE__,__LINE__,dlopen_int_funcs.size(),
                                       DL_OPEN_FUNC_INTERNAL);
                    }
                    dlopen_int_funcs[0]->getStaticCallers(dlopen_funcs);
                    if(dlopen_funcs.size() > 1) {
                        startup_printf("%s[%d] warning: found %d do_dlopen candidates\n",
                                       __FILE__,__LINE__,dlopen_funcs.size());
                    }

                    if(dlopen_funcs.size() > 0) {
                        // give it a name
                        dlopen_funcs[0]->addSymTabName("do_dlopen",true);
                    }
                }else{
                    startup_printf("%s[%d]: failed to find dlopen function to load RT lib\n",
                                   FILE__, __LINE__);
                    return AstNodePtr();
                }
            }
        }
    }

    assert( dlopen_funcs.size() != 0 );

    if (dlopen_funcs.size() > 1) {
        logLine("WARNING: More than one dlopen found, using the first\n");
    }

    int_function *dlopen_func = dlopen_funcs[0];

    if( !useHiddenFunction ) {
        // For now, we cannot use inferiorMalloc because that requires the RT library
        // Hopefully, we can transition inferiorMalloc to use ProcControlAPI for 
        // allocating memory
        rtLibLoadHeap_ = pcProc_->mallocMemory(dyninstRT_name.length());
        if( !rtLibLoadHeap_ ) {
            startup_printf("%s[%d]: failed to allocate memory for RT library load\n",
                    FILE__, __LINE__);
            return AstNodePtr();
        }

        if( !writeDataSpace((char *)rtLibLoadHeap_, dyninstRT_name.length(), dyninstRT_name.c_str()) ) {
            startup_printf("%s[%d]: failed to write RT lib name into mutatee\n",
                    FILE__, __LINE__);
            return AstNodePtr();
        }

        pdvector<AstNodePtr> args;
        args.push_back(AstNode::operandNode(AstNode::Constant, (void *)rtLibLoadHeap_));
        args.push_back(AstNode::operandNode(AstNode::Constant, (void *)DLOPEN_MODE));

        return AstNode::funcCallNode(dlopen_func, args);
    }
    pdvector<AstNodePtr> sequence;

    AstNodePtr unprotectStackAST = createUnprotectStackAST();
    if( unprotectStackAST == AstNodePtr() ) {
        startup_printf("%s[%d]: failed to generate unprotect stack AST\n",
                FILE__, __LINE__);
        return AstNodePtr();
    }

    sequence.push_back(unprotectStackAST);

    startup_printf("%s[%d]: Creating AST to call libc's internal dlopen\n", FILE__, __LINE__);
    struct libc_dlopen_args_32 {
        uint32_t namePtr;
        uint32_t mode;
        uint32_t linkMapPtr;
    };

    struct libc_dlopen_args_64 {
        uint64_t namePtr;
        uint32_t mode;
        uint64_t linkMapPtr;
    };

    // Construct the argument to the internal function
    struct libc_dlopen_args_32 args32;
    struct libc_dlopen_args_64 args64;

    unsigned argsSize = 0;
    void *argsPtr;
    if( getAddressWidth() == 4 ) {
        argsSize = sizeof(args32);
        argsPtr = &args32;
    }else{
        argsSize = sizeof(args64);
        argsPtr = &args64;
    }

    // Allocate memory for the arguments
    rtLibLoadHeap_ = pcProc_->mallocMemory(dyninstRT_name.length()+1 + argsSize);
    if( !rtLibLoadHeap_ ) {
        startup_printf("%s[%d]: failed to allocate memory for RT library load\n",
                FILE__, __LINE__);
        return AstNodePtr();
    }

    if( !writeDataSpace((char *)rtLibLoadHeap_, dyninstRT_name.length()+1, dyninstRT_name.c_str()) ) {
        startup_printf("%s[%d]: failed to write RT lib name into mutatee\n",
                FILE__, __LINE__);
        return AstNodePtr();
    }

    if( getAddressWidth() == 4 ) {
        args32.namePtr = (uint32_t)rtLibLoadHeap_;
        args32.mode = DLOPEN_MODE;
    }else{
        args64.namePtr = (uint64_t)rtLibLoadHeap_;
        args64.mode = DLOPEN_MODE;
    }

    Address argsAddr = rtLibLoadHeap_ + dyninstRT_name.length()+1;
    if( !writeDataSpace((char *) argsAddr, argsSize, argsPtr) ) {
        startup_printf("%s[%d]: failed to write arguments to libc dlopen\n",
                FILE__, __LINE__);
        return AstNodePtr();
    }

    pdvector<AstNodePtr> args;
    args.push_back(AstNode::operandNode(AstNode::Constant, (void *)argsAddr));

    sequence.push_back(AstNode::funcCallNode(dlopen_func, args));

    return AstNode::sequenceNode(sequence);
}

AstNodePtr PCProcess::createUnprotectStackAST() {
    startup_printf("%s[%d]: creating AST to call mprotect to unprotect libc stack protection variable\n",
            FILE__, __LINE__);

    // find variable __stack_prot

    // mprotect READ/WRITE __stack_prot
    pdvector<int_variable *> vars;
    pdvector<int_function *> funcs;

    Address var_addr;
    int size;
    int pagesize;
    Address page_start;
    bool ret;

    ret = findVarsByAll("__stack_prot", vars);

    if(!ret || vars.size() == 0) {
        return AstNodePtr();
    } else if(vars.size() > 1) {
        startup_printf("%s[%d]: Warning: found more than one __stack_prot variable\n",
                FILE__, __LINE__);
    }

    pagesize = getpagesize();

    var_addr = vars[0]->getAddress();
    page_start = var_addr & ~(pagesize -1);
    size = var_addr - page_start +sizeof(int);

    ret = findFuncsByAll("mprotect",funcs);

    if(!ret || funcs.size() == 0) {
        startup_printf("%s[%d]: Couldn't find mprotect\n",
                FILE__, __LINE__);
        return AstNodePtr();
    }

    // mprotect: int mprotect(const void *addr, size_t len, int prot);
    int_function *mprot = funcs[0];
    
    pdvector<AstNodePtr> args;
    args.push_back(AstNode::operandNode(AstNode::Constant, (void *)page_start));
    args.push_back(AstNode::operandNode(AstNode::Constant, (void *)size));
    // prot = READ|WRITE|EXECUTE
    args.push_back(AstNode::operandNode(AstNode::Constant, (void *)7));

    return AstNode::funcCallNode(mprot, args);
}

// For now, this isn't defined
#if 0 
/* Find libc and add it as a shared object
 * Search for __libc_start_main
 * Save old code at beginning of __libc_start_main
 * Insert trap
 * Signal thread to continue
 */
bool PCProcess::instrumentLibcStartMain() 
{
    unsigned int maps_size =0;
    map_entries *maps = getVMMaps(getPid(), maps_size);
    unsigned int libcIdx=0;
    while (libcIdx < maps_size &&
           ! (strstr(maps[libcIdx].path,"/libc")
              && strstr(maps[libcIdx].path,".so"))) {
       libcIdx++;
    }
    assert(libcIdx != maps_size);
    //KEVINTODO: address code and data are not always 0,0: need to fix this
    fileDescriptor libcFD = fileDescriptor(maps[libcIdx].path,0,0,true);
    mapped_object *libc = mapped_object::createMappedObject(libcFD, this);
    addASharedObject(libc);

    // find __libc_startmain
    const pdvector<int_function*> *funcs;
    funcs = libc->findFuncVectorByPretty("__libc_start_main");
    if(funcs->size() == 0 || (*funcs)[0] == NULL) {
        logLine( "Couldn't find __libc_start_main\n");
        return false;
    } else if (funcs->size() > 1) {
       startup_printf("[%s:%u] - Found %d functions called __libc_start_main, weird\n",
                      FILE__, __LINE__, funcs->size());
    }
    if (!(*funcs)[0]->isInstrumentable()) {
        logLine( "__libc_start_main is not instrumentable\n");
        return false;
    }
    Address addr = (*funcs)[0]->getAddress();
    startup_printf("%s[%d]: Instrumenting libc.so:__libc_start_main() at 0x%x\n", 
                   FILE__, __LINE__, (int)addr);

    // save original code at beginning of function
    if (!readDataSpace((void *)addr, sizeof(savedCodeBuffer),savedCodeBuffer,true)) {
        fprintf(stderr, "%s[%d]:  readDataSpace\n", __FILE__, __LINE__);
        fprintf(stderr, "%s[%d][%s]:  failing instrumentLibcStartMain\n",
            __FILE__, __LINE__, getThreadStr(getExecThreadID()));
      fprintf(stderr, "Failed to read at address 0x%lx\n", addr);
      return false;
   }
   startup_printf("%s[%d]: Saved %d bytes from entry of __libc_start_main\n", 
         FILE__, __LINE__, sizeof(savedCodeBuffer));
   // insert trap
   codeGen gen(1);
   insnCodeGen::generateTrap(gen);
   if (!writeDataSpace((void *)addr, gen.used(), gen.start_ptr())) {
      fprintf(stderr, "%s[%d][%s]:  failing instrumentLibcStartMain\n",
            __FILE__, __LINE__, getThreadStr(getExecThreadID()));
      return false;
   }
   libcstartmain_brk_addr = addr;
   // TODO continueProc(); // signal process to continue
   return true;
}// end instrumentLibcStartMain
#endif

extern bool isFramePush(instruction &i);

/**
 * Signal handler return points can be found in the vsyscall page.
 * this function reads the symbol information that describes these
 * points out of the vsyscall page.
 **/
#define VSYS_SIGRETURN_NAME "_sigreturn"
static void getVSyscallSignalSyms(char *buffer, unsigned dso_size, PCProcess *p)
{
   Elf_Scn *sec;
   Elf32_Shdr *shdr;
   Elf32_Ehdr *ehdr;
   Elf32_Sym *syms;
   unsigned i;
   size_t dynstr = 0, shstr;
   
   Elf *elf = elf_memory(buffer, dso_size);
   if (elf == NULL)
      goto err_handler;
   ehdr = elf32_getehdr(elf);
   if (ehdr == NULL)
      goto err_handler;
   
   //Get string section indexes
   shstr = ehdr->e_shstrndx;
   for (i = 0; i < ehdr->e_shnum; i++)
   {
      shdr = elf32_getshdr(elf_getscn(elf, i));
      if (shdr != NULL && shdr->sh_type == SHT_STRTAB && 	
          strcmp(elf_strptr(elf, shstr, shdr->sh_name), ".dynstr") == 0)
      {
         dynstr = i;
         break;
      }
   }
   
   //For each section..
   for (sec = elf_nextscn(elf, NULL); sec != NULL; sec = elf_nextscn(elf, sec))
   {
      shdr = elf32_getshdr(sec);
      if (shdr == NULL) goto err_handler;
      if (!p->getVsyscallText() && (shdr->sh_flags & SHF_EXECINSTR)) {
         p->setVsyscallText(shdr->sh_addr);
      }
      if (shdr->sh_type == SHT_DYNSYM)
      {
         syms = (Elf32_Sym *) elf_getdata(sec, NULL)->d_buf;
         //For each symbol ..
         for (i = 0; i < (shdr->sh_size / sizeof(Elf32_Sym)); i++)
         {
            if ((syms[i].st_info & 0xf) == STT_FUNC)
            {
               //Check if this is a function symbol
               char *name = elf_strptr(elf, dynstr, syms[i].st_name);
               if (strstr(name, VSYS_SIGRETURN_NAME) != NULL)
               {	    
                  // Aggravating... FC3 has these as offsets from the entry
                  // of the vsyscall page. Others have these as absolutes.
                  // We hates them, my precioussss....
                  Address signal_addr = syms[i].st_value;
                  if (signal_addr < p->getVsyscallEnd() - p->getVsyscallStart()) {
                     p->addSignalHandler(syms[i].st_value + 
                                         p->getVsyscallStart(), 4);
                  }
                  else if (signal_addr < p->getVsyscallStart() ||
                           signal_addr >= p->getVsyscallEnd()) {
                     //FC-9 moved the vsyscall page, but didn't update its debug
                     // info or symbols from some hardcoded values.  Fix up the
                     // bad signal values.
                     p->addSignalHandler(syms[i].st_value - 0xffffe000 +
                                         p->getVsyscallStart(), 4);
                  }
                  else 
                     p->addSignalHandler(syms[i].st_value, 4);
               } 
            }
         }
      }
   }
   
   elf_end(elf);
   return;

err_handler:
   if (elf != NULL)
      elf_end(elf);
}

static volatile int segfaulted = 0;
static void catchSigSegV(int) {
   segfaulted = 1;
}

static char *execVsyscallFetch(PCProcess *p, char *buffer) {
   //We can't read the Vsyscall page out of the process addr
   // space do to a kernel bug.  However, the versions of the
   // kernel with this bug don't move the vsyscall page between
   // processes, so let's read it out of the mutator's.
   //Latter versions of the kernel do move the page between
   // processes, which makes our read attempt seg fault.  Let's
   // install a handler to be safe.
   volatile char *start;
   unsigned size;
   sighandler_t old_segv;

   segfaulted = 0;
   start = (char *) p->getVsyscallStart();
   size = p->getVsyscallEnd() - p->getVsyscallStart();

   //Install SegSegV handler.
   old_segv = signal(SIGSEGV, catchSigSegV);

   //Copy buffer
   for (unsigned i=0; i<size; i++) {
      buffer[i] = start[i];
      if (segfaulted) 
         break;
   }

   //Restore handler.
   signal(SIGSEGV, old_segv);
   if (segfaulted)
      return NULL;
   return buffer;
}

static bool isVsyscallData(char *buffer, int dso_size) {
   //Start with an elf header?
   if (dso_size < 4)
      return false;
   return (buffer[0] == 0x7f && buffer[1] == 'E' && buffer[2] == 'L' &&
           buffer[3] == 'F');
   
}

void calcVSyscallFrame(PCProcess *p)
{
  unsigned dso_size;
  char *buffer;

  /**
   * If we've already calculated and cached the DSO information then 
   * just return.
   **/
  if (p->getVsyscallObject())
     return;
  

  if (p->getAddressWidth() == 8) {
     // FIXME: HACK to disable vsyscall page for AMD64, for now.
     //  Reading the VSyscall data on ginger seems to trigger a
     //  kernel panic.
     p->setVsyscallRange(0x1000, 0x0);
     return;
  }

  /**
   * Read the location of the vsyscall page from /proc/.
   **/
  p->readAuxvInfo();
  if (p->getVsyscallStatus() != vsys_found) {
     p->setVsyscallRange(0x0, 0x0);
     return;
  }
  
  /**
   * Read the vsyscall page out of process memory.
   **/
  dso_size = p->getVsyscallEnd() - p->getVsyscallStart();
  buffer = (char *) calloc(1, dso_size);
  assert(buffer);
  if (!p->readDataSpace((caddr_t)p->getVsyscallStart(), dso_size, buffer,false))     
  {
     int major, minor, sub;
     get_linux_version(major, minor, sub);
     if (major == 2 && minor == 6 && sub <= 2 && sub >= 0) {
        //Linux 2.6.0 - Linux 2.6.2 has a  bug where ptrace 
        // can't read from the DSO.  The process can read the memory, 
        // it's just ptrace that's acting stubborn.
        if (!execVsyscallFetch(p, buffer))
        {
           p->setVsyscallStatus(vsys_notfound);
           return;
        }
     }
  }

  if (!isVsyscallData(buffer, dso_size)) {
     p->setVsyscallRange(0x0, 0x0);
     p->setVsyscallStatus(vsys_notfound);
     return;     
  }
  getVSyscallSignalSyms(buffer, dso_size, p);

  Symtab *obj;
  bool result = Symtab::openFile(obj, buffer, dso_size);
  if (result)
     p->setVsyscallObject(obj);
  return;
}

bool Frame::setPC(Address newpc) {

   Address pcAddr = getPClocation();
   if (!pcAddr)
   {
       //fprintf(stderr, "[%s:%u] - Frame::setPC aborted", __FILE__, __LINE__);
      return false;
   }

   //fprintf(stderr, "[%s:%u] - Frame::setPC setting %x to %x",
   //__FILE__, __LINE__, pcAddr_, newpc);
   if (!getProc()->writeDataSpace((void*)pcAddr, sizeof(Address), &newpc))
      return false;
   sw_frame_.setRA(newpc);
   range_ = NULL;

   return true;
}

void print_read_error_info(const relocationEntry entry, 
      int_function *&target_pdf, Address base_addr) {

    sprintf(errorLine, "  entry      : target_addr 0x%x\n",
	    (unsigned)entry.target_addr());
    logLine(errorLine);
    sprintf(errorLine, "               rel_addr 0x%x\n", (unsigned)entry.rel_addr());
    logLine(errorLine);
    sprintf(errorLine, "               name %s\n", (entry.name()).c_str());
    logLine(errorLine);

    if (target_pdf) {
      sprintf(errorLine, "  target_pdf : symTabName %s\n",
	      (target_pdf->symTabName()).c_str());
      logLine(errorLine);    
      sprintf(errorLine , "              prettyName %s\n",
	      (target_pdf->symTabName()).c_str());
      logLine(errorLine);
      /*
      // Size bad. <smack>
      sprintf(errorLine , "              size %i\n",
      target_pdf->getSize());
      logLine(errorLine);
      */
      sprintf(errorLine , "              addr 0x%x\n",
	      (unsigned)target_pdf->getAddress());
      logLine(errorLine);
    }
    sprintf(errorLine, "  base_addr  0x%x\n", (unsigned)base_addr);
    logLine(errorLine);
}

// hasBeenBound: returns true if the runtime linker has bound the
// function symbol corresponding to the relocation entry in at the address
// specified by entry and base_addr.  If it has been bound, then the callee 
// function is returned in "target_pdf", else it returns false.
bool PCProcess::hasBeenBound(const relocationEntry &entry, 
			   int_function *&target_pdf, Address base_addr) {

    if (isTerminated()) return false;

    // if the relocationEntry has not been bound yet, then the value
    // at rel_addr is the address of the instruction immediately following
    // the first instruction in the PLT entry (which is at the target_addr) 
    // The PLT entries are never modified, instead they use an indirrect 
    // jump to an address stored in the _GLOBAL_OFFSET_TABLE_.  When the 
    // function symbol is bound by the runtime linker, it changes the address
    // in the _GLOBAL_OFFSET_TABLE_ corresponding to the PLT entry

    Address got_entry = entry.rel_addr() + base_addr;
    Address bound_addr = 0;
    if(!readDataSpace((const void*)got_entry, sizeof(Address), 
			&bound_addr, true)){
        sprintf(errorLine, "read error in process::hasBeenBound addr 0x%x, pid=%d\n (readDataSpace returns 0)",(unsigned)got_entry,getPid());
	logLine(errorLine);
	print_read_error_info(entry, target_pdf, base_addr);
        return false;
    }

    if( !( bound_addr == (entry.target_addr()+6+base_addr)) ) {
        // the callee function has been bound by the runtime linker
	// find the function and return it
        target_pdf = findFuncByAddr(bound_addr);
	if(!target_pdf){
            return false;
	}
        return true;	
    }
    return false;
}

bool AddressSpace::getDyninstRTLibName() {
   startup_printf("dyninstRT_name: %s\n", dyninstRT_name.c_str());
    if (dyninstRT_name.length() == 0) {
        // Get env variable
        if (getenv("DYNINSTAPI_RT_LIB") != NULL) {
            dyninstRT_name = getenv("DYNINSTAPI_RT_LIB");
        }
        else {
           std::string msg;
           PCProcess *proc;
           if ((proc = dynamic_cast<PCProcess *>(this)) != NULL) {
              msg = std::string("Environment variable ") +
                 std::string("DYNINSTAPI_RT_LIB") +
                 std::string(" has not been defined for process ") +
                 utos(proc->getPid());
           }
           else {
              msg = std::string("Environment variable ") +
                 std::string("DYNINSTAPI_RT_LIB") +
                 std::string(" has not been defined");
           }           
           showErrorCallback(101, msg);
           return false;
        }
    }

    // Automatically choose 32-bit library if necessary.
    const char *modifier = "_m32";
    const char *name = dyninstRT_name.c_str();

    const char *split = P_strrchr(name, '/');
    if ( !split ) split = name;
    split = P_strchr(split, '.');
    if ( !split || P_strlen(split) <= 1 ) {
        // We should probably print some error here.
        // Then, of course, the user will find out soon enough.
        startup_printf("Invalid Dyninst RT lib name: %s\n", 
                dyninstRT_name.c_str());
        return false;
    }

    if ( getAddressWidth() == sizeof(void *) || P_strstr(name, modifier) ) {
        modifier = "";
    }

    const char *suffix = split;
    if( getAOut()->isStaticExec() ) {
        suffix = ".a";
    }else{
        if( P_strncmp(suffix, ".a", 2) == 0 ) {
	  // Add symlinks in makefiles as follows:
	  // (lib).so => lib.so.(major)
	  // lib.so.major => lib.so.major.minor
	  // lib.so.major.minor => lib.so.major.minor.maintenance
	  suffix = ".so";
        }
    }

    dyninstRT_name = std::string(name, split - name) +
                     std::string(modifier) +
                     std::string(suffix);

    startup_printf("Dyninst RT Library name set to '%s'\n",
            dyninstRT_name.c_str());

    // Check to see if the library given exists.
    if (access(dyninstRT_name.c_str(), R_OK)) {
        std::string msg = std::string("Runtime library ") + dyninstRT_name
        + std::string(" does not exist or cannot be accessed!");
        showErrorCallback(101, msg);
        return false;
    }

    return true;
}
