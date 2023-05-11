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
#include "Parser.h"

#include <algorithm>

#include "dyntypes.h"

#include "CodeObject.h"
#include "CFG.h"

#include "debug_parse.h"
#include "util.h"
#include "LoopAnalyzer.h"
#include "dominator.h"

#include "dataflowAPI/h/slicing.h"
#include "dataflowAPI/h/AbslocInterface.h"
#include "instructionAPI/h/InstructionDecoder.h"
#include "common/h/Graph.h"
#include "StackTamperVisitor.h"

#include "common/src/dthread.h"
#include <boost/thread/lock_guard.hpp>

using namespace std;

using namespace Dyninst;
using namespace Dyninst::ParseAPI;


Function::Function() :
        _start(0),
        _obj(NULL),
        _region(NULL),
        _isrc(NULL),
        _cache_valid(false),
        _src(RT),
        _rs(UNSET),
        _entry(NULL),
	 _is_leaf_function(true),
	 _ret_addr(0),
        _parsed(false),
        _no_stack_frame(true),
        _saves_fp(false),
        _cleans_stack(false),
        _tamper(TAMPER_UNSET),
        _tamper_addr(0),
	_loop_analyzed(false),
	_loop_root(NULL),
	isDominatorInfoReady(false),
	isPostDominatorInfoReady(false)

{
    fprintf(stderr,"PROBABLE ERROR, default ParseAPI::Function constructor\n");
}

Function::Function(Address addr, string name, CodeObject * obj, 
    CodeRegion * region, InstructionSource * isrc) :
        _start(addr),
        _obj(obj),
        _region(region),
        _isrc(isrc),
        _cache_valid(false),
        _src(RT),
        _rs(UNSET),
        _name(name),
        _entry(NULL),
	 _is_leaf_function(true),
	 _ret_addr(0),
        _parsed(false),
        _no_stack_frame(true),
        _saves_fp(false),
        _cleans_stack(false),
        _tamper(TAMPER_UNSET),
        _tamper_addr(0),
	_loop_analyzed(false),
	_loop_root(NULL),
	isDominatorInfoReady(false),
	isPostDominatorInfoReady(false)


{
    if (obj->defensiveMode()) {
        mal_printf("new funct at %lx\n",addr);
    }
    if (obj && obj->cs()) {
        obj->cs()->incrementCounter(PARSE_FUNCTION_COUNT);
    }
    if (obj && obj->cs() && obj->cs()->nonReturning(name)) {
        set_retstatus(NORETURN);
    }
}

ParseAPI::Edge::~Edge() {
}

Function::~Function()
{
    if (_obj && _obj->cs()) {
        _obj->cs()->decrementCounter(PARSE_FUNCTION_COUNT);
    }
    vector<FuncExtent *>::iterator eit = _extents.begin();
    for( ; eit != _extents.end(); ++eit) {
        delete *eit;
    }
    for (auto lit = _loops.begin(); lit != _loops.end(); ++lit)
        delete *lit;
}

Function::blocklist
Function::blocks()
{
    boost::lock_guard<Function> g(*this);
    if(!_cache_valid)
        finalize();
    return blocklist(blocks_begin(), blocks_end());
}

// Get the current set of blocks,
// as a const operation
Function::const_blocklist
Function::blocks() const
{
    boost::lock_guard<const Function> g(*this);
  assert(_cache_valid);
  
  return const_blocklist(blocks_begin(), blocks_end());
}


const Function::edgelist & 
Function::callEdges() {
    boost::lock_guard<Function> g(*this);
    if(!_cache_valid)
        finalize();
    return _call_edge_list; 
}

Function::const_blocklist
Function::returnBlocks() {
    boost::lock_guard<Function> g(*this);
  if (!_cache_valid)
    finalize();
  return const_blocklist(ret_begin(), ret_end());
}

Function::const_blocklist
Function::exitBlocks() {
    boost::lock_guard<Function> g(*this);
  if (!_cache_valid)
    finalize();
  return const_blocklist(exit_begin(), exit_end());
  
}


Function::const_blocklist
Function::exitBlocks() const {
    boost::lock_guard<const Function> g(*this);
    assert(_cache_valid);
    return const_blocklist(exit_begin(), exit_end());

}

vector<FuncExtent *> const&
Function::extents()
{
    boost::lock_guard<Function> g(*this);
    if(!_cache_valid)
        finalize(); 
    return _extents;
}

