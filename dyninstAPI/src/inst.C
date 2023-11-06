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

// $Id: inst.C,v 1.166 2008/06/26 20:40:15 bill Exp $
// Code to install and remove instrumentation from a running process.
// Misc constructs.

#include <assert.h>
#include "dyninstAPI/src/image.h"
#include "dyninstAPI/src/inst.h"
#include "dyninstAPI/src/instP.h"
#include "dyninstAPI/src/ast.h"
#include "dyninstAPI/src/util.h"
#include "common/src/stats.h"
#include "dyninstAPI/src/debug.h"
#include "dyninstAPI/src/instPoint.h"
#include "dyninstAPI/src/baseTramp.h"
#include "dyninstAPI/src/function.h"
#include "dyninstAPI/src/parse-cfg.h"
#include "dyninstAPI/src/codegen.h"
#include "dyninstAPI/src/addressSpace.h"

using namespace Dyninst;
using namespace PatchAPI;

/*
 * return the time required to execute the passed primitive.
 *
 */
std::map<std::string, unsigned> primitiveCosts;

unsigned getPrimitiveCost(const std::string &name)
{
   std::map<string, unsigned>::iterator iter = primitiveCosts.find(name);
   if (iter != primitiveCosts.end()) return iter->second;
   return 1;
}


// find any tags to associate semantic meaning to function
unsigned findTags(const std::string ) {
  return 0;
#ifdef notdef
  if (tagDict.defines(funcName))
    return (tagDict[funcName]);
  else
    return 0;
#endif
}

unsigned generateAndWriteBranch(AddressSpace *proc, 
                                Dyninst::Address fromAddr,
                                Dyninst::Address newAddr,
                                unsigned fillSize)
{
    assert(fillSize != 0);

    codeGen gen(fillSize);

    insnCodeGen::generateBranch(gen, fromAddr, newAddr);
    gen.fillRemaining(codeGen::cgNOP);
    
    proc->writeTextSpace((void*)(fromAddr), gen.used(), gen.start_ptr());
    return gen.used();
}

instMapping::instMapping(const instMapping *parIM,
                         AddressSpace *child) :
    func(parIM->func),
    inst(parIM->inst),
    where(parIM->where),
    when(parIM->when),
    order(parIM->order),
    useTrampGuard(parIM->useTrampGuard),
    mt_only(parIM->mt_only),
    allow_trap(parIM->allow_trap)
{
    for (unsigned i = 0; i < parIM->args.size(); i++) {
        args.push_back(parIM->args[i]);
    }
    for (unsigned j = 0; j < parIM->instances.size(); j++) {
       InstancePtr cMT = getChildInstance(parIM->instances[j], child);
       assert(cMT);
       instances.push_back(cMT);
    }
}

