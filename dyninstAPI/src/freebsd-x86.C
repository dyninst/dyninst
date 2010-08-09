/*
 * Copyright (c) 1996-2010 Barton P. Miller
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

#include <cstdio>

#include "debug.h"
#include "addressSpace.h"
#include "process.h"
#include "emit-x86.h"
#include "inst-x86.h"
#include "binaryEdit.h"
#include "image-func.h"

bool AddressSpace::getDyninstRTLibName() {
   startup_printf("dyninstRT_name: %s\n", dyninstRT_name.c_str());
    if (dyninstRT_name.length() == 0) {
        // Get env variable
        if (getenv("DYNINSTAPI_RT_LIB") != NULL) {
            dyninstRT_name = getenv("DYNINSTAPI_RT_LIB");
        }
        else {
           std::string msg;
           process *proc;
           if ((proc = dynamic_cast<process *>(this)) != NULL) {
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

#if defined(arch_x86_64)
void print_regs(dyn_lwp *lwp)
{
    // XXX this is a debugging function
}
#endif

bool image::findGlobalConstructorFunc(const std::string &ctorHandler) {
    using namespace Dyninst::InstructionAPI;
    using namespace Dyninst::SymtabAPI;

    vector<Function *> funcs;
    if( linkedFile->findFunctionsByName(funcs, ctorHandler) ) {
        return true;
    }

    /* If the symbol isn't found, try looking for it in a call instruction in
     * the .init section
     *
     * The instruction sequence is:
     * ...
     * some instructions
     * ...
     * call frame_dummy
     * call ctor_handler
     */
    Region *initRegion = NULL;
    if( !linkedFile->findRegion(initRegion, ".init") ) {
        if( linkedFile->findFunctionsByName(funcs, "_init") ) {
            initRegion = funcs[0]->getRegion();
        }else{
            logLine("failed to locate .init Region or _init function\n");
            return false;
        }
    }

    if( initRegion == NULL ) {
        logLine("failed to locate .init Region or _init function\n");
        return false;
    }

    /* 
     * If the function associated with the .init Region doesn't exist, it needs to
     * be created
     */
    if( !findFuncByEntry(initRegion->getRegionAddr()) ) {
        image_func *initStub = addFunction(initRegion->getRegionAddr(), "_init");
        if( initStub == NULL ) {
            logLine("unable to create function for .init \n");
            return false;
        }else{
            inst_printf("%s[%d]: set _init function address to 0x%lx\n", FILE__, __LINE__,
                initRegion->getRegionAddr());
        }
    }

    // Search for last of 2 calls
    Address ctorAddress = 0;
    unsigned bytesSeen = 0;
    unsigned numCalls = 0;
    const unsigned char *p = reinterpret_cast<const unsigned char *>(initRegion->getPtrToRawData());

    InstructionDecoder decoder(p, initRegion->getRegionSize(), 
            codeObject()->cs()->getArch());

    Instruction::Ptr curInsn = decoder.decode();
    while(numCalls < 2 && curInsn && curInsn->isValid() &&
          bytesSeen < initRegion->getRegionSize()) 
    {
        InsnCategory category = curInsn->getCategory();
        if( category == c_CallInsn ) {
            numCalls++;
        }
        if( numCalls < 2 ) {
            bytesSeen += curInsn->size();
            curInsn = decoder.decode();
        }
    }

    if( numCalls != 2 ) {
        logLine("heuristic for finding global constructor function failed\n");
        return false;
    }

    Address callAddress = initRegion->getRegionAddr() + bytesSeen;

    RegisterAST thePC = RegisterAST(Dyninst::MachRegister::getPC(
                codeObject()->cs()->getArch()));

    Expression::Ptr callTarget = curInsn->getControlFlowTarget();
    if( !callTarget.get() ) {
        logLine("failed to find global constructor function\n");
        return false;
    }
    callTarget->bind(&thePC, Result(s64, callAddress));
    //callTarget->bind(&rip, Result(s64, callAddress));

    Result actualTarget = callTarget->eval();
    if( actualTarget.defined ) {
        ctorAddress = actualTarget.convert<Address>();
    }else{
        logLine("failed to find global constructor function\n");
        return false;
    }

    if( !ctorAddress || !codeObject()->cs()->isValidAddress(ctorAddress) ) {
        logLine("invalid address for global constructor function\n");
        return false;
    }

    if( addFunction(ctorAddress, ctorHandler.c_str()) == NULL ) {
        logLine("unable to create representation for global constructor function\n");
        return false;
    }else{
        inst_printf("%s[%d]: set global constructor address to 0x%lx\n", FILE__, __LINE__,
                ctorAddress);
    }

    return true;
}

