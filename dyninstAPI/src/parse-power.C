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

#include "common/src/Vector.h"
#include <unordered_map>
#include "common/src/Vector.h"
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

static const std::string LIBC_CTOR_HANDLER("__do_global_ctors_aux");
static const std::string LIBC_DTOR_HANDLER("__do_global_dtors_aux");
static const std::string DYNINST_CTOR_HANDLER("DYNINSTglobal_ctors_handler");
static const std::string DYNINST_CTOR_LIST("DYNINSTctors_addr");
static const std::string DYNINST_DTOR_HANDLER("DYNINSTglobal_dtors_handler");
static const std::string DYNINST_DTOR_LIST("DYNINSTdtors_addr");
static const std::string SYMTAB_CTOR_LIST_REL("__SYMTABAPI_CTOR_LIST__");
static const std::string SYMTAB_DTOR_LIST_REL("__SYMTABAPI_DTOR_LIST__");

static bool replaceHandler(func_instance *origHandler, func_instance *newHandler, 
        int_symbol *newList, const std::string &listRelName)
{
    // Add instrumentation to replace the function
   // TODO: this should be a function replacement!
   // And why the hell is it in parse-x86.C?
   origHandler->proc()->replaceFunction(origHandler, newHandler);
   // origHandler->proc()->relocate();
    /* PatchAPI stuffs */
   AddressSpace::patch(origHandler->proc());
    /* End of PatchAPI stuffs */
    
    /* create the special relocation for the new list -- search the RT library for
     * the symbol
     */
    Symbol *newListSym = const_cast<Symbol *>(newList->sym());

    std::vector<Region *> allRegions;
    if( !newListSym->getSymtab()->getAllRegions(allRegions) ) {
        return false;
    }

    bool success = false;
    std::vector<Region *>::iterator reg_it;
    for(reg_it = allRegions.begin(); reg_it != allRegions.end(); ++reg_it) {
        std::vector<relocationEntry> &region_rels = (*reg_it)->getRelocations();
        vector<relocationEntry>::iterator rel_it;
        for( rel_it = region_rels.begin(); rel_it != region_rels.end(); ++rel_it) {
            if( rel_it->getDynSym() == newListSym ) {
                relocationEntry *rel = &(*rel_it);
                rel->setName(listRelName);
                success = true;
            }
        }
    }

    return success;
}
/*
By parsing the function that actually sets up the parameters for the OMP
region we discover informations such as what type of parallel region we're
dealing with */
bool parse_func::parseOMPParent(image_parRegion * /*iPar*/, int /*desiredNum*/, int & /*currentSectionNum*/ )
{
#if 0 //!defined(cap_instruction_api)
   Address funcBegin = getOffset();
   InstrucIter ah(funcBegin, this);
   InstrucIter callFind(funcBegin, this);
   int currentNum = 0;
   int regValues[10 + 1];  /* Only care about registers 3-10 (params) */
   Address regWriteLocations[10 + 1];

   for (int i = 0; i < 11; i++)
      regValues[i] = -1;
  
  
   const char * regionFuncName = iPar->getAssociatedFunc()->symTabName().c_str();

   while (callFind.hasMore())
   {
      if( callFind.isACallInstruction() ||
          callFind.isADynamicCallInstruction() )
      {
         bool isAbsolute = false;
         Address target = callFind.getBranchTargetAddress(&isAbsolute);
         image * im = img();

         parse_func *ppdf = im->findFuncByEntry(target);

         if (ppdf != NULL)
         {
            if (strstr(ppdf->symTabName().c_str(),regionFuncName)!=NULL)
               return 0;
         }
      }
      callFind++;
   }

   while (ah.hasMore())
   {
      if( ah.isRegConstantAssignment(regValues, regWriteLocations) ) /* Record param values */
      {	}
      else if( ah.isACallInstruction() ||
               ah.isADynamicCallInstruction() )
      {
	  
         bool isAbsolute = false;
         Address target = ah.getBranchTargetAddress(&isAbsolute);
	  
	  
         /* Finding Out if the call is to OpenMP Functions */
	  
         /* Return one of the following 
            OMP_PARALLEL, OMP_DO_FOR, OMP_SECTIONS, OMP_SINGLE, 
            OMP_PAR_DO, OMP_PAR_SECTIONS, OMP_MASTER, OMP_CRITICAL,
            OMP_BARRIER, OMP_ATOMIC, OMP_FLUSH, OMP_ORDERED */
         image * im = img();
         parse_func *ppdf = im->findFuncByEntry(target);
	  
         if (ppdf != NULL)
         {
            if (strstr(ppdf->symTabName().c_str(),"_TPO_linkage")!=NULL
                && ( strstr(ppdf->symTabName().c_str(), "ParRegionSetup")!= NULL
                     || strstr(ppdf->symTabName().c_str(), "WSDoSetup")     != NULL
                     || strstr(ppdf->symTabName().c_str(), "WSSectSetup")   != NULL
                     || strstr(ppdf->symTabName().c_str(), "SingleSetup")   != NULL		   
                     || strstr(ppdf->symTabName().c_str(), "ParallelDoSetup")!=NULL		
                     || strstr(ppdf->symTabName().c_str(), "WSSectSetup")   != NULL))		    
            {

               if (currentNum != desiredNum)
                  currentNum++;
               else
               {
                  /* Standard outlined function */
                  if (strstr(ppdf->symTabName().c_str(), "ParRegionSetup")!=NULL)
                     iPar->setRegionType(OMP_PARALLEL);
		      
                  /* Standard outlined function */
                  else if(strstr(ppdf->symTabName().c_str(), "WSDoSetup")!=NULL)
                  {			
                     iPar->setRegionType(OMP_DO_FOR);
                     iPar->setClause("NUM_ITERATIONS",regValues[6]); 
                     iPar->setClause("SCHEDULE",regValues[7]);
                     iPar->setClause("CHUNK_SIZE", regValues[8]);
                     iPar->setClauseLoc("CHUNK_SIZE", regWriteLocations[8]);
                     iPar->setClauseLoc("SCHEDULE", regWriteLocations[7]);
                  }
		      
                  /* Standard outlined function */
                  else if(strstr(ppdf->symTabName().c_str(), "WSSectSetup")!=NULL)
                  {
                     iPar->setRegionType(OMP_SECTIONS);
                     iPar->setClause("NUM_SECTIONS",regValues[5]);
			  
                     currentSectionNum++;
			  
                     if (currentSectionNum == regValues[5])
                        currentSectionNum = 0;
                  }
		      
                  /* Standard outlined function */
                  else if(strstr(ppdf->symTabName().c_str(), "SingleSetup")!=NULL)		    
                     iPar->setRegionType(OMP_SINGLE);
		      
                  /* Standard outlined function */
                  else if(strstr(ppdf->symTabName().c_str(), "ParallelDoSetup")!=NULL)
                  {		    
                     iPar->setRegionType(OMP_PAR_DO);
                     iPar->setClause("NUM_ITERATIONS",regValues[6]); 
                     iPar->setClause("SCHEDULE",regValues[7]);
                     iPar->setClause("CHUNK_SIZE", regValues[8]);
                     iPar->setClauseLoc("CHUNK_SIZE", regWriteLocations[8]);
                     iPar->setClauseLoc("SCHEDULE", regWriteLocations[7]);
                  }
                  /* Standard outlined function */
                  else if(strstr(ppdf->symTabName().c_str(), "WSSectSetup")!=NULL)		    
                     iPar->setRegionType(OMP_PAR_SECTIONS);
                  else		      
                  {
                     ah++;
                     continue;
			  
                  }/* End Checking Different Directive Types */

                  iPar->decodeClauses(regValues[3]);
                  iPar->setParentFunc(this);

		  parRegionsList.push_back(iPar);

                  if (iPar->getRegionType() == OMP_DO_FOR || 
                      iPar->getRegionType() == OMP_PAR_DO)
                     return true;
                  else
                     return false;
		    
               }		
            } 
         } 
      }
      ah++;
   }
   return true;
#else
//#warning "convert to IAPI!"
    return false;
#endif
}



	
std::string parse_func::calcParentFunc(const parse_func * imf,
                                    pdvector<image_parRegion *> &/*pR*/)
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
      char tempBuf[strlen(nameStart)];
      strncpy(tempBuf, nameStart, strSize);
      tempBuf[strSize] = '\0';
      std::string tempPDS(tempBuf);
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
  
   /* We parse the parent to get info if we are in an outlined function, but there can be some
      inlined functions we might miss out on if we don't check those out too */
   int regValues[10 + 1];  /* Only care about registers 3-10 (params) */
   for (int i = 0; i < 11; i++)
      regValues[i] = -1;
  
