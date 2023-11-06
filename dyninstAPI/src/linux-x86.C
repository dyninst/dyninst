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

// $Id: linux-x86.C,v 1.142 2008/08/01 17:55:12 roundy Exp $

#include <fstream>
#include <string>

#include <fcntl.h>
#include <sys/user.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <math.h> // for floor()
#include <unistd.h> // for sysconf()
//#include <elf.h>
//#include <libelf.h>

#include "dyninstAPI/src/baseTramp.h"
#include "dyninstAPI/src/image.h"
#include "dyninstAPI/src/function.h"
#include "dyninstAPI/src/instPoint.h"
#include "common/src/headers.h"
#include "dyninstAPI/src/os.h"
#include "common/src/stats.h"
#include "dyninstAPI/src/debug.h"
#include "dyninstAPI/src/util.h" // getCurrWallTime
#include "common/src/pathName.h"
#include "dyninstAPI/src/inst-x86.h"
#include "dyninstAPI/src/emit-x86.h"
#include "registers/x86_regs.h"
#include "registers/x86_64_regs.h"
#include "dyninstAPI/src/mapped_object.h" 

#include "dyninstAPI/src/linux.h"

#include "dyninstAPI/src/registerSpace.h"

#include <sstream>

#include "dyninstAPI/src/ast.h"
#include "dyninstAPI/src/binaryEdit.h"
#include "dyninstAPI/src/dynThread.h"
#include "dyninstAPI/src/dynProcess.h"
#include "common/src/linuxKludges.h"

#include "instructionAPI/h/InstructionDecoder.h"
#include "instructionAPI/h/Instruction.h"

using namespace Dyninst;
using namespace Dyninst::SymtabAPI;
using namespace Dyninst::ProcControlAPI;

Address PCProcess::getLibcStartMainParam(PCThread *trappingThread) {
   Address mainaddr = 0;
   unsigned addrWidth = getAddressWidth();

   RegisterPool regs;
   trappingThread->getRegisters(regs);

   Address targetAddr = 0;
   if( getArch() == Arch_x86_64 ) {
       if( addrWidth == 4 ) {
           targetAddr = regs[x86_64::rsp] + addrWidth;
       }else{
           targetAddr = regs[x86_64::rdi];
       }
   }else{
       targetAddr = regs[x86::esp] + addrWidth;
   }

   if( !readDataSpace((const void *)targetAddr, addrWidth, (void *)&mainaddr, false) ) {
       proccontrol_printf("%s[%d]: failed to read address of main out of libc\n",
               FILE__, __LINE__);
   }

   return mainaddr;
}

Address PCProcess::getTOCoffsetInfo(Address) {
    assert(!"This function is unimplemented");
    return 0;
}

Address PCProcess::getTOCoffsetInfo(func_instance *) {
    assert(!"This function is unimplemented");
    return 0;
}

bool PCProcess::getOPDFunctionAddr(Address &) {
    return true;
}

AstNodePtr PCProcess::createUnprotectStackAST() {
    // Since we are punching our way down to an internal function, we
    // may run into problems due to stack execute protection. Basically,
    // glibc knows that it needs to be able to execute on the stack in
    // in order to load libraries with dl_open(). It has code in
    // _dl_map_object_from_fd (the workhorse of dynamic library loading)
    // that unprotects a global, exported variable (__stack_prot), sets
    // the execute flag, and reprotects it. This only happens, however,
    // when the higher-level dl_open() functions (which we skip) are called,
    // as they append an undocumented flag to the library open mode. Otherwise,
    // assignment to the variable happens without protection, which will
    // cause a fault.
    //
    // Instead of chasing the value of the undocumented flag, we will
    // unprotect the __stack_prot variable ourselves (if we can find it).

    startup_printf("%s[%d]: creating AST to call mprotect to unprotect libc stack protection variable\n",
            FILE__, __LINE__);

    // find variable __stack_prot

    // mprotect READ/WRITE __stack_prot
    std::vector<int_variable *> vars;
    std::vector<func_instance *> funcs;

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
    func_instance *mprot = funcs[0];
    
    std::vector<AstNodePtr> args;
    args.push_back(AstNode::operandNode(AstNode::operandType::Constant, (void *)page_start));
    args.push_back(AstNode::operandNode(AstNode::operandType::Constant, (void *)(intptr_t)size));
    // prot = READ|WRITE|EXECUTE
    args.push_back(AstNode::operandNode(AstNode::operandType::Constant, (void *)7));

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
    //TODO: address code and data are not always 0,0: need to fix this
    fileDescriptor libcFD = fileDescriptor(maps[libcIdx].path,0,0,true);
    mapped_object *libc = mapped_object::createMappedObject(libcFD, this);
    addASharedObject(libc);

    // find __libc_startmain
    const std::vector<func_instance*> *funcs;
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
    Address addr = (*funcs)[0]->addr();
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


bool PCProcess::bindPLTEntry(const SymtabAPI::relocationEntry &entry, Address base_addr, 
                           func_instance *, Address target_addr) {
   // We just want to overwrite the GOT entry with our target address. 
   Address got_entry = entry.rel_addr() + base_addr;
   return writeDataSpace((void *)got_entry, sizeof(Address), &target_addr);
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
    const char *modifier = "";
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

    if (getAddressWidth() == 4 &&
        (sizeof(void *) == 8)) {
       // Need _m32...
       if (P_strstr(name, "_m32") == NULL) {
          modifier = "_m32";
       }
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
