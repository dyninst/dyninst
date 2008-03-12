/*
 * Copyright (c) 1996-2004 Barton P. Miller
 * 
 * We provide the Paradyn Parallel Performance Tools (below
 * described as "Paradyn") on an AS IS basis, and do not warrant its
 * validity or performance.  We reserve the right to update, modify,
 * or discontinue this software at any time.  We shall have no
 * obligation to supply such updates or modifications or any other
 * form of support to you.
 * 
 * This license is for research uses.  For such uses, there is no
 * charge. We define "research use" to mean you may freely use it
 * inside your organization for whatever purposes you see fit. But you
 * may not re-distribute Paradyn or parts of Paradyn, in any form
 * source or binary (including derivatives), electronic or otherwise,
 * to any other organization or entity without our permission.
 * 
 * (for other uses, please contact us at paradyn@cs.wisc.edu)
 * 
 * All warranties, including without limitation, any warranty of
 * merchantability or fitness for a particular purpose, are hereby
 * excluded.
 * 
 * By your use of Paradyn, you understand and agree that we (or any
 * other person or entity with proprietary rights in Paradyn) are
 * under no obligation to provide either maintenance services,
 * update services, notices of latent defects, or correction of
 * defects for Paradyn.
 * 
 * Even if advised of the possibility of such damages, under no
 * circumstances shall we (or any other person or entity with
 * proprietary rights in the software licensed hereunder) be liable
 * to you or any third party for direct, indirect, or consequential
 * damages of any character regardless of type of action, including,
 * without limitation, loss of profits, loss of use, loss of good
 * will, or computer failure or malfunction.  You agree to indemnify
 * us (and any other person or entity with proprietary rights in the
 * software licensed hereunder) for any and all liability it may
 * incur to third parties resulting from your use of Paradyn.
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
#include "dyninstAPI/h/BPatch_Set.h"
#include "InstrucIter.h"
#include "debug.h"
#include "arch.h"

// Not used on power
bool image_func::archIsRealCall(InstrucIter & /* ah */,
                bool &/*validTarget*/,
                bool & /*simulateJump*/)
{   
    return true;
}

bool image_func::archCheckEntry(InstrucIter &ah, image_func * /* func */)
{   
    // XXX Cheating a little -- this has nothing to do with the
    // "check entry" as seen on x86 & sparc, but is just a convenient place
    // to put this code.

    parsing_printf("calling archCheckEntry for 0x%lx, function %s\n", *ah, symTabName().c_str());

    if (ah.isReturnValueSave())
        makesNoCalls_ = false;
    else
        makesNoCalls_ = true;

    // end cheating

    if (!ah.getInstruction().valid()) return false;

    // We see if we're a procedure linkage table; if so, we are _not_
    // a function (and return false)

    // We don't consider linkage snippets "functions". 
    /*
    Address dontcare1;
    if (ah.isInterModuleCallSnippet(dontcare1))
      {
	return false;
      }
    */
    return true;
}

// Not used on power
bool image_func::archIsUnparseable()
{   
    return false;
}

// Not used on power
bool image_func::archAvoidParsing()
{   
    return false;
}

void image_func::archGetFuncEntryAddr(Address & /* funcEntryAddr */)
{   
    return;
}

// Not used on power
bool image_func::archNoRelocate()
{   
    return false;
}

// Not used on power
void image_func::archSetFrameSize(int /* fsize */)                  
{
    return;
}

// As Drew has noted, this really, really should not be an InstructIter
// operation. The extraneous arguments support architectures like x86,
// which (rightly) treat jump table processing as a control-sensitive
// data flow operation.
bool image_func::archGetMultipleJumpTargets(
                                BPatch_Set< Address >& targets,
                                image_basicBlock * /* currBlk */,
                                InstrucIter &ah,
                                pdvector< instruction >& /* allInstructions */)
{   
    return ah.getMultipleJumpTargets( targets );
}

// not implemented on power
bool image_func::archProcExceptionBlock(Address & /* catchStart */, Address /* a */)
{   
    // Agnostic about exception blocks; the policy of champions
    return false;
}

// not implemented on power
bool image_func::archIsATailCall(InstrucIter & /* ah */,
                                 pdvector< instruction >& /* allInstructions */)
{   
    return false;
}

// not implemented on power
bool image_func::archIsIndirectTailCall(InstrucIter & /* ah */)
{   
    return false;
}

bool image_func::archIsAbortOrInvalid(InstrucIter &ah)
{
    return ah.isAnAbortInstruction();
}

