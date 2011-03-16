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


// $Id: image-sparc.C,v 1.20 2008/06/19 19:53:20 legendre Exp $

#include "common/h/Vector.h"
#include "common/h/Dictionary.h"
#include "common/h/Vector.h"
#include "common/h/arch.h"

#include "image-func.h"
#include "instPoint.h"
#include "symtab.h"
#include "debug.h"
#include "inst-sparc.h" // REG_? should be in arch-sparc, but isn't

#include "parseAPI/src/InstrucIter.h"
/*
bool image_func::archIsUnparseable()
{
    // And here we have hackage. Our jumptable code is b0rken, but I don't know
    // how to fix it. So we define anything that doesn't work as... argh.
    // Better: a size limit on a jump table.
    if (symTabName().c_str() == "__rtboot") {
        return true;
    }

    return false;
}
*/

std::string image_func::calcParentFunc(const image_func * imf, pdvector<image_parRegion *> & pR)
{
  /* We need to figure out the function that called the outlined
     parallel region function.  

     We do this by chopping off the "_$<identifier>." prefix
  */

  const char * nameStart = imf->prettyName().c_str();
  char * newNameStart = strrchr(nameStart, '.');
  newNameStart++;
  
  const char * nameEnd = nameStart + strlen(nameStart);
  
  int strSize = nameEnd - newNameStart;
  char tempBuf[strSize + 1];
  strncpy(tempBuf, newNameStart, strSize);
  tempBuf[strSize] = '\0';


  /* These two regions have associated functions that have the _$p beginning */
  if (strstr(imf->prettyName().c_str(),"_$s") != NULL ||
      strstr(imf->prettyName().c_str(),"_$d") != NULL )
    {
      /* We need to find the associated function */

      for (unsigned i = 0; i < pR.size(); i++)
	{
	  image_parRegion * tempParReg = pR[i];
          image_func * imf = const_cast<image_func*>(tempParReg->getAssociatedFunc());

	  const char * nameStart2 = imf->prettyName().c_str();
	  
	  	  
	  if (strstr(nameStart2,tempBuf) != NULL &&
	      strstr(nameStart2, "_$p") != NULL)
	    {
	      char * endPtr = strstr(nameStart2, tempBuf);
	      int strSize2 = endPtr - nameStart2;
	      int totalStrSize = strSize2 + strlen(tempBuf);
	      char tempBuf2[totalStrSize + 1];
	      strncpy(tempBuf2, nameStart2, strSize2);
	      strncpy(tempBuf2 + strSize2, tempBuf,sizeof(tempBuf));
	      tempBuf2[totalStrSize] = '\0';
	     
	      std::string tempPDS(tempBuf2);
	      return tempPDS;
	    }
	}
      std::string tempPDS(tempBuf);
      return tempPDS;
    }
  else if (strstr(imf->prettyName().c_str(),"_$p") != NULL)
    {
      std::string tempPDS(tempBuf);
      return tempPDS;
    }
  else
    {
      return NULL;
    }
}

void image_func::parseOMP(image_parRegion * parReg, image_func * parentFunc, int & currentSectionNum)
{
  /* Parsing section functions */
  if (strstr(symTabName().c_str(),"_$s")!=NULL)
    {
      parseOMPSectFunc(parentFunc);
      parseOMPFunc(false);
      parentFunc->parseOMPParent(parReg, 0, currentSectionNum);
    }
  /* Parsing workshare functions */
  else if (strstr(symTabName().c_str(),"_$d")!=NULL)
    {
      parseOMPFunc(true);
      parentFunc->parseOMPParent(parReg, 0, currentSectionNum);
    }
  /* Parsing parallel regions */
  else if (strstr(symTabName().c_str(),"_$p") !=NULL)
    {
      parseOMPFunc(false);
      parentFunc->parseOMPParent(parReg, 0, currentSectionNum);
    }
  parReg->setLastInsn(getEndOffset());
}