void
Function::finalize()
{
    boost::lock_guard<Function> g(*this);
    bool done;
    do {
  _extents.clear();
  _exitBL.clear();

  // for each block, decrement its refcount
  for (auto blk = blocks_begin(); blk != blocks_end(); blk++) {
    (*blk)->_func_cnt.fetch_add(-1);
  }
  _bmap.clear();
  _retBL.clear(); 
  _call_edge_list.clear();
  _cache_valid = false;

    // The Parser knows how to finalize
    // a Function's parse data
    done  = _obj->parser->finalize(this);
    } while (!done);
}

Function::blocklist
Function::blocks_int() 
{
    boost::lock_guard<Function> g(*this);
    if(_cache_valid || !_entry)
      return blocklist(blocks_begin(), blocks_end());

    // overloaded map warning:
    // visited[addr] == 1 means visited
    // visited[addr] == 2 means already on the return list
    dyn_hash_map<Address,short> visited;
    vector<Block *> worklist;

    bool need_entry = true;
    for(auto bit=blocks_begin();
        bit!=blocks_end();++bit) 
    {
        Block * b = *bit;
        visited[b->start()] = 1;
        need_entry = need_entry && (b != _entry);
    }
    worklist.insert(worklist.begin(),blocks_begin(), blocks_end());

    if(need_entry) {
        worklist.push_back(_entry);
        visited[_entry->start()] = 1;
        add_block(_entry);
    }

    // We need to revalidate that the exit blocks we found before are still exit blocks
    blockmap::iterator nextIt, curIt;
    for (curIt = _exitBL.begin(); curIt != _exitBL.end(); ) {
        Block *cur = curIt->second;
        bool exit_func = false;
        bool found_call = false;
        bool found_call_ft = false;
        boost::lock_guard<Block> blockGuard(*cur);
        
        Block::edgelist targets;
        cur->copy_targets(targets);
        if (targets.empty()) exit_func = true;
        for (auto e : targets) {
            Block *t = e->trg();
            if(e->type() == CALL || e->interproc()) {
                found_call = true;
            }
            if (e->type() == CALL_FT) {
                found_call_ft = true;
            }
            
            if (e->type() == RET || !t || t->obj() != cur->obj()) {
                exit_func = true;
                break;
            }
        }
        if (found_call && !found_call_ft && !obj()->defensiveMode()) exit_func = true;
        
        if (!exit_func) { 
            nextIt = curIt;
            ++nextIt;
            _exitBL.erase(curIt);
            curIt = nextIt;
        } else ++curIt;
    }

    // avoid adding duplicate return blocks
    for(auto bit=exit_begin();
        bit!=exit_end();++bit)
    {
        Block * b = *bit;
        visited[b->start()] = 2;
    }
    
    while(!worklist.empty()) {
        Block * cur = worklist.back();
        worklist.pop_back();

        bool link_return = false;
        bool exits_func = false;
        bool found_call = false;
        bool found_call_ft = false;
        Block::edgelist trgs;
        cur->copy_targets(trgs);
        if (trgs.empty()) {
            // Woo hlt!
            parsing_printf("No targets, exits func\n");
            exits_func = true;
        }
        for(auto e : trgs) {
            Block * t = e->trg();
            if(t) {
                parsing_printf("\t Considering target block [0x%lx,0x%lx) from edge %p\n",
                               t->start(), t->end(), (void*)e);
            }

            if (e->type() == CALL_FT) {
                found_call_ft = true;
            }

            if(e->type() == CALL) {
                parsing_printf("\t Call typed\n");
                _call_edge_list.insert(e);
                found_call = true;
                continue;
            }

            if(e->type() == RET) {
                link_return = true;
                exits_func = true;
                parsing_printf("Block has return edge\n");
                if (obj()->defensiveMode()) {
                    if (_tamper != TAMPER_UNSET && _tamper != TAMPER_NONE) 
                        continue;
                }
                continue;
            }

            /* Handle tailcall edges */
            if(e->interproc()) {
                parsing_printf("\t Interprocedural\n");
                _call_edge_list.insert(e);
                found_call = true;
                continue;
            }
            // If we are heading to a different CodeObject, call it a return
            // and don't add target blocks.
            if (t && (t->obj() != cur->obj())) {
                // This is a jump to a different CodeObject; call it an exit
                parsing_printf("Block exits object\n");
                exits_func = true;
                continue;
            }

            /* sink edges receive no further processing */
            if(e->sinkEdge()) {
                parsing_printf("\t Sink edge, skipping\n");
                continue;
            }

            if(!HASHDEF(visited,e->trg_addr())) {
                if(t) {
                    parsing_printf("\t Adding target block [%lx,%lx) to worklist according to edge from %lx, type %d\n", t->start(), t->end(), e->src()->last(), e->type());
                    worklist.push_back(t);
                    visited[e->trg_addr()] = 1;
                    add_block(t);
                }
            }
        }

        if (found_call && !found_call_ft && !obj()->defensiveMode()) {
            parsing_printf("\t exits func\n");
            exits_func = true;
        }

        if (link_return) assert(exits_func);

        if(exits_func) {
            if (link_return)
                delayed_link_return(_obj,cur);
            if(visited[cur->start()] <= 1) {
                _exitBL[cur->start()] = cur;
                parsing_printf("Adding block 0x%lx as exit\n", cur->start());
                if (link_return) {
                    _retBL[cur->start()] = cur;
                }
            }
        }
    }

    return blocklist(blocks_begin(), blocks_end());
}