#if 0//!defined(cap_instruction_api)
   Address funcBegin = getOffset();
   InstrucIter ah(funcBegin, this);
   while (ah.hasMore())
   {
      if( /*ah.isRegConstantAssignment(regValues)*/ 0 ) /* Record param values */
      {	}
      // Loop parsing for the Do/For constructs
      else if( hasLoop && ah.isACondBDZInstruction())
      {
         InstrucIter ah2(ah.getCurrentAddress(), this);
	  
         Address startLoop = ah.getCurrentAddress() + 4;
	  
         while (ah2.hasMore())
         {
            if (ah2.isACondBDNInstruction())
            {
                addParRegion(startLoop, ah2.getCurrentAddress(), OMP_DO_FOR_LOOP_BODY);
               break;
            }
            ah2++;
         }
      }
      // Here we get all the info for the inlined constructs that don't have outlined functions
      else if( ah.isACallInstruction() ||
               ah.isADynamicCallInstruction() )
      {
         bool isAbsolute = false;
         Address target = ah.getBranchTargetAddress(&isAbsolute);
	  
	  
         /* Finding Out if the call is to OpenMP Functions */
	  
         /* Return one of the following 
            OMP_PARALLEL, OMP_DO_FOR, OMP_SECTIONS, OMP_SINGLE, 
            OMP_PAR_DO, OMP_PAR_SECTIONS, OMP_MASTER, OMP_CRITICAL,
            OMP_BARRIER, OMP_ATOMIC, OMP_FLUSH, OMP_ORDERED */
         image * im = img();
         parse_func *ppdf = im->findFuncByEntry(target);
         if (ppdf != NULL)
         {
            if (strstr(ppdf->symTabName().c_str(),"_xlsmp")!=NULL)
            {
               /* Section consists of only one instruction, call to "_xlsmpBarrier_TPO" */
               if(strstr(ppdf->symTabName().c_str(), "Barrier")!=NULL)
               {
                   addParRegion(ah.getCurrentAddress(), ah.getCurrentAddress()+4, OMP_BARRIER);
               }
               /* Section begins with "BeginOrdered, ends with EndOrdered" */
               else if(strstr(ppdf->symTabName().c_str(), "BeginOrdered") !=NULL)
               {
		      
                  InstrucIter ah2(ah.getCurrentAddress(), this);
                  while (ah2.hasMore())
                  {
                     if( ah2.isACallInstruction() ||
                         ah2.isADynamicCallInstruction() )
                     {
                        Address target2 = ah2.getBranchTargetAddress(&isAbsolute);
			      
                        parse_func *ppdf2 = im->findFuncByEntry(target2);
                        if (ppdf2 != NULL)
                        {
                           if(strstr(ppdf2->symTabName().c_str(), "EndOrdered") !=NULL)
                              break;
                        }
                     }
                     ah2++;
                  }
                  addParRegion(ah.getCurrentAddress(), ah2.getCurrentAddress(), OMP_ORDERED);
		  parRegionsList.push_back(iPar);
               }
               /* Master construct */
               else if(strstr(ppdf->symTabName().c_str(), "Master") !=NULL)
               {
                   addParRegion(ah.getCurrentAddress(), ah.getCurrentAddress() + 0x04, OMP_MASTER);
               }
               /* Flush construct */
               else if(strstr(ppdf->symTabName().c_str(), "Flush") !=NULL)
               {
                   addParRegion(ah.getCurrentAddress(), ah.getCurrentAddress() + 0x04, OMP_FLUSH);
               }
               /* Critical Construct, Starts with GetDefaultSLock, ends with RelDefaultSLock */
               else if(strstr(ppdf->symTabName().c_str(), "GetDefaultSLock") != NULL)
               {
                  InstrucIter ah2(ah.getCurrentAddress(), this);
                  while (ah2.hasMore())
                  {
                     if( ah2.isACallInstruction() ||
                         ah2.isADynamicCallInstruction() )
                     {
                        Address target2 = ah2.getBranchTargetAddress(&isAbsolute);
			      
                        parse_func *ppdf2 = im->findFuncByEntry(target2);
                        if (ppdf2 != NULL)
                        {
                           if(strstr(ppdf2->symTabName().c_str(), "RelDefaultSLock") !=NULL)
                              break;
                        }
                     }
                     ah2++;
                  }
                  addParRegion(ah.getCurrentAddress(), ah2.getCurrentAddress(), OMP_CRITICAL);
               }
               /*Atomic Construct,  Begins with GetAtomicLock, ends with RelAtomicLock */
               else if(strstr(ppdf->symTabName().c_str(), "GetAtomicLock") != NULL)
               {
                  image_parRegion * iPar = new image_parRegion(ah.getCurrentAddress(),this);
                  iPar->setRegionType(OMP_ATOMIC);

                  InstrucIter ah2(ah.getCurrentAddress(), this);
                  while (ah2.hasMore())
                  {
                     if( ah2.isACallInstruction() ||
                         ah2.isADynamicCallInstruction() )
                     {
                        Address target2 = ah2.getBranchTargetAddress(&isAbsolute);
			      
                        parse_func *ppdf2 = im->findFuncByEntry(target2);
                        if (ppdf2 != NULL)
                        {
                           if(strstr(ppdf2->symTabName().c_str(), "RelDefaultSLock") !=NULL)
                              break;
                        }
                     }
                     ah2++;
                  }
                  iPar->setLastInsn(ah2.getCurrentAddress());
		      
                  iPar->setParentFunc(this); // when not outlined, parent func will be same as regular
                  iPar->setLastInsn(ah.getCurrentAddress() + 0x4); //Only one instruction long
		      
		  parRegionsList.push_back(iPar);
               }
               else
               {
               }/* End Checking Different Directive Types */
		 
            }
         }
      }
      ah++;
   }