// This parses the parent functions that generated outlined do/for, parallel constructs */
bool image_func::parseOMPParent(image_parRegion * iPar, 
                                int /*desiredNum*/, int & /*currentSectionNum*/)
{
   Address funcBegin = getOffset();
   InstrucIter ah(funcBegin, isrc());

   bool isDoFor = false;
   while (ah.hasMore())
   {

      if( ah.isAOMPDoFor() ) /* Record param values */
      {
         isDoFor = true;
      }
      if( ah.isACallInstruction() ||
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

            if (strstr(ppdf->symTabName().c_str(),"__mt_MasterFunction")!=NULL)
            {
               if (isDoFor)
               {
                  iPar->setRegionType(OMP_PAR_DO);
                  iPar->setParentFunc(this);
                  parRegionsList.push_back(iPar);
               }
               else
               {
                  iPar->setRegionType(OMP_PARALLEL);
                  iPar->setParentFunc(this);
                  parRegionsList.push_back(iPar);
               }
               return false;
            }
            else if(strstr(ppdf->symTabName().c_str(),"__mt_WorkSharing_")!=NULL)
            {
               if (isDoFor)
               {
                  iPar->setRegionType(OMP_DO_FOR);
                  iPar->setParentFunc(this);
                  parRegionsList.push_back(iPar);
                  //return;
               }
               return true;
            }
         }
      }
      ah++;
   }
   return true;
}

// Parses through all the inlined sections within one outlined function for entire section construct

void image_func::parseOMPSectFunc(image_func * parentFunc)
{
   Address funcBegin = getOffset();
   InstrucIter ah(funcBegin,getEndOffset(),  isrc());

   Address sectBegin = funcBegin;
   bool inSection = false;

   while (ah.hasMore())
   {
      if (ah.isACondBranchInstruction())
      {
         if (inSection == false)
            sectBegin = ah.getCurrentAddress();
         inSection = true;
      }
      if (ah.isAReturnInstruction())
      {
         image_parRegion * iPar = new image_parRegion(sectBegin,this);
         iPar->setRegionType(OMP_SECTIONS);

         iPar->setParentFunc(parentFunc); 
         iPar->setLastInsn(ah.getCurrentAddress()); 
         parRegionsList.push_back(iPar);
         sectBegin = ah.getCurrentAddress() + 0x8;
      }      
      ah++;
   }
}