/* Adds return edges to the CFG for a particular retblk, based 
 * on callers to this function.  Handles case of return block 
 * that targets the entry block of a new function separately
 * (ret to entry happens if the function tampers with its stack 
 *  and maybe if this function is a signal handler?) 
 */ 
void
Function::delayed_link_return(CodeObject * o, Block * retblk)
{
    boost::lock_guard<Function> g(*this);
    bool link_entry = false;
    Block::edgelist::const_iterator eit;
    dyn_hash_map<Address,bool> linked;
    {
        boost::lock_guard<Block> blockGuard(*retblk);
        eit = retblk->targets().begin();
        for( ; eit != retblk->targets().end(); ++eit) {
            Edge * e = *eit;
            linked[e->trg_addr()] = true;
        }

    }

    boost::lock_guard<Block> g2(*_entry);
    eit = _entry->sources().begin();
    for( ; eit != _entry->sources().end(); ++eit) {
        Edge * e = *eit;
        if(e->type() == CALL) {
            parsing_printf("[%s:%d] linking return edge %lx -> %lx\n",
                FILE__,__LINE__,retblk->lastInsnAddr(),e->src()->end());

            // XXX opportunity here to be more conservative about delayed
            //     determination of return status
    
            Block * call_ft = _obj->findBlockByEntry(region(),e->src()->end());
            if(!call_ft) {
                parsing_printf("[%s:%d] no block found, error!\n",
                    FILE__,__LINE__);
            } 
            else if(!HASHDEF(linked,call_ft->start())) {
                if(call_ft == _entry)
                    link_entry = true;
                else 
                    o->add_edge(retblk,call_ft,RET);
                linked[call_ft->start()] = true;
            }
        }
    }
    // can't do this during iteration
    if(link_entry)
        o->add_edge(retblk,_entry,RET);
}

void
Function::add_block(Block *b)
{
    boost::lock_guard<Function> g(*this);
    b->_func_cnt.fetch_add(1);            // block counts references
    _bmap[b->start()] = b;
}

const string &
Function::name() const
{
    return _name;
}

bool
Function::contains(Block *b)
{
    boost::lock_guard<Function> g(*this);
    if (b == NULL) return false;
    if(!_cache_valid)
        finalize();

    return HASHDEF(_bmap,b->start());
}

bool
Function::contains(Block *b) const
{
//    boost::lock_guard<const Function> g(*this);
    if (b == NULL) return false;
    return HASHDEF(_bmap,b->start());
}


void Function::setEntryBlock(Block *new_entry)
{
    boost::lock_guard<Function> g(*this);
    obj()->parser->move_func(this, new_entry->start(), new_entry->region());
    _region = new_entry->region();
    _start = new_entry->start();
    _entry = new_entry;
}

