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


#include <map>
#include <vector>
#include <stack>

#include "fingerprint.h"
#include "util.h"
#include "database.h"
#include "predicates.h"

using namespace std;
using namespace Dyninst;

void Fingerprint::findMain(SymtabAPI::Symtab * symtab, 
        ParseAPI::SymtabCodeSource * sts, 
        SymtabAPI::Module * defmod)
{
    SymtabAPI::Region* textsection;
    if(!symtab->findRegion(textsection, ".text"))
    {
        fprintf(stderr, "couldn't find text section\n");
        exit(1);
    }

    const unsigned char* textBuffer = (const unsigned char*) textsection->getPtrToRawData();
    using namespace Dyninst::InstructionAPI;
    InstructionDecoder d(textBuffer, textsection->getMemSize(), sts->getArch());
    Instruction::Ptr curInsn, prevInsn;
    curInsn = d.decode();
    while(curInsn && curInsn->getCategory() != c_CallInsn)
    {
        prevInsn = curInsn;
        curInsn = d.decode();
    }
    unsigned int mainAddr = prevInsn->getOperand(0).getValue()->eval().convert<unsigned int>();
    //fprintf(stderr, "found main at 0x%lx\n", mainAddr);

    using namespace SymtabAPI;
    symtab->createFunction("main",mainAddr,0,defmod);

}

/* 
 * Run library fingerprinting over the binary.
 */
void Fingerprint::run(SymtabAPI::Symtab * symtab)
{  
    /* Convert trap addresses into mapping between wrapper function and its
     * trap instructions */
    buildMappings();

    map<ParseAPI::Function *, vector<trapLoc> >::iterator tIter; 
    for (tIter = trapAddresses.begin(); tIter != trapAddresses.end(); ++tIter) {
        SymtabAPI::Function * symFunc; 
        symtab->findFuncByEntryOffset(symFunc, ((*tIter).first)->addr()); 
        SymtabAPI::Symbol * sym = symFunc->getFirstSymbol();

        /* Skip functions that are not exported */
        if ((sym->getLinkage() != SymtabAPI::Symbol::SL_GLOBAL) &&
                (sym->getLinkage() != SymtabAPI::Symbol::SL_WEAK)) {
            continue;
        }

        processFunc((*tIter).first, symFunc);
    }
}

/* 
 * Build mapping between function and the traps it contains
 */
void Fingerprint::buildMappings()
{
    /* Build <block, tLoc> pairs */ 
    map<ParseAPI::Block *, trapLoc> traps;    

    /* Build set of functions that contain traps */
    set<ParseAPI::Function *> funcs;
    set<pair<ParseAPI::Function *, trapLoc> >::iterator iter;
    for (iter = trapInfo.begin(); iter != trapInfo.end(); ++iter) {
        ParseAPI::Function * f = iter->first;
        trapLoc tloc = iter->second;

        /* From the function, get the code region */
        ParseAPI::CodeRegion * region = f->region();
        
        /* Find the basic block that contains this address */
        std::set<ParseAPI::Block*> blocks;
        f->obj()->findBlocks(region, tloc.addr(), blocks);
        ParseAPI::Block * block = *(blocks.begin());
        
        trapLoc tlocComplete(tloc.addr(), tloc.instr(), block);

        traps.insert(make_pair(block, tlocComplete));

        vector<ParseAPI::Function *> containFuncs;
        block->getFuncs(containFuncs);
        vector<ParseAPI::Function *>::iterator fIter;
        for (fIter = containFuncs.begin(); fIter != containFuncs.end(); ++fIter) {
            funcs.insert(*fIter);
        }
    }

    /* For each function that contain one or more traps, build the set of all
     * traps */
    set<ParseAPI::Function*>::iterator fIter;
    for (fIter = funcs.begin(); fIter != funcs.end(); ++fIter) {
        vector<trapLoc> curTraps;

        /* For each block, check if it had traps */
        ParseAPI::Function::blocklist blocks = (*fIter)->blocks();
        ParseAPI::Function::blocklist::iterator bIter;
        for (bIter = blocks.begin(); bIter != blocks.end(); ++bIter) {
            map<ParseAPI::Block *, trapLoc>::iterator tIter;
            tIter = traps.find(*bIter);
            if (tIter != traps.end()) {
                /* We found a trap, add it to the current set */
                curTraps.push_back(tIter->second);
            }
        } 

        /* Once we've processed all the blocks, add to mapping */
        trapAddresses.insert(make_pair((*fIter), curTraps));
    } 

}

/*
 * Appropriately process function f 
 */
void Fingerprint::processFunc(ParseAPI::Function * f,
        SymtabAPI::Function * symFunc)
{
    string name = f->name();

    stack<ParseAPI::Function *> callstack;
    SemanticDescriptor sd;
    parse(f, callstack, sd);

    if (mode == _learn) {
        learn(f, sd);
    } else {
        identify(f, sd, symFunc);
    }
}