// not implemented on power
void image_func::archInstructionProc(InstrucIter & /* ah */)
{
    return;
}

/*
By parsing the function that actually sets up the parameters for the OMP
region we discover informations such as what type of parallel region we're
dealing with */
bool image_func::parseOMPParent(image_parRegion * iPar, int desiredNum, int & currentSectionNum )
{
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
	  
         // XXX move this out of archIsRealCall protection... safe?
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
   parReg->setLastInsn(get_address() + get_size());
   
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
   Address funcBegin = getOffset();
   InstrucIter ah(funcBegin, this);
   int regValues[10 + 1];  /* Only care about registers 3-10 (params) */
   for (int i = 0; i < 11; i++)
      regValues[i] = -1;
  
  
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
               image_parRegion * iPar = new image_parRegion(startLoop,this);
               iPar->setRegionType(OMP_DO_FOR_LOOP_BODY);
               iPar->setParentFunc(this); // when not outlined, parent func will be same as regular
               Address endLoop = ah2.getCurrentAddress();
               iPar->setLastInsn(endLoop);
	       parRegionsList.push_back(iPar);
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
                  image_parRegion * iPar = new image_parRegion(ah.getCurrentAddress(),this);
                  iPar->setRegionType(OMP_BARRIER);
		      
                  iPar->setParentFunc(this); // when not outlined, parent func will be same as regular
                  iPar->setLastInsn(ah.getCurrentAddress() + 0x4); //Only one instruction long
		      
		  parRegionsList.push_back(iPar);
               }
               /* Section begins with "BeginOrdered, ends with EndOrdered" */
               else if(strstr(ppdf->symTabName().c_str(), "BeginOrdered") !=NULL)
               {
                  image_parRegion * iPar = new image_parRegion(ah.getCurrentAddress(),this);
                  iPar->setRegionType(OMP_ORDERED);
		      
                  iPar->setParentFunc(this); // when not outlined, parent func will be same as regular
		      
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
                  iPar->setLastInsn(ah2.getCurrentAddress());
		  parRegionsList.push_back(iPar);
               }
               /* Master construct */
               else if(strstr(ppdf->symTabName().c_str(), "Master") !=NULL)
               {
                  image_parRegion * iPar = new image_parRegion(ah.getCurrentAddress(),this);
                  iPar->setRegionType(OMP_MASTER);
		      
                  iPar->setParentFunc(this); // when not outlined, parent func will be same as regular
                  iPar->setLastInsn(ah.getCurrentAddress() + 0x4); //Only one instruction long
		      
		  parRegionsList.push_back(iPar);
               }
               /* Flush construct */
               else if(strstr(ppdf->symTabName().c_str(), "Flush") !=NULL)
               {
                  image_parRegion * iPar = new image_parRegion(ah.getCurrentAddress(),this);
                  iPar->setRegionType(OMP_FLUSH);
		      
                  iPar->setParentFunc(this); // when not outlined, parent func will be same as regular
                  iPar->setLastInsn(ah.getCurrentAddress() + 0x4); //Only one instruction long
		      
		  parRegionsList.push_back(iPar);
               }
               /* Critical Construct, Starts with GetDefaultSLock, ends with RelDefaultSLock */
               else if(strstr(ppdf->symTabName().c_str(), "GetDefaultSLock") != NULL)
               {
                  image_parRegion * iPar = new image_parRegion(ah.getCurrentAddress(),this);
                  iPar->setRegionType(OMP_CRITICAL);
		      
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
		      
		  parRegionsList.push_back(iPar);
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
      InstrucIter ah(this);
      
      //while there are still instructions to check for in the
      //address space of the function      
      while (ah.hasMore()) 
      {
         if (ah.isA_RT_WriteInstruction())
            if (ah.getRTValue() >= 3 && ah.getRTValue() <= 12)
               usedRegisters->generalPurposeRegisters.insert(ah.getRTValue());
         if (ah.isA_RA_WriteInstruction())
            if (ah.getRAValue() >= 3 && ah.getRAValue() <= 12)
               usedRegisters->generalPurposeRegisters.insert(ah.getRAValue());
         if (ah.isA_FRT_WriteInstruction())
            if (ah.getRTValue() <= 13)
               usedRegisters->floatingPointRegisters.insert(ah.getRTValue());
         if (ah.isA_FRA_WriteInstruction())
            if (ah.getRAValue() <= 13)
               usedRegisters->floatingPointRegisters.insert(ah.getRAValue());
         ah++;
      }
   }
   return;
}
