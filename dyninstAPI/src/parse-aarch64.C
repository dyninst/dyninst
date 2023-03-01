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

// $Id: image-power.C,v 1.23 2008/03/12 20:09:10 legendre Exp $

// Determine if the called function is a "library" function or a "user" function
// This cannot be done until all of the functions have been seen, verified, and
// classified
//

#include <unordered_map>
#include "parse-cfg.h"
#include "instPoint.h"
#include "image.h"
#include "parRegion.h"
#include "debug.h"

#include "debug.h"
#include <deque>
#include <set>
#include <algorithm>

#include "common/src/arch.h"

#include "instructionAPI/h/Instruction.h"
#include "instructionAPI/h/InstructionDecoder.h"

#include "mapped_object.h"
#include "binaryEdit.h"
#include "addressSpace.h"
#include "function.h"
#include "baseTramp.h"

//#warning "This file is not implemented yet!"
using namespace Dyninst::SymtabAPI;

namespace {
  char const* LIBC_CTOR_HANDLER("__libc_csu_init");
  char const* LIBC_DTOR_HANDLER("__libc_csu_fini");
  char const* DYNINST_CTOR_HANDLER("DYNINSTglobal_ctors_handler");
  char const* DYNINST_DTOR_HANDLER("DYNINSTglobal_dtors_handler");
}

/*
By parsing the function that actually sets up the parameters for the OMP
region we discover informations such as what type of parallel region we're
dealing with */
bool parse_func::parseOMPParent(image_parRegion * /*iPar*/, int /*desiredNum*/, int & /*currentSectionNum*/ )
{
	assert(0);
	return false;
}




std::string parse_func::calcParentFunc(const parse_func *,
                                    std::vector<image_parRegion *> &/*pR*/)
{
	assert(0);
	return {};
}


void parse_func::parseOMP(image_parRegion *, parse_func *, int &)
{
	assert(0);
}

void parse_func::parseOMPFunc(bool /*hasLoop*/)
{
	assert(0);
}

/* This does a linear scan to find out which registers are used in the function,
   it then stores these registers so the scan only needs to be done once.
   It returns true or false based on whether the function is a leaf function,
   since if it is not the function could call out to another function that
   clobbers more registers so more analysis would be needed */
void parse_func::calcUsedRegs()
{
    if (!usedRegisters)
    {
        usedRegisters = new parse_func_registers();
        using namespace Dyninst::InstructionAPI;
        std::set<RegisterAST::Ptr> writtenRegs;

        auto bl = blocks();
        auto curBlock = bl.begin();
        for( ; curBlock != bl.end(); ++curBlock)
        {
            InstructionDecoder d(getPtrToInstruction((*curBlock)->start()),
                    (*curBlock)->size(),
                    isrc()->getArch());
            Instruction i;
            i = d.decode();
            while(i.isValid())
            {
                i.getWriteSet(writtenRegs);
                i = d.decode();
            }
        }
        
        for(std::set<RegisterAST::Ptr>::const_iterator curReg = writtenRegs.begin();
                curReg != writtenRegs.end();
                ++curReg)
        {
            MachRegister r = (*curReg)->getID();
            if((r & aarch64::GPR) && (r <= aarch64::w30))
            {
                usedRegisters->generalPurposeRegisters.insert(r & 0xFF);
            }
            else if(((r & aarch64::FPR) && (r <= aarch64::s31)))
            {
                usedRegisters->floatingPointRegisters.insert(r & 0xFFFF);
            }
        }
    }
    return;
}

static void add_handler(instPoint* pt, func_instance* add_me)
{
  vector<AstNodePtr> args;
  // no args, just add
  AstNodePtr snip = AstNode::funcCallNode(add_me, args);
  auto instrumentation = pt->pushFront(snip);
  instrumentation->disableRecursiveGuard();
}

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
    Symtab *origBinary = mobj->parse_img()->getObject();
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

    vector<Archive *> libs1;
    vector<Archive *>::iterator libIter1;
    bool loadLibc = true;
    if( origBinary->getLinkingResources(libs1) ) {
        for(libIter1 = libs1.begin(); libIter1 != libs1.end(); ++libIter1) {
            if( (*libIter1)->name().find("libc.a") != std::string::npos ) {
                loadLibc = false;
            }
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
                logLine("Failed to load DyninstAPI_RT library dependency (libc.a)");
                fprintf (stderr,"Failed to load DyninstAPI_RT library dependency (libc.a)");
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

    const std::vector<func_instance *> *ctorFuncs = findFuncVectorByMangled(ctorHandler);
    if( ctorFuncs != NULL ) {
        return ctorFuncs->at(0);
    }
    return NULL;
}

func_instance *mapped_object::findGlobalDestructorFunc(const std::string &dtorHandler) {
    using namespace Dyninst::InstructionAPI;

    const std::vector<func_instance *> *ctorFuncs = findFuncVectorByMangled(dtorHandler);
    if( ctorFuncs != NULL ) {
        return ctorFuncs->at(0);
    }
    return NULL;
}

