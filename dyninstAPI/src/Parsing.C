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
#include <stdio.h>

#include "InstructionDecoder.h"
#include "Instruction.h"

#include "image.h"
#include "parse-cfg.h"
#include "instPoint.h"
#include "Parsing.h"
#include "debug.h"
#include "BPatch.h"
#include "mapped_object.h"
#include "dynProcess.h"

using namespace Dyninst;
using namespace Dyninst::ParseAPI;
using namespace Dyninst::InstructionAPI;

// windows.h pollutes namespace with min/max macros
#undef min
#undef max

#if defined(VERBOSE_CFG_FACTORY)
#define record_func_alloc(x) do { _record_func_alloc(x); } while(0)
#define record_edge_alloc(x,s) do { _record_edge_alloc(x,s); } while(0)
#define record_block_alloc(s) do { _record_block_alloc(s); } while(0)
#else
#define record_func_alloc(x) do { } while(0)
#define record_edge_alloc(x,s) do { } while(0)
#define record_block_alloc(s) do { } while(0)
#endif

#ifdef __GNUC__
#define likely(x) __builtin_expect(!!(x), 1)
#define unlikely(x) __builtin_expect(!!(x), 0)
#else
#define likely(x) x
#define unlikely(x) x
#endif

void DynCFGFactory::dump_stats()
{
    fprintf(stderr,"===DynCFGFactory for image %p===\n", (void*)_img);
    fprintf(stderr,"   Functions:\n");
    fprintf(stderr,"   %-12s src\n","cnt");
    for(int i=0;i<_funcsource_end_;++i) {
        fprintf(stderr,"   %-12d %3d\n",_func_allocs[i],i);
    }
    fprintf(stderr,"   Edges:\n");
    fprintf(stderr,"   %-12s type\n","cnt");
    for(int i=0;i<_edgetype_end_;++i) {
        fprintf(stderr,"   %-12d %4d\n",_edge_allocs[i],i);
    }
    fprintf(stderr,"   Blocks:\n");
    fprintf(stderr,"   %-12d total\n",_block_allocs);
    fprintf(stderr,"   %-12d sink\n",_sink_block_allocs);
}

DynCFGFactory::DynCFGFactory(image * im) :
    _img(im),
    _func_allocs(_funcsource_end_),
    _edge_allocs(_edgetype_end_),
    _block_allocs(0),
    _sink_block_allocs(0)
{

}

class PLTFunction : public Dyninst::SymtabAPI::Function {
	Dyninst::SymtabAPI::relocationEntry r;
public:
    explicit PLTFunction(Dyninst::SymtabAPI::relocationEntry re) : Dyninst::SymtabAPI::Function(re.getDynSym()), r{re} {}

    std::string getName() const override {
    	return r.name();
    }

    Offset getOffset() const override {
    	return r.target_addr();
    }

    unsigned int getSize() const override {
    	return 0;
    }

    SymtabAPI::Module* getModule() const override {
    	return nullptr;
    }
};

Function *
DynCFGFactory::mkfunc(
    Address addr, 
    FuncSource src, 
    std::string name,
    CodeObject * obj,
    CodeRegion * reg,
    InstructionSource * isrc)
{
    _mtx.lock();
    parse_func * ret;
    SymtabAPI::Symtab * st;
    SymtabAPI::Function * stf = NULL;
    pdmodule * pdmod;
    record_func_alloc(src);

    st = _img->getObject();
    auto found = obj->cs()->linkage().find(addr);
    // PLT stub
    if(found != obj->cs()->linkage().end()) {
        name = found->second;
        pdmod = _img->getOrCreateModule(st->getDefaultModule());
        std::vector<SymtabAPI::relocationEntry> relocs;
        st->getFuncBindingTable(relocs);
        for(auto i = relocs.begin(); i != relocs.end(); i++)
        {
            if(i->target_addr() == found->first)
            {
                stf = new PLTFunction(*i);
                break;
            }
        }
        if(stf && stf->getFirstSymbol()) {
            ret = new parse_func(stf, pdmod,_img,obj,reg,isrc,src);
            ret->isPLTFunction_ = true;
            // PLT stubs are typically are undefined symbols in the binary,
            // so there is no corresponding SymtabAPI::Function at Symtab level.
            // PLTFunction is a subclass of SymtabAPI::Function to represent PLT stubs.
            // However, since there is no easy way to add a PLTFunction back to the
            // Symtab object, we need to add PLTFunction to a data structure for
            // future lookup.
            _img->insertPLTParseFuncMap(stf->getName(), ret);
            _mtx.unlock();
            return ret;
        }
    }
    if(!st->findFuncByEntryOffset(stf,addr)) {
        pdmod = _img->getOrCreateModule(st->getDefaultModule());
        stf = st->createFunction(
            name,addr,0,pdmod->mod());
    } else {
        pdmod = _img->getOrCreateModule(stf->getModule());
    }
    assert(stf);

    ret = new parse_func(stf,pdmod,_img,obj,reg,isrc,src);

    _mtx.unlock();
    return ret;
}