bool image::findGlobalDestructorFunc(const std::string &dtorHandler) {
    using namespace Dyninst::InstructionAPI;
    using namespace Dyninst::SymtabAPI;

    vector<Function *> funcs;
    if( linkedFile->findFunctionsByName(funcs, dtorHandler) ) {
        return true;
    }

    /*
     * If the symbol isn't found, try looking for it in a call in the
     * .fini section. It is the last call in .fini.
     *
     * The pattern is:
     *
     * _fini:
     *
     * ... some code ...
     *
     * call dtor_handler
     *
     * ... prologue ...
     */
    Region *finiRegion = NULL;
    if( !linkedFile->findRegion(finiRegion, ".fini") ) {
        if( linkedFile->findFunctionsByName(funcs, "_fini") ) {
            finiRegion = funcs[0]->getRegion();
        }else{
            logLine("failed to locate .fini Region or _fini function\n");
            return false;
        }
    }

    if( finiRegion == NULL ) {
        logLine("failed to locate .fini Region or _fini function\n");
        return false;
    }

    /* 
     * If the function associated with the .fini Region doesn't exist, it needs to
     * be created
     */
    if( !findFuncByEntry(finiRegion->getRegionAddr()) ) {
        image_func *finiStub = addFunction(finiRegion->getRegionAddr(), "_fini");
        if( finiStub == NULL ) {
            logLine("unable to create function for .fini \n");
            return false;
        }else{
            inst_printf("%s[%d]: set _fini function address to 0x%lx\n", FILE__, __LINE__,
                finiRegion->getRegionAddr());
        }
    }

    // Search for last call in the function
    Address dtorAddress = 0;
    unsigned bytesSeen = 0;
    const unsigned char *p = reinterpret_cast<const unsigned char *>(finiRegion->getPtrToRawData());

    InstructionDecoder decoder(p, finiRegion->getRegionSize(), 
            codeObject()->cs()->getArch());

    Instruction::Ptr lastCall;
    Instruction::Ptr curInsn = decoder.decode();

    while(curInsn && curInsn->isValid() &&
          bytesSeen < finiRegion->getRegionSize()) 
    {
        InsnCategory category = curInsn->getCategory();
        if( category == c_CallInsn ) {
            lastCall = curInsn;
            break;
        }
            bytesSeen += curInsn->size();
            curInsn = decoder.decode();
    }

    if( !lastCall.get() || !lastCall->isValid() ) {
        logLine("heuristic for finding global destructor function failed\n");
        return false;
    }

    Address callAddress = finiRegion->getRegionAddr() + bytesSeen;

    RegisterAST thePC = RegisterAST(Dyninst::MachRegister::getPC(
                codeObject()->cs()->getArch()));

    Expression::Ptr callTarget = lastCall->getControlFlowTarget();
    if( !callTarget.get() ) {
        logLine("failed to find global destructor function\n");
        return false;
    }
    callTarget->bind(&thePC, Result(s64, callAddress));
    //callTarget->bind(&rip, Result(s64, callAddress));

    Result actualTarget = callTarget->eval();
    if( actualTarget.defined ) {
        dtorAddress = actualTarget.convert<Address>();
    }else{
        logLine("failed to find global destructor function\n");
        return false;
    }

    if( !dtorAddress || !codeObject()->cs()->isValidAddress(dtorAddress) ) {
        logLine("invalid address for global destructor function\n");
        return false;
    }

    if( addFunction(dtorAddress, dtorHandler.c_str()) == NULL ) {
        logLine("unable to create representation for global destructor function\n");
        return false;
    }else{
        inst_printf("%s[%d]: set global destructor address to 0x%lx\n", FILE__, __LINE__,
                dtorAddress);
    }

    return true;
}
