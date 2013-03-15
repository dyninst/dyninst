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

/*
 * inst-x86.C - x86 dependent functions and code generator
 */

#include "common/h/Vector.h"
#include <unordered_map>
#include "common/h/Vector.h"
#include "parse-cfg.h"
#include "instPoint.h"
#include "mapped_object.h"
#include "image.h"
#include "debug.h"
#include <deque>
#include <set>
#include <algorithm>
//#include "arch.h"

#include "instructionAPI/h/Instruction.h"
#include "instructionAPI/h/InstructionDecoder.h"

using namespace Dyninst::ParseAPI;

bool parse_func::writesFPRs(unsigned level) {
    
    using namespace Dyninst::InstructionAPI;
    // Oh, we should be parsed by now...
    if (!parsed()) image_->analyzeIfNeeded();

    if (containsFPRWrites_ == unknown) {
        // Iterate down and find out...
        // We know if we have callees because we can
        // check the instPoints; no reason to iterate over.
        // We also cache callee values here for speed.

        if (level >= 3) {
            return true; // Arbitrarily decided level 3 iteration.
        }        
        const Function::edgelist & calls = callEdges();
        Function::edgelist::const_iterator cit = calls.begin();
        for( ; cit != calls.end(); ++cit) {
            image_edge * ce = static_cast<image_edge*>(*cit);
            parse_func * ct = static_cast<parse_func*>(
                obj()->findFuncByEntry(region(),ce->trg()->start()));
            if(ct && ct != this) {
                if (ct->writesFPRs(level+1)) {
                    // One of our kids does... if we're top-level, cache it; in 
                    // any case, return
                    if (level == 0)
                        containsFPRWrites_ = used;
                    return true;
                }
            }
            else if(!ct){
                // Indirect call... oh, yeah. 
                if (level == 0)
                    containsFPRWrites_ = used;
                return true;
            }
        }

        // No kids contain writes. See if our code does.
        static RegisterAST::Ptr st0(new RegisterAST(x86::st0));
        static RegisterAST::Ptr st1(new RegisterAST(x86::st1));
        static RegisterAST::Ptr st2(new RegisterAST(x86::st2));
        static RegisterAST::Ptr st3(new RegisterAST(x86::st3));
        static RegisterAST::Ptr st4(new RegisterAST(x86::st4));
        static RegisterAST::Ptr st5(new RegisterAST(x86::st5));
        static RegisterAST::Ptr st6(new RegisterAST(x86::st6));
        static RegisterAST::Ptr st7(new RegisterAST(x86::st7));
        static RegisterAST::Ptr xmm0(new RegisterAST(x86::xmm0));
        static RegisterAST::Ptr xmm1(new RegisterAST(x86::xmm1));
        static RegisterAST::Ptr xmm2(new RegisterAST(x86::xmm2));
        static RegisterAST::Ptr xmm3(new RegisterAST(x86::xmm3));
        static RegisterAST::Ptr xmm4(new RegisterAST(x86::xmm4));
        static RegisterAST::Ptr xmm5(new RegisterAST(x86::xmm5));
        static RegisterAST::Ptr xmm6(new RegisterAST(x86::xmm6));
        static RegisterAST::Ptr xmm7(new RegisterAST(x86::xmm7));

        vector<FuncExtent *>::const_iterator eit = extents().begin();
        for( ; eit != extents().end(); ++eit) {
            FuncExtent * fe = *eit;
        
            const unsigned char* buf = (const unsigned char*)
                isrc()->getPtrToInstruction(fe->start());
            if(!buf) {
                parsing_printf("%s[%d]: failed to get insn ptr at %lx\n",
                    FILE__, __LINE__,fe->start());
                // if the function cannot be parsed, it is only safe to 
                // assume that the FPRs are written -- mcnulty
                return true; 
            }
            InstructionDecoder d(buf,fe->end()-fe->start(),isrc()->getArch());
            Instruction::Ptr i;

            while(i = d.decode()) {
                if(i->isWritten(st0) ||
                    i->isWritten(st1) ||
                    i->isWritten(st2) ||
                    i->isWritten(st3) ||
                    i->isWritten(st4) ||
                    i->isWritten(st5) ||
                    i->isWritten(st6) ||
                    i->isWritten(st7) ||
                   i->isWritten(xmm0) ||
                   i->isWritten(xmm1) ||
                   i->isWritten(xmm2) ||
                   i->isWritten(xmm3) ||
                   i->isWritten(xmm4) ||
                   i->isWritten(xmm5) ||
                   i->isWritten(xmm6) ||
                   i->isWritten(xmm7)
                  )
                {
                    containsFPRWrites_ = used;
                    return true;
                }
            }
        }
        // No kids do, and we don't. Impressive.
        containsFPRWrites_ = unused;
        return false;
    }
    else if (containsFPRWrites_ == used) {
        return true;
    }
    else if (containsFPRWrites_ == unused) {
        return false;
    }

    fprintf(stderr, "ERROR: function %s, containsFPRWrites_ is %d (illegal value!)\n", 
	    symTabName().c_str(), containsFPRWrites_);
    
    assert(0);
    return false;
}

