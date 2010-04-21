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

/*
 * XXX This was copied from Linux -- it needs to be verified
 */
int EmitterIA32::emitCallParams(codeGen &gen, 
                              const pdvector<AstNodePtr> &operands,
                              int_function */*target*/, 
                              pdvector<Register> &/*extra_saves*/, 
                              bool noCost)
{
    pdvector <Register> srcs;
    unsigned frame_size = 0;
    unsigned u;
    for (u = 0; u < operands.size(); u++) {
        Address unused = ADDR_NULL;
        Register reg = REG_NULL;
        if (!operands[u]->generateCode_phase2(gen,
                                              noCost,
                                              unused,
                                              reg)) assert(0); // ARGH....
        assert (reg != REG_NULL); // Give me a real return path!
        srcs.push_back(reg);
    }
    
    // push arguments in reverse order, last argument first
    // must use int instead of unsigned to avoid nasty underflow problem:
    for (int i=srcs.size() - 1; i >= 0; i--) {
       RealRegister r = gen.rs()->loadVirtual(srcs[i], gen);
       ::emitPush(r, gen);
       frame_size += 4;
       if (operands[i]->decRefCount())
          gen.rs()->freeRegister(srcs[i]);
    }
    return frame_size;
}

/*
 * XXX This was copied from Linux -- it needs to be verified.
 */
