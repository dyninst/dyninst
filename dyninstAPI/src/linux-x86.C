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

#define DLOPEN_MODE (RTLD_NOW | RTLD_GLOBAL)

const char *DL_OPEN_FUNC_USER = NULL;
const char DL_OPEN_FUNC_EXPORTED[] = "dlopen";
const char DL_OPEN_FUNC_NAME[] = "do_dlopen";
const char DL_OPEN_FUNC_INTERNAL[] = "_dl_open";

const char libc_version_symname[] = "__libc_version";

#define NUM_REGS (17 /*+ NUM_FREGS*/)
#define NUM_FREGS 8
#define FP0_REGNUM NUM_REGS
#define FP7_REGNUM (FP0_REGNUM+7)
#define INTREGSIZE (sizeof(long))
#define FPREGSIZE 10
#define MAX_REGISTER_RAW_SIZE 10

#define REGISTER_RAW_SIZE(N) (((N) < FP0_REGNUM) ? INTREGSIZE : FPREGSIZE)
#define REGS_SIZE ( NUM_REGS * REGISTER_RAW_SIZE(0) + NUM_FREGS * REGISTER_RAW_SIZE(FP0_REGNUM) )
#define REGS_INTS ( REGS_SIZE / INTREGSIZE )

const int GENREGS_STRUCT_SIZE = sizeof( user_regs_struct );
#ifdef _SYS_USER_H 
const int FPREGS_STRUCT_SIZE = sizeof( user_fpregs_struct );
#else
const int FPREGS_STRUCT_SIZE = sizeof( user_i387_struct );
#endif

#define P_offsetof(s, m) (Address) &(((s *) NULL)->m)

/* ********************************************************************** */

Frame PCThread::getActiveFrame() {
    Address pc = 0, fp = 0, sp = 0;
    //TODO get register values
    return Frame(pc, fp, sp, proc_->getPid(), proc_, this, true);
}

#if defined(arch_x86_64)
bool PCProcess::getSysCallParameters(dyn_saved_regs *regs, long *params, int numparams) 
{
   if (getAddressWidth() == 4) { // 32 bit mutatee
   } else { // 64 bit mutatee, have to use ifdef, otherwise it won't
      // compile on a 32 bit machine
      if (numparams > 0) {
         params[0] = regs->gprs.rdi;
      }
      if (numparams > 1) {
         params[1] = regs->gprs.rsi;
      }
      if (numparams > 2) {
         params[2] = regs->gprs.rdx;
      }
      if (numparams > 3) {
         params[3] = regs->gprs.r8;
      }
      if (numparams > 4) {
         params[4] = regs->gprs.r9;
      }
      if (numparams > 5) {
         params[5] = regs->gprs.r10;
      }
      for (int i=6; i < numparams; i++) {
         if (!readDataSpace((void*)regs->gprs.rsp, getAddressWidth(), 
                  (void*)(params + i * getAddressWidth()), true)) {
            return false;
         }
      }
   }
   return true;
}

int PCProcess::getSysCallNumber(dyn_saved_regs *regs) {
   return regs->gprs.orig_rax;
}

long PCProcess::getSysCallReturnValue(dyn_saved_regs *regs) {
   return regs->gprs.rax;
}

Address PCProcess::getSysCallProgramCounter(dyn_saved_regs *regs) {
   return regs->gprs.rip;
}

bool PCProcess::isMmapSysCall(int callnum) {
   if (getAddressWidth() == 4) {
      startup_printf("CALLNUM=%d\n",callnum);
   }
   return callnum == SYS_mmap;
}

Offset PCProcess::getMmapLength(int, dyn_saved_regs *regs) {
   return (Offset) regs->gprs.rsi;
}

