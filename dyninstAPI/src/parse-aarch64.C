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

//#warning "This file is not implemented yet!"
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
	assert(0);
    return false;
}
/*
By parsing the function that actually sets up the parameters for the OMP
region we discover informations such as what type of parallel region we're
dealing with */
bool parse_func::parseOMPParent(image_parRegion * /*iPar*/, int /*desiredNum*/, int & /*currentSectionNum*/ )
{
	assert(0);
}




std::string parse_func::calcParentFunc(const parse_func * imf,
                                    pdvector<image_parRegion *> &/*pR*/)
{
	assert(0);
}


void parse_func::parseOMP(image_parRegion * parReg, parse_func * parentFunc, int & currentSectionNum)
{
	assert(0);
}

void parse_func::parseOMPFunc(bool /*hasLoop*/)
{
	assert(0);

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
	assert(0);
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
	assert(0);
    return true;
}

func_instance *mapped_object::findGlobalConstructorFunc(const std::string &ctorHandler) {
	assert(0);
    return NULL;
}

func_instance *mapped_object::findGlobalDestructorFunc(const std::string &dtorHandler) {
	assert(0);
	return NULL;
}

