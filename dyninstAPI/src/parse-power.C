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


using namespace Dyninst::SymtabAPI;
namespace {
  char const* LIBC_CTOR_HANDLER("__libc_csu_init");
  char const* LIBC_DTOR_HANDLER("__libc_csu_fini");
  char const* DYNINST_CTOR_HANDLER("DYNINSTglobal_ctors_handler");
  char const* DYNINST_DTOR_HANDLER("DYNINSTglobal_dtors_handler");
}

static void add_handler(instPoint* pt, func_instance* add_me)
{
  vector<AstNodePtr> args;
  // no args, just add
  AstNodePtr snip = AstNode::funcCallNode(add_me, args);
  auto instrumentation = pt->pushFront(snip);
  instrumentation->disableRecursiveGuard();
}

/*
By parsing the function that actually sets up the parameters for the OMP
region we discover informations such as what type of parallel region we're
dealing with */
bool parse_func::parseOMPParent(image_parRegion * /*iPar*/, int /*desiredNum*/, int & /*currentSectionNum*/ )
{
    return false;
}



	
std::string parse_func::calcParentFunc(const parse_func * imf,
                                    std::vector<image_parRegion *> &/*pR*/)
{
  /* We need to figure out the function that called the outlined
     parallel region function.  We do this by chopping off the
     last @OL@number */
   auto const& tmp = imf->prettyName();
   const char * nameStart = tmp.c_str();
   const char * nameEnd = strrchr(nameStart, '@');
   int strSize = nameEnd - nameStart - 3;
   
   /* Make sure that the shortened string is not of size 0,
      this would happen if a function started with @ or if there
      was less than two characters between the beginning and @
      This wouldn't happen for OpenMP functions, but might for imposters*/
   if (strSize > 0)
   {
      std::string tempPDS(nameStart, strSize);
      return tempPDS;
   }
   else   /* if it starts with @ just return the full function as its parent, we'll sort it out later */
   {
      std::string tempPDS(nameStart);
      return tempPDS;
   }
}


void parse_func::parseOMP(image_parRegion * parReg, parse_func * parentFunc, int & currentSectionNum)
{  
  /* Each region is contained in a function, for the worksharing constructs
     usually the parralel region encompasses the entire function 
     The "desiredNum" variable is the desired construct we want to "skip" to
     when we parse the parent function, which we do whenever we encounter
     a new outlined function */
  
   int desiredNum = 0;
       
  /* This will fill in the directive type and all possible clauses 
     Multiple section numbers can occur with only one call to a section setup
     function in the parent function, so we need to always offset the desired 
     region we are looking for with the current section number we are on*/
      
   int lastSecSize = 0;	 // the number of sections in the most recent sections construct  
   int totalSectionGroups = 0; // the number of sections constructs encountered (in source), 
                               // not the number of total sections


  /* First, we increment the desired num for each region we've already
     parsed from the parent function.  A parent function can spawn multiple
     outlined functions, so we don't want to mix up already parsed ones */
   for(unsigned a = 0; a < parentFunc->parRegions().size();a++)
   {		  
      image_parRegion * tempReg = parentFunc->parRegions()[a];
      
      if (tempReg->getRegionType() == OMP_PARALLEL ||
          tempReg->getRegionType() == OMP_DO_FOR ||
          tempReg->getRegionType() == OMP_PAR_DO ||
          tempReg->getRegionType() == OMP_PAR_SECTIONS ||
          tempReg->getRegionType() == OMP_SINGLE )
      {
         desiredNum++;
      }

      /* For the sections, we can't just count the number of section constructs
         we run into, since multiple outlined functions line up to one section construct */
      if (tempReg->getRegionType() == OMP_SECTIONS)
      {
         lastSecSize = tempReg->getClause("NUM_SECTIONS");
         a += (lastSecSize-1);
         totalSectionGroups++;
      }
   }
   
   
   // if the currentSectionNum is not zero, it means there are still more outlined functions
   // for a single section construct still out there, so we don't count the current section towards the total
   if (currentSectionNum != 0)
   {
      totalSectionGroups--;
   }
   
   // sets the last instruction of the region
   //parReg->setLastInsn(get_address() + get_size());
   // XXX this is equivalent to the above, but is it right?
   //     it seems to be after the last instruction
   Address last = extents().back()->end();
   parReg->setLastInsn(last);
   
   // we need to parse the parent function to get all the information about the region, mostly for worksharing constructs
   bool hasLoop = parentFunc->parseOMPParent(parReg, desiredNum + totalSectionGroups, currentSectionNum);	    
   
   // we parse the outlined function to look for inlined constructs like "Master" and "Ordered"
   parseOMPFunc(hasLoop);
}	  

void parse_func::parseOMPFunc(bool /*hasLoop*/)
{
   if (OMPparsed_)
      return;
   OMPparsed_ = true;
}

/* This does a linear scan to find out which registers are used in the function,
   it then stores these registers so the scan only needs to be done once.
   It returns true or false based on whether the function is a leaf function,
   since if it is not the function could call out to another function that
   clobbers more registers so more analysis would be needed */
void parse_func::calcUsedRegs()
{
   if (usedRegisters != NULL)
      return; 
   else
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
        unsigned size = 0;
        while(size < (*curBlock)->size())
        {
            i = d.decode();
            size += i.size();
            i.getWriteSet(writtenRegs);
        }
    }
    for(std::set<RegisterAST::Ptr>::const_iterator curReg = writtenRegs.begin();
        curReg != writtenRegs.end();
       ++curReg)
    {
        MachRegister r = (*curReg)->getID();
        if((r & ppc32::GPR) && (r <= ppc32::r13))
        {
            usedRegisters->generalPurposeRegisters.insert(r & 0xFFFF);
        }
        else if(((r & ppc32::FPR) && (r <= ppc32::fpr13)) ||
                  ((r & ppc32::FSR) && (r <= ppc32::fsr13)))
        {
            usedRegisters->floatingPointRegisters.insert(r & 0xFFFF);
        }
    }
   }
   return;
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