#endif
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
        Instruction::Ptr i;
        while(i = d.decode())
        {
            i->getWriteSet(writtenRegs);
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
 * Some of the following functions replace the standard ctor and dtor handlers
 * in a binary. Currently, these operations only work with binaries linked with
 * the GNU toolchain. However, it should be straightforward to extend these
 * operations to other toolchains.
 */

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
         fprintf (stderr,"failed to find ctors list symbol\n");
         return false;
    }

    if( !dtorFound ) {
        logLine("failed to find dtors list symbol\n");
        fprintf (stderr,"failed to find dtors list symbol\n");
        return false;
    }

    /*
     * Replace the libc ctor and dtor handlers with our special handlers
     */

    if( !replaceHandler(globalCtorHandler, dyninstCtorHandler,
                &ctorsListInt, SYMTAB_CTOR_LIST_REL) ) {
        logLine("Failed to replace libc ctor handler with special handler");
        fprintf (stderr,"Failed to replace libc ctor handler with special handler");
        return false;
    }else{
        inst_printf("%s[%d]: replaced ctor function %s with %s\n",
                FILE__, __LINE__, LIBC_CTOR_HANDLER.c_str(),
                DYNINST_CTOR_HANDLER.c_str());
    }

    if( !replaceHandler(globalDtorHandler, dyninstDtorHandler,
                &dtorsListInt, SYMTAB_DTOR_LIST_REL) ) {
        logLine("Failed to replace libc dtor handler with special handler");
        fprintf (stderr,"Failed to replace libc dtor handler with special handler");
        return false;
    }else{
        inst_printf("%s[%d]: replaced dtor function %s with %s\n",
                FILE__, __LINE__, LIBC_DTOR_HANDLER.c_str(),
                DYNINST_DTOR_HANDLER.c_str());
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
        std::map<std::string, BinaryEdit *>::iterator bedit_it;
        for(bedit_it = res.begin(); bedit_it != res.end(); ++bedit_it) {
            if( bedit_it->second == NULL ) {
                logLine("Failed to load DyninstAPI_RT library dependency (libc.a)");
                fprintf (stderr,"Failed to load DyninstAPI_RT library dependency (libc.a)");
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