/* 
 * Identify the function f with semantic descriptor sd 
 */
void Fingerprint::identify(ParseAPI::Function * f, SemanticDescriptor & sd, SymtabAPI::Function * symFunc) 
{
    /* Find matches in the DDB */
    Matches matches = sd.find(db);
    SymtabAPI::Symtab * symtab = (dynamic_cast<ParseAPI::SymtabCodeSource *>(f->obj()->cs()))->getSymtabObject();

    string matchNames = "";

    /* Add new name(s) to function */
    if (matches.size()) {
        Matches::iterator iter;
        for (iter = matches.begin(); iter != matches.end(); ++iter) {
            matchNames.append((*iter)+",");

            // Add a new function name 
            symFunc->addMangledName((*iter).c_str(), true);
            symFunc->addPrettyName((*iter).c_str(), true);
        }

        if (verbose)
            cout << "Added symbols " << matchNames << " to function at " << std::hex << f->addr() << endl;
        
        // Optional: only provide one symbol--if possible, the real name, else the targXXX
        if (oneSymbol) {
            vector<SymtabAPI::Symbol *> symbols;
            vector<SymtabAPI::Symbol *> toRemove;
            symFunc->getSymbols(symbols);
            vector<SymtabAPI::Symbol *>::iterator sit;
            for (sit = symbols.begin(); sit != symbols.end(); ++sit) {
                if ((*sit)->getMangledName().find("targ") != string::npos) {
                    toRemove.push_back(*sit);
                }
            }    

            for (sit = toRemove.begin(); sit != toRemove.end(); ++sit) {
                symtab->deleteSymbol(*sit);
            }
        }
    }
}

/* 
 * Add function f to the database.
 */
void Fingerprint::learn(ParseAPI::Function * f, SemanticDescriptor & sd)
{
    string outputFile = "ddb.db";
    outputFile.insert(0, relPath);
    ofstream file;
    file.open(outputFile.c_str(), ios::app);

    string tuple = sd.format(db);
    file << f->name() << ";" << tuple << "||\n";

    file.close();
}


/* 
 * Generate the semantic descriptor for function f
 */
