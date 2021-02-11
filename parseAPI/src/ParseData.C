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
#include "ParseData.h"
#include "Parser.h"
#include "CodeObject.h"
#include "CFGFactory.h"
#include "util.h"
#include "debug_parse.h"

using namespace std;
using namespace Dyninst;
using namespace Dyninst::ParseAPI;

void ParseFrame::set_status(Status s)
{
    _status.store(s);
    _pd->setFrameStatus(codereg,func->addr(),s);
}

ParseWorkElem * ParseFrame::mkWork(
    ParseWorkBundle * b,
    Edge * e,
    Address source,
    Address target,
    bool resolvable,
    bool tailcall)
{
    if(!b) {
        b = new ParseWorkBundle();
        work_bundles.push_back(b); 
    }
    ParseWorkElem * ret = new ParseWorkElem(b,e,source, target,resolvable,tailcall);
    b->add( ret );
    return ret;
}
ParseWorkElem * ParseFrame::mkWork(
    ParseWorkBundle * b,
    Block *block,
    const InsnAdapter::IA_IAPI *ah)
{
    if(!b) {
        b = new ParseWorkBundle();
        work_bundles.push_back(b); 
    }
    ParseWorkElem * ret = new ParseWorkElem(b,block,ah);
    b->add( ret );
    return ret;
}

ParseWorkElem * ParseFrame::mkWork(
    ParseWorkBundle *b,
    Function * shared_func)
{
    if(!b) {
        b = new ParseWorkBundle();
        work_bundles.push_back(b); 
    }
    ParseWorkElem * ret = new ParseWorkElem(b,shared_func);
    b->add( ret );
    return ret;

}
/**** Standard [no overlapping regions] ParseData ****/

StandardParseData::StandardParseData(Parser *p) :
    ParseData(p), _rdata{}
{ }

StandardParseData::~StandardParseData() 
{ }

Function *
StandardParseData::findFunc(CodeRegion * /* cr */, Address entry)
{
    return _rdata.findFunc(entry);
}

Block *
StandardParseData::findBlock(CodeRegion * /* cr */, Address entry)
{
    return _rdata.findBlock(entry);
}

int
StandardParseData::findFuncs(CodeRegion * /* cr */, Address addr, 
    set<Function *> & funcs)
{
    return _rdata.findFuncs(addr,funcs);
}

int
StandardParseData::findFuncs(CodeRegion * /* cr */, Address start, 
    Address end, set<Function *> & funcs)
{
    return _rdata.findFuncs(start,end,funcs);
}
int StandardParseData::findBlocks(CodeRegion * /* cr */, Address addr,
    set<Block *> & blocks)
{
    int ret = _rdata.findBlocks(addr,blocks);
    return ret;
}

Function *
StandardParseData::createAndRecordFunc(CodeRegion * cr, Address entry, FuncSource src)
{
    Function * ret = findFunc(NULL, entry);
    if (ret != NULL) return NULL;

    CodeRegion * reg = reglookup(cr,entry);
    char name[32];
        if(reg && reg->isCode(entry)) {
           if (src == MODIFICATION) {
              snprintf(name,32,"mod%lx",entry);
           } else {
              snprintf(name,32,"targ%lx",entry);
           }
            parsing_printf("[%s] new function for target %lx\n",FILE__,entry);
            ret = _parser->factory()._mkfunc(
               entry,src,name,&_parser->obj(),reg,_parser->obj().cs());
            if (record_func(ret) == NULL) {
                // No need to delete this function that is not registered because
                // CFGFactory has an accounting data structure for it.
                // Delete the new function here would cause double-free.                
                return NULL;
            } else {
                _parser->record_func(ret);
            }
        }
    return ret;
}

void
StandardParseData::remove_frame(ParseFrame * pf)
{
    {
      dyn_c_hash_map<Address, ParseFrame*>::accessor a;
      if(_rdata.frame_map.find(a, pf->func->addr())) _rdata.frame_map.erase(a);
    }
}

ParseFrame *
StandardParseData::findFrame(CodeRegion * /* cr */, Address addr)
{
    return _rdata.findFrame(addr);
}
ParseFrame::Status
StandardParseData::frameStatus(CodeRegion * /* cr */, Address addr)
{
    return _rdata.getFrameStatus(addr);
}
void
StandardParseData::setFrameStatus(CodeRegion * /* cr */, Address addr,
    ParseFrame::Status status)
{
    _rdata.setFrameStatus(addr, status);
}

