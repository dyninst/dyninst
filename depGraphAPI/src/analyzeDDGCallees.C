/*
 * Copyright (c) 1996-2009 Barton P. Miller
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

#include <map>
#include <algorithm>
#include <queue>

#include "analyzeDDG.h"

#include "Absloc.h"
#include "Instruction.h"

#include "BPatch_basicBlock.h"
#include "BPatch_edge.h"
#include "BPatch_function.h"

#include "dataflowAPI/h/stackanalysis.h"
#include "dyninstAPI/src/function.h"
#include "dyninstAPI/src/image-func.h"
#include "dyninstAPI/src/addressSpace.h"
#include "Annotatable.h"

// Prototype intra-function DDG creator

// TODO
//   Handle calls
//     Trust the ABI to start
//     Analyze the callee function
//   Interprocedural

using namespace Dyninst;
using namespace Dyninst::DepGraphAPI;
using namespace Dyninst::InstructionAPI;
using namespace std;
using namespace dyn_detail::boost;


///////////////////////////////////////
// Handling callees
///////////////////////////////////////

void DDGAnalyzer::summarizeABIGenKill(Address placeholder,
                                      Function *callee,
                                      DefMap &gens,
                                      KillMap &kills) {
    //assert(callee);
    AbslocSet abslocs;

    // Figure out what platform we're on...
    if (addr_width == 0) {
        std::vector<DDGAnalyzer::Block *> entry;
        func_->getCFG()->getEntryBasicBlock(entry);    
        addr_width = entry[0]->lowlevel_block()->proc()->getAddressWidth();
    }

    if (addr_width == 4) {
        // x86...
        
        // Callee preserves ebp, ebx, edi, esi, esp
        // All others are clobbered

        abslocs.insert(RegisterLoc::getRegLoc(makeRegister(r_EAX)));
        abslocs.insert(RegisterLoc::getRegLoc(makeRegister(r_ECX)));
        abslocs.insert(RegisterLoc::getRegLoc(makeRegister(r_EDX)));
#if 0
        for (unsigned i = r_OF; i <= r_RF; i++) {
            abslocs.insert(RegisterLoc::getRegLoc(makeRegister(i)));
        }
#endif
    }
    else {
        // amd-64...
        abslocs.insert(RegisterLoc::getRegLoc(makeRegister(r_RAX)));
        abslocs.insert(RegisterLoc::getRegLoc(makeRegister(r_RCX)));
        abslocs.insert(RegisterLoc::getRegLoc(makeRegister(r_RDX)));
        abslocs.insert(RegisterLoc::getRegLoc(makeRegister(r_R8)));
        abslocs.insert(RegisterLoc::getRegLoc(makeRegister(r_R9)));
        abslocs.insert(RegisterLoc::getRegLoc(makeRegister(r_R10)));
        abslocs.insert(RegisterLoc::getRegLoc(makeRegister(r_R11)));
        abslocs.insert(RegisterLoc::getRegLoc(makeRegister(r_RDI)));
        abslocs.insert(RegisterLoc::getRegLoc(makeRegister(r_RSI)));

        for (unsigned i = r_OF; i <= r_RF; i++) {
            abslocs.insert(RegisterLoc::getRegLoc(makeRegister(i)));
        }

    }

    for (AbslocSet::iterator iter = abslocs.begin(); iter != abslocs.end();
         ++iter) {
        Absloc::Ptr D = *iter;
        // We need to make a virtual node on our side
        // representing the definition made by the child.
        // For now, that's a cNode...
        
        cNode cnode(placeholder, D, actualReturn, callee);

        updateDefSet(D, gens, cnode);

        // Unlike the other versions of this, we can trust the ABI with
        // kill information.
        updateKillSet(D, kills);

        actualReturnMap_[placeholder].insert(cnode);

    }

}


void DDGAnalyzer::summarizeConservativeGenKill(Address placeholder,
                                               Function *callee,
                                               DefMap &gens,
                                               KillMap &) {
    AbslocSet abslocs;
    
    // Figure out what platform we're on...
    if (addr_width == 0) {
        std::vector<DDGAnalyzer::Block *> entry;
        func_->getCFG()->getEntryBasicBlock(entry);    
        addr_width = entry[0]->lowlevel_block()->proc()->getAddressWidth();
    }
    
    if (addr_width == 4) {
        // x86...
        
        // Defines everything (but doesn't kill)
        
        for (unsigned i = r_EAX; i <= r_EDI; ++i) {
            abslocs.insert(RegisterLoc::getRegLoc(makeRegister(i)));
        }

        for (unsigned i = r_ESP; i <= r_EBP; ++i) {
            abslocs.insert(RegisterLoc::getRegLoc(makeRegister(i)));
        }

        for (unsigned i = r_OF; i <= r_RF; ++i) {
            abslocs.insert(RegisterLoc::getRegLoc(makeRegister(i)));
        }

        abslocs.insert(StackLoc::getStackLoc());

    }
    else {
        for (unsigned i = r_RAX; i <= r_RDI; ++i) {
            abslocs.insert(RegisterLoc::getRegLoc(makeRegister(i)));
        }

        for (unsigned i = r_RSP; i <= r_RBP; ++i) {
            abslocs.insert(RegisterLoc::getRegLoc(makeRegister(i)));
        }

        for (unsigned i = r_R8; i <= r_R15; ++i) {
            abslocs.insert(RegisterLoc::getRegLoc(makeRegister(i)));
        }

        for (unsigned i = r_OF; i <= r_RF; ++i) {
            abslocs.insert(RegisterLoc::getRegLoc(makeRegister(i)));
        }
    }

    for (AbslocSet::iterator iter = abslocs.begin(); iter != abslocs.end();
         iter++) {
        Absloc::Ptr D = *iter;
        // We need to make a virtual node on our side
        // representing the definition made by the child.
        // For now, that's a cNode...
        
        cNode cnode(placeholder, D, actualReturn, callee);

        updateDefSet(D, gens, cnode);
        // We explicitly do not add a kill set here. Thus these definitions
        // will be treated as possible-defines.
        //updateKillSet(D, kills);

        actualReturnMap_[placeholder].insert(cnode);

    }

}


void DDGAnalyzer::summarizeLinearGenKill(Address placeholder,
                                         Function *callee,
                                         int height,
                                         int region,
                                         DefMap &gens,
                                         KillMap &kills) {
    AbslocSet abslocs;

    // We assume this function defines anything it... well, defines.
    BlockSet blocks;
    callee->getCFG()->getAllBasicBlocks(blocks);

    for (BlockSet::const_iterator iter = blocks.begin(); iter != blocks.end(); iter++) {
        std::vector<std::pair<InsnPtr, Address> > insns;
        (*iter)->getInstructions(insns);

        for (std::vector<std::pair<InsnPtr,Address> >::iterator j = insns.begin();
             j != insns.end(); 
             j++) {
            DefSet writtenAbslocs = getDefinedAbslocs(j->first, j->second);

            for (DefSet::iterator k = writtenAbslocs.begin(); k != writtenAbslocs.end(); ++k) {
                Absloc::Ptr D = *k;
                // We need to make a virtual node on our side
                // representing the definition made by the child.
                // For now, that's a cNode...
                
                // If we're a stack slot then do some impedance matching magic...
                StackLoc::Ptr SD = dyn_detail::boost::dynamic_pointer_cast<StackLoc>(D);
                if (SD) {
                    if (height != -1 && SD->isPrecise()) {
                        int slot = SD->slot();
                        D = StackLoc::getStackLoc(slot + height, region);
                    }
                    else {
                        D = StackLoc::getStackLoc();
                    }
                }
                abslocs.insert(D);
            }
        }
    }

    for (AbslocSet::iterator iter = abslocs.begin(); iter != abslocs.end();
         ++iter) {
        Absloc::Ptr D = *iter;
        // We need to make a virtual node on our side
        // representing the definition made by the child.
        // For now, that's a cNode...
        
        cNode cnode(placeholder, D, actualReturn, callee);

        updateDefSet(D, gens, cnode);

        // Again, we can't categorically state that the absloc was killed. So
        // we don't update the kill set and leave it as a possibly-defined.
        //updateKillSet(D, kills);

        actualReturnMap_[placeholder].insert(cnode);

    }

}


void DDGAnalyzer::summarizeAnalyzeGenKill(Address placeholder,
                                          Function *callee,
                                          int height,
                                          int region,
                                          DefMap &gens,
                                          KillMap &kills) {
    AbslocSet abslocs;

    DDG::Ptr cDDG = DDG::analyze(callee); 

    NodeIterator begin, end;
    cDDG->formalReturnNodes(begin, end);

    for (; begin != end; ++begin) {
        FormalReturnNode::Ptr p = dyn_detail::boost::dynamic_pointer_cast<FormalReturnNode> (*begin);
        if (!p) continue;
        Absloc::Ptr D = p->absloc();
                
        // If we're a stack slot then do some impedance matching magic...
        StackLoc::Ptr SD = dyn_detail::boost::dynamic_pointer_cast<StackLoc>(D);
        if (SD) {
            if (height != -1 && SD->isPrecise()) {
                int slot = SD->slot();
                D = StackLoc::getStackLoc(slot + height, region);
            }
            else {
                D = StackLoc::getStackLoc();
            }
        }
        abslocs.insert(D);
    }

    for (AbslocSet::iterator iter = abslocs.begin(); iter != abslocs.end();
         ++iter) {
        Absloc::Ptr D = *iter;
        // We need to make a virtual node on our side
        // representing the definition made by the child.
        // For now, that's a cNode...
        
        cNode cnode(placeholder, D, actualReturn, callee);

        updateDefSet(D, gens, cnode);

        // Again, we can't categorically state that the absloc was killed. So
        // we don't update the kill set and leave it as a possibly-defined.
        // This analysis type provides us the opportunity for doing a precise analysis..
        // we've have to determine if D was defined along all paths from the entry 
        // rather than along at least one. For now, leave it as a possibly-defined
        // and move on. 
        
        //updateKillSet(D, kills);

        actualReturnMap_[placeholder].insert(cnode);

    }

}


void DDGAnalyzer::summarizeABIUsed(Address placeholder, 
                                   Function *callee, 
                                   const DefMap &reachingDefs,
                                   NodeVec &actualParams) {
    AbslocSet abslocs;

    // Figure out what platform we're on...
    if (addr_width == 0) {
        std::vector<DDGAnalyzer::Block *> entry;
        func_->getCFG()->getEntryBasicBlock(entry);    
        addr_width = entry[0]->lowlevel_block()->proc()->getAddressWidth();
    }

    if (addr_width == 4) {
        // x86...
        
        // Parameters go on the stack. Anyone know how many of those there are?
        // For now, just alias that to "the stack" and see if it works...
        abslocs.insert(StackLoc::getStackLoc());
    }
    else {
        // amd-64...
        abslocs.insert(RegisterLoc::getRegLoc(makeRegister(r_RCX)));
        abslocs.insert(RegisterLoc::getRegLoc(makeRegister(r_RDX)));
        abslocs.insert(RegisterLoc::getRegLoc(makeRegister(r_R8)));
        abslocs.insert(RegisterLoc::getRegLoc(makeRegister(r_R9)));
        abslocs.insert(RegisterLoc::getRegLoc(makeRegister(r_RDI)));
        abslocs.insert(RegisterLoc::getRegLoc(makeRegister(r_RSI)));

        // We use the stack. Technically, we use the return address. 
        abslocs.insert(StackLoc::getStackLoc());
    }

    for (AbslocSet::iterator iter = abslocs.begin(); iter != abslocs.end();
         ++iter) {
        Absloc::Ptr U = *iter;

        // Create an actualParameterNode for each absloc and hook up edges
        // according to reachingDefs...
        cNode cnode(placeholder, U, actualParam, callee); 
        
        Node::Ptr T = makeNode(cnode);
        actualParams.push_back(T);

        // Okay, now we need to find reaching defs to this one
        DefMap::const_iterator tmp = reachingDefs.find(*iter);
        if (tmp != reachingDefs.end()) {
            for (cNodeSet::const_iterator c_iter = tmp->second.begin();
                 c_iter != tmp->second.end(); ++c_iter) {
                NodePtr S = makeNode(*c_iter);
                // By definition we know S is in nodes
                ddg->insertPair(S,T);
                //fprintf(stderr, "\t\t\t\t ... from local definition %s/0x%lx\n",
                //c_iter->first->name().c_str(),
                //c_iter->second.addr);
            }
        }
        else { 
            // It's entirely possible we haven't seen it yet at all. 
            // Just go ahead and parameter node it. 
            cNode fp(0, *iter, formalParam);
            NodePtr S = makeNode(fp);
            ddg->insertEntryNode(S);
            ddg->insertPair(S,T);
        }
    }

}

void DDGAnalyzer::summarizeConservativeUsed(Address placeholder,
                                            Function *callee,
                                            const DefMap &reachingDefs,
                                            NodeVec &actualParams) {
    AbslocSet abslocs;
    // Figure out what platform we're on...
    if (addr_width == 0) {
        std::vector<DDGAnalyzer::Block *> entry;
        func_->getCFG()->getEntryBasicBlock(entry);    
        addr_width = entry[0]->lowlevel_block()->proc()->getAddressWidth();
    }

    if (addr_width == 4) {
        // x86...
        
        // Defines everything (but doesn't kill)
        
        for (unsigned i = r_EAX; i <= r_EDI; ++i) {
            abslocs.insert(RegisterLoc::getRegLoc(makeRegister(i)));
        }

        for (unsigned i = r_ESP; i <= r_EBP; ++i) {
            abslocs.insert(RegisterLoc::getRegLoc(makeRegister(i)));
        }

        for (unsigned i = r_OF; i <= r_RF; ++i) {
            abslocs.insert(RegisterLoc::getRegLoc(makeRegister(i)));
        }
        
        abslocs.insert(StackLoc::getStackLoc());
    }
    else {
        for (unsigned i = r_RAX; i <= r_RDI; ++i) {
            abslocs.insert(RegisterLoc::getRegLoc(makeRegister(i)));
        }

        for (unsigned i = r_RSP; i <= r_RBP; ++i) {
            abslocs.insert(RegisterLoc::getRegLoc(makeRegister(i)));
        }

        for (unsigned i = r_R8; i <= r_R15; ++i) {
            abslocs.insert(RegisterLoc::getRegLoc(makeRegister(i)));
        }

        for (unsigned i = r_OF; i <= r_RF; ++i) {
            abslocs.insert(RegisterLoc::getRegLoc(makeRegister(i)));
        }

        abslocs.insert(StackLoc::getStackLoc());
    }

    for (AbslocSet::iterator iter = abslocs.begin(); iter != abslocs.end();
         ++iter) {
        Absloc::Ptr U = *iter;

        // Create an actualParameterNode for each absloc and hook up edges
        // according to reachingDefs...
        cNode cnode(placeholder, U, actualParam, callee); 
        
        Node::Ptr T = makeNode(cnode);
        actualParams.push_back(T);
        
        // Okay, now we need to find reaching defs to this one
        DefMap::const_iterator tmp = reachingDefs.find(*iter);
        if (tmp != reachingDefs.end()) {
            for (cNodeSet::const_iterator c_iter = tmp->second.begin();
                 c_iter != tmp->second.end(); ++c_iter) {
                NodePtr S = makeNode(*c_iter);
                // By definition we know S is in nodes
                ddg->insertPair(S,T);
                //fprintf(stderr, "\t\t\t\t ... from local definition %s/0x%lx\n",
                //c_iter->first->name().c_str(),
                //c_iter->second.addr);
            }
        }
        else { 
            // It's entirely possible we haven't seen it yet at all. 
            // Just go ahead and parameter node it. 
            cNode fp(0, *iter, formalParam);
            NodePtr S = makeNode(fp);
            ddg->insertEntryNode(S);
            ddg->insertPair(S,T);
        }
    }
}

void DDGAnalyzer::summarizeLinearUsed(Address placeholder,
                                      Function *callee,
                                      int height,
                                      int region,
                                      const DefMap &reachingDefs,
                                      NodeVec &actualParams) {
    AbslocSet abslocs;

    // We assume this function defines anything it... well, defines.
    BlockSet blocks;
    callee->getCFG()->getAllBasicBlocks(blocks);

    for (BlockSet::const_iterator iter = blocks.begin(); iter != blocks.end(); ++iter) {
        std::vector<std::pair<InsnPtr, Address> > insns;
        (*iter)->getInstructions(insns);

        for (std::vector<std::pair<InsnPtr,Address> >::iterator j = insns.begin();
             j != insns.end(); 
             ++j) {
            AbslocSet readAbslocs = getUsedAbslocs(j->first, j->second);

            for (AbslocSet::iterator k = readAbslocs.begin(); k != readAbslocs.end(); ++k) {
                Absloc::Ptr U = *k;
                // We need to make a virtual node on our side
                // representing the definition made by the child.
                // For now, that's a cNode...
                
                // If we're a stack slot then do some impedance matching magic...
                StackLoc::Ptr SU = dyn_detail::boost::dynamic_pointer_cast<StackLoc>(U);
                if (SU) {
                    if (height != -1 && SU->isPrecise()) {
                        int slot = SU->slot();
                        U = StackLoc::getStackLoc(slot + height, region);
                    }
                    else {
                        U = StackLoc::getStackLoc();
                    }
                }
                abslocs.insert(U);
            }
        }
    }
    
    for (AbslocSet::iterator iter = abslocs.begin(); iter != abslocs.end();
         ++iter) {
        Absloc::Ptr U = *iter;

        // Create an actualParameterNode for each absloc and hook up edges
        // according to reachingDefs...
        cNode cnode(placeholder, U, actualParam, callee); 
        
        Node::Ptr T = makeNode(cnode);
        actualParams.push_back(T);
        
        // Okay, now we need to find reaching defs to this one
        DefMap::const_iterator tmp = reachingDefs.find(*iter);
        if (tmp != reachingDefs.end()) {
            for (cNodeSet::const_iterator c_iter = tmp->second.begin();
                 c_iter != tmp->second.end(); ++c_iter) {
                NodePtr S = makeNode(*c_iter);
                // By definition we know S is in nodes
                ddg->insertPair(S,T);
                //fprintf(stderr, "\t\t\t\t ... from local definition %s/0x%lx\n",
                //c_iter->first->name().c_str(),
                //c_iter->second.addr);
            }
        }
        else { 
            // It's entirely possible we haven't seen it yet at all. 
            // Just go ahead and parameter node it. 
            cNode fp(0, *iter, formalParam);
            NodePtr S = makeNode(fp);
            ddg->insertEntryNode(S);
            ddg->insertPair(S,T);
        }
    }
}


void DDGAnalyzer::summarizeAnalyzeUsed(Address placeholder,
                                       Function *callee,
                                       int height,
                                       int region,
                                       const DefMap &reachingDefs,
                                       NodeVec &actualParams) {
    AbslocSet abslocs;

    DDG::Ptr cDDG = DDG::analyze(callee); 

    NodeIterator begin, end;
    cDDG->formalParamNodes(begin, end);
    
    for (; begin != end; ++begin) {
        FormalParamNode::Ptr p = dyn_detail::boost::dynamic_pointer_cast<FormalParamNode> (*begin);
        if (!p) continue;
        Absloc::Ptr U = p->absloc();
        
        // If we're a stack slot then do some impedance matching magic...
        StackLoc::Ptr SU = dyn_detail::boost::dynamic_pointer_cast<StackLoc>(U);
        if (SU) {
            if (height != -1 && SU->isPrecise()) {
                int slot = SU->slot();
                U = StackLoc::getStackLoc(slot + height, region);
            }
            else {
                U = StackLoc::getStackLoc();
            }
        }
        abslocs.insert(U);
    }
    
    for (AbslocSet::iterator iter = abslocs.begin(); iter != abslocs.end();
         ++iter) {
        Absloc::Ptr U = *iter;

        // Create an actualParameterNode for each absloc and hook up edges
        // according to reachingDefs...
        cNode cnode(placeholder, U, actualParam, callee); 
        
        Node::Ptr T = makeNode(cnode);
        actualParams.push_back(T);
        
        // Okay, now we need to find reaching defs to this one
        DefMap::const_iterator tmp = reachingDefs.find(*iter);
        if (tmp != reachingDefs.end()) {
            for (cNodeSet::const_iterator c_iter = tmp->second.begin();
                 c_iter != tmp->second.end(); ++c_iter) {
                NodePtr S = makeNode(*c_iter);
                // By definition we know S is in nodes
                ddg->insertPair(S,T);
                //fprintf(stderr, "\t\t\t\t ... from local definition %s/0x%lx\n",
                //c_iter->first->name().c_str(),
                //c_iter->second.addr);
            }
        }
        else { 
            // It's entirely possible we haven't seen it yet at all. 
            // Just go ahead and parameter node it. 
            cNode fp(0, *iter, formalParam);
            NodePtr S = makeNode(fp);
            ddg->insertEntryNode(S);
            ddg->insertPair(S,T);
        }
    }
}
