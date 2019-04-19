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
#ifndef _PARSE_DATA_H_
#define _PARSE_DATA_H_

#include "common/src/vgannotations.h"

#include <set>
#include <vector>
#include <queue>

#include "dyntypes.h"
#include "IBSTree.h"
#include "IBSTree-fast.h"
#include "CodeObject.h"
#include "CFG.h"
#include "ParserDetails.h"
#include "debug_parse.h"

#include <boost/thread/locks.hpp>
#include <boost/thread/lockable_adapter.hpp>
#include <boost/thread/recursive_mutex.hpp>
#include <boost/atomic.hpp>

#include "concurrent.h"

using namespace std;

namespace Dyninst {
namespace ParseAPI {

class Parser;
class ParseData;

/** Describes a saved frame during recursive parsing **/
// Parsing data for a function. 
class ParseFrame : public boost::lockable_adapter<boost::recursive_mutex> {
 public:
    enum Status {
        UNPARSED,
        PROGRESS,
        CALL_BLOCKED,
        PARSED,
        FRAME_ERROR,
        BAD_LOOKUP,  // error for lookups that return Status
        FRAME_DELAYED // discovered cyclic dependency, delaying parse
    };

    /* worklist details */
    typedef std::priority_queue<
        ParseWorkElem *,
        vector<ParseWorkElem*>,
        ParseWorkElem::compare
       > worklist_t;

    vector<ParseWorkBundle*> work_bundles; 
   
    /* convenience generator for work elements. If a NULL bundle is
       supplied, one will be provided. */
    ParseWorkElem * mkWork(
        ParseWorkBundle * b,
        Edge * e,
        Address source,
        Address target,
        bool resolvable,
        bool tailcall);
    ParseWorkElem * mkWork(
        ParseWorkBundle * b,
	Block* block,
        const InsnAdapter::IA_IAPI *ah);
    ParseWorkElem * mkWork(
        ParseWorkBundle *b,
        Function* shared_func);
    void pushWork(ParseWorkElem * elem) {
        boost::lock_guard<ParseFrame> g(*this);
        parsing_printf("\t pushing work element for block %p, edge %p, target %p\n", elem->cur(), elem->edge(), elem->target());
        worklist.push(elem);
    }
    ParseWorkElem * popWork() {
        boost::lock_guard<ParseFrame> g(*this);
        ParseWorkElem * ret = NULL;
        if(!worklist.empty()) {
            ret = worklist.top();
            worklist.pop();
        }
        return ret;
    }

    void pushDelayedWork(ParseWorkElem * elem, Function * ct) {
        boost::lock_guard<ParseFrame> g(*this);
        delayedWork.insert(make_pair(elem, ct));
    }

    void cleanup();

    worklist_t worklist;
    std::set<Address> knownTargets; // This set contains known potential targets in this function 
   
    // Delayed work elements 
    std::map<ParseWorkElem *, Function *> delayedWork;

    std::map<Address, Block*> leadersToBlock;
    //dyn_hash_map<Address, Block*> leadersToBlock;  // block map
    Address curAddr;                           // current insn address
    unsigned num_insns;
    dyn_hash_map<Address, bool> visited;

    /* These are set when status goes to CALL_BLOCKED */
    Function * call_target;     // discovered callee

    Function * func;
    CodeRegion * codereg;

    ParseWorkElem * seed; // stored for cleanup
    std::set<Address> value_driven_jump_tables;

    ParseFrame(Function * f,ParseData *pd) :
        curAddr(0),
        num_insns(0),
        call_target(NULL),
        func(f),
        codereg(f->region()),
        seed(NULL),
        _pd(pd)
    {
	busy.store(false);
    }

    ~ParseFrame();

    Status status() const {
      // acquire(_status);
      Status result = _status.load();
      // release(_status);
      return result;
    }
    void set_status(Status);
    bool swap_busy(bool value) {
      return busy.exchange(value);
    }
;
 private:
    boost::atomic<Status> _status;
    boost::atomic<bool> busy;
    ParseData * _pd;
};

class edge_parsing_data {

 public:
    Block* b;
    Function *f;       
    edge_parsing_data(Function *ff, Block *bb) {
        b = bb;
	f = ff;
    }
    edge_parsing_data() : b(NULL), f(NULL) {}
};


/* per-CodeRegion parsing data */
class region_data {
public:
    // Function lookups
    Dyninst::IBSTree_fast<FuncExtent> funcsByRange;
    dyn_c_hash_map<Address, Function *> funcsByAddr;

