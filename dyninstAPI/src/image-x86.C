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

/*
 * inst-x86.C - x86 dependent functions and code generator
 */

#include "common/h/Vector.h"
#include "common/h/Dictionary.h"
#include "common/h/Vector.h"
#include "image-func.h"
#include "instPoint.h"
#include "symtab.h"
#include "dyninstAPI/h/BPatch_Set.h"
#include "debug.h"
#include <deque>
#include <set>
#include <algorithm>
//#include "arch.h"

#include "instructionAPI/h/Instruction.h"
#include "instructionAPI/h/InstructionDecoder.h"

#include "Instruction.h"
#include "dynutil/h/dyn_regs.h"
#include "dynutil/h/AST.h"
#include "Graph.h"
#include "instructionAPI/h/Register.h"
#include "symEval/h/Absloc.h"
#include "symEval/h/AbslocInterface.h"
#include "symEval/h/slicing.h"
#include "symEval/h/SymEval.h"
#include "symEval/src/SymEvalPolicy.h"
#include "boost/tuple/tuple.hpp"
#include "boost/tuple/tuple_comparison.hpp"
#include "boost/tuple/tuple_io.hpp"

#include "dynutil/h/dyntypes.h"

#include "Parsing.h"

using namespace Dyninst::ParseAPI;

bool image_func::writesFPRs(unsigned level) {
    using namespace Dyninst::InstructionAPI;
    // Oh, we should be parsed by now...
    if (!parsed()) image_->analyzeIfNeeded();

    if (containsFPRWrites_ == unknown) {
        // Iterate down and find out...
        // We know if we have callees because we can
        // check the instPoints; no reason to iterate over.
        // We also cache callee values here for speed.

        if (level >= 3) {
            return true; // Arbitrarily decided level 3 iteration.
        }        
        Function::edgelist & calls = callEdges();
        Function::edgelist::iterator cit = calls.begin();
        for( ; cit != calls.end(); ++cit) {
            image_edge * ce = static_cast<image_edge*>(*cit);
            image_func * ct = static_cast<image_func*>(
                obj()->findFuncByEntry(region(),ce->trg()->start()));
            if(ct && ct != this) {
                if (ct->writesFPRs(level+1)) {
                    // One of our kids does... if we're top-level, cache it; in 
                    // any case, return
                    if (level == 0)
                        containsFPRWrites_ = used;
                    return true;
                }
            }
            else if(!ct){
                // Indirect call... oh, yeah. 
                if (level == 0)
                    containsFPRWrites_ = used;
                return true;
            }
        }

        // No kids contain writes. See if our code does.
        static RegisterAST::Ptr st0(new RegisterAST(x86::st0));
        static RegisterAST::Ptr st1(new RegisterAST(x86::st1));
        static RegisterAST::Ptr st2(new RegisterAST(x86::st2));
        static RegisterAST::Ptr st3(new RegisterAST(x86::st3));
        static RegisterAST::Ptr st4(new RegisterAST(x86::st4));
        static RegisterAST::Ptr st5(new RegisterAST(x86::st5));
        static RegisterAST::Ptr st6(new RegisterAST(x86::st6));
        static RegisterAST::Ptr st7(new RegisterAST(x86::st7));

        vector<FuncExtent *>::const_iterator eit = extents().begin();
        for( ; eit != extents().end(); ++eit) {
            FuncExtent * fe = *eit;
        
            const unsigned char* buf = (const unsigned char*)
                isrc()->getPtrToInstruction(fe->start());
            if(!buf) {
                parsing_printf("%s[%d]: failed to get insn ptr at %lx\n",
                    FILE__, __LINE__,fe->start());
                // if the function cannot be parsed, it is only safe to 
                // assume that the FPRs are written -- mcnulty
                return true; 
            }
            InstructionDecoder d(buf,fe->end()-fe->start(),isrc()->getArch());
            Instruction::Ptr i;

            while(i = d.decode()) {
                if(i->isWritten(st0) ||
                    i->isWritten(st1) ||
                    i->isWritten(st2) ||
                    i->isWritten(st3) ||
                    i->isWritten(st4) ||
                    i->isWritten(st5) ||
                    i->isWritten(st6) ||
                    i->isWritten(st7)
                   )
                {
                    containsFPRWrites_ = used;
                    return true;
                }
            }
        }
        // No kids do, and we don't. Impressive.
        containsFPRWrites_ = unused;
        return false;
    }
    else if (containsFPRWrites_ == used) {
        return true;
    }
    else if (containsFPRWrites_ == unused) {
        return false;
    }

    fprintf(stderr, "ERROR: function %s, containsFPRWrites_ is %d (illegal value!)\n", 
	    symTabName().c_str(), containsFPRWrites_);
    
    assert(0);
    return false;
}