#if defined(os_linux) || defined(os_freebsd)

#include "binaryEdit.h"
#include "addressSpace.h"
#include "function.h"
#include "baseTramp.h"
#include "image.h"

using namespace Dyninst::SymtabAPI;

/*
 * Static binary rewriting support
 *
 * Some of the following functions replace the standard ctor and dtor handlers
 * in a binary. Currently, these operations only work with binaries linked with
 * the GNU toolchain. However, it should be straightforward to extend these
 * operations to other toolchains.
 */
static const std::string LIBC_CTOR_HANDLER("__do_global_ctors_aux");
static const std::string LIBC_DTOR_HANDLER("__do_global_dtors_aux");
static const std::string DYNINST_CTOR_HANDLER("DYNINSTglobal_ctors_handler");
static const std::string DYNINST_CTOR_LIST("DYNINSTctors_addr");
static const std::string DYNINST_DTOR_HANDLER("DYNINSTglobal_dtors_handler");
static const std::string DYNINST_DTOR_LIST("DYNINSTdtors_addr");
static const std::string SYMTAB_CTOR_LIST_REL("__SYMTABAPI_CTOR_LIST__");
static const std::string SYMTAB_DTOR_LIST_REL("__SYMTABAPI_DTOR_LIST__");
static const std::string LIBC_IREL_HANDLER("__libc_csu_irel");
static const std::string DYNINST_IREL_HANDLER("DYNINSTglobal_irel_handler");
static const std::string DYNINST_IREL_START("DYNINSTirel_start");
static const std::string DYNINST_IREL_END("DYNINSTirel_end");
static const std::string SYMTAB_IREL_START("__SYMTABAPI_IREL_START__");
static const std::string SYMTAB_IREL_END("__SYMTABAPI_IREL_END__");


static bool replaceHandler(func_instance *origHandler, func_instance *newHandler, 
			   std::vector<std::pair<int_symbol *, std::string> > &reloc_replacements) {
    // Add instrumentation to replace the function
   // TODO: this should be a function replacement!
   // And why the hell is it in parse-x86.C?
   origHandler->proc()->replaceFunction(origHandler, newHandler);
   AddressSpace::patch(origHandler->proc());

   for (auto iter = reloc_replacements.begin(); iter != reloc_replacements.end(); ++iter) {
     int_symbol *newList = iter->first;
     std::string listRelName = iter->second;

     /* create the special relocation for the new list -- search the RT library for
      * the symbol
      */
     Symbol *newListSym = const_cast<Symbol *>(newList->sym());
     
     std::vector<Region *> allRegions;
     if( !newListSym->getSymtab()->getAllRegions(allRegions) ) {
       return false;
     }
     
     std::vector<Region *>::iterator reg_it;
     bool found = false;
     for(reg_it = allRegions.begin(); reg_it != allRegions.end(); ++reg_it) {
       std::vector<relocationEntry> &region_rels = (*reg_it)->getRelocations();
       vector<relocationEntry>::iterator rel_it;
       for( rel_it = region_rels.begin(); rel_it != region_rels.end(); ++rel_it) {
	 if( rel_it->getDynSym() == newListSym ) {
	   relocationEntry *rel = &(*rel_it);
	   rel->setName(listRelName);
	   found = true;
	 }
       }
     }
     if (!found) {
       return false;
     }
   }

   return true;
}


