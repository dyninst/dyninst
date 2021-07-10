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
static const std::string LIBC_CTOR_HANDLER("__libc_csu_init");
static const std::string LIBC_DTOR_HANDLER("__libc_csu_fini");
static const std::string DYNINST_CTOR_HANDLER("DYNINSTglobal_ctors_handler");
static const std::string DYNINST_CTOR_BEGIN("DYNINSTctors_begin");
static const std::string DYNINST_CTOR_END("DYNINSTctors_end");
static const std::string DYNINST_DTOR_HANDLER("DYNINSTglobal_dtors_handler");
static const std::string DYNINST_DTOR_BEGIN("DYNINSTdtors_begin");
static const std::string DYNINST_DTOR_END("DYNINSTdtors_end");
static const std::string SYMTAB_CTOR_LIST_REL("__SYMTABAPI_CTOR_LIST__");
static const std::string SYMTAB_DTOR_LIST_REL("__SYMTABAPI_DTOR_LIST__");

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
   const char * nameStart = imf->prettyName().c_str();
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
    Symtab *origBinary = mobj->parse_img()->getObject();

    /* Special Case 1: Handling global constructor and destructor Regions
     * Invoke Dyninst constructor after all static constructors are called
     * and invoke Dyninst destructor before staitc destructors
     */

    // First, find all the necessary symbol info.

    func_instance *globalCtorHandler = mobj->findGlobalConstructorFunc(LIBC_CTOR_HANDLER);
    if( !globalCtorHandler ) {
        logLine("failed to find libc constructor handler\n");
        fprintf (stderr, "failed to find libc constructor handler\n");
        return false;
    }

    func_instance *dyninstCtorHandler = findOnlyOneFunction(DYNINST_CTOR_HANDLER);
    if( !dyninstCtorHandler ) {
        logLine("failed to find Dyninst constructor handler\n");
        fprintf (stderr,"failed to find Dyninst constructor handler\n");
        return false;
    }

    func_instance *globalDtorHandler = mobj->findGlobalDestructorFunc(LIBC_DTOR_HANDLER);
    if( !globalDtorHandler ) {
        logLine ("failed to find libc destructor handler\n");
        fprintf (stderr,"failed to find libc destructor handler\n");
        return false;
    }

    func_instance *dyninstDtorHandler = findOnlyOneFunction(DYNINST_DTOR_HANDLER);
    if( !dyninstDtorHandler ) {
        logLine("failed to find Dyninst destructor handler\n");
        fprintf (stderr,"failed to find Dyninst destructor handler\n");
        return false;
    }

    // Instrument the exits of global constructor function
    vector<instPoint*> init_pts;
    instPoint* fini_point;
    globalCtorHandler->funcExitPoints(&init_pts);

    // Instrument the entry of global destructor function
    fini_point = globalDtorHandler->funcEntryPoint(true);
    // convert points to instpoints
    for(auto exit_pt = init_pts.begin();
	exit_pt != init_pts.end();
	++exit_pt)
    {
      add_handler(*exit_pt, dyninstCtorHandler);
    }
    add_handler(fini_point, dyninstDtorHandler);
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

    Instruction curInsn = decoder.decode();
    while(numCalls < CTOR_NUM_CALLS && curInsn.isValid() &&
          bytesSeen < initRegion->getDiskSize()) 
    {
        InsnCategory category = curInsn.getCategory();
        if( category == c_CallInsn ) {
            numCalls++;
        }
        if( numCalls < CTOR_NUM_CALLS ) {
            bytesSeen += curInsn.size();
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

    Expression::Ptr callTarget = curInsn.getControlFlowTarget();
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

    const std::vector<func_instance *> *ctorFuncs = findFuncVectorByMangled(dtorHandler);
    if( ctorFuncs != NULL ) {
        return ctorFuncs->at(0);
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

    Instruction lastCall;
    Instruction curInsn = decoder.decode();
    bool find = false;

    while(curInsn.isValid() &&
          bytesSeen < finiRegion->getDiskSize()) 
    {
        InsnCategory category = curInsn.getCategory();
        if( category == c_CallInsn ) {
            find = true;
            lastCall = curInsn;
            break;
        }

        bytesSeen += curInsn.size();
        curInsn = decoder.decode();
    }

    if( !find || !lastCall.isValid() ) {
        logLine("heuristic for finding global destructor function failed\n");
        return NULL;
    }

    Address callAddress = finiRegion->getMemOffset() + bytesSeen;

    RegisterAST thePC = RegisterAST(
        Dyninst::MachRegister::getPC(parse_img()->codeObject()->cs()->getArch()));

    Expression::Ptr callTarget = lastCall.getControlFlowTarget();
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