bool EmitterIA32::emitCallCleanup(codeGen &gen,
                                int_function * /*target*/, 
                                int frame_size, 
                                pdvector<Register> &/*extra_saves*/)
{
   if (frame_size)
      emitOpRegImm(0, RealRegister(REGNUM_ESP), frame_size, gen); // add esp, frame_size
   gen.rs()->incStack(-1 * frame_size);
   return true;
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

static const std::string LIBC_CTOR_HANDLER("__do_global_ctors_aux");
static const std::string LIBC_DTOR_HANDLER("__do_global_dtors_aux");
static const std::string DYNINST_CTOR_HANDLER("DYNINSTglobal_ctors_handler");
static const std::string DYNINST_CTOR_LIST("DYNINSTctors_addr");
static const std::string DYNINST_DTOR_HANDLER("DYNINSTglobal_dtors_handler");
static const std::string DYNINST_DTOR_LIST("DYNINSTdtors_addr");
static const std::string SYMTAB_CTOR_LIST_REL("__SYMTABAPI_CTOR_LIST__");
static const std::string SYMTAB_DTOR_LIST_REL("__SYMTABAPI_DTOR_LIST__");

bool BinaryEdit::doStaticBinarySpecialCases() {
    Symtab *origBinary = mobj->parse_img()->getObject();

    /* Special Case 1: Handling global constructor and destructor Regions
     *
     * Replace global ctors function with special ctors function,
     * and create a special relocation for the ctors list used by the special
     * ctors function
     *
     * Replace global dtors function with special dtors function,
     * and create a special relocation for the dtors list used by the special
     * dtors function
     */
    if( !mobj->parse_img()->findGlobalConstructorFunc() ) {
        return false;
    }

    if( !mobj->parse_img()->findGlobalDestructorFunc() ) {
        return false;
    }

    int_function *globalCtorHandler = findOnlyOneFunction(LIBC_CTOR_HANDLER);
    if( !globalCtorHandler ) {
        logLine("failed to find libc constructor handler\n");
        return false;
    }

    int_function *dyninstCtorHandler = findOnlyOneFunction(DYNINST_CTOR_HANDLER);
    if( !dyninstCtorHandler ) {
        logLine("failed to find Dyninst constructor handler\n");
        return false;
    }

    int_function *globalDtorHandler = findOnlyOneFunction(LIBC_DTOR_HANDLER);
    if( !globalDtorHandler ) {
        logLine("failed to find libc destructor handler\n");
        return false;
    }

    int_function *dyninstDtorHandler = findOnlyOneFunction(DYNINST_DTOR_HANDLER);
    if( !dyninstDtorHandler ) {
        logLine("failed to find Dyninst destructor handler\n");
        return false;
    }

    /* 
     * Replace all calls to the global ctor and dtor handlers with the special
     * handler 
     */
    pdvector<int_function *> allFuncs;
    getAllFunctions(allFuncs);

    bool success = false;
    for(unsigned i = 0; i < allFuncs.size(); ++i) {
        int_function *func = allFuncs[i];
        if( !func ) continue;

        const pdvector<instPoint *> &calls = func->funcCalls();

        for(unsigned j = 0; j < calls.size(); ++j) {
            instPoint *point = calls[j];
            if ( !point ) continue;

            Address target = point->callTarget();

            if( !target ) continue;

            if( target == globalCtorHandler->getAddress() ) {
                if( !replaceFunctionCall(point, dyninstCtorHandler) ) {
                    success = false;
                }else{
                    inst_printf("%s[%d]: replaced call to %s in %s with %s\n",
                            FILE__, __LINE__, LIBC_CTOR_HANDLER.c_str(),
                            func->prettyName().c_str(), DYNINST_CTOR_HANDLER.c_str());
                    success = true;
                }
            }else if( target == globalDtorHandler->getAddress() ) {
                if( !replaceFunctionCall(point, dyninstDtorHandler) ) {
                    success = false;
                }else{
                    inst_printf("%s[%d]: replaced call to %s in %s with %s\n",
                            FILE__, __LINE__, LIBC_CTOR_HANDLER.c_str(),
                            func->prettyName().c_str(), DYNINST_DTOR_HANDLER.c_str());
                    success = true;
                }
            }
        }
    }

    if( !success ) {
        logLine("failed to replace libc ctor or dtor handler with special handler\n");
        return false;
    }

    /* create the special relocation for the ctors and dtors list -- search the
     * RT library for the symbol 
     */
    int_symbol ctorsListInt;
    int_symbol dtorsListInt;
    bool ctorFound = false, dtorFound = false; 
    std::vector<BinaryEdit *>::iterator rtlib_it;
    for(rtlib_it = rtlib.begin(); rtlib_it != rtlib.end(); ++rtlib_it) {
        if( (*rtlib_it)->getSymbolInfo(DYNINST_CTOR_LIST, ctorsListInt) ) {
            ctorFound = true;
            if( dtorFound ) break;
        }

        if( (*rtlib_it)->getSymbolInfo(DYNINST_DTOR_LIST, dtorsListInt) ) {
            dtorFound = true;
            if( ctorFound ) break;
        }
    }

    if( !ctorFound ) {
         logLine("failed to find ctors list symbol\n");
         return false;
    }

    if( !dtorFound ) {
        logLine("failed to find dtors list symbol\n");
        return false;
    }

    Symbol *ctorsList = const_cast<Symbol *>(ctorsListInt.sym());
    
    std::vector<Region *> allRegions;
    if( !ctorsList->getSymtab()->getAllRegions(allRegions) ) {
        logLine("failed to find ctors list relocation\n");
        return false;
    }

    success = false;
    std::vector<Region *>::iterator reg_it;
    for(reg_it = allRegions.begin(); reg_it != allRegions.end(); ++reg_it) {
        std::vector<relocationEntry> &region_rels = (*reg_it)->getRelocations();
        vector<relocationEntry>::iterator rel_it;
        for( rel_it = region_rels.begin(); rel_it != region_rels.end(); ++rel_it) {
            if( rel_it->getDynSym() == ctorsList ) {
                relocationEntry *rel = &(*rel_it);
                rel->setName(SYMTAB_CTOR_LIST_REL);
                success = true;
            }
        }
    }

    if( !success ) {
        logLine("failed to change relocation for ctors list\n");
        return false;
    }

    Symbol *dtorsList = const_cast<Symbol *>(dtorsListInt.sym());
    
    if( !dtorsList->getSymtab()->getAllRegions(allRegions) ) {
        logLine("failed to find dtors list relocation\n");
        return false;
    }

    success = false;
    for(reg_it = allRegions.begin(); reg_it != allRegions.end(); ++reg_it) {
        std::vector<relocationEntry> &region_rels = (*reg_it)->getRelocations();
        vector<relocationEntry>::iterator rel_it;
        for( rel_it = region_rels.begin(); rel_it != region_rels.end(); ++rel_it) {
            if( rel_it->getDynSym() == dtorsList ) {
                relocationEntry *rel = &(*rel_it);
                rel->setName(SYMTAB_DTOR_LIST_REL);
                success = true;
            }
        }
    }

    if( !success ) {
        logLine("failed to change relocation for dtors list\n");
        return false;
    }

    /*
     * Special Case 3: Issue a warning if attempting to link pthreads into a binary
     * that originally did not support it or into a binary that is stripped. This
     * scenario is not supported with the initial release of the binary rewriter for
     * static binaries.
     *
     * The other side of the coin, if working with a binary that does have pthreads
     * support, it is a good idea to load pthreads.
     */
    bool isMTCapable = isMultiThreadCapable();
    bool foundPthreads = false;

    vector<Archive *> libs;
    vector<Archive *>::iterator libIter;
    if( origBinary->getLinkingResources(libs) ) {
        for(libIter = libs.begin(); libIter != libs.end(); ++libIter) {
            if( (*libIter)->name().find("libpthread") != std::string::npos ) {
                foundPthreads = true;
                break;
            }
        }
    }

    if( foundPthreads && (!isMTCapable || origBinary->isStripped()) ) {
        fprintf(stderr,
            "\nWARNING: the pthreads library has been loaded and\n"
            "the original binary is not multithread-capable or\n"
            "it is stripped. Currently, the combination of these two\n"
            "scenarios is unsupported and unexpected behavior may occur.\n");
    }else if( !foundPthreads && isMTCapable ) {
        fprintf(stderr,
            "\nWARNING: the pthreads library has not been loaded and\n"
            "the original binary is multithread-capable. Unexpected\n"
            "behavior may occur because some pthreads routines are\n"
            "unavailable in the original binary\n");
    }

    /* 
     * Special Case 4:
     * The RT library has some dependencies -- Symtab always needs to know
     * about these dependencies. So if the dependencies haven't already been
     * loaded, load them.
     */
    bool loadLibc = true;

    for(libIter = libs.begin(); libIter != libs.end(); ++libIter) {
        if( (*libIter)->name().find("libc") != std::string::npos ) {
            loadLibc = false;
        }
    }

    if( loadLibc ) {
        std::map<std::string, BinaryEdit *> res = openResolvedLibraryName("libc.a");
        std::map<std::string, BinaryEdit *>::iterator bedit_it;
        for(bedit_it = res.begin(); bedit_it != res.end(); ++bedit_it) {
            if( bedit_it->second == NULL ) {
                logLine("Failed to load DyninstAPI_RT library dependency (libc.a)");
                return false;
            }
        }
    }

    return true;
}

bool image::findGlobalConstructorFunc() {
    using namespace Dyninst::InstructionAPI;

    vector<Function *> funcs;
    if( linkedFile->findFunctionsByName(funcs, LIBC_CTOR_HANDLER) ) {
        return true;
    }

    /* If the symbol isn't found, try looking for it in a call instruction in
     * the .init section
     *
     * The instruction sequence is:
     * ...
     * some instructions
     * ...
     * call call_gmon_start
     * call frame_dummy
     * call __do_global_ctors_aux
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
        image_func *initStub = addFunctionStub(initRegion->getRegionAddr(), "_init");
        if( initStub == NULL ) {
            logLine("unable to create function for .init \n");
            return false;
        }else{
            initStub->parse();
            inst_printf("%s[%d]: set _init function address to 0x%lx\n", FILE__, __LINE__,
                initRegion->getRegionAddr());
        }
    }

    // Search for last of 3 calls
    Address ctorAddress = 0;
    unsigned bytesSeen = 0;
    unsigned numCalls = 0;
    const unsigned char *p = reinterpret_cast<const unsigned char *>(initRegion->getPtrToRawData());

    dyn_detail::boost::shared_ptr<InstructionDecoder> decoder =
            makeDecoder(getArch(), p, initRegion->getRegionSize());
    decoder->setMode(getAddressWidth() == 8);

    Instruction::Ptr curInsn = decoder->decode();
    while(numCalls < 3 && curInsn && curInsn->isValid() &&
          bytesSeen < initRegion->getRegionSize()) 
    {
        InsnCategory category = curInsn->getCategory();
        if( category == c_CallInsn ) {
            numCalls++;
        }
        if( numCalls < 3 ) {
            bytesSeen += curInsn->size();
            curInsn = decoder->decode();
        }
    }

    if( numCalls != 3 ) {
        logLine("heuristic for finding global constructor function failed\n");
        return false;
    }

    Address callAddress = initRegion->getRegionAddr() + bytesSeen;

    RegisterAST thePC = RegisterAST(Dyninst::MachRegister::getPC(getArch()));

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

    if( !ctorAddress || !isValidAddress(ctorAddress) ) {
        logLine("invalid address for global constructor function\n");
        return false;
    }

    if( addFunctionStub(ctorAddress, LIBC_CTOR_HANDLER.c_str()) == NULL ) {
        logLine("unable to create representation for global constructor function\n");
        return false;
    }else{
        inst_printf("%s[%d]: set global constructor address to 0x%lx\n", FILE__, __LINE__,
                ctorAddress);
    }

    return true;
}

bool image::findGlobalDestructorFunc() {
    using namespace Dyninst::InstructionAPI;

    vector<Function *> funcs;
    if( linkedFile->findFunctionsByName(funcs, LIBC_DTOR_HANDLER) ) {
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
     * call LIBC_DTOR_HANDLER
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
        image_func *finiStub = addFunctionStub(finiRegion->getRegionAddr(), "_fini");
        if( finiStub == NULL ) {
            logLine("unable to create function for .fini \n");
            return false;
        }else{
            finiStub->parse();
            inst_printf("%s[%d]: set _fini function address to 0x%lx\n", FILE__, __LINE__,
                finiRegion->getRegionAddr());
        }
    }

    // Search for last call in the function
    Address dtorAddress = 0;
    unsigned bytesSeen = 0;
    const unsigned char *p = reinterpret_cast<const unsigned char *>(finiRegion->getPtrToRawData());

    dyn_detail::boost::shared_ptr<InstructionDecoder> decoder =
            makeDecoder(getArch(), p, finiRegion->getRegionSize());
    decoder->setMode(getAddressWidth() == 8);

    Instruction::Ptr lastCall;
    Instruction::Ptr curInsn = decoder->decode();

    while(curInsn && curInsn->isValid() &&
          bytesSeen < finiRegion->getRegionSize()) 
    {
        InsnCategory category = curInsn->getCategory();
        if( category == c_CallInsn ) {
            lastCall = curInsn;
        }
            bytesSeen += curInsn->size();
            curInsn = decoder->decode();
    }

    if( !lastCall.get() || !lastCall->isValid() ) {
        logLine("heuristic for finding global destructor function failed\n");
        return false;
    }

    Address callAddress = finiRegion->getRegionAddr() + bytesSeen;

    RegisterAST thePC = RegisterAST(Dyninst::MachRegister::getPC(getArch()));

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

    if( !dtorAddress || !isValidAddress(dtorAddress) ) {
        logLine("invalid address for global destructor function\n");
        return false;
    }

    if( addFunctionStub(dtorAddress, LIBC_DTOR_HANDLER.c_str()) == NULL ) {
        logLine("unable to create representation for global destructor function\n");
        return false;
    }else{
        inst_printf("%s[%d]: set global destructor address to 0x%lx\n", FILE__, __LINE__,
                dtorAddress);
    }

    return true;
}