Block *
DynCFGFactory::mkblock(Function * f, CodeRegion *r, Address addr) {
    parse_block * ret;

    record_block_alloc(false);

    ret = new parse_block((parse_func*)f,r,addr);

    //fprintf(stderr,"mkbloc(%lx, %lx) returning %p\n",f->addr(),addr,ret);

    if ( _img->trackNewBlocks_ ) 
    {
        _img->newBlocks_.push_back(ret);
    }
    return ret;
}
Block *
DynCFGFactory::mksink(CodeObject *obj, CodeRegion *r) {
    parse_block * ret;

    record_block_alloc(true);

    ret = new parse_block(obj,r,numeric_limits<Address>::max());
    return ret;
}

Edge *
DynCFGFactory::mkedge(Block * src, Block * trg, EdgeTypeEnum type) {
    image_edge * ret;
    record_edge_alloc(type,false); // FIXME can't tell if it's a sink

    ret = new image_edge((parse_block*)src,
                         (parse_block*)trg,
                         type);

    return ret;
}

void
DynParseCallback::abruptEnd_cf(Address /*addr*/,ParseAPI::Block *b,default_details*)
{
  static_cast<parse_block*>(b)->setAbruptEnd(true);
}

void
DynParseCallback::newfunction_retstatus(Function *func)
{
    dynamic_cast<parse_func*>(func)->setinit_retstatus( func->retstatus() );
}

void DynParseCallback::destroy_cb(Block *b) {
   _img->destroy(b);
}

void DynParseCallback::destroy_cb(Edge *e) {
   _img->destroy(e);
}

void DynParseCallback::destroy_cb(Function *f) {
   _img->destroy(f);
}

void DynParseCallback::remove_edge_cb(ParseAPI::Block *, ParseAPI::Edge *, edge_type_t) {
   //cerr << "Warning: edge removal callback unimplemented" << endl;
}

void DynParseCallback::remove_block_cb(ParseAPI::Function *, ParseAPI::Block *) {
   // we currently do all necessary cleanup during destroy
   //cerr << "Warning: block removal callback unimplemented" << endl;
}

void
DynParseCallback::patch_nop_jump(Address addr)
{
    Architecture arch = _img->codeObject()->cs()->getArch();
    assert( Arch_x86 == arch || Arch_x86_64 == arch );

    unsigned char * ptr = (unsigned char *) 
        _img->codeObject()->cs()->getPtrToInstruction(addr);
    ptr[0] = 0x90;
}

void
DynParseCallback::interproc_cf(Function*f,Block *b,Address /*addr*/,interproc_details*det)
{
    (void) f; // compiler warning
    if (det->type == ParseCallback::interproc_details::unresolved) {
        static_cast<parse_block*>(b)->setUnresolvedCF(true);
    }
}

void
DynParseCallback::overlapping_blocks(Block*b1,Block*b2)
{
    parsing_printf("[%s:%d] blocks [%lx,%lx) and [%lx,%lx) overlap"
                   "inconsistently\n",
        FILE__,__LINE__,b1->start(),b1->end(),b2->start(),b2->end());
    static_cast<parse_block*>(b1)->markAsNeedingRelocation();
    static_cast<parse_block*>(b2)->markAsNeedingRelocation();
}

extern bool codeBytesUpdateCB(void *objCB, Address targ);
bool 
DynParseCallback::updateCodeBytes(Address target_)
{   // calls function that updates bytes if needed
    assert(BPatch_normalMode != _img->hybridMode());
    return codeBytesUpdateCB( _img->cb_arg0(), 
                              target_ + _img->desc().code() );
}

bool
DynParseCallback::hasWeirdInsns(const ParseAPI::Function* func) const
{
    return static_cast<parse_func*>
        (const_cast<ParseAPI::Function*>
            (func))->hasWeirdInsns();
}

void 
DynParseCallback::foundWeirdInsns(ParseAPI::Function* func)
{
    static_cast<parse_func*>(func)->setHasWeirdInsns(true);
}

void DynParseCallback::split_block_cb(ParseAPI::Block *first, ParseAPI::Block *second)
{
    if (SCAST_PB(first)->abruptEnd()) {
        SCAST_PB(first)->setAbruptEnd(false);
        SCAST_PB(second)->setAbruptEnd(true);
    }
    if (SCAST_PB(first)->unresolvedCF()) {
        SCAST_PB(first)->setUnresolvedCF(false);
        SCAST_PB(second)->setUnresolvedCF(true);
    }
    if (SCAST_PB(first)->needsRelocation()) {
        SCAST_PB(second)->markAsNeedingRelocation();
    }
    //parse_block::canBeRelocated_ doesn't need to be set, it's only ever
    // true for the sink block, which is never split
}