void Function::set_retstatus(FuncReturnStatus rs) 
{
    boost::lock_guard<Function> g(*this);
    // If this function is a known non-returning function,
    // we should ignore this result.
    // A exmaple is .Unwind_Resume, which is non-returning.
    // But on powerpc, the function contains a BLR instruction,
    // looking like a return instruction, but actually is not.
    if (obj()->cs()->nonReturning(_name) && rs != NORETURN) return;
    parsing_printf("Set function %s at %lx ret status from %d to %d\n", _name.c_str(), addr(), _rs.load(), rs);
    if (_rs == RETURN && rs == NORETURN) {
        parsing_printf("\tERROR: ret status is already set to RETURN, now setting to NORETURN\n");
    }
    if (_rs == NORETURN && rs == RETURN) {
        parsing_printf("\tERROR: ret status is already set to NORETURN, now setting to RETURN\n");
    }

    // If we are changing the return status, update prev counter
    if (_rs != UNSET) {
        if (_rs == NORETURN) {
            _obj->cs()->decrementCounter(PARSE_NORETURN_COUNT);
        } else if (_rs == RETURN) {
            _obj->cs()->decrementCounter(PARSE_RETURN_COUNT);
        } else if (_rs == UNKNOWN) {
            _obj->cs()->decrementCounter(PARSE_UNKNOWN_COUNT);
        }
    }

    // Update counter information
    if (rs == NORETURN) {
        _obj->cs()->incrementCounter(PARSE_NORETURN_COUNT);
    } else if (rs == RETURN) {
        _obj->cs()->incrementCounter(PARSE_RETURN_COUNT);
    } else if (rs == UNKNOWN) {
        _obj->cs()->incrementCounter(PARSE_UNKNOWN_COUNT);
    }
    // Write access is handled by the lock, so this should always work.
    // Helgrind gets confused, so the cmp&swap hides the actual write.
    FuncReturnStatus e = _rs;
    assert(_rs.compare_exchange_strong(e, rs));
}

void 
Function::removeBlock(Block* dead)
{
    boost::lock_guard<Function> g(*this);
    _cache_valid = false;
    // specify replacement entry prior to deleting entry block, unless 
    // deleting all blocks
    if (dead == _entry) {
        mal_printf("Warning: removing entry block [%lx %lx) for function at "
                   "%lx\n", dead->start(), dead->end(), addr());
        _entry = NULL;
        assert(0);
    }

    // remove dead block from _retBL and _call_edge_list
    boost::lock_guard<Block> g2(*dead);
    const Block::edgelist & outs = dead->targets();
    for (Block::edgelist::const_iterator oit = outs.begin();
         outs.end() != oit; 
         oit++ ) 
    {
        switch((*oit)->type()) {
            case CALL: {
                bool foundEdge = false;
                for (set<Edge*>::iterator cit = _call_edge_list.begin();
                     _call_edge_list.end() != cit;
                     cit++) 
                {
                    if (*oit == *cit) {
                        foundEdge = true;
                        _call_edge_list.erase(cit);
                        break;
                    }
                }
                assert(foundEdge || (*oit)->sinkEdge());
                break;
            }
            case RET:
	      _retBL.erase(dead->start());
	      break;
            default:
                break;
        }
    }
    // remove dead block from block map
    _bmap.erase(dead->start());
    _exitBL.erase(dead->start());
}

class ST_Predicates : public Slicer::Predicates {};