// This function return NULL
// if a frame for input function f has exists,
// or is being created by another thread.
//
// Otherwise, the calling thread will create and 
// register a new frame.
ParseFrame*
StandardParseData::createAndRecordFrame(Function* f)
{
    // Fast path: if there exists a frame, then directly return NULL
    ParseFrame * pf = findFrame(NULL, f->addr());
    if (pf != NULL) return NULL;

    // Every thread reaches here will try to create a new frame
    pf = new ParseFrame(f, this);
    _parser->init_frame(*pf);
    // It should be safe to prematurely set frame status to UNPARSED
    // because it is almost the same as BAD_LOOKUP
    pf->set_internal_status(ParseFrame::UNPARSED);

    // Only one thread will register the frame
    ParseFrame * ret_pf = _rdata.registerFrame(f->addr(), pf);
    if (ret_pf != pf) {
        // The thread who loses will destory the redundant frame
        delete pf;
        return NULL;
    } else {
        // The thread who wins publish the frame status to global status look up map
        // NOTE: it is possible that another does not see the updated global frame 
        // status, and then continues to call createAndRecordFrame, but that thread
        // will not be able to register a new frame.
        setFrameStatus(f->region(), f->addr(), ParseFrame::UNPARSED);
        return pf;
    }
}
CodeRegion *
StandardParseData::reglookup(CodeRegion * /* cr */, Address addr)
{
    set<CodeRegion *> regions;
    int rcnt = _parser->obj().cs()->findRegions(addr,regions);
    
    if(rcnt > 1) {
        fprintf(stderr,"Error, overlapping regoins at %lx:\n",addr);
        set<CodeRegion *>::iterator sit = regions.begin();
        for( ; sit != regions.end(); ++sit) {
            fprintf(stderr,"\t[%lx,%lx)\n",
                (*sit)->offset(),
                (*sit)->offset()+(*sit)->length());
        }
        return NULL;
    } else if(rcnt == 1)
        return *regions.begin();
    else
        return NULL;
}
void
StandardParseData::remove_func(Function *f)
{
    remove_extents(f->extents());
    _rdata.frame_status.erase(f->addr());
    _rdata.funcsByAddr.erase(f->addr());
}
void
StandardParseData::remove_block(Block *b)
{
    _rdata.blocksByAddr.erase(b->start());
    _rdata.blocksByRange.remove(b);
}
void
StandardParseData::remove_extents(const std::vector<FuncExtent*> & extents)
{
    for (unsigned idx=0; idx < extents.size(); idx++) {
        _rdata.funcsByRange.remove( extents[idx] );
    }
}

/**** Overlapping region ParseData ****/
OverlappingParseData::OverlappingParseData(
        Parser *p, vector<CodeRegion *> & regions) :
    ParseData(p)
{    
    for(unsigned i=0;i<regions.size();++i) {
        rmap.insert(make_pair(regions[i], new region_data{}));
    } 
}

OverlappingParseData::~OverlappingParseData()
{ 
    reg_map_t::iterator it = rmap.begin();
    for( ; it != rmap.end(); ++it)
        delete it->second;
}

Function *
OverlappingParseData::findFunc(CodeRegion * cr, Address entry)
{
    region_data * rd = findRegion(cr);
    if (rd == NULL) return NULL;
    return rd->findFunc(entry);
}
Block *
OverlappingParseData::findBlock(CodeRegion * cr, Address entry)
{
    region_data * rd = findRegion(cr);
    if (rd == NULL) return NULL;
    return rd->findBlock(entry);
}
int
OverlappingParseData::findFuncs(CodeRegion * cr, Address addr, 
    set<Function *> & funcs)
{
    region_data * rd = findRegion(cr);
    if (rd == NULL) return 0;
    return rd->findFuncs(addr,funcs);
}
int
OverlappingParseData::findFuncs(CodeRegion * cr, Address start, 
    Address end, set<Function *> & funcs)
{
    region_data * rd = findRegion(cr);
    if (rd == NULL) return 0;
    return rd->findFuncs(start,end,funcs);
}
int 
OverlappingParseData::findBlocks(CodeRegion * cr, Address addr,
    set<Block *> & blocks)
{
    region_data * rd = findRegion(cr);
    if (rd == NULL) return 0;
    return rd->findBlocks(addr,blocks);
}
ParseFrame *
OverlappingParseData::findFrame(CodeRegion *cr, Address addr)
{
    region_data * rd = findRegion(cr);
    if (rd == NULL) return NULL;
    return rd->findFrame(addr);
}
void
OverlappingParseData::setFrameStatus(CodeRegion *cr, Address addr, 
    ParseFrame::Status status)
{
    region_data * rd = findRegion(cr);
    if (rd == NULL) return;
    rd->setFrameStatus(addr, status);
}

ParseFrame::Status
OverlappingParseData::frameStatus(CodeRegion *cr, Address addr)
{
    region_data * rd = findRegion(cr);
    if (rd == NULL) return ParseFrame::BAD_LOOKUP;
    return rd->getFrameStatus(addr);
}

