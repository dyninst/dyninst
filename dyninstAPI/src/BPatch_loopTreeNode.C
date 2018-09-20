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

#define BPATCH_FILE

#include "dyninstAPI/src/util.h"
#include "dyninstAPI/src/function.h"
#include "dyninstAPI/src/BPatch_libInfo.h"
#include "dyninstAPI/h/BPatch_loopTreeNode.h"
#include "dyninstAPI/h/BPatch_basicBlockLoop.h"
#include "dyninstAPI/h/BPatch_function.h"
#include "dyninstAPI/h/BPatch_process.h"
#include "BPatch_flowGraph.h"
#include "PatchCFG.h"

using namespace Dyninst::PatchAPI;


BPatch_loopTreeNode::BPatch_loopTreeNode(BPatch_flowGraph* fg, 
                                         PatchLoopTreeNode* tree,
					 std::map<PatchLoop*, BPatch_basicBlockLoop*>& loopMap)
{
    if (tree->loop != NULL) {
        loop = loopMap[tree->loop];
	hierarchicalName = strdup(tree->name());
    } else {
        loop = NULL;
	hierarchicalName = NULL;
    }
    
    for (auto cit = tree->children.begin(); cit != tree->children.end(); ++cit)
        children.push_back(new BPatch_loopTreeNode(fg, *cit, loopMap));

    vector<PatchFunction*> patchCallees;
    tree->getCallees(patchCallees);
    for (auto fit = patchCallees.begin(); fit != patchCallees.end(); ++fit) {
        func_instance * func = SCAST_FI(*fit);
        if (func->getPowerPreambleFunc() != NULL) continue; 
        callees.push_back(func);
    }
}
 

std::string
BPatch_loopTreeNode::getCalleeName(unsigned int i) 
{
    assert(i < callees.size());
    assert(callees[i] != NULL);
    return callees[i]->prettyName();
}

const char * 
BPatch_loopTreeNode::name()
{
    assert(loop != NULL);
    return hierarchicalName; 
}

unsigned int
BPatch_loopTreeNode::numCallees() { 
    return callees.size(); 
}


BPatch_loopTreeNode::~BPatch_loopTreeNode() {
    // Loops are deleted by BPatch_flowGraph...
    for (unsigned i = 0; i < children.size(); i++)
	delete children[i];
    if (hierarchicalName)
        free(hierarchicalName);
    // don't delete callees!
}


BPatch_basicBlockLoop *
BPatch_loopTreeNode::findLoop(const char *name) 
{ 
    if (loop) {
        if (0==strcmp(name,hierarchicalName)) 
            return loop;
    }
    for (unsigned i = 0; i < children.size(); i++) {
        BPatch_basicBlockLoop *lp = children[i]->findLoop(name);
        if (lp) return lp;
    }
    return NULL;
}

bool BPatch_loopTreeNode::getCallees(BPatch_Vector<BPatch_function *> &v,
                                        BPatch_addressSpace *p)
{
   for (unsigned i=0; i<callees.size(); i++) {
      //  get() will not allocate a NULL entry in the map
      BPatch_function *f = p->findOrCreateBPFunc(callees[i], NULL);
      v.push_back(f);
   }
   return true;
}