StackTamper 
Function::tampersStack(bool recalculate)
{
    boost::lock_guard<Function> g(*this);
    using namespace SymbolicEvaluation;
    using namespace InstructionAPI;

    if ( ! obj()->defensiveMode() ) { 
        assert(0);
        _tamper = TAMPER_NONE;
        return _tamper;
    }
    // this is above the cond'n below b/c it finalizes the function, 
    // which could in turn call this function
    Function::const_blocklist retblks(returnBlocks());
    if ( retblks.begin() == retblks.end() ) {
        _tamper = TAMPER_NONE;
        return _tamper;
    }
        // The following line leads to dangling pointers, but leaving
        // in until we understand why it was originally there.
	//_cache_valid = false;

    // if we want to re-calculate the tamper address
    if (!recalculate && TAMPER_UNSET != _tamper) {
        return _tamper;
    }
	assert(_cache_valid);
    AssignmentConverter converter(true, true);
    vector<Assignment::Ptr> assgns;
    ST_Predicates preds;
    _tamper = TAMPER_UNSET;
    for (auto bit = retblks.begin(); retblks.end() != bit; ++bit) {
		assert(_cache_valid);
        Address retnAddr = (*bit)->lastInsnAddr();
        InstructionDecoder retdec(this->isrc()->getPtrToInstruction(retnAddr), 
                                  InstructionDecoder::maxInstructionLength, 
                                  this->region()->getArch() );
        Instruction retn = retdec.decode();
        converter.convert(retn, retnAddr, this, *bit, assgns);
        vector<Assignment::Ptr>::iterator ait;
        AST::Ptr sliceAtRet;

        for (ait = assgns.begin(); assgns.end() != ait; ait++) {
            AbsRegion & outReg = (*ait)->out();
            if ( outReg.absloc().isPC() ) {
                // First check to see if an input is an unresolved stack slot 
                // (or worse, the heap) - since if that's the case there's no use
                // in spending a lot of time slicing.
                std::vector<AbsRegion>::const_iterator in_iter;
                for (in_iter = (*ait)->inputs().begin();
                    in_iter != (*ait)->inputs().end(); ++in_iter) {
                    if (in_iter->type() != Absloc::Unknown) {
                        _tamper = TAMPER_NONZERO;
                        _tamper_addr = 0;
                        set_retstatus(NORETURN);
                        mal_printf("Stack tamper analysis for ret block at "
                               "%lx found unresolved stack slot or heap "
                               "addr, marking as TAMPER_NONZERO\n", retnAddr);
                        return _tamper;
                    }
                }

                Slicer slicer(*ait,*bit,this);
                Graph::Ptr slGraph = slicer.backwardSlice(preds);
                DataflowAPI::Result_t slRes;
                DataflowAPI::SymEval::expand(slGraph,slRes);
                sliceAtRet = slRes[*ait];
                if (dyn_debug_malware && sliceAtRet != NULL) {
                    cerr << "assignment " << (*ait)->format() << " is "
                         << sliceAtRet->format() << "\n";
                }
                break;
            }
        }
        if (sliceAtRet == NULL) {
            mal_printf("Failed to produce a slice for retn at %lx %s[%d]\n",
                       retnAddr, FILE__,__LINE__);
            continue;
        } 
        StackTamperVisitor vis(Absloc(-static_cast<int>(isrc()->getAddressWidth()), 0, this));
        Address curTamperAddr=0;
        StackTamper curtamper = vis.tampersStack(sliceAtRet, curTamperAddr);
        mal_printf("StackTamperVisitor for func at 0x%lx block[%lx %lx) w/ "
                   "lastInsn at 0x%lx returns tamper=%d tamperAddr=0x%lx\n",
                   _start, (*bit)->start(), (*bit)->end(), retnAddr, 
                   curtamper, curTamperAddr);
        if (TAMPER_UNSET == _tamper || TAMPER_NONE == _tamper ||
            (TAMPER_NONZERO == _tamper && 
             TAMPER_NONE != curtamper))
        {
            _tamper = curtamper;
            _tamper_addr = curTamperAddr;
        } 
        else if ((TAMPER_REL == _tamper   || TAMPER_ABS == _tamper) &&
                 (TAMPER_REL == curtamper || TAMPER_ABS == curtamper))
        {
            if (_tamper != curtamper || _tamper_addr != curTamperAddr) {
                fprintf(stderr, "WARNING! Unhandled case in stackTamper "
                        "analysis, func at %lx has distinct tamperAddrs "
                        "%d:%lx %d:%lx at different return instructions, "
                        "setting to TAMPER_NONZERO %s[%d]\n", 
                        this->addr(), _tamper,_tamper_addr, curtamper, 
                        curTamperAddr, FILE__, __LINE__);
                _tamper = TAMPER_NONZERO; // let instrumentation take care of it
            }
        }
        assgns.clear();
    }

    if ( TAMPER_UNSET == _tamper ) {
        mal_printf("WARNING: we found no valid slices for function at %lx "
                   "%s[%d]\n", _start, FILE__,__LINE__);
        _tamper = TAMPER_NONZERO;
    }

    if ( TAMPER_NONE != _tamper && TAMPER_REL != _tamper && RETURN == _rs ) {
        set_retstatus(NORETURN);
    }
    return _tamper;
}

void Function::destroy(Function *f) {
   f->obj()->destroy(f);
}

LoopTreeNode* Function::getLoopTree() const{
    boost::lock_guard<const Function> g(*this);
  if (_loop_root == NULL) {
      LoopAnalyzer la(this);
      la.createLoopHierarchy();
  }
  return _loop_root;
}

// this methods returns the loop objects that exist in the control flow
// grap. It returns a set. And if there are no loops, then it returns the empty
// set. not NULL.
void Function::getLoopsByNestingLevel(vector<Loop*>& lbb,
                                              bool outerMostOnly) const
{
    boost::lock_guard<const Function> g(*this);
  if (_loop_analyzed == false) {
      LoopAnalyzer la(this);
      la.analyzeLoops();
      _loop_analyzed = true;
  }

  for (std::set<Loop *>::iterator iter = _loops.begin();
       iter != _loops.end(); ++iter) {
     // if we are only getting the outermost loops
     if (outerMostOnly && 
         (*iter)->parentLoop() != NULL) continue;

     lbb.push_back(*iter);
  }
  return;
}


