#include <algorithm>

#include "dyntypes.h"

#include "CodeObject.h"
#include "CFG.h"

#include "Parser.h"
#include "debug_parse.h"
#include "util.h"

#include "dataflowAPI/h/slicing.h"
#include "dataflowAPI/h/AbslocInterface.h"
#include "instructionAPI/h/InstructionDecoder.h"
#include "dynutil/h/Graph.h"
#include "StackTamperVisitor.h"

using namespace std;

using namespace Dyninst;
using namespace Dyninst::ParseAPI;

Function::Function() :
        _start(0),
        _obj(NULL),
        _region(NULL),
        _isrc(NULL),
        _src(RT),
        _rs(UNSET),
        _entry(NULL),
	 _is_leaf_function(true),
	 _ret_addr(0),
        _parsed(false),
        _cache_valid(false),
        _bl(_blocks),
        _call_edge_list(_call_edges),
	_retBL(_return_blocks),
        _no_stack_frame(true),
        _saves_fp(false),
        _cleans_stack(false),
        _tamper(TAMPER_UNSET),
        _tamper_addr(0)
{
    fprintf(stderr,"PROBABLE ERROR, default ParseAPI::Function constructor\n");
}

Function::Function(Address addr, string name, CodeObject * obj, 
    CodeRegion * region, InstructionSource * isrc) :
        _start(addr),
        _obj(obj),
        _region(region),
        _isrc(isrc),
        _src(RT),
        _rs(UNSET),
        _name(name),
        _entry(NULL),
	 _is_leaf_function(true),
	 _ret_addr(0),
        _parsed(false),
        _cache_valid(false),
        _bl(_blocks),
        _call_edge_list(_call_edges),
	_retBL(_return_blocks),
        _no_stack_frame(true),
        _saves_fp(false),
        _cleans_stack(false),
        _tamper(TAMPER_UNSET),
        _tamper_addr(0)
{
    if (obj->defensiveMode()) {
        mal_printf("new funct at %lx\n",addr);
    }
}

ParseAPI::Edge::~Edge() {
}

Function::~Function()
{
    vector<FuncExtent *>::iterator eit = _extents.begin();
    for( ; eit != _extents.end(); ++eit) {
        delete *eit;
    }
}

Function::blocklist &
Function::blocks()
{
    if(!_cache_valid)
        finalize();
    return _bl;
}

Function::edgelist & 
Function::callEdges() {
    if(!_cache_valid)
        finalize();
    return _call_edge_list; 
}

Function::blocklist &
Function::returnBlocks() {
  if (!_cache_valid) 
    finalize();
  return _retBL;
}

vector<FuncExtent *> const&
Function::extents()
{
    if(!_cache_valid)
        finalize(); 
    return _extents;
}

void
Function::finalize()
{
    // The Parser knows how to finalize
    // a Function's parse data
    _obj->parser->finalize(this);
}

vector<Block *> const&
Function::blocks_int()
{
    if(_cache_valid)
        return _blocks;

    // overloaded map warning:
    // visited[addr] == 1 means visited
    // visited[addr] == 2 means already on the return list
    dyn_hash_map<Address,short> visited;
    vector<Block *> worklist;

    bool need_entry = true;
    for(vector<Block*>::iterator bit=_blocks.begin();
        bit!=_blocks.end();++bit) 
    {
        Block * b = *bit;
        visited[b->start()] = 1;
        need_entry = need_entry && (b != _entry);
    }
    worklist.insert(worklist.begin(),_blocks.begin(),_blocks.end());

    if(need_entry) {
        worklist.push_back(_entry);
        visited[_entry->start()] = 1;
        add_block(_entry);
    }

    // avoid duplicating return edges
    for(vector<Block*>::iterator bit=_return_blocks.begin();
        bit!=_return_blocks.end();++bit)
    {
        Block * b = *bit;
        visited[b->start()] = 2;
    }
    
    while(!worklist.empty()) {
        Block * cur = worklist.back();
        worklist.pop_back();

        bool link_return = false;
        const Block::edgelist & trgs = cur->targets();
        for(Block::edgelist::iterator tit=trgs.begin();
            tit!=trgs.end();++tit) {
            Edge * e = *tit;
            Block * t = e->trg();


            if(e->type() == CALL) {
                _call_edges.insert(e);
                continue;
            }

            if(e->type() == RET) {
                link_return = true;
				_rs = RETURN;
                continue;
            }

            // If we are heading to a different CodeObject, call it a return
            // and don't add target blocks.
            if (t->obj() != cur->obj()) {
                // Wowza
                // Call or return?
                continue;
            }

            /* sink edges receive no further processing */
            if(e->sinkEdge())
                continue;

            if(!HASHDEF(visited,t->start())) {
                worklist.push_back(t);
                visited[t->start()] = true;
                add_block(t);
            }
        } 
        if(link_return) {
            delayed_link_return(_obj,cur);
            if(visited[cur->start()] <= 1)
                _return_blocks.push_back(cur);
        }
    }

    Block::compare comp;
    sort(_blocks.begin(),_blocks.end(),comp);

    return _blocks;
}

