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

#include "common/src/arch-aarch64.h"

#include "instructionAPI/h/Instruction.h"
#include "instructionAPI/h/InstructionDecoder.h"

#include "mapped_object.h"
#include "binaryEdit.h"
#include "addressSpace.h"
#include "function.h"
#include "baseTramp.h"
#include "RegisterConversion.h"
#include "registerSpace.h"

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

        for(auto const& reg : writtenRegs) {
          MachRegister r = reg->getID();
          auto regID = convertRegID(r.getBaseRegister());
          if(regID == registerSpace::ignored) {
            logLine("parse_func::calcUsedRegs: unknown written register\n");
            continue;
          }
          if(r.isGeneralPurpose()) {
            usedRegisters->generalPurposeRegisters.insert(regID);
          }
          else if(r.isFloatingPoint()) {
            usedRegisters->floatingPointRegisters.insert(regID);
          }
        }
    }
    return;
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