// get all the loops in this flow graph
bool
Function::getLoops(vector<Loop*>& lbb) const
{
    boost::lock_guard<const Function> g(*this);
  getLoopsByNestingLevel(lbb, false);
  return true;
}

// get the outermost loops in this flow graph
bool
Function::getOuterLoops(vector<Loop*>& lbb) const
{
    boost::lock_guard<const Function> g(*this);
  getLoopsByNestingLevel(lbb, true);
  return true;
}

Loop *Function::findLoop(const char *name) const
{
    boost::lock_guard<const Function> g(*this);
  return getLoopTree()->findLoop(name);
}


//this method fill the dominator information of each basic block
//looking at the control flow edges. It uses a fixed point calculation
//to find the immediate dominator of the basic blocks and the set of
//basic blocks that are immediately dominated by this one.
//Before calling this method all the dominator information
//is going to give incorrect results. So first this function must
//be called to process dominator related fields and methods.
void Function::fillDominatorInfo() const
{
    boost::lock_guard<const Function> g(*this);
    if (!isDominatorInfoReady) {
        dominatorCFG domcfg(this);
	domcfg.calcDominators();
	isDominatorInfoReady = true;
    }
}

void Function::fillPostDominatorInfo() const
{
    boost::lock_guard<const Function> g(*this);
    if (!isPostDominatorInfoReady) {
        dominatorCFG domcfg(this);
	domcfg.calcPostDominators();
	isPostDominatorInfoReady = true;
    }
}

bool Function::dominates(Block* A, Block *B) const {
    boost::lock_guard<const Function> g(*this);
    if (A == NULL || B == NULL) return false;
    if (A == B) return true;

    fillDominatorInfo();

    if (!immediateDominates[A]) return false;

    for (auto bit = immediateDominates[A]->begin(); bit != immediateDominates[A]->end(); ++bit)
        if (dominates(*bit, B)) return true;
    return false;
}
        
Block* Function::getImmediateDominator(Block *A) const {
    boost::lock_guard<const Function> g(*this);
    fillDominatorInfo();
    return immediateDominator[A];
}

void Function::getImmediateDominates(Block *A, set<Block*> &imd) const {
    boost::lock_guard<const Function> g(*this);
    fillDominatorInfo();
    if (immediateDominates[A] != NULL)
        imd.insert(immediateDominates[A]->begin(), immediateDominates[A]->end());
}

void Function::getAllDominates(Block *A, set<Block*> &d) const {
    boost::lock_guard<const Function> g(*this);
    fillDominatorInfo();
    d.insert(A);
    if (immediateDominates[A] == NULL) return;

    for (auto bit = immediateDominates[A]->begin(); bit != immediateDominates[A]->end(); ++bit)
        getAllDominates(*bit, d);
}

bool Function::postDominates(Block* A, Block *B) const {
    boost::lock_guard<const Function> g(*this);
    if (A == NULL || B == NULL) return false;
    if (A == B) return true;

    fillPostDominatorInfo();

    if (!immediatePostDominates[A]) return false;

    for (auto bit = immediatePostDominates[A]->begin(); bit != immediatePostDominates[A]->end(); ++bit)
        if (postDominates(*bit, B)) return true;
    return false;
}
        
Block* Function::getImmediatePostDominator(Block *A) const {
    boost::lock_guard<const Function> g(*this);
    fillPostDominatorInfo();
    return immediatePostDominator[A];
}

void Function::getImmediatePostDominates(Block *A, set<Block*> &imd) const {
    boost::lock_guard<const Function> g(*this);
    fillPostDominatorInfo();
    if (immediatePostDominates[A] != NULL)
        imd.insert(immediatePostDominates[A]->begin(), immediatePostDominates[A]->end());
}

void Function::getAllPostDominates(Block *A, set<Block*> &d) const {
    boost::lock_guard<const Function> g(*this);
    fillPostDominatorInfo();
    d.insert(A);
    if (immediatePostDominates[A] == NULL) return;

    for (auto bit = immediatePostDominates[A]->begin(); bit != immediatePostDominates[A]->end(); ++bit)
        getAllPostDominates(*bit, d);
}