bool Fingerprint::parse(ParseAPI::Function * f,
        stack<ParseAPI::Function *> & callstack,
        SemanticDescriptor & retSD)
{
    ParseAPI::Block * callBlock;

    const ParseAPI::Function::blocklist & blocks = f->blocks();
    ParseAPI::Function::blocklist::iterator bIter;
    
    // Get the set of traps associated with this function
    map<ParseAPI::Function *, vector<trapLoc> >::iterator mIter;
    mIter = trapAddresses.find(f);
    if (mIter == trapAddresses.end()) return false;
    vector<trapLoc> trapLocs = mIter->second;
    std::sort(trapLocs.begin(), trapLocs.end());

    // Push the current function onto the callstack
    callstack.push(f);

    SemanticDescriptor sd;
    SemanticDescriptorElem curTrap;

    Dyninst::Absloc reg0(x86::eax);
    Dyninst::Absloc reg1(x86::ebx);
    Dyninst::Absloc reg2(x86::ecx);
    Dyninst::Absloc reg3(x86::edx);
    Dyninst::Absloc reg4(x86::esi);
    Dyninst::Absloc reg5(x86::edi);
    Dyninst::Absloc reg6(x86::ebp);

    long int curEAX;

    vector<ParamType> params;
    map<string, vector<ParamType> >::iterator pIter;

    SemanticDescriptor calledSD;
    SemanticDescriptor::iterator calledSDIter;

    vector<trapLoc>::iterator tIter;    
    for (tIter = trapLocs.begin(); tIter != trapLocs.end(); ++tIter) {

        InstructionAPI::Instruction::Ptr insn = (*tIter).instr();
        curTrap.clear();

        /* Backward slice to get the value in EAX, which will dictate how we proceed */
        if (retrieveValue(f, *tIter, reg0, curEAX, callstack)) {
            map<int, string>::iterator sysNumIter;
            sysNumIter = db.snDB.find(curEAX);
            string curEAXname;
            if (sysNumIter == db.snDB.end()) {
                char * curTrapNum = (char*)malloc(sizeof(char)*10);
                assert(curTrapNum);
                sprintf(curTrapNum, "%ld", curEAX);

                char * cur = (char*)malloc(sizeof(char)*32);
                assert(cur);
                strcpy(cur, "");
                strcpy(cur, "unknownTrap");
                strcat(cur, curTrapNum);
                curTrap.push_back((void*)cur);
            } else {
                curEAXname = sysNumIter->second;
                curTrap.push_back((void*)curEAXname.c_str());
            }

            /* Based on the trap number, determine how many parameters to examine */
            pIter = db.spDB.find(curEAXname);
            int numParams = 0;
            params.clear();
            if (pIter != db.spDB.end()) {
                params = pIter->second;
                numParams = params.size() - 1; // don't count EAX as a param
            } else {
                params.push_back(_s);
                numParams = 0;
            }

            vector<AbsRegion> regsToSliceFor;
            if (numParams > 0) {
                regsToSliceFor.push_back(reg1);
                if (numParams > 1) {
                    regsToSliceFor.push_back(reg2);
                    if (numParams > 2) {
                        regsToSliceFor.push_back(reg3);
                        if (numParams > 3) {
                            regsToSliceFor.push_back(reg4);
                            if (numParams > 4) { 
                                regsToSliceFor.push_back(reg5);
                                if (numParams > 5) {
                                    regsToSliceFor.push_back(reg6);
                                }}}}}
                                retrieveValues(f, *tIter, regsToSliceFor, callstack, params, curTrap);
            }

        } else {
            /* Could not identify the system call, so not slicing for additional parameters */
            curTrap.push_back((void*)"unknownTrap");
        }
        sd.insert(curTrap);

    }
       
    /* Incorporate information for wrapper functions called from f */
    for (bIter = blocks.begin(); bIter != blocks.end(); ++bIter) {
        callBlock = *bIter;

        /* Look for any calls made from this basic block */
        const ParseAPI::Block::edgelist & edges = callBlock->targets();
        ParseAPI::Block::edgelist::const_iterator eIter;
        if (!edges.size()) continue;
        for (eIter = edges.begin(); eIter != edges.end(); ++eIter) {
            if ((*eIter)->type() == ParseAPI::CALL) {
                ParseAPI::Block * targetBB = (*eIter)->trg();
                ParseAPI::CodeRegion * cr = targetBB->region();
                Address blockEntry = targetBB->start();

                ParseAPI::Function * calledFunc = f->obj()->findFuncByEntry(cr, blockEntry);

                if (calledFunc){ 
                    /* Recursively generate the semantic descriptor for the called wrapper function */ 
                    if (calledFunc == f) break; // Don't recurse on self
                    bool hadTraps = parse(calledFunc, callstack, calledSD);

                    /* Incorporate (union) semantic descriptor elements for the
                     * called wrapper function with f's */
                    if (hadTraps) { 
                        for (calledSDIter = calledSD.begin();
                                calledSDIter != calledSD.end(); 
                                ++calledSDIter) {
                            sd.insert(*calledSDIter); 
                        } 
                    } 
                } 
            } 
        } 
    }

    /* Impose deterministic ordering on the semantic descriptor */
    sd.sort();

    callstack.pop();
    retSD = sd;
    
    return true;
}

/* 
 * Use backward slicing and symbolic expansion to get the integer value stored
 * in reg.
 */
bool Fingerprint::retrieveValue(ParseAPI::Function * f,
        trapLoc & tloc,
        Dyninst::Absloc & reg,
        long int & val,
        std::stack<ParseAPI::Function *> & callstack) 
{
    val = 0;

    /* Create a new instance of a slicer */
    std::vector<AbsRegion> ins;
    ins.push_back(AbsRegion(reg));
    Assignment::Ptr assign = Assignment::Ptr(new Assignment(tloc.instr(),
                tloc.addr(),
                f,
                tloc.block(),
                ins,
                AbsRegion(reg)));
    Slicer slicer(assign, tloc.block(), f);
    ID_Predicates predicates(callstack);
    
    /* Perform backward slicing */
    Graph::Ptr g = slicer.backwardSlice(predicates);

    /* Symbolic expansion on the slice */
    DataflowAPI::Result_t res;
    DataflowAPI::SymEval::expand(g, res);

    NodeIterator exitBegin, exitEnd;
    g->exitNodes(exitBegin, exitEnd);
    bool foundExit = false;
    bool ret = false;

    for ( ; exitBegin != exitEnd; ++exitBegin) {
        if (foundExit) return false;
        foundExit = true;

        NodeIterator inBegin, inEnd;
        (*exitBegin)->ins(inBegin, inEnd);
        bool mult = false;
        long int curVal;

        for ( ; inBegin != inEnd; ++inBegin) {
            Node::Ptr ptr = *inBegin;
            SliceNode::Ptr aNode = boost::dynamic_pointer_cast<SliceNode>(ptr);
            Assignment::Ptr aAssign = aNode->assign();
            DataflowAPI::Result_t::const_iterator iter = res.find(aAssign);
            if (iter == res.end()) return false;

            AST::Ptr p = iter->second;
            if (!p) return false;

            if (p->getID() == AST::V_ConstantAST) {
                if (mult) {
                    curVal = (long int)DataflowAPI::ConstantAST::convert(p)->val().val;
                    if (curVal != val) return false;
                } else {
                    val = (long int)DataflowAPI::ConstantAST::convert(p)->val().val;
                    mult = true;
                    ret = true;
                }
            }
        }
    }

    return ret;
}