class Predicates {
    public:
        static bool end (Assignment::Ptr) { return false;}
        static bool widen (Assignment::Ptr) { return false;}
        static bool call (ParseAPI::Function *, std::stack<std::pair<ParseAPI::Function *, int> > &, AbsRegion) {return false;}
        static bool abs (const AbsRegion &, const AbsRegion &) {return false;}
            ;
};

string getTrapRegString(int reg)
{
    switch(reg) {
        case EAX:
            return "EAX";
            break;
        case EBX:
            return "EBX";
            break;
        case ECX:
            return "ECX";
            break;
        case EDX:
            return "EDX";
            break;
        case EDI:
            return "EDI";
            break;
        case ESI:
            return "ESI";
            break;
        case EBP:
            return "EBP";
            break;
        default:
            return "Unknown";
            break;
    }
}

string idToGenerateTuple(idTuple id, string name);
string printCSV(idTuple id);
string printidTuple(idTuple id);

bool image_func::identifyLibraryFunc()
{
    cout << "Entered identifyLibraryFunc() for " << symTabName() << endl;

    char* mappingType = getenv("MAPPING_TYPE");
    if (!mappingType) mappingType="tpc";

    using namespace SymbolicEvaluation;
    vector<Address> addrs;
    getTrapAddresses(addrs);

    set< vector<int> > traps;
    set<string> calls;

    /* for each address that was a trap instruction, backward slice
     * for the value of EAX (the trap num) and also any constant
     * parameter values */    
    vector<Address>::iterator addr_iter;

    ParseAPI::Block * block;
    ParseAPI::Block * callBlock;
    InstructionAPI::Instruction::Ptr instr;

    blocklist & BBs = blocks();
    blocklist::iterator BB_iter;

    for (addr_iter = addrs.begin();
            addr_iter != addrs.end();
            addr_iter++) {
        
        Address addr = (*addr_iter);

        /* Find the basic block and the instruction associated
         * with the address */
        
        bool foundBB=false, foundInstr=false;

        
        for (BB_iter = BBs.begin();
                BB_iter != BBs.end();
                ++BB_iter) {
            block = *BB_iter;
            if (block->region()->contains(addr)) {
                void * insn = block->region()->getPtrToInstruction(addr);
                if (insn) {
                    InstructionAPI::InstructionDecoder decoder(reinterpret_cast<const unsigned char*>(insn), ((image_basicBlock*)block)->getSize(), obj()->cs()->getArch());
                    instr = decoder.decode();
                    cout << "found instruction: " << instr->format() << endl;
                    foundInstr = true;
                }
                break;
            } 
        }
        for (BB_iter = BBs.begin();
                BB_iter != BBs.end();
                ++BB_iter) {
            block = *BB_iter;
            if ((block->start() <= addr) &&
                    (block->end() >= addr)) {
                foundBB = true;
                break;
            }
        }

        cout << "block: " << block->start() << " to " << block->end() << endl;
        cout << "addr: " << addr << endl;

        assert(foundBB);
        assert(foundInstr);
              
        /* Backward slice to get the value in 
         * EAX, EBX, ECX, EDX, EDI, ESI, and EBP at this addr */
        vector<int> curTraps;
        
        Dyninst::Absloc eax_reg(x86::eax);
        Dyninst::Absloc ebx_reg(x86::ebx);
        Dyninst::Absloc ecx_reg(x86::ecx);
        Dyninst::Absloc edx_reg(x86::edx);
        Dyninst::Absloc edi_reg(x86::edi);
        Dyninst::Absloc esi_reg(x86::esi);
        Dyninst::Absloc ebp_reg(x86::ebp);
        unsigned int curEAX, curEBX, curECX, curEDX, curEDI, curESI, curEBP;

        if (retrieveValue(block, instr, addr, eax_reg, curEAX))
            curTraps.push_back(curEAX);
        else curTraps.push_back(-1);

        /* based on the trap #, decide how many params to examine */
        map<int,int> syscallParams = img()->codeObject()->getSyscallParams();
        map<int,int>::iterator param_iter = syscallParams.find(curEAX);
        int numParams = 0;
        if (param_iter != syscallParams.end()) {
            numParams = param_iter->second;
        } 
        //cout << curEAX << " uses " << numParams << " params" << endl;

        if ( (!strcmp(mappingType, "tpc")) || (!strcmp(mappingType, "tp"))) {
            if (numParams > 0) {
                if (retrieveValue(block, instr, addr, ebx_reg, curEBX))
                    curTraps.push_back(curEBX);
                else curTraps.push_back(-1);
                if (numParams > 1) {
                    if (retrieveValue(block, instr, addr, ecx_reg, curECX))
                        curTraps.push_back(curECX);
                    else curTraps.push_back(-1);
                    if (numParams > 2) {
                        if (retrieveValue(block, instr, addr, edx_reg, curEDX))
                            curTraps.push_back(curEDX);
                        else curTraps.push_back(-1);
                        if (numParams > 3) {
                            if (retrieveValue(block, instr, addr, edi_reg, curEDI))
                                curTraps.push_back(curEDI);
                            else curTraps.push_back(-1);
                            if (numParams > 4) {
                                if (retrieveValue(block, instr, addr, esi_reg, curESI))
                                    curTraps.push_back(curESI);
                                else curTraps.push_back(-1);
                                if (numParams > 5) {
                                    if (retrieveValue(block, instr, addr, ebp_reg, curEBP))
                                        curTraps.push_back(curEBP);
                                    else curTraps.push_back(-1);
                                }}}}}
            }}
        traps.insert(curTraps);
    }

    /* add call information */ 
    if ( (!strcmp(mappingType, "tpc")) || (!strcmp(mappingType, "tc"))) {
        for (BB_iter = BBs.begin();
                BB_iter != BBs.end();
                ++BB_iter) {
            callBlock = *BB_iter;

            /* Look for any calls made from this basic block */
            ParseAPI::Block::edgelist & edges = callBlock->targets();
            ParseAPI::Block::edgelist::iterator edge_iter;
            for (edge_iter = edges.begin();
                    edge_iter != edges.end();
                    ++edge_iter) {
                bool foundCallName = false;
                if ((*edge_iter)->type() == CALL) {
                    if (!foundCallName) {
                        ParseAPI::Block * targetBB = (*edge_iter)->trg();
                        vector<ParseAPI::Function *> funcs;
                        targetBB->getFuncs(funcs);
                        if (funcs.size()) {
                            ParseAPI::Function * curFunc = funcs[0];
                            string curFuncName = curFunc->name();

                            set<string> syscallFuncs = img()->codeObject()->getSyscallFuncs();
                            if (syscallFuncs.find(curFuncName) != syscallFuncs.end()) {
                                const char * curName = curFuncName.c_str();
                                calls.insert(curName);
                                cout << "Found call: " << curName << endl;
                            }
                        } else {
                            cout << "targetBB didn't have any funcs associated with it" << endl;
                        }
                    }
                }
            }
        }
    }

    id = boost::make_tuple("", traps, calls);

#if 0
    if (addrs.size()) 
        cout << idstring << symTabName() << endl;
#endif
#if 0
    if (addrs.size()) {
        string toprint = idToGenerateTuple(id, symTabName());
        cout << toprint << endl;
    }
#endif
    map<idTuple,string> ids = img()->codeObject()->getLibraryIDMappings();
    map<idTuple,string>::iterator id_iter = ids.find(id);
    if (addrs.size()) {

        if (id_iter != ids.end()) {
            cout << printidTuple(id) << " ( " << symTabName() << " was identified" << endl;
        } else {
            cout << printidTuple(id) << " ( " << symTabName() << " UNIDENTIFIED" << endl;
        }
    }
#if 0
    if (addrs.size()) {
        string toprint = printCSV(id);
        cout << symTabName() << "&" << toprint << endl;
    }
#endif
    
    return true;
}

