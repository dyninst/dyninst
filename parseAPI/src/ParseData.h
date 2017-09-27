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


using namespace std;

namespace Dyninst {
namespace ParseAPI {

class Parser;
class ParseData;

/** Describes a saved frame during recursive parsing **/
// Parsing data for a function. 
class ParseFrame {
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
        Address target,
        bool resolvable,
        bool tailcall);
    ParseWorkElem * mkWork(
        ParseWorkBundle * b,
	Block* block,
        const InsnAdapter::IA_IAPI *ah);

    void pushWork(ParseWorkElem * elem) {
        worklist.push(elem);
    }
    ParseWorkElem * popWork() {
        ParseWorkElem * ret = NULL;
        if(!worklist.empty()) {
            ret = worklist.top();
            worklist.pop();
        }
        return ret;
    }

    void pushDelayedWork(ParseWorkElem * elem, Function * ct) {
        delayedWork.insert(make_pair(elem, ct));
    }

    void cleanup();

    worklist_t worklist;
    std::set<Address> knownTargets; // This set contains known potential targets in this function 
   
    // Delayed work elements 
    std::map<ParseWorkElem *, Function *> delayedWork;

    dyn_hash_map<Address, Block*> leadersToBlock;  // block map
    Address curAddr;                           // current insn address
    unsigned num_insns;
    dyn_hash_map<Address, bool> visited;

    /* These are set when status goes to CALL_BLOCKED */
    Function * call_target;     // discovered callee

    Function * func;
    CodeRegion * codereg;

    ParseWorkElem * seed; // stored for cleanup

    ParseFrame(Function * f,ParseData *pd) :
        curAddr(0),
        num_insns(0),
        call_target(NULL),
        func(f),
        codereg(f->region()),
        seed(NULL),
        _pd(pd)
    {
        set_status(UNPARSED);
    }

    ~ParseFrame();

    Status status() const { return _status; }
    void set_status(Status);
 private:
    Status _status;
    ParseData * _pd;
};

/* per-CodeRegion parsing data */
class region_data { 
 public:
  // Function lookups
  Dyninst::IBSTree_fast<FuncExtent> funcsByRange;
    dyn_hash_map<Address, Function *> funcsByAddr;

    // Block lookups
    Dyninst::IBSTree_fast<Block> blocksByRange;
    dyn_hash_map<Address, Block *> blocksByAddr;

    // Parsing internals 
    dyn_hash_map<Address, ParseFrame *> frame_map;
    dyn_hash_map<Address, ParseFrame::Status> frame_status;

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
    
	 // Find functions within [start,end)
	 int findFuncs(Address start, Address end, set<Function *> & funcs);
};

/** region_data inlines **/

inline Function *
region_data::findFunc(Address entry)
{
    dyn_hash_map<Address, Function *>::iterator fit;
    if((fit = funcsByAddr.find(entry)) != funcsByAddr.end())
        return fit->second;
    else
        return NULL;
}
inline Block *
region_data::findBlock(Address entry)
{
    dyn_hash_map<Address, Block *>::iterator bit;
    if((bit = blocksByAddr.find(entry)) != blocksByAddr.end())
        return bit->second;
    else
        return NULL;
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

class ParseData {
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
    virtual ParseFrame::Status frameStatus(CodeRegion *, Address) = 0;
    virtual void setFrameStatus(CodeRegion*,Address,ParseFrame::Status) = 0;

    // creation (if non-existing)
    virtual Function * get_func(CodeRegion *, Address, FuncSource) =0;

    // mapping
    virtual region_data * findRegion(CodeRegion *) =0;

    // accounting
    virtual void record_func(Function *) =0;
    virtual void record_block(CodeRegion *, Block *) =0;
    virtual void record_frame(ParseFrame *) =0;

    // removal
    virtual void remove_frame(ParseFrame *) =0;
    virtual void remove_func(Function *) =0;
    virtual void remove_block(Block *) =0;
    virtual void remove_extents(const std::vector<FuncExtent*> &extents) =0;

    // does the Right Thing(TM) for standard- and overlapping-region 
    // object types
    virtual CodeRegion * reglookup(CodeRegion *cr, Address addr) =0;
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

    Function * get_func(CodeRegion * cr, Address addr, FuncSource src);

    region_data * findRegion(CodeRegion *cr);

    void record_func(Function *f);
    void record_block(CodeRegion *cr, Block *b);
    void record_frame(ParseFrame *pf);

    void remove_frame(ParseFrame *);
    void remove_func(Function *);
    void remove_block(Block *);
    void remove_extents(const std::vector<FuncExtent*> &extents);

    CodeRegion * reglookup(CodeRegion *cr, Address addr);
};

inline region_data * StandardParseData::findRegion(CodeRegion * /* cr */)
{
    return &_rdata;
}
inline void StandardParseData::record_func(Function *f)
{
    _rdata.funcsByAddr[f->addr()] = f;
}
inline void StandardParseData::record_block(CodeRegion * /* cr */, Block *b)
{
    _rdata.blocksByAddr[b->start()] = b;
    _rdata.blocksByRange.insert(b);
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

    Function * get_func(CodeRegion * cr, Address addr, FuncSource src);

    region_data * findRegion(CodeRegion *cr);

    void record_func(Function *f);
    void record_block(CodeRegion *cr, Block *b);
    void record_frame(ParseFrame *pf);

    void remove_frame(ParseFrame *);
    void remove_func(Function *);
    void remove_block(Block *);
    void remove_extents(const std::vector<FuncExtent*> &extents);

    CodeRegion * reglookup(CodeRegion *cr, Address addr);
};

}
}

#endif
