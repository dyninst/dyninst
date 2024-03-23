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

#ifndef _BPatch_loopTreeNode_h_
#define _BPatch_loopTreeNode_h_

#include <map>
#include <string>
#include "BPatch_dll.h"
#include "BPatch_Vector.h"
#include "BPatch_function.h"



class BPatch_basicBlockLoop;
class func_instance;
class BPatch_flowGraph;

namespace Dyninst{
namespace PatchAPI{
class PatchLoop;
class PatchLoopTreeNode;
}
}

class BPATCH_DLL_EXPORT BPatch_loopTreeNode {
    friend class BPatch_flowGraph;

 public:
    BPatch_basicBlockLoop *loop;

    BPatch_Vector<BPatch_loopTreeNode *> children;

    BPatch_loopTreeNode(BPatch_flowGraph*, 
                        Dyninst::PatchAPI::PatchLoopTreeNode*, 
			std::map<Dyninst::PatchAPI::PatchLoop*, BPatch_basicBlockLoop*>&);

    ~BPatch_loopTreeNode();

    const char * name(); 

    std::string getCalleeName(unsigned int i);

    unsigned int numCallees();

    bool getCallees(BPatch_Vector<BPatch_function *> &v, BPatch_addressSpace *p);
    
    BPatch_basicBlockLoop * findLoop(const char *name);

 private:

    char *hierarchicalName;

    BPatch_Vector<func_instance *> callees;

};


#endif /* _BPatch_loopTreeNode_h_ */