void
Function::delayed_link_return(CodeObject * o, Block * retblk)
{
    bool link_entry = false;

    dyn_hash_map<Address,bool> linked;
    Block::edgelist::iterator eit = retblk->targets().begin();
    for( ; eit != retblk->targets().end(); ++eit) {
        Edge * e = *eit;
        linked[e->trg()->start()] = true;
    }

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
	//cerr << "Adding block @ " << hex << b->start() << " to func @ " << addr() << dec << endl;
	++b->_func_cnt;            // block counts references
    _blocks.push_back(b);
    _bmap[b->start()] = b;
}

const string &
Function::name() 
{
    return _name;
}

bool
Function::contains(Block *b)
{
    if(!_cache_valid)
        finalize();

    return HASHDEF(_bmap,b->start());
}

void Function::setEntryBlock(Block *new_entry)
{
    obj()->parser->move_func(this, new_entry->start(), new_entry->region());
    _region = new_entry->region();
    _start = new_entry->start();
    _entry = new_entry;
}

void 
Function::deleteBlocks(vector<Block*> dead_blocks)
{
    _cache_valid = false;
    bool deleteAll = (dead_blocks.size() == _blocks.size());
    bool hasSharedDeadBlocks = false;

    for (unsigned didx=0; didx < dead_blocks.size(); didx++) {
        bool found = false;
        Block *dead = dead_blocks[didx];

        // remove dead block from _blocks
        std::vector<Block *>::iterator biter = _blocks.begin();
        while ( !found && _blocks.end() != biter ) {
            if (dead == *biter) {
                found = true;
                biter = _blocks.erase(biter);
            }
            else {
                biter++;
            }
        }
        if (!found) {
            fprintf(stderr,"Error, tried to remove block [%lx,%lx) from "
                    "function at %lx that it does not belong to at %s[%d]\n",
                    dead->start(),dead->end(), addr(), FILE__,__LINE__);
            assert(0);
        }

        // specify replacement entry prior to deleting entry block, unless 
        // deleting all blocks
        assert(deleteAll || dead != _entry);

        // remove dead block from _return_blocks and its call edges from vector
        Block::edgelist & outs = dead->targets();
        found = false;
        for (Block::edgelist::iterator oit = outs.begin();
             !found && outs.end() != oit; 
             oit++ ) 
        {
            switch((*oit)->type()) {
                case CALL:
                    for (set<Edge*>::iterator cit = _call_edges.begin(); 
                         _call_edges.end() != cit;
                         cit++) 
                    {
                        if (*oit == *cit) {
                            found = true;
                            _call_edges.erase(cit);
                            break;
                        }
                    }
                    assert(found);
                    break;
                case RET:
                    _return_blocks.erase(std::remove(_return_blocks.begin(),
                                                     _return_blocks.end(),
                                                     dead),
                                         _return_blocks.end());
                    found = true;
                    break;
                default:
                    break;
            }
        }
        // remove dead block from block map
        _bmap.erase(dead->start());

        // disconnect dead block from CFG (if not shared by other funcs)
        if (1 == dead->containingFuncs()) {
            for (unsigned sidx=0; sidx < dead->_sources.size(); sidx++) {
                Edge *edge = dead->_sources[sidx];
                if (edge->type() == CALL) {
                    std::vector<Function *> funcs;
                    edge->src()->getFuncs(funcs);
                    for (unsigned k = 0; k < funcs.size(); ++k) {
                        funcs[k]->_call_edges.erase(edge);
                    }
                }
                edge->src()->removeTarget( edge );
                obj()->fact()->free_edge(edge);
            }
            for (unsigned tidx=0; tidx < dead->_targets.size(); tidx++) {
                Edge *edge = dead->_targets[tidx];
                edge->trg()->removeSource( edge );
                obj()->fact()->free_edge(edge);
            }
        }
        // KEVIN TODO
        // Moved remove_block farther down to guard against shared code
    }

    // delete the blocks
    for (unsigned didx=0; didx < dead_blocks.size(); didx++) {
        Block *dead = dead_blocks[didx];
        if (dead->_func_cnt >= 2) {
            dead->removeFunc(this);
            hasSharedDeadBlocks = true;
            mal_printf("WARNING: removing shared block [%lx %lx] rather "
                       "than deleting it %s[%d]\n", dead->start(), 
                       dead->end(), FILE__,__LINE__);
        } else {
            // remove from internal parsing datastructures
            obj()->parser->remove_block(dead);

            obj()->fact()->free_block(dead);
        }
    }

    // call finalize, fixes extents
    _cache_valid = false;
    if (!deleteAll && !hasSharedDeadBlocks) {
        //Don't think this is necessary or wanted, Jan 4, 2011
        //obj()->parser->finalize(this);
    }
}