/* Use backward slicing and symbolic expansion to get the integer values stored
 * in regsToSliceFor.
 */
bool Fingerprint::retrieveValues(ParseAPI::Function * f,
        trapLoc & tloc,
        vector<AbsRegion> & regsToSliceFor,
        std::stack<ParseAPI::Function *> & callstack,
        vector<ParamType> & params,
        SemanticDescriptorElem & curTrap)
{
    /* Create a new instance of a slicer */
    Assignment::Ptr assign = Assignment::Ptr(new Assignment(tloc.instr(),
                tloc.addr(),
                f,
                tloc.block(),
                regsToSliceFor,
                AbsRegion(regsToSliceFor[0])));

    Slicer slicer(assign, tloc.block(), f);
    ID_Predicates predicates(callstack);
    
    /* Perform backward slicing */
    Graph::Ptr g = slicer.backwardSlice(predicates);

    /* Symbolic expansion on the slice */
    DataflowAPI::Result_t res;
    DataflowAPI::SymEval::expand(g, res);

    vector<pair<bool, long int> > foundValues;
    for (unsigned i = 0; i < regsToSliceFor.size(); i++) {
        foundValues.push_back(make_pair(false, (long int)-1));
    }

    NodeIterator exitBegin, exitEnd;
    g->exitNodes(exitBegin, exitEnd);
    bool foundExit = false;
    long int curVal;
    long int val;

    for ( ; exitBegin != exitEnd; ++exitBegin) {
        if (foundExit) break;
        foundExit = true;

        NodeIterator inBegin, inEnd;
        (*exitBegin)->ins(inBegin, inEnd);

        for ( ; inBegin != inEnd; ++inBegin) {
            Node::Ptr ptr = *inBegin;
            SliceNode::Ptr aNode = boost::dynamic_pointer_cast<SliceNode>(ptr);
            Assignment::Ptr aAssign = aNode->assign();
            AbsRegion & out = aAssign->out();

            /* Find the region that this edge belongs to */
            int correspondingAbsRegion = -1;
            for (unsigned i = 0; i < regsToSliceFor.size(); i++) {
                AbsRegion a = regsToSliceFor[i];

                if (out == a) {
                    correspondingAbsRegion = i;
                    break;
                }
            }

            if (correspondingAbsRegion == -1) break;

            //DataflowAPI::SymEval::Result_t::const_iterator iter = res.find(aAssign);
            DataflowAPI::Result_t::const_iterator iter = res.find(aAssign);
            if (iter == res.end()) break;

            AST::Ptr p = iter->second;
            if (!p) break;

            /* If we have a constant value, add it to our findings */
            if (p->getID() == AST::V_ConstantAST) {
                if (foundValues[correspondingAbsRegion].first == true) {
                    curVal = (long int)DataflowAPI::ConstantAST::convert(p)->val().val;
                    if (curVal != foundValues[correspondingAbsRegion].second) {
                        foundValues[correspondingAbsRegion] = make_pair(true, (long int)-1);
                        break;
                    } 
                } else {
                    val = (long int)DataflowAPI::ConstantAST::convert(p)->val().val;
                    foundValues[correspondingAbsRegion] = make_pair(true, val);
                }

            }
        }
    }

    /* When doing final processing, we'll take care of parameter types */
    long int foundVal;
    char * stringVal;
    int * intVal;

    ParseAPI::CodeSource * cs = f->obj()->cs();

    for (unsigned i = 1; i < regsToSliceFor.size()+1; i++) {
        foundVal = foundValues[i-1].second;
        if (params[i] == _s) {
            if (foundVal == -1) {
                curTrap.push_back((void*)"unknown");
            } else {
                stringVal = (char*)(cs->getPtrToData(foundVal));
                if (stringVal != NULL) {
                    curTrap.push_back((void*)stringVal);
                } else {
                    curTrap.push_back((void*)"unknown");
                }
            }
        } else if (params[i] == _p) {
            if (foundVal == -1) {
                curTrap.push_back((void*)(long int)-1);
            } else {
                intVal = (int *)(cs->getPtrToData(foundVal));
                if (intVal != NULL) {
                    curTrap.push_back((void*)(long int)(*intVal));
                } else {
                    curTrap.push_back((void*)(long int)-1);
                }
            }
        } else if (params[i] == _i) {
            curTrap.push_back((void*)foundVal);
        } else {
            curTrap.push_back((void*)-1);
        }
    }

    return true;
}