Address PCProcess::getLibcStartMainParam(PCThread *trappingThread) {
   Address mainaddr = 0;
   dyn_saved_regs regs;
   trappingThread->getRegisters(&regs);
   if (getAddressWidth() == 4) { // 32 bit mutatee
      if (!readDataSpace((void*)(regs.gprs.rsp + getAddressWidth()),
               getAddressWidth(), (void*)&mainaddr,true)) {
         fprintf(stderr,"[%s][%d]: failed readDataSpace\n", __FILE__,__LINE__); 
      }
   } else { // 64 bit mutatee
      mainaddr = regs.gprs.rdi;
   }
   return mainaddr;
}
// 64 bit architecture
#else 
// 32 bit architecture
bool PCProcess::getSysCallParameters(dyn_saved_regs *regs, 
      long *params, int numparams) {
   if (numparams > 0) {
      params[0] = regs->gprs.ebx;
   }
   if (numparams > 1) {
      params[1] = regs->gprs.ecx;
   }
   if (numparams > 2) {
      params[2] = regs->gprs.edx;
   }
   if (numparams > 3) {
      params[3] = regs->gprs.esi;
   }
   if (numparams > 4) {
      params[4] = regs->gprs.edi;
   }
   if (numparams > 5) {
      params[5] = regs->gprs.ebp;
   }
   for (int i=6; i < numparams; i++) {
      if (!readDataSpace((void*)regs->gprs.esp, getAddressWidth(), 
               (void*)(params + i * getAddressWidth()), true)) {
         return false;
      }
   }
   return true;
}

int PCProcess::getSysCallNumber(dyn_saved_regs *regs) 
{
   return regs->gprs.orig_eax;
}

long PCProcess::getSysCallReturnValue(dyn_saved_regs *regs) 
{
   return regs->gprs.eax;
}

Address PCProcess::getSysCallProgramCounter(dyn_saved_regs *regs) 
{
   return regs->gprs.eip;
}

bool PCProcess::isMmapSysCall(int callnum) {
   return (callnum == SYS_mmap || callnum == SYS_mmap2);
}

Offset PCProcess::getMmapLength(int callnum, dyn_saved_regs *regs) 
{
   if (callnum == SYS_mmap) {
      Offset length;
      readDataSpace((void*)(regs->gprs.ebx + getAddressWidth()),
            getAddressWidth(), (void*)&length, true);
      return length;
   }
   else {
      return (Offset) regs->gprs.ecx;
   }
}

Address PCProcess::getLibcStartMainParam(PCThread *trappingThread) 
{
   dyn_saved_regs regs;
   trappingThread->getRegisters(&regs);
   Address mainaddr;
   if (!readDataSpace((void*)(regs.gprs.esp + getAddressWidth()),
            getAddressWidth(), (void*)&mainaddr,true)) {
      fprintf(stderr,"[%s][%d]: failed readDataSpace\n", __FILE__,__LINE__); 
   }
   return mainaddr;
} 

#endif

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
   if (!pcAddr_)
   {
       //fprintf(stderr, "[%s:%u] - Frame::setPC aborted", __FILE__, __LINE__);
      return false;
   }

   //fprintf(stderr, "[%s:%u] - Frame::setPC setting %x to %x",
   //__FILE__, __LINE__, pcAddr_, newpc);
   getProc()->writeDataSpace((void*)pcAddr_, sizeof(Address), &newpc);
   pc_ = newpc;
   range_ = NULL;

   return false;
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

    if (hasExited()) return false;

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
            // This will be incorrect if the RT library's version changes
            suffix = ".so.1";
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

Frame PCProcess::preStackWalkInit(Frame startFrame) 
{
  /* Do a special check for the vsyscall page.  Silently drop
     the page if it exists. */
  calcVSyscallFrame( this );
  
  Address next_pc = startFrame.getPC();
  if ((next_pc >= getVsyscallStart() && next_pc < getVsyscallEnd()) ||
      /* RH9 Hack */ (next_pc >= 0xffffe000 && next_pc < 0xfffff000)) {
     return startFrame.getCallerFrame();
  }
  return startFrame;
}