ParseFrame*
OverlappingParseData::createAndRecordFrame(Function* f)
{
    // Fast path: if there exists a frame, then directly return NULL
    ParseFrame * pf = findFrame(f->region(), f->addr());
    if (pf != NULL) return NULL;

    // Every thread reaches here will try to create a new frame
    pf = new ParseFrame(f, this);
    _parser->init_frame(*pf);
    // It should be safe to prematurely set frame status to UNPARSED
    // because it is almost the same as BAD_LOOKUP    
    pf->set_status(ParseFrame::UNPARSED);

    // Get the correct region data
    region_data * rd = findRegion(f->region());
    if (rd == NULL) return NULL;
    
    // Only one thread will register the frame
    ParseFrame * ret_pf = rd->registerFrame(f->addr(), pf);
    if (ret_pf != pf) {
        // The thread who loses will destory the redundant frame
        delete pf;
        return NULL;
    } else {
        setFrameStatus(f->region(), f->addr(), ParseFrame::UNPARSED);
        return pf;
    }
}


Function * 
OverlappingParseData::createAndRecordFunc(CodeRegion * cr, Address addr, FuncSource src)
{
    Function * ret = findFunc(cr, addr);
    if (ret != NULL) return NULL;
    
    char name[32];
    if(cr && cr->isCode(addr)) {
        if (src == MODIFICATION) {
            snprintf(name,32,"mod%lx",addr);
        } else {
            snprintf(name,32,"targ%lx",addr);
        }
        parsing_printf("[%s] new function for target %lx\n",FILE__,addr);
        ret = _parser->factory()._mkfunc(
           addr,src,name,&_parser->obj(),cr,_parser->obj().cs());
        if (record_func(ret) == NULL) {
            // No need to delete this function that is not registered because
            // CFGFactory has an accounting data structure for it.
            // Delete the new function here would cause double-free.                
            return NULL;
        } else {
            _parser->record_func(ret);
        }
    }
    return ret;
}
region_data * 
OverlappingParseData::findRegion(CodeRegion *cr)
{
    dyn_c_hash_map<CodeRegion*, region_data*>::accessor a;
    if (!rmap.find(a, cr)) return NULL;
    //region_data uses concurrent data structure, so no more lock is needed.
    return a->second;        
}
Function*
OverlappingParseData::record_func(Function *f)
{
    region_data * rd = findRegion(f->region());
    if (rd == NULL) return NULL;
    return rd->record_func(f);
}
Block*
OverlappingParseData::record_block(CodeRegion *cr, Block *b)
{
    region_data * rd = findRegion(cr);
    if (rd == NULL) return b;
    return rd->record_block(b);
}
void
OverlappingParseData::remove_func(Function *f)
{
    remove_extents(f->extents());

    CodeRegion * cr = f->region();
    region_data * rd = findRegion(cr);
    if (rd == NULL) return;
    rd->frame_status.erase(f->addr());
    rd->funcsByAddr.erase(f->addr());
}
void
OverlappingParseData::remove_block(Block *b)
{    
    CodeRegion * cr = b->region();
    region_data * rd = findRegion(cr);
    if (rd == NULL) return;
    rd->blocksByAddr.erase(b->start());
    rd->blocksByRange.remove(b); 
}
void //extents should all belong to the same code region
OverlappingParseData::remove_extents(const vector<FuncExtent*> & extents)
{
    if (0 == extents.size()) {
        return;
    }
    CodeRegion * cr = extents[0]->func()->region();
    region_data * rd = findRegion(cr);
    if (rd == NULL) return;
    vector<FuncExtent*>::const_iterator fit;
    for (fit = extents.begin(); fit != extents.end(); fit++) {
        assert( (*fit)->func()->region() == cr );
        rd->funcsByRange.remove( *fit );
    }
}
void
OverlappingParseData::remove_frame(ParseFrame *pf)
{    
    CodeRegion * cr = pf->codereg;
    region_data * rd = findRegion(cr);
    if (rd == NULL) return;
    rd->frame_map.erase(pf->func->addr());
}
CodeRegion * 
OverlappingParseData::reglookup(CodeRegion *cr, Address /* addr */) 
{
    return cr;
}

edge_parsing_data
OverlappingParseData::setEdgeParsingStatus(CodeRegion *cr, Address addr, Function *f, Block *b)
{
    region_data * rd = findRegion(cr);
    if (rd == NULL) return edge_parsing_data();    
    return rd->set_edge_parsed(addr,f, b);
}

void
OverlappingParseData::getAllRegionData(vector<region_data*> &rds) {
    for (auto rit = rmap.begin(); rit != rmap.end(); ++rit)
        rds.push_back(rit->second);
}

region_data::edge_data_map*
OverlappingParseData::get_edge_data_map(CodeRegion *cr) {
    region_data * rd = findRegion(cr);
    if (rd == NULL) return NULL;
    return rd->get_edge_data_map();
}
