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

    dyn_hash_map<Address,bool> visited;
    vector<Block *> worklist;

    bool need_entry = true;
    for(vector<Block*>::iterator bit=_blocks.begin();
        bit!=_blocks.end();++bit) 
    {
        Block * b = *bit;
        visited[b->start()] = true;
        need_entry = need_entry && (b != _entry);
    }
    worklist.insert(worklist.begin(),_blocks.begin(),_blocks.end());

    if(need_entry) {
        worklist.push_back(_entry);
        visited[_entry->start()] = true;
        add_block(_entry);
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
                _call_edges.push_back(e);
                continue;
            }

            if(e->type() == RET) {
                link_return = true;
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
    Block::edgelist::iterator eit = _entry->sources().begin();
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
            } else if(call_ft == _entry)
                link_entry = true;
            else 
                o->add_edge(retblk,call_ft,RET);
        }
    }
    // can't do this during iteration
    if(link_entry)
        o->add_edge(retblk,_entry,RET);
}

void
Function::add_block(Block *b)
{
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

void 
Function::deleteBlocks(vector<Block*> &dead_blocks, Block * new_entry)
{
    _cache_valid = false;
    if (new_entry) {
        _start = new_entry->start();
        _entry = new_entry;
    }

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

        // remove dead block from _return_blocks and its call edges from vector
        Block::edgelist & outs = dead->targets();
        found = false;
        
        for (Block::edgelist::iterator oit = outs.begin();
             !found && outs.end() != oit; 
             oit++ ) 
        {
            switch((*oit)->type()) {
                case CALL:
                    for (vector<Edge*>::iterator cit = _call_edges.begin(); 
                         !found && _call_edges.end() != cit; 
                         cit++) 
                    {
                        if (*oit == *cit) {
                            found = true;
                            _call_edges.erase(cit);
                        }
                    }
                    assert(found);
                    break;
                case RET:
                    for (vector<Block*>::iterator rit = _return_blocks.begin();
                         !found && _return_blocks.end() != rit; 
                         rit++) 
                    {
                        if ((*oit)->trg() == *rit) {
                            found = true;
                            _return_blocks.erase(rit);
                        }
                    }
                    break;
                default:
                    break;
            }
        }
        // remove dead block from block map
        _bmap.erase(dead->start());

        // disconnect dead block from CFG
        if (1 < dead->containingFuncs()) {
            for (unsigned sidx=0; sidx < dead->_sources.size(); sidx++) {
                Edge *edge = dead->_sources[sidx];
                edge->src()->removeTarget( edge );
            }
            for (unsigned tidx=0; tidx < dead->_targets.size(); tidx++) {
                Edge *edge = dead->_targets[tidx];
                edge->trg()->removeSource( edge );
            }
        }
    }

    // call finalize, fixes extents
    obj()->parser->finalize(this);

    // delete the blocks
    for (unsigned didx=0; didx < dead_blocks.size(); didx++) {
        Block *dead = dead_blocks[didx];
        if (1 <= dead->containingFuncs()) {
            dead->removeFunc(this);
            mal_printf("WARNING: removing shared block [%lx %lx] rather "
                       "than deleting it %s[%d]\n", dead->start(), 
                       dead->end(), FILE__,__LINE__);
        } else {
            obj()->fact()->free_block(dead);
        }
    }
}

class ST_Predicates : public Slicer::Predicates {};

StackTamper 
Function::tampersStack(bool recalculate)
{
    using namespace SymbolicEvaluation;
    using namespace InstructionAPI;
    // want to re-calculate the tamper address
    if (!recalculate && TAMPER_UNSET != _tamper) {
        return _tamper;
    }

    if ( ! obj()->defensiveMode() ) { 
        assert(0);
        _tamper = TAMPER_NONE;
        return _tamper;
    }

    Function::blocklist & retblks = returnBlocks();
    if ( retblks.begin() == retblks.end() ) {
        _tamper = TAMPER_NONE;
        return _tamper;
    }

    AssignmentConverter converter(true);
    vector<Assignment::Ptr> assgns;
    ST_Predicates preds;
    _tamper = TAMPER_UNSET;
    Function::blocklist::iterator bit;
    for (bit = retblks.begin(); retblks.end() != bit; ++bit) {
        Address retnAddr = (*bit)->lastInsnAddr();
        InstructionDecoder retdec(this->isrc()->getPtrToInstruction( retnAddr ), 
                                  InstructionDecoder::maxInstructionLength, 
                                  this->region()->getArch() );
        Instruction::Ptr retn = retdec.decode();
        converter.convert(retn, retnAddr, this, assgns);
        vector<Assignment::Ptr>::iterator ait;
        AST::Ptr sliceAtRet;

        for (ait = assgns.begin(); assgns.end() != ait; ait++) {
            AbsRegion & outReg = (*ait)->out();
            if ( outReg.absloc().isPC() ) {
                Slicer slicer(*ait,*bit,this);
                Graph::Ptr slGraph = slicer.backwardSlice(preds);
                DataflowAPI::Result_t slRes;
                DataflowAPI::SymEval::expand(slGraph,slRes);
                if (dyn_debug_malware) {
                    stringstream graphDump;
                    graphDump << "sliceDump_" << this->name() 
                              << "_" << hex << retnAddr << ".dot";
                    slGraph->printDOT(graphDump.str());
                }
                sliceAtRet = slRes[*ait];
                mal_printf("assignment %s is %s\n", (*ait)->format().c_str(),
                           sliceAtRet->format().c_str());
                break;
            }
        }
        assert(sliceAtRet != NULL);
        StackTamperVisitor vis(Absloc(-1 * isrc()->getAddressWidth(), 0, this));
        Address curTamperAddr=0;
        StackTamper curtamper = vis.tampersStack(sliceAtRet, curTamperAddr);
        if (TAMPER_UNSET == _tamper || 
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
                fprintf(stderr, "WARNING! Unhandled case in stackTmaper "
                        "analysis, func at %lx has distinct tamperAddrs "
                        "%d:%lx %d:%lx at different return instructions, "
                        "discarding second tamperAddr %s[%d]\n", 
                        this->addr(), _tamper,_tamper_addr, curtamper, 
                        curTamperAddr, FILE__, __LINE__);
            }
        }
        assgns.clear();
    }
    return _tamper;
}