string printCSV(idTuple id) {
    stringstream idstring;

    set<vector<int> > traps = id.get<1>();
    set<string> calls = id.get<2>();

    set<vector<int> >::iterator trap_iter;
    for (trap_iter = traps.begin();
            trap_iter != traps.end();
            ++trap_iter) {
        vector<int> values = *trap_iter;
        for (unsigned i = 0; i < values.size(); i++) {
            idstring << values[i];
            if (i+1 != values.size())
                idstring << ",";
        }
        idstring << ";";
    }

    set<string>::iterator call_iter;
    for (call_iter = calls.begin(); 
            call_iter != calls.end();
            ++call_iter) {
        idstring << *call_iter << ";";
    }

    return idstring.str();
}

string printidTuple(idTuple id) {
    char* mappingType = getenv("MAPPING_TYPE");
    if (!mappingType) {
        mappingType="tpc";
    } else {
        cout << "mapping type = " << mappingType << endl;
    }
    stringstream idstring;
    idstring << "{ ";

    set< vector<int> > traps =  id.get<1>();
    set<string> calls = id.get<2>();

    /* print values */
    set< vector<int> >::iterator trap_iter;
    for (trap_iter = traps.begin();
            trap_iter != traps.end();
            ++trap_iter) {
        idstring << "( ";
        vector<int> values = (*trap_iter);
        if ((!strcmp(mappingType, "tpc")) || (!strcmp(mappingType,"tp"))) {
            for (unsigned i = 0; i < values.size(); i++) {
                idstring << getTrapRegString(i) << "=" << values[i] << " ";
            }
        } else {
            if (values[0] != -1) {
                idstring << getTrapRegString(0) << "=" << values[0] << " ";
            }
        }
        idstring << ") ";
    }

    if ((!strcmp(mappingType,"tpc")) || (!strcmp(mappingType, "tc"))) {
        /* print calls */
        set<string>::iterator call_iter;
        for (call_iter = calls.begin(); call_iter != calls.end(); ++call_iter) {
            idstring << (*call_iter) << " ";
        }
    } 

    idstring << "}";

    return idstring.str();
}