class ST_Predicates : public Slicer::Predicates {};

StackTamper 
Function::tampersStack(bool recalculate)
{
    using namespace SymbolicEvaluation;
    using namespace InstructionAPI;

    if ( ! obj()->defensiveMode() ) { 
        assert(0);
        _tamper = TAMPER_NONE;
        return _tamper;
    }

    // this is above the cond'n below b/c it finalizes the function, 
    // which could in turn call this function
    Function::blocklist & retblks = returnBlocks();
    if ( retblks.begin() == retblks.end() ) {
        _tamper = TAMPER_NONE;
        return _tamper;
    }
	_cache_valid = false;

    // if we want to re-calculate the tamper address
    if (!recalculate && TAMPER_UNSET != _tamper) {
        return _tamper;
    }

    AssignmentConverter converter(true);
    vector<Assignment::Ptr> assgns;
    ST_Predicates preds;
    _tamper = TAMPER_UNSET;
    Function::blocklist::iterator bit;
    for (bit = retblks.begin(); retblks.end() != bit; ++bit) {
        Address retnAddr = (*bit)->lastInsnAddr();
        InstructionDecoder retdec(this->isrc()->getPtrToInstruction(retnAddr), 
                                  InstructionDecoder::maxInstructionLength, 
                                  this->region()->getArch() );
        Instruction::Ptr retn = retdec.decode();
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
                        mal_printf("Stack tamper analysis for ret block at "
                               "%lx found unresolved stack slot or heap "
                               "addr, marking as TAMPER_NONZERO\n", retnAddr);
                        return _tamper;
                    }
                }

                Slicer slicer(*ait,*bit,this);
                Graph::Ptr slGraph = slicer.backwardSlice(preds);
                if (dyn_debug_malware && 0) {
                    stringstream graphDump;
                    graphDump << "sliceDump_" << this->name() << "_" 
                              << hex << retnAddr << dec << ".dot";
                    slGraph->printDOT(graphDump.str());
                }
                DataflowAPI::Result_t slRes;
                DataflowAPI::SymEval::expand(slGraph,slRes);
                sliceAtRet = slRes[*ait];
                if (dyn_debug_malware && sliceAtRet != NULL) {
                    cout << "assignment " << (*ait)->format() << " is "
                         << sliceAtRet->format() << "\n";
                }
                break;
            }
        }
        if (sliceAtRet == NULL) {
            mal_printf("Failed to produce a slice for retn at %x %s[%d]\n",
                       retnAddr, FILE__,__LINE__);
            continue;
        } 
        StackTamperVisitor vis(Absloc(-1 * isrc()->getAddressWidth(), 0, this));
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
                        "discarding second tamperAddr %s[%d]\n", 
                        this->addr(), _tamper,_tamper_addr, curtamper, 
                        curTamperAddr, FILE__, __LINE__);
            }
        }
        assgns.clear();
    }

    if ( TAMPER_UNSET == _tamper ) {
        mal_printf("WARNING: we found no valid slices for function at %lx "
                   "%s[%d]\n", _start, _tamper_addr, FILE__,__LINE__);
        _tamper = TAMPER_NONZERO;
    }

    //if (TAMPER_ABS == _tamper) {
        //Address loadAddr = 0;
        //if (_tamper_addr <  obj()->cs()->loadAddress()) {
        //    _tamper = TAMPER_NONZERO;
        //}
        //else {
        //    _tamper_addr -= obj()->cs()->loadAddress();
        //    if (! obj()->cs()->isCode(_tamper_addr)) {
        //        mal_printf("WARNING: function at %lx tampers its stack to point at "
        //                   "invalid address 0x%lx %s[%d]\n", _start, _tamper_addr,
        //                   FILE__,__LINE__);
        //        _tamper = TAMPER_NONZERO;
        //    }
        //}
    //}
    if ( TAMPER_NONE != _tamper && TAMPER_REL != _tamper && RETURN == _rs ) {
        _rs = NORETURN;
    }
    return _tamper;
}

