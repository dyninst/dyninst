/*
 * Copyright (c) 1996-2011 Barton P. Miller
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

// $Id: image-power.C,v 1.23 2008/03/12 20:09:10 legendre Exp $

// Determine if the called function is a "library" function or a "user" function
// This cannot be done until all of the functions have been seen, verified, and
// classified
//

#include "common/h/Vector.h"
#include "common/h/Dictionary.h"
#include "common/h/Vector.h"
#include "image-func.h"
#include "instPoint.h"
#include "symtab.h"
#include "parRegion.h"
#include "debug.h"

#include "common/h/arch.h"

#if !defined(cap_instruction_api)
#include "parseAPI/h/InstrucIter.h"
#endif

/*
By parsing the function that actually sets up the parameters for the OMP
region we discover informations such as what type of parallel region we're
dealing with */
bool image_func::parseOMPParent(image_parRegion * iPar, int desiredNum, int & currentSectionNum )
{
#if !defined(cap_instruction_api)
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

         image_func *ppdf = im->findFuncByEntry(target);

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
         image_func *ppdf = im->findFuncByEntry(target);
	  
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
#warning "convert to IAPI!"
    return false;
#endif
}



	
std::string image_func::calcParentFunc(const image_func * imf,
                                    pdvector<image_parRegion *> &/*pR*/)
{
  /* We need to figure out the function that called the outlined
     parallel region function.  We do this by chopping off the
     last @OL@number */
   const char * nameStart = imf->prettyName().c_str();
   char * nameEnd = strrchr(nameStart, '@');
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


void image_func::parseOMP(image_parRegion * parReg, image_func * parentFunc, int & currentSectionNum)
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

void image_func::parseOMPFunc(bool hasLoop)
{
   if (OMPparsed_)
      return;
   OMPparsed_ = true;
  
   /* We parse the parent to get info if we are in an outlined function, but there can be some
      inlined functions we might miss out on if we don't check those out too */
   int regValues[10 + 1];  /* Only care about registers 3-10 (params) */
   for (int i = 0; i < 11; i++)
      regValues[i] = -1;
  
#if !defined(cap_instruction_api)
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
         image_func *ppdf = im->findFuncByEntry(target);
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
			      
                        image_func *ppdf2 = im->findFuncByEntry(target2);
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
			      
                        image_func *ppdf2 = im->findFuncByEntry(target2);
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
			      
                        image_func *ppdf2 = im->findFuncByEntry(target2);
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
void image_func::calcUsedRegs()
{
   if (usedRegisters != NULL)
      return; 
   else
   {
      usedRegisters = new image_func_registers();
    using namespace Dyninst::InstructionAPI;
    std::set<RegisterAST::Ptr> writtenRegs;

    Function::blocklist & bl = blocks();
    Function::blocklist::iterator curBlock = bl.begin();
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
        if((r & ppc32::GPR) && (r <= ppc32::r12))
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