    // Block lookups
    Dyninst::IBSTree_fast<Block > blocksByRange;
    dyn_c_hash_map<Address, Block *> blocksByAddr;

    // Parsing internals 
    dyn_c_hash_map<Address, ParseFrame *> frame_map;
    dyn_c_hash_map<Address, ParseFrame::Status> frame_status;

    // Edge parsing records
    // We only want one thread to create edges for a location
    typedef dyn_c_hash_map<Address, edge_parsing_data> edge_data_map;
    edge_data_map edge_parsing_status;

    Function * findFunc(Address entry);
    Block * findBlock(Address entry);
    int findFuncs(Address addr, set<Function *> & funcs);
    int findBlocks(Address addr, set<Block *> & blocks);

    /* 
     * Look up the next block for detection of straight-line
     * fallthrough edges into existing blocks.
     */
    inline std::pair<Address, Block*> get_next_block(Address addr)
    {
        Block * nextBlock = NULL;
        Address nextBlockAddr = numeric_limits<Address>::max();

        if((nextBlock = blocksByRange.successor(addr)) &&
           nextBlock->start() > addr)
        {
            nextBlockAddr = nextBlock->start();   
        }

        return std::pair<Address,Block*>(nextBlockAddr,nextBlock);
    }
    ParseFrame* findFrame(Address addr) const {
        ParseFrame *result = NULL;
        // acquire(frame_map);
	{
	  dyn_c_hash_map<Address, ParseFrame*>::const_accessor a;
	  if(frame_map.find(a, addr)) result = a->second;
	}
        // release(frame_map);
        return result;
    }
    ParseFrame::Status getFrameStatus(Address addr) {
        ParseFrame::Status ret;
        dyn_c_hash_map<Address, ParseFrame::Status>::const_accessor a;
        if(frame_status.find(a, addr)) {
            ANNOTATE_HAPPENS_AFTER(&a->second); // After last writer
            ret = a->second;
            ANNOTATE_HAPPENS_BEFORE(&a->second + 1);
        } else {
            ret = ParseFrame::BAD_LOOKUP;
        }
        return ret;
    }


    void setFrameStatus(Address addr, ParseFrame::Status status)
    {
        dyn_c_hash_map<Address, ParseFrame::Status>::accessor a;
        if(!frame_status.insert(a, make_pair(addr, status))) {
            ANNOTATE_HAPPENS_AFTER(&a->second); // After last writer
            ANNOTATE_HAPPENS_AFTER(&a->second + 1); // After last reader
            a->second = status;
        }
        ANNOTATE_HAPPENS_BEFORE(&a->second);
    }

    void record_func(Function* f) {
        // acquire(funcsByAddr);
	{
	  dyn_c_hash_map<Address, Function*>::accessor a;
	  funcsByAddr.insert(a, std::make_pair(f->addr(), f));
	}
        // release(funcsByAddr);

    }
    Block* record_block(Block* b) {
        Block* ret = NULL;
        // acquire(blocksByAddr);
	    {
            dyn_c_hash_map<Address, Block*>::accessor a;
            bool inserted = blocksByAddr.insert(a, std::make_pair(b->start(), b));
            // Inserting failed when another thread has inserted a block with the same starting address
            if(!inserted) {
                ret = a->second;
            } else {
                ret = b;
            }
        }
        // release(blocksByAddr);
        return ret;
    }
    void insertBlockByRange(Block* b) {
        blocksByRange.insert(b);
    }
    void record_frame(ParseFrame* pf) {
        // acquire(frame_map);
	{
	  dyn_c_hash_map<Address, ParseFrame*>::accessor a;
	  frame_map.insert(a, make_pair(pf->func->addr(), pf));
	}
        // release(frame_map);
    }

	 // Find functions within [start,end)
	 int findFuncs(Address start, Address end, set<Function *> & funcs);
    region_data(CodeObject* obj, CodeRegion* reg) {
        Block* sink = new Block(obj, reg, numeric_limits<Address>::max());
        blocksByAddr.insert(make_pair(sink->start(),sink));
        blocksByRange.insert(sink);
    }

    edge_data_map* get_edge_data_map() { return &edge_parsing_status; }

    edge_parsing_data set_edge_parsed(Address addr, Function *f, Block *b) {
        edge_parsing_data ret;
        // acquire(edge_parsing_status);
	{
	  dyn_c_hash_map<Address, edge_parsing_data>::accessor a;
          // A successful insertion means the thread should 
          // continue to create edges. We return the passed in Function*
          // as indication of successful insertion.
          //
          // Otherwise, another thread has started creating edges.
          // The current thread should give up. We return
          // the function who succeeded.
          if (!edge_parsing_status.insert(a, addr))
              ret = a->second;
	  else {
              ret.f = f;
	      ret.b = b;
	  }
	}
        // release(edge_parsing_status);
        return ret;
    }