string idToGenerateTuple(idTuple id, string name) {
    char* mappingType = getenv("MAPPING_TYPE");
    if (!mappingType) {
        mappingType="tpc";
    } else {
        cout << "mapping type = " << mappingType << endl;
    }

    //if (!strcmp(mappingType,"tpc")) {

    stringstream idstring;
    idstring << "traps.clear(); ";

    set< vector<int> > traps =  id.get<1>();
    set<string> calls = id.get<2>();

    /* print values */
    set< vector<int> >::iterator trap_iter;
    for (trap_iter = traps.begin();
            trap_iter != traps.end();
            ++trap_iter) {
        vector<int> values = (*trap_iter);
        if ((!strcmp(mappingType, "tpc")) || (!strcmp(mappingType,"tp"))) {
            idstring << "makeTrapVector(traps, " << values.size() << ", ";
            for (unsigned i = 0; i < values.size(); i++) {
                if ( i+1 == values.size()) {
                    idstring << values[i] << ");";
                } else {
                    idstring << values[i] << ",";
                }
                //if (values[i] != -1) {
                    //idstring << getTrapRegString(i) << "=" << values[i] << " ";
                //}
            }
        } else {
            //if (values[0] != -1) {
                //idstring << getTrapRegString(0) << "=" << values[0] << " ";
            //}
            idstring << "makeTrapVector(traps, 1, " << values[0] << ");";
        }
        //idstring << ") ";
        set<vector<int> >::iterator trap_iter_tmp = trap_iter;
        trap_iter_tmp++;
    }
    idstring << "addMapping(traps, ";

    if ((!strcmp(mappingType,"tpc")) || (!strcmp(mappingType, "tc"))) {
    idstring << "makeCallSet(" << calls.size();
    if (calls.size() > 0) {
        idstring << ", ";
    }
        /* print calls */
        set<string>::iterator call_iter;
        for (call_iter = calls.begin(); call_iter != calls.end(); ++call_iter) {
            //idstring << (*call_iter) << " ";
            idstring << "\"" << *call_iter << "\"";
            set<string>::iterator call_iter_tmp = call_iter;
            call_iter_tmp++;
            if (call_iter_tmp != calls.end()) {
                idstring << ",";
            }
        }
    } else {
        idstring << "makeCallSet(0";
    } 
    idstring << "), ";

    idstring << "\"" << name << "\");";

    return idstring.str();
}

string image_func::idToString() {
    char* mappingType = getenv("MAPPING_TYPE");
    if (!mappingType) {
        mappingType="tpc";
    } else {
        cout << "mapping type = " << mappingType << endl;
    }

    //if (!strcmp(mappingType,"tpc")) {

    stringstream idstring;
    idstring << "{ ";

    set< vector<int> > traps =  id.get<1>();
    set<string> calls = id.get<2>();

    /* print values */
    set< vector<int> >::iterator trap_iter;
    for (trap_iter = traps.begin();
            trap_iter != traps.end();
            ++trap_iter) {
        idstring << "( ";
        vector<int> values = (*trap_iter);
        if ((!strcmp(mappingType, "tpc")) || (!strcmp(mappingType,"tp"))) {
            for (unsigned i = 0; i < values.size(); i++) {
                if (values[i] != -1) {
                    idstring << getTrapRegString(i) << "=" << values[i] << " ";
                }
            }
        } else {
            if (values[0] != -1) {
                idstring << getTrapRegString(0) << "=" << values[0] << " ";
            }
        }
        idstring << ") ";
    }

    if (!strcmp(mappingType,"tpc")) {
        /* print calls */
        set<string>::iterator call_iter;
        for (call_iter = calls.begin(); call_iter != calls.end(); ++call_iter) {
            idstring << (*call_iter) << " ";
        }
    } 

    idstring << "}";

    return idstring.str();
}

