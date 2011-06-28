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
#include <stdio.h>

#include "InstructionDecoder.h"
#include "Instruction.h"

#include "symtab.h"
#include "parse-cfg.h"
#include "instPoint.h"
#include "Parsing.h"
#include "debug.h"
#include "BPatch.h"
#include "mapped_object.h"
#include "pcProcess.h"

#if defined(os_aix)
#include "parRegion.h"
#endif

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

void DynCFGFactory::dump_stats()
{
    fprintf(stderr,"===DynCFGFactory for image %p===\n",_img);
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

DynCFGFactory::~DynCFGFactory()
{
    free_all();
}

Function *
DynCFGFactory::mkfunc(
    Address addr, 
    FuncSource src, 
    std::string name,
    CodeObject * obj,
    CodeRegion * reg,
    InstructionSource * isrc)
{
    parse_func * ret;
    SymtabAPI::Symtab * st;
    SymtabAPI::Function * stf;
    pdmodule * pdmod;

    record_func_alloc(src);

    st = _img->getObject();
    if(!st->findFuncByEntryOffset(stf,addr)) {
        pdmod = _img->getOrCreateModule(st->getDefaultModule());
        stf = st->createFunction(
            name,addr,std::numeric_limits<size_t>::max(),pdmod->mod());
    } else
        pdmod = _img->getOrCreateModule(stf->getModule());
    assert(stf);

    ret = new parse_func(stf,pdmod,_img,obj,reg,isrc,src);
    funcs_.add(*ret);

    if(obj->cs()->linkage().find(ret->addr()) != obj->cs()->linkage().end())
        ret->isPLTFunction_ = true;

    // make an entry instpoint
    size_t insn_size = 0;
    unsigned char * insn_buf = (unsigned char *)isrc->getPtrToInstruction(addr);
    InstructionDecoder dec(insn_buf,InstructionDecoder::maxInstructionLength,
        isrc->getArch());
    Instruction::Ptr insn = dec.decode();
    if(insn)
        insn_size = insn->size();

#if defined(os_vxworks)
   // Relocatable objects (kernel modules) are instrumentable on VxWorks.
    if(!ret->isInstrumentableByFunctionName())
#else
    if(!ret->isInstrumentableByFunctionName() || _img->isRelocatableObj())
#endif
        ret->setInstLevel(UNINSTRUMENTABLE);
    else {
        // Create instrumentation points for non-plt functions 
        if(obj->cs()->linkage().find(addr) != obj->cs()->linkage().end()) { 
            ret->setInstLevel(UNINSTRUMENTABLE);
        }
    }

    /*
     * OMP parallel regions support
     */
#if defined(os_aix)
    if(strstr(stf->getAllMangledNames()[0].c_str(), "@OL@") != NULL){
        image_parRegion * pR = new image_parRegion(stf->getOffset(),ret);
        _img->parallelRegions.push_back(pR);
    }
#endif

    return ret;
}

Block *
DynCFGFactory::mkblock(Function * f, CodeRegion *r, Address addr) {
    parse_block * ret;

    record_block_alloc(false);

    ret = new parse_block((parse_func*)f,r,addr);
    //fprintf(stderr,"mkbloc(%lx, %lx) produced %p\n",f->addr(),addr,ret);
    blocks_.add(*ret);

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
    blocks_.add(*ret);
    return ret;
}

Edge *
DynCFGFactory::mkedge(Block * src, Block * trg, EdgeTypeEnum type) {
    image_edge * ret;

    record_edge_alloc(type,false); // FIXME can't tell if it's a sink

    ret = new image_edge((parse_block*)src,
                         (parse_block*)trg,
                         type);

    //fprintf(stderr,"mkedge between Block %p and %p, img_bb: %p and %p\n",
        //src,trg,(parse_block*)src,(parse_block*)trg);
    edges_.add(*ret);

    return ret;
}

void
DynParseCallback::abruptEnd_cf(Address /*addr*/,ParseAPI::Block * /*b*/,default_details*)
{
}

void
DynParseCallback::newfunction_retstatus(Function *func)
{
    dynamic_cast<parse_func*>(func)->setinit_retstatus( func->retstatus() );
}

void
DynParseCallback::block_split(Block *first, Block *second)
{
   _img->addSplitBlock(static_cast<parse_block *>(first),
                       static_cast<parse_block *>(second));
}

void DynParseCallback::block_delete(Block * /*b*/) {

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
DynParseCallback::interproc_cf(Function*f,Block * /*b*/,Address /*addr*/,interproc_details* det)
{
#if defined(ppc32_linux) || defined(ppc32_bgp)
    if(det->type == interproc_details::call) {
        parse_func * ifunc = static_cast<parse_func*>(f);
        _img->updatePltFunc(ifunc,det->data.call.target);
    }
#else
    f = f; // compiler warning
    det = det;
#endif
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
DynParseCallback::updateCodeBytes(Address target)
{   // calls function that updates bytes if needed
    assert(BPatch_normalMode != _img->hybridMode());
    return codeBytesUpdateCB( _img->cb_arg0(), 
                              target + _img->desc().loadAddr() );
}

bool 
DynParseCallback::loadAddr(Address absoluteAddr, Address & loadAddr) 
{ 
    std::vector<BPatch_process*> * procs = BPatch::bpatch->getProcesses();
    for (unsigned pidx=0; pidx < procs->size(); pidx++) {
        if ((*procs)[pidx]->lowlevel_process()->findObject(_img->desc())) {
            mapped_object * obj = (*procs)[pidx]->lowlevel_process()->
                findObject(absoluteAddr);
            if (obj) {
                loadAddr = obj->codeBase();
                return true;
            }
            return false;
        }
    }
    return false; 
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