void image_func::parseOMPFunc(bool hasLoop)
{
   if (OMPparsed_)
      return;
   OMPparsed_ = true;

   /* We parse the parent to get info if we are in an outlined function, but there can be some
      inlined functions we might miss out on if we don't check those out too */
   Address funcBegin = getOffset();
   InstrucIter ah(funcBegin, getEndOffset(), isrc());

   while (ah.hasMore())
   {
      if ( hasLoop && ah.isACondBLEInstruction() )
      {
         if (ah.getBranchTargetAddress() < ah.getCurrentAddress())
         {
            image_parRegion * iPar = new image_parRegion(ah.getBranchTargetAddress()+4, this);				
            iPar->setRegionType(OMP_DO_FOR_LOOP_BODY);
            iPar->setParentFunc(this); // when not outlined, parent func will be same as regular
            iPar->setLastInsn(ah.getCurrentAddress());
            parRegionsList.push_back(iPar);
         }
      }
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
            if (strstr(ppdf->symTabName().c_str(),"__mt_")!=NULL)
            {
               /* Section consists of only one instruction, call to "_xlsmpBarrier_TPO" */
               if(strstr(ppdf->symTabName().c_str(), "barrier")!=NULL)
               {
                  image_parRegion * iPar = new image_parRegion(ah.getCurrentAddress(),this);
                  iPar->setRegionType(OMP_BARRIER);

                  iPar->setParentFunc(this); // when not outlined, parent func will be same as regular
                  iPar->setLastInsn(ah.getCurrentAddress() + 0x4); //Only one instruction long

                  parRegionsList.push_back(iPar);
               }
               /* Section begins with "BeginOrdered, ends with EndOrdered" */
               else if(strstr(ppdf->symTabName().c_str(), "begin_ordered") !=NULL)
               {
                  image_parRegion * iPar = new image_parRegion(ah.getCurrentAddress(),this);
                  iPar->setRegionType(OMP_ORDERED);

                  iPar->setParentFunc(this); // when not outlined, parent func will be same as regular

                  InstrucIter ah2(ah.getCurrentAddress(),getEndOffset(), isrc());
                  while (ah2.hasMore())
                  {
                     if( ah2.isACallInstruction() ||
                           ah2.isADynamicCallInstruction() )
                     {
                        Address target2 = ah2.getBranchTargetAddress(&isAbsolute);

                        image_func *ppdf2 = im->findFuncByEntry(target2);
                        if (ppdf2 != NULL)
                        {
                           if(strstr(ppdf2->symTabName().c_str(), "end_ordered") !=NULL)
                              break;
                        }
                     }
                     ah2++;
                  }
                  iPar->setLastInsn(ah2.getCurrentAddress());

                  parRegionsList.push_back(iPar);
               }
               /* Section begins with "single_begin, ends with single_end" */
               else if(strstr(ppdf->symTabName().c_str(), "single_begin") !=NULL)
               {
                  image_parRegion * iPar = new image_parRegion(ah.getCurrentAddress(),this);
                  iPar->setRegionType(OMP_SINGLE);

                  iPar->setParentFunc(this); // when not outlined, parent func will be same as regular

                  InstrucIter ah2(ah.getCurrentAddress(),getEndOffset(), isrc());
                  while (ah2.hasMore())
                  {
                     if( ah2.isACallInstruction() ||
                           ah2.isADynamicCallInstruction() )
                     {
                        Address target2 = ah2.getBranchTargetAddress(&isAbsolute);

                        image_func *ppdf2 = im->findFuncByEntry(target2);
                        if (ppdf2 != NULL)
                        {
                           if(strstr(ppdf2->symTabName().c_str(), "single_end") !=NULL)
                              break;
                        }
                     }
                     ah2++;
                  }
                  iPar->setLastInsn(ah2.getCurrentAddress());

                  parRegionsList.push_back(iPar);
               }
               /* Section is precursored by a call to mt_get_thread_num followed by a nop, 
                  then a test on that thread number, followed by a bne
                  the instruction after the branch is the first instruction, the 
                  branch destination is the end of the section */
               else if(strstr(ppdf->symTabName().c_str(), "get_thread_num") !=NULL)
               {
                  InstrucIter ah2(ah.getCurrentAddress(),getEndOffset(), isrc());

                  /* This should put us at the nop */
                  ah2++;

                  /* This should put us at the tst instruction */
                  ah2++;
                  if (ah2.isTstInsn())
                  {

                     ah2++;
                     /* Now we should be at the branch, and we know this is a Master section */

                     if (ah2.isACondBranchInstruction())
                     {
                        image_parRegion * iPar = new image_parRegion(ah.getCurrentAddress(),this);
                        iPar->setRegionType(OMP_MASTER);

                        iPar->setParentFunc(this); // when not outlined, parent func will be same as regular

                        bool xx = true; // I don't really think the function uses this
                        iPar->setLastInsn(ah2.getBranchTargetAddress(&xx));

                        parRegionsList.push_back(iPar);

                     }
                  }

               }		      

               else if(strstr(ppdf->symTabName().c_str(), "flush") !=NULL)
               {
                  image_parRegion * iPar = new image_parRegion(ah.getCurrentAddress(),this);
                  iPar->setRegionType(OMP_FLUSH);

                  iPar->setParentFunc(this); // when not outlined, parent func will be same as regular
                  iPar->setLastInsn(ah.getCurrentAddress() + 0x4); //Only one instruction long

                  parRegionsList.push_back(iPar);
               }
               /* Starts with BeginCritSect, ends with EndCritSect */
               else if(strstr(ppdf->symTabName().c_str(), "BeginCritSect") != NULL)
               {
                  image_parRegion * iPar = new image_parRegion(ah.getCurrentAddress(),this);
                  iPar->setRegionType(OMP_CRITICAL);

                  InstrucIter ah2(ah.getCurrentAddress(),getEndOffset(), isrc());
                  while (ah2.hasMore())
                  {
                     if( ah2.isACallInstruction() ||
                           ah2.isADynamicCallInstruction() )
                     {
                        Address target2 = ah2.getBranchTargetAddress(&isAbsolute);

                        image_func *ppdf2 = im->findFuncByEntry(target2);
                        if (ppdf2 != NULL)
                        {
                           if(strstr(ppdf2->symTabName().c_str(), "EndCritSect") !=NULL)
                              break;
                        }
                     }
                     ah2++;
                  }
                  iPar->setLastInsn(ah2.getCurrentAddress());

                  iPar->setParentFunc(this); // when not outlined, parent func will be same as regular

                  parRegionsList.push_back(iPar);
               }
               /* Begins with b_atomic, ends with e_atomic */
               else if(strstr(ppdf->symTabName().c_str(), "b_atomic") != NULL)
               {
                  image_parRegion * iPar = new image_parRegion(ah.getCurrentAddress(),this);
                  iPar->setRegionType(OMP_ATOMIC);

                  InstrucIter ah2(ah.getCurrentAddress(),getEndOffset(), isrc());
                  while (ah2.hasMore())
                  {
                     if( ah2.isACallInstruction() ||
                           ah2.isADynamicCallInstruction() )
                     {
                        Address target2 = ah2.getBranchTargetAddress(&isAbsolute);

                        image_func *ppdf2 = im->findFuncByEntry(target2);
                        if (ppdf2 != NULL)
                        {
                           if(strstr(ppdf2->symTabName().c_str(), "e_atomic") !=NULL)
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