bool image_func::retrieveValue(ParseAPI::Block* block, InstructionAPI::Instruction::Ptr instr, Offset addr, Dyninst::Absloc reg, unsigned int & val) {
    using namespace SymbolicEvaluation;

    ParseAPI::Function * func = (ParseAPI::Function*)this;

    std::vector<AbsRegion> ins;
    ins.push_back(AbsRegion(reg));
    Assignment::Ptr assign = Assignment::Ptr(new Assignment(instr, 
                addr, 
                func, 
                ins, 
                AbsRegion(reg)));

    Slicer slicer(assign, block, func);

    cout << "Assignment being sliced for is " << assign->format() << endl;

    Slicer::PredicateFunc end = &(Predicates::end);
    Slicer::PredicateFunc widen = &(Predicates::widen);
    Slicer::CallStackFunc call = &(Predicates::call);
    Slicer::AbsRegionFunc abs = &(Predicates::abs);

    Graph::Ptr g = slicer.backwardSlice(end, widen, call, abs);
    
    /* print the graph for verification */
    stringstream name;
    static int called = 0;
    name << "/p/paradyn/development/jacobson/Dyninst/libc/tmp/" << symTabName() << std::hex
        << "_" << addr << std::dec << "_" << called++ << ".dot";
    g->printDOT(name.str());

    //cout << "Nodes in the graph:" << endl;
    NodeIterator nodeIter, nodeBegin, nodeEnd;
    g->allNodes(nodeBegin, nodeEnd);
    for (nodeIter = nodeBegin; nodeIter != nodeEnd; ++nodeIter) {
        //cout << (*nodeIter)->format() << endl;        
    }

    //cout << "Entry nodes in the graph: " << endl;
    g->entryNodes(nodeBegin, nodeEnd);
    for (nodeIter = nodeBegin; nodeIter != nodeEnd; ++nodeIter) {
        //cout << (*nodeIter)->format() << endl;        
    }

    /* Retreive value in register */
    SymEval::Result_t res;
    SymEval::expand(g, res);

    /* We want the value at the assignment node, but because this is 
     * a contrived instruction, we'll get the value at its predecessor */
    NodeIterator exitIter, exitBegin, exitEnd;
    g->exitNodes(exitBegin, exitEnd);
    bool foundExit = false;
    bool ret = false;
    for (exitIter = exitBegin; exitIter != exitEnd; ++exitIter) {
        if (foundExit) {
            cout << "There shouldn't be more than on exit node..." << endl;
            return false;
        } else {
            foundExit = true;
        }
        NodeIterator inIter, inBegin, inEnd;
        (*exitIter)->ins(inBegin, inEnd);
        bool mult = false;
        unsigned int cur_val;

        for (inIter = inBegin; inIter != inEnd; ++inIter) {
            Node::Ptr ptr = *inIter;
            AssignNode::Ptr aNode = dyn_detail::boost::dynamic_pointer_cast<AssignNode>(ptr);
            Assignment::Ptr aAssign = aNode->assign();
            SymEval::Result_t::const_iterator iter = res.find(aAssign);
            if (iter == res.end()) return false;
            AST::Ptr p = iter->second;
            if (!p) return false;

            if (p->getID() == AST::V_ConstantAST) {
                if (mult) {
                    //cout << "Found an additional input, ";
                    cur_val = SymbolicEvaluation::ConstantAST::convert(p)->val().val;
                    //cout << " val is " << cur_val << endl;
                    if (cur_val != val) {
                        return false;
                    }
                } else {
                    //cout << "First time through, ";
                    val = SymbolicEvaluation::ConstantAST::convert(p)->val().val;
                    //cout << " val is " << val << endl;
                    mult = true;
                    ret = true;
                }
            } else {
                return false;
            }
        }
    }

    return ret;
}