    void getAllBlocks(std::map<Address, Block*> &allBlocks) {
        // This function should be only called in single-threaded mode,
        // such as when we are finalizing parsing

        // We convert hash map to an ordered tree
        for (auto it = blocksByAddr.begin(); it != blocksByAddr.end(); ++it)
            allBlocks[it->first] = it->second;
    }
};

/** region_data inlines **/

inline Function *
region_data::findFunc(Address entry)
{
    Function *result = NULL;
    // acquire(funcsByAddr);
    {
      dyn_c_hash_map<Address, Function *>::const_accessor a;
      if(funcsByAddr.find(a, entry)) result = a->second;
    }
    // release(funcsByAddr);
    return result;
}
inline Block *
region_data::findBlock(Address entry)
{
    Block *result = NULL;
    // acquire(blocksByAddr);
    {
      dyn_c_hash_map<Address, Block *>::const_accessor a;
      if(blocksByAddr.find(a, entry)) result = a->second;
    }
    // release(blocksByAddr);
    return result;
}
inline int
region_data::findFuncs(Address addr, set<Function *> & funcs)
{
    int sz = funcs.size();

    set<FuncExtent *> extents;
    set<FuncExtent *>::iterator eit;
    
    funcsByRange.find(addr,extents);
    for(eit = extents.begin(); eit != extents.end(); ++eit)
        funcs.insert((*eit)->func());
 
    return funcs.size() - sz;
}
inline int
region_data::findFuncs(Address start, Address end, set<Function *> & funcs)
{
	 FuncExtent dummy(NULL,start,end);
    int sz = funcs.size();

    set<FuncExtent *> extents;
    set<FuncExtent *>::iterator eit;
    
    funcsByRange.find(&dummy,extents);
    for(eit = extents.begin(); eit != extents.end(); ++eit)
        funcs.insert((*eit)->func());
 
    return funcs.size() - sz;
}
inline int
region_data::findBlocks(Address addr, set<Block *> & blocks)
{
    int sz = blocks.size();
    blocksByRange.find(addr,blocks);
    return blocks.size() - sz;
}


/** end region_data **/

class ParseData : public boost::lockable_adapter<boost::recursive_mutex>  {
 protected:
    ParseData(Parser *p) : _parser(p) { }
    Parser * _parser;
 public:
    virtual ~ParseData() { }

    //
    virtual Function * findFunc(CodeRegion *, Address) =0;
    virtual Block * findBlock(CodeRegion *, Address) =0;
    virtual int findFuncs(CodeRegion *, Address, set<Function*> &) =0;
    virtual int findFuncs(CodeRegion *, Address, Address, set<Function*> &) =0;
    virtual int findBlocks(CodeRegion *, Address, set<Block*> &) =0;
    virtual ParseFrame * findFrame(CodeRegion *, Address) = 0;
    virtual ParseFrame::Status frameStatus(CodeRegion *, Address addr) = 0;
    virtual void setFrameStatus(CodeRegion*,Address,ParseFrame::Status) = 0;

    // Atomically lookup whether there is a frame for a Function object.
    // If there is no frame for the Function, create a new frame and record it.
    // Return NULL if a frame already exists;
    // Return the pointer to the new frame if a new frame is created 
    virtual ParseFrame* createAndRecordFrame(Function*) = 0;

    // creation (if non-existing)
    virtual Function * createAndRecordFunc(CodeRegion *, Address, FuncSource) =0;

    // mapping
    virtual region_data * findRegion(CodeRegion *) =0;

    // accounting
    virtual void record_func(Function *) =0;
    virtual Block* record_block(CodeRegion *, Block *) =0;
    virtual void record_frame(ParseFrame *) =0;

    // removal
    virtual void remove_frame(ParseFrame *) =0;
    virtual void remove_func(Function *) =0;
    virtual void remove_block(Block *) =0;
    virtual void remove_extents(const std::vector<FuncExtent*> &extents) =0;

