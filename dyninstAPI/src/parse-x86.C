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

#include <unordered_map>
#include "parse-cfg.h"
#include "instPoint.h"
#include "mapped_object.h"
#include "image.h"
#include "debug.h"
#include <deque>
#include <set>
#include <algorithm>
#include "registers/x86_regs.h"

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
            if(!(*cit)->trg()) continue;
            parse_func * ct = dynamic_cast<parse_func*>(
                obj()->findFuncByEntry(region(),(*cit)->trg()->start()));
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
            Instruction i = d.decode();

            while((i.isValid())) {
                if(i.isWritten(st0) ||
                    i.isWritten(st1) ||
                    i.isWritten(st2) ||
                    i.isWritten(st3) ||
                    i.isWritten(st4) ||
                    i.isWritten(st5) ||
                    i.isWritten(st6) ||
                    i.isWritten(st7) ||
                   i.isWritten(xmm0) ||
                   i.isWritten(xmm1) ||
                   i.isWritten(xmm2) ||
                   i.isWritten(xmm3) ||
                   i.isWritten(xmm4) ||
                   i.isWritten(xmm5) ||
                   i.isWritten(xmm6) ||
                   i.isWritten(xmm7)
                  )
                {
                    containsFPRWrites_ = used;
                    return true;
                }
                i = d.decode();
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
namespace {
  char const* LIBC_CTOR_HANDLER("__libc_csu_init");
  char const* LIBC_DTOR_HANDLER("__libc_csu_fini");
  char const* DYNINST_CTOR_HANDLER("DYNINSTglobal_ctors_handler");
  char const* DYNINST_DTOR_HANDLER("DYNINSTglobal_dtors_handler");
  char const* LIBC_IREL_HANDLER("__libc_csu_irel");
  char const* DYNINST_IREL_HANDLER("DYNINSTglobal_irel_handler");
  char const* DYNINST_IREL_START("DYNINSTirel_start");
  char const* DYNINST_IREL_END("DYNINSTirel_end");
  char const* SYMTAB_IREL_START("__SYMTABAPI_IREL_START__");
  char const* SYMTAB_IREL_END("__SYMTABAPI_IREL_END__");
}


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

static void add_handler(instPoint* pt, func_instance* add_me)
{
  vector<AstNodePtr> args;
  // no args, just add
  AstNodePtr snip = AstNode::funcCallNode(add_me, args);
  auto instrumentation = pt->pushFront(snip);
  instrumentation->disableRecursiveGuard();
}


bool BinaryEdit::doStaticBinarySpecialCases() {
    /* Special Case 1A: Handling global constructors
     *
     * Place the Dyninst constructor handler after the global ELF ctors so it is invoked last.
     *
     * Prior to glibc-2.34, this was in the exit point(s) of __libc_csu_init which
     * calls all of the initializers in preinit_array and init_array as per SystemV
     * before __libc_start_main is invoked.
     *
     * In glibc-2.34, the code from the csu_* functions was moved into __libc_start_main, so
     * now the only place where we are guaranteed that the global constructors have all been
     * called is at the beginning of 'main'.
    */
    func_instance *dyninstCtorHandler = findOnlyOneFunction(DYNINST_CTOR_HANDLER);
    if( !dyninstCtorHandler ) {
        logLine("failed to find Dyninst constructor handler\n");
        return false;
    }
    if(auto *ctor = mobj->findGlobalConstructorFunc(LIBC_CTOR_HANDLER)) {
        // Wire in our handler at libc ctor exits
        vector<instPoint*> init_pts;
        ctor->funcExitPoints(&init_pts);
        for(auto *exit_pt : init_pts) {
          add_handler(exit_pt, dyninstCtorHandler);
        }
    } else if(auto *main = findOnlyOneFunction("main")) {
    	// Insert constructor into the beginning of 'main'
        add_handler(main->funcEntryPoint(true), dyninstCtorHandler);
    } else {
   	    logLine("failed to find place to insert Dyninst constructors\n");
   	    return false;
    }

    /* Special Case 1B: Handling global destructors
     *
     * Place the Dyninst destructor handler before the global ELF dtors so it is invoked first.
     *
     * Prior to glibc-2.34, this was in the entry point of __libc_csu_fini.
     *
     * In glibc-2.34, the code in __libc_csu_fini was moved into a hidden function that is
     * registered with atexit. To ensure the Dyninst destructors are always called first, we
     * have to insert the handler at the beginning of `exit`.
     *
     * This is a fragile solution as there is no requirement that a symbol for `exit` is
     * exported. If we can't find it, we'll just fail here.
    */
    func_instance *dyninstDtorHandler = findOnlyOneFunction(DYNINST_DTOR_HANDLER);
    if( !dyninstDtorHandler ) {
        logLine("failed to find Dyninst destructor handler\n");
        return false;
    }
    if(auto *dtor = mobj->findGlobalDestructorFunc(LIBC_DTOR_HANDLER)) {
    	// Insert destructor into beginning of libc global dtor handler
        add_handler(dtor->funcEntryPoint(true), dyninstDtorHandler);
    } else if(auto *exit_ = findOnlyOneFunction("exit")) {
    	// Insert destructor into beginning of `exit`
    	add_handler(exit_->funcEntryPoint(true), dyninstDtorHandler);
    } else {
    	logLine("failed to find place to insert Dyninst destructors\n");
        return false;
    }

    AddressSpace::patch(this);
    
    /* Special Case 1C: Instrument irel handlers
     *
     * Replace the irel handler with our extended version, since they hard-code
     * ALL THE OFFSETS in the function.
     *
     * __libc_csu_irel was removed from glibc-2.19 in 2013.
     *
     * irel handlers are not instrumented on the other architectures. We leave this
     * here for posterity.
    */
    if(auto *globalIrelHandler = findOnlyOneFunction(LIBC_IREL_HANDLER)) {
      func_instance *dyninstIrelHandler = findOnlyOneFunction(DYNINST_IREL_HANDLER);
      int_symbol irelStart;
      int_symbol irelEnd;
      bool irs_found = false;
      bool ire_found = false;
      for (auto rtlib_it = rtlib.begin(); rtlib_it != rtlib.end(); ++rtlib_it) {
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
    Symtab *origBinary = mobj->parse_img()->getObject();
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

        // libc.a may be depending on libgcc.a
        res.clear();
        if (!openResolvedLibraryName("libgcc.a", res)) {
            cerr << "Failed to find libgcc.a, which can be needed by libc.a on certain platforms" << endl;
            cerr << "Set LD_LIBRARY_PATH to the directory containing libgcc.a" << endl;
        }
    }
    
    return true;
}

func_instance *mapped_object::findGlobalConstructorFunc(const std::string &ctorHandler) {
    using namespace Dyninst::InstructionAPI;

    const std::vector<func_instance *> *matching_funcs = findFuncVectorByMangled(ctorHandler);
    if( matching_funcs != NULL ) {
        return matching_funcs->at(0);
    }
    return NULL;
}

func_instance *mapped_object::findGlobalDestructorFunc(const std::string &dtorHandler) {
    using namespace Dyninst::InstructionAPI;

    const std::vector<func_instance *> *matching_funcs = findFuncVectorByMangled(dtorHandler);
    if( matching_funcs != NULL ) {
        return matching_funcs->at(0);
    }
    return NULL;
}


#endif
