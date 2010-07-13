#include <algorithm>

#include "dyntypes.h"

#include "CodeObject.h"
#include "CFG.h"

#include "Parser.h"
#include "debug.h"
#include "util.h"

using namespace std;

using namespace Dyninst;
using namespace Dyninst::ParseAPI;

Function::Function() :
        start_(0),
        obj_(NULL),
        region_(NULL),
        isrc_(NULL),
        src_(RT),
        rs_(UNSET),
        entry_(NULL),
        parsed_(false),
        cache_valid_(false),
        bl_(blocks_),
        call_edge_list_(call_edges_),
	retBL_(return_blocks_),
        no_stack_frame_(true),
        saves_fp_(false),
        cleans_stack_(false),
        dangling_(NULL)
{
    fprintf(stderr,"PROBABLE ERROR, default ParseAPI::Function constructor\n");
}

Function::Function(Address addr, string name, CodeObject * obj, 
    CodeRegion * region, InstructionSource * isrc) :
        start_(addr),
        obj_(obj),
        region_(region),
        isrc_(isrc),
        src_(RT),
        rs_(UNSET),
        name_(name),
        entry_(NULL),
        parsed_(false),
        cache_valid_(false),
        bl_(blocks_),
        call_edge_list_(call_edges_),
	retBL_(return_blocks_),
        no_stack_frame_(true),
        saves_fp_(false),
        cleans_stack_(false),
        dangling_(NULL)
{
    
}


Function::~Function()
{
    vector<FuncExtent *>::iterator eit = extents_.begin();
    for( ; eit != extents_.end(); ++eit) {
        delete *eit;
    }

    if(dangling_) delete dangling_;
}

Function::blocklist &
Function::blocks()
{
    if(!cache_valid_)
        finalize();
    return bl_;
}

Function::edgelist & 
Function::callEdges() {
    if(!cache_valid_)
        finalize();
    return call_edge_list_; 
}

Function::blocklist &
Function::returnBlocks() {
  if (!cache_valid_) 
    finalize();
  return retBL_;
}

vector<FuncExtent *> const&
Function::extents()
{
    if(!cache_valid_)
        finalize(); 
    return extents_;
}

void
Function::finalize()
{
    // The Parser knows how to finalize
    // a Function's parse data
    obj_->parser->finalize(this);
}

vector<Block *> const&
Function::blocks_int()
{
    if(cache_valid_)
        return blocks_;

    dyn_hash_map<Address,bool> visited;
    vector<Block *> worklist;

    bool need_entry = true;
    for(vector<Block*>::iterator bit=blocks_.begin();
        bit!=blocks_.end();++bit) 
    {
        Block * b = *bit;
        visited[b->start()] = true;
        need_entry = need_entry && (b != entry_);
    }
    worklist.insert(worklist.begin(),blocks_.begin(),blocks_.end());

    if(need_entry) {
        worklist.push_back(entry_);
        visited[entry_->start()] = true;
        add_block(entry_);
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
                call_edges_.push_back(e);
                continue;
            }

            /* sink edges receive no further processing */
            if(e->sinkEdge())
                continue;

            if(e->type() == RET) {
                link_return = true;
                continue;
            }

            if(!HASHDEF(visited,t->start())) {
                worklist.push_back(t);
                visited[t->start()] = true;
                add_block(t);
            }
        } 
        if(link_return) {
            delayed_link_return(obj_,cur);
            return_blocks_.push_back(cur);
        }
    }

    Block::compare comp;
    sort(blocks_.begin(),blocks_.end(),comp);

    return blocks_;
}

void
Function::delayed_link_return(CodeObject * o, Block * retblk)
{
    bool link_entry = false;
    Block::edgelist::iterator eit = entry_->sources().begin();
    for( ; eit != entry_->sources().end(); ++eit) {
        Edge * e = *eit;
        if(e->type() == CALL) {
            parsing_printf("[%s:%d] linking return edge %lx -> %lx\n",
                FILE__,__LINE__,retblk->lastInsnAddr(),e->src()->end());

            // XXX opportunity here to be more conservative about delayed
            //     determination of return status
    
            Block * call_ft = obj_->findBlockByEntry(region(),e->src()->end());
            if(!call_ft) {
                parsing_printf("[%s:%d] no block found, error!\n",
                    FILE__,__LINE__);
            } else if(call_ft == entry_)
                link_entry = true;
            else 
                o->add_edge(retblk,call_ft,RET);
        }
    }
    // can't do this during iteration
    if(link_entry)
        o->add_edge(retblk,entry_,RET);
}

void
Function::add_block(Block *b)
{
    ++b->func_cnt_;            // block counts references
    blocks_.push_back(b);
    bmap_[b->start()] = b;
}

const string &
Function::name() 
{
    return name_;
}

bool
Function::contains(Block *b)
{
    if(!cache_valid_)
        finalize();

    return HASHDEF(bmap_,b->start());
}