    // does the Right Thing(TM) for standard- and overlapping-region 
    // object types
    virtual CodeRegion * reglookup(CodeRegion *cr, Address addr) =0;
    virtual edge_parsing_data setEdgeParsingStatus(CodeRegion *cr, Address addr, Function *f, Block *b) = 0;
    virtual void getAllRegionData(std::vector<region_data*>&) = 0;
    virtual region_data::edge_data_map* get_edge_data_map(CodeRegion*) = 0;

};

/* StandardParseData represents parse data for Parsers that disallow
   overlapping CodeRegions. It has fast paths for lookup */
class StandardParseData : public ParseData {
 private:
    region_data _rdata;
 public:
    /* interface implementation */
    StandardParseData(Parser *p);
    ~StandardParseData();

    Function * findFunc(CodeRegion * pf, Address addr);
    Block * findBlock(CodeRegion * pf, Address addr);
    int findFuncs(CodeRegion *, Address, set<Function*> &);
    int findFuncs(CodeRegion *, Address, Address, set<Function*> &);
    int findBlocks(CodeRegion *, Address, set<Block*> &);
    ParseFrame * findFrame(CodeRegion *, Address);
    ParseFrame::Status frameStatus(CodeRegion *, Address);
    void setFrameStatus(CodeRegion*,Address,ParseFrame::Status);

    virtual ParseFrame* createAndRecordFrame(Function*);

    Function * createAndRecordFunc(CodeRegion * cr, Address addr, FuncSource src);

    region_data * findRegion(CodeRegion *cr);

    void record_func(Function *f);
    Block* record_block(CodeRegion *cr, Block *b);
    void record_frame(ParseFrame *pf);

    void remove_frame(ParseFrame *);
    void remove_func(Function *);
    void remove_block(Block *);
    void remove_extents(const std::vector<FuncExtent*> &extents);

    CodeRegion * reglookup(CodeRegion *cr, Address addr);
    edge_parsing_data setEdgeParsingStatus(CodeRegion *cr, Address addr, Function *f, Block *b); 
    void getAllRegionData(std::vector<region_data*>& rds);
    region_data::edge_data_map* get_edge_data_map(CodeRegion* cr);

};

inline region_data * StandardParseData::findRegion(CodeRegion * /* cr */)
{
    return &_rdata;
}
inline void StandardParseData::record_func(Function *f)
{
    _rdata.record_func(f);
}
inline Block* StandardParseData::record_block(CodeRegion * /* cr */, Block *b)
{
    return _rdata.record_block(b);
}

inline edge_parsing_data StandardParseData::setEdgeParsingStatus(CodeRegion *, Address addr, Function *f, Block *b)
{
    return _rdata.set_edge_parsed(addr, f, b);
}

inline void StandardParseData::getAllRegionData(std::vector<region_data*> & rds) {
    rds.push_back(&_rdata);
} 
inline region_data::edge_data_map* StandardParseData::get_edge_data_map(CodeRegion*) {
    return _rdata.get_edge_data_map();
}


/* OverlappingParseData handles binary code objects like .o files
   where CodeRegions may overlap on the same linear address space */
class OverlappingParseData : public ParseData {
    typedef dyn_hash_map<void *, region_data *> reg_map_t;
 private:
    reg_map_t rmap;
 public:
    OverlappingParseData(Parser *p, vector<CodeRegion *> & regions);
    ~OverlappingParseData();

    /* interface implementation */
    Function * findFunc(CodeRegion *, Address addr);
    Block * findBlock(CodeRegion *, Address addr);
    int findFuncs(CodeRegion *, Address, set<Function*> &);
    int findFuncs(CodeRegion *, Address, Address, set<Function*> &);
    int findBlocks(CodeRegion *, Address, set<Block*> &);
    ParseFrame * findFrame(CodeRegion *, Address);
    ParseFrame::Status frameStatus(CodeRegion *, Address);
    void setFrameStatus(CodeRegion*,Address,ParseFrame::Status);

    virtual ParseFrame* createAndRecordFrame(Function*);

    Function * createAndRecordFunc(CodeRegion * cr, Address addr, FuncSource src);

    region_data * findRegion(CodeRegion *cr);

    void record_func(Function *f);
    Block* record_block(CodeRegion *cr, Block *b);
    void record_frame(ParseFrame *pf);

    void remove_frame(ParseFrame *);
    void remove_func(Function *);
    void remove_block(Block *);
    void remove_extents(const std::vector<FuncExtent*> &extents);

    CodeRegion * reglookup(CodeRegion *cr, Address addr);
    edge_parsing_data setEdgeParsingStatus(CodeRegion *cr, Address addr, Function* f, Block *b); 
    void getAllRegionData(std::vector<region_data*>&);
    region_data::edge_data_map* get_edge_data_map(CodeRegion* cr);

};

}
}

#endif