static bool replaceHandler(func_instance *origHandler, func_instance *newHandler,
			   int_symbol *sym, std::string name) {
  std::vector<std::pair<int_symbol *, std::string> > tmp;
  tmp.push_back(make_pair(sym, name));
  return replaceHandler(origHandler, newHandler, tmp);
}

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

    // First, find all the necessary symbol info.
    func_instance *globalCtorHandler = mobj->findGlobalConstructorFunc(LIBC_CTOR_HANDLER);
    if( !globalCtorHandler ) {
        logLine("failed to find libc constructor handler\n");
        return false;
    }

    func_instance *dyninstCtorHandler = findOnlyOneFunction(DYNINST_CTOR_HANDLER);
    if( !dyninstCtorHandler ) {
        logLine("failed to find Dyninst constructor handler\n");
        return false;
    }

    func_instance *globalDtorHandler = mobj->findGlobalDestructorFunc(LIBC_DTOR_HANDLER);
    if( !globalDtorHandler ) {
        logLine("failed to find libc destructor handler\n");
        return false;
    }

    func_instance *dyninstDtorHandler = findOnlyOneFunction(DYNINST_DTOR_HANDLER);
    if( !dyninstDtorHandler ) {
        logLine("failed to find Dyninst destructor handler\n");
        return false;
    }

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

    /*
     * Replace the libc ctor and dtor handlers with our special handlers
     */
    if( !replaceHandler(globalCtorHandler, dyninstCtorHandler,
                &ctorsListInt, SYMTAB_CTOR_LIST_REL) ) {
        logLine("Failed to replace libc ctor handler with special handler");
        return false;
    }else{
        inst_printf("%s[%d]: replaced ctor function %s with %s\n",
                FILE__, __LINE__, LIBC_CTOR_HANDLER.c_str(),
                DYNINST_CTOR_HANDLER.c_str());
    }

    if( !replaceHandler(globalDtorHandler, dyninstDtorHandler,
                &dtorsListInt, SYMTAB_DTOR_LIST_REL) ) {
        logLine("Failed to replace libc dtor handler with special handler");
        return false;
    }else{
        inst_printf("%s[%d]: replaced dtor function %s with %s\n",
                FILE__, __LINE__, LIBC_DTOR_HANDLER.c_str(),
                DYNINST_DTOR_HANDLER.c_str());
    }

    /*
     * Replace the irel handler with our extended version, since they
     * hard-code ALL THE OFFSETS in the function
     */
    func_instance *globalIrelHandler = findOnlyOneFunction(LIBC_IREL_HANDLER);
    func_instance *dyninstIrelHandler = findOnlyOneFunction(DYNINST_IREL_HANDLER);
    int_symbol irelStart;
    int_symbol irelEnd;
    bool irs_found = false;
    bool ire_found = false;
    for (auto iter = rtlib.begin(); iter != rtlib.end(); ++iter) {
      if( (*rtlib_it)->getSymbolInfo(DYNINST_IREL_START, irelStart) ) {
	irs_found = true;
      }
      
      if( (*rtlib_it)->getSymbolInfo(DYNINST_IREL_END, irelEnd) ) {
	ire_found = true;
      }
      if (irs_found && ire_found) break;
    }
    if (globalIrelHandler) {
      assert(dyninstIrelHandler);
      assert(irs_found);
      assert(ire_found);
      std::vector<std::pair<int_symbol *, string> > tmp;
      tmp.push_back(make_pair(&irelStart, SYMTAB_IREL_START));
      tmp.push_back(make_pair(&irelEnd, SYMTAB_IREL_END));
      if (!replaceHandler(globalIrelHandler, dyninstIrelHandler, tmp)) {
	return false;
      }
    }


    /*
     * Special Case 2: Issue a warning if attempting to link pthreads into a binary
     * that originally did not support it or into a binary that is stripped. This
     * scenario is not supported with the initial release of the binary rewriter for
     * static binaries.
     *
     * The other side of the coin, if working with a binary that does have pthreads
     * support, pthreads needs to be loaded.
     */
    bool isMTCapable = isMultiThreadCapable();
    bool foundPthreads = false;

    vector<Archive *> libs;
    vector<Archive *>::iterator libIter;
    if( origBinary->getLinkingResources(libs) ) {
        for(libIter = libs.begin(); libIter != libs.end(); ++libIter) {
            if( (*libIter)->name().find("libpthread") != std::string::npos ||
                (*libIter)->name().find("libthr") != std::string::npos ) 
            {
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
     * Special Case 3:
     * The RT library has some dependencies -- Symtab always needs to know
     * about these dependencies. So if the dependencies haven't already been
     * loaded, load them.
     */
    bool loadLibc = true;

    for(libIter = libs.begin(); libIter != libs.end(); ++libIter) {
        if( (*libIter)->name().find("libc.a") != std::string::npos ) {
            loadLibc = false;
        }
    }

    if( loadLibc ) {
       std::map<std::string, BinaryEdit *> res;
       openResolvedLibraryName("libc.a", res);

       if (res.empty()) {
	 cerr << "Fatal error: failed to load DyninstAPI_RT library dependency (libc.a)" << endl;
	 return false;
       }

       std::map<std::string, BinaryEdit *>::iterator bedit_it;
       for(bedit_it = res.begin(); bedit_it != res.end(); ++bedit_it) {
          if( bedit_it->second == NULL ) {
	    cerr << "Fatal error: failed to load DyninstAPI_RT library dependency (libc.a)" << endl;
	    return false;
          }
       }
    }
    
    return true;
}

func_instance *mapped_object::findGlobalConstructorFunc(const std::string &ctorHandler) {
    using namespace Dyninst::InstructionAPI;

    const pdvector<func_instance *> *funcs = findFuncVectorByMangled(ctorHandler);
    if( funcs != NULL ) {
        return funcs->at(0);
    }

    /* If the symbol isn't found, try looking for it in a call instruction in
     * the .init section
     *
     * On Linux, the instruction sequence is:
     * ...
     * some instructions
     * ...
     * call call_gmon_start
     * call frame_dummy
     * call ctor_handler
     *
     * On FreeBSD, the instruction sequence is:
     * ...
     * some instructions
     * ...
     * call frame_dummy
     * call ctor_handler
     */
    Symtab *linkedFile = parse_img()->getObject();
    Region *initRegion = NULL;
    if( !linkedFile->findRegion(initRegion, ".init") ) {
        vector<Dyninst::SymtabAPI::Function *> symFuncs;
        if( linkedFile->findFunctionsByName(symFuncs, "_init") ) {
            initRegion = symFuncs[0]->getRegion();
        }else{
            logLine("failed to locate .init Region or _init function\n");
            return NULL;
        }
    }

    if( initRegion == NULL ) {
        logLine("failed to locate .init Region or _init function\n");
        return NULL;
    }

    // Search for last of a fixed number of calls
#if defined(os_freebsd)
    const unsigned CTOR_NUM_CALLS = 2;
#else
    const unsigned CTOR_NUM_CALLS = 3;
#endif

    Address ctorAddress = 0;
    unsigned bytesSeen = 0;
    unsigned numCalls = 0;
    const unsigned char *p = reinterpret_cast<const unsigned char *>(initRegion->getPtrToRawData());

    InstructionDecoder decoder(p, initRegion->getDiskSize(),
        parse_img()->codeObject()->cs()->getArch()); 

    Instruction::Ptr curInsn = decoder.decode();
    while(numCalls < CTOR_NUM_CALLS && curInsn && curInsn->isValid() &&
          bytesSeen < initRegion->getDiskSize()) 
    {
        InsnCategory category = curInsn->getCategory();
        if( category == c_CallInsn ) {
            numCalls++;
        }
        if( numCalls < CTOR_NUM_CALLS ) {
            bytesSeen += curInsn->size();
            curInsn = decoder.decode();
        }
    }

    if( numCalls != CTOR_NUM_CALLS ) {
        logLine("heuristic for finding global constructor function failed\n");
        return NULL;
    }

    Address callAddress = initRegion->getMemOffset() + bytesSeen;

    RegisterAST thePC = RegisterAST(
        Dyninst::MachRegister::getPC(parse_img()->codeObject()->cs()->getArch()));

    Expression::Ptr callTarget = curInsn->getControlFlowTarget();
    if( !callTarget.get() ) {
        logLine("failed to find global constructor function\n");
        return NULL;
    }
    callTarget->bind(&thePC, Result(s64, callAddress));

    Result actualTarget = callTarget->eval();
    if( actualTarget.defined ) {
        ctorAddress = actualTarget.convert<Address>();
    }else{
        logLine("failed to find global constructor function\n");
        return NULL;
    }

    if( !ctorAddress || !parse_img()->codeObject()->cs()->isValidAddress(ctorAddress) ) {
        logLine("invalid address for global constructor function\n");
        return NULL;
    }

    func_instance *ret;
    if( (ret = findFuncByEntry(ctorAddress)) == NULL ) {
        logLine("unable to create representation for global constructor function\n");
        return NULL;
    }

    inst_printf("%s[%d]: set global constructor address to 0x%lx\n", FILE__, __LINE__,
            ctorAddress);

    return ret;
}

func_instance *mapped_object::findGlobalDestructorFunc(const std::string &dtorHandler) {
    using namespace Dyninst::InstructionAPI;

    const pdvector<func_instance *> *funcs = findFuncVectorByMangled(dtorHandler);
    if( funcs != NULL ) {
        return funcs->at(0);
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
    Symtab *linkedFile = parse_img()->getObject();
    Region *finiRegion = NULL;
    if( !linkedFile->findRegion(finiRegion, ".fini") ) {
        vector<Dyninst::SymtabAPI::Function *> symFuncs;
        if( linkedFile->findFunctionsByName(symFuncs, "_fini") ) {
            finiRegion = symFuncs[0]->getRegion();
        }else{
            logLine("failed to locate .fini Region or _fini function\n");
            return NULL;
        }
    }

    if( finiRegion == NULL ) {
        logLine("failed to locate .fini Region or _fini function\n");
        return NULL;
    }

    // Search for last call in the function
    Address dtorAddress = 0;
    unsigned bytesSeen = 0;
    const unsigned char *p = reinterpret_cast<const unsigned char *>(finiRegion->getPtrToRawData());

    InstructionDecoder decoder(p, finiRegion->getDiskSize(),
        parse_img()->codeObject()->cs()->getArch());

    Instruction::Ptr lastCall;
    Instruction::Ptr curInsn = decoder.decode();

    while(curInsn && curInsn->isValid() &&
          bytesSeen < finiRegion->getDiskSize()) 
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
        return NULL;
    }

    Address callAddress = finiRegion->getMemOffset() + bytesSeen;

    RegisterAST thePC = RegisterAST(
        Dyninst::MachRegister::getPC(parse_img()->codeObject()->cs()->getArch()));

    Expression::Ptr callTarget = lastCall->getControlFlowTarget();
    if( !callTarget.get() ) {
        logLine("failed to find global destructor function\n");
        return NULL;
    }
    callTarget->bind(&thePC, Result(s64, callAddress));

    Result actualTarget = callTarget->eval();
    if( actualTarget.defined ) {
        dtorAddress = actualTarget.convert<Address>();
    }else{
        logLine("failed to find global destructor function\n");
        return NULL;
    }

    if( !dtorAddress || !parse_img()->codeObject()->cs()->isValidAddress(dtorAddress) ) {
        logLine("invalid address for global destructor function\n");
        return NULL;
    }

    // A targ stub should have been created at the address
    func_instance *ret = NULL;
    if( (ret = findFuncByEntry(dtorAddress)) == NULL ) {
        logLine("unable to find global destructor function\n");
        return NULL;
    }
    inst_printf("%s[%d]: set global destructor address to 0x%lx\n", FILE__, __LINE__,
            dtorAddress);

    return ret;
}


#endif
