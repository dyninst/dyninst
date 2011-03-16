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

#if defined(cap_instruction_api)
#include "InstructionDecoder.h"
#include "Instruction.h"
#else
#include "parseAPI/src/InstrucIter.h"
#endif

#include "symtab.h"
#include "image-func.h"
#include "instPoint.h"
#include "Parsing.h"
#include "debug.h"

#if defined(os_aix) || defined(os_solaris)
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
    image_func * ret;
    SymtabAPI::Symtab * st;
    SymtabAPI::Function * stf;
    pdmodule * pdmod;
    image_instPoint * entry;

    record_func_alloc(src);

    st = _img->getObject();
    if(!st->findFuncByEntryOffset(stf,addr)) {
        pdmod = _img->getOrCreateModule(st->getDefaultModule());
        stf = st->createFunction(
            name,addr,std::numeric_limits<size_t>::max(),pdmod->mod());
    } else
        pdmod = _img->getOrCreateModule(stf->getModule());
    assert(stf);

    ret = new image_func(stf,pdmod,_img,obj,reg,isrc,src);
    funcs_.add(*ret);

    if(obj->cs()->linkage().find(ret->addr()) != obj->cs()->linkage().end())
        ret->isPLTFunction_ = true;

    // make an entry instpoint
    size_t insn_size = 0;
    unsigned char * insn_buf = (unsigned char *)isrc->getPtrToInstruction(addr);
#if defined(cap_instruction_api)
    InstructionDecoder dec(insn_buf,InstructionDecoder::maxInstructionLength,
        isrc->getArch());
    Instruction::Ptr insn = dec.decode();
    if(insn)
        insn_size = insn->size();
#else
   InstrucIter ah(addr,isrc);
   instruction insn = ah.getInstruction();
   insn_size = insn.size();
#endif

#if defined(os_vxworks)
   // Relocatable objects (kernel modules) are instrumentable on VxWorks.
    if(!ret->isInstrumentableByFunctionName())
#else
    if(!ret->isInstrumentableByFunctionName() || _img->isRelocatableObj())
#endif
        ret->setInstLevel(UNINSTRUMENTABLE);
    else {
        // Create instrumentation points for non-plt functions 
        if(obj->cs()->linkage().find(addr) == obj->cs()->linkage().end()) { 
            entry = new image_instPoint(addr,insn_buf,insn_size,
                                        _img,functionEntry);
            _img->addInstPoint(entry);
        } else {
            ret->setInstLevel(UNINSTRUMENTABLE);
        }
    }

    /*
     * OMP parallel regions support
     */
#if defined(os_solaris)
    if(strstr(stf->getAllMangledNames()[0].c_str(), "_$") != NULL){
        image_parRegion * pR = new image_parRegion(stf->getOffset(),ret);
        _img->parallelRegions.push_back(pR);
    }
#elif defined(os_aix)
    if(strstr(stf->getAllMangledNames()[0].c_str(), "@OL@") != NULL){
        image_parRegion * pR = new image_parRegion(stf->getOffset(),ret);
        _img->parallelRegions.push_back(pR);
    }
#endif

    return ret;
}

Block *
DynCFGFactory::mkblock(Function * f, CodeRegion *r, Address addr) {
    image_basicBlock * ret;

    record_block_alloc(false);

    ret = new image_basicBlock((image_func*)f,r,addr);
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
    image_basicBlock * ret;

    record_block_alloc(true);

    ret = new image_basicBlock(obj,r,numeric_limits<Address>::max());
    blocks_.add(*ret);
    return ret;
}

Edge *
DynCFGFactory::mkedge(Block * src, Block * trg, EdgeTypeEnum type) {
    image_edge * ret;

    record_edge_alloc(type,false); // FIXME can't tell if it's a sink

    ret = new image_edge((image_basicBlock*)src,
                         (image_basicBlock*)trg,
                         type);

    //fprintf(stderr,"mkedge between Block %p and %p, img_bb: %p and %p\n",
        //src,trg,(image_basicBlock*)src,(image_basicBlock*)trg);
    edges_.add(*ret);

    return ret;
}

void
DynParseCallback::unresolved_cf(Function *f,Address addr,default_details*det)
{
    image_instPoint * p =
        new image_instPoint(
            addr,
            det->ibuf,
            det->isize,
            _img,
            otherPoint,
            true);

    if (det->isbranch)
    	static_cast<image_func*>(f)->setInstLevel(UNINSTRUMENTABLE);

    _img->addInstPoint(p);
}

void
DynParseCallback::abruptEnd_cf(Address addr,default_details*det)
{
    image_instPoint * p =
        new image_instPoint(
            addr,
            det->ibuf,
            det->isize,
            _img,
            abruptEnd,
            false);

    // check for instrumentability? FIXME
    // ah.getInstLevel or something

    _img->addInstPoint(p);
}

void
DynParseCallback::newfunction_retstatus(Function *func)
{
    dynamic_cast<image_func*>(func)->setinit_retstatus( func->retstatus() );
}

void
DynParseCallback::block_split(Block * /*first_*/, Block *second_)
{
    image_basicBlock *second = (image_basicBlock*) second_;
    _img->addSplitBlock(second);
}

void
DynParseCallback::patch_jump_neg1(Address addr)
{
    Architecture arch = _img->codeObject()->cs()->getArch();
    assert( Arch_x86 == arch || Arch_x86_64 == arch );

    unsigned char * ptr = (unsigned char *) _img->getPtrToInstruction(addr);
    ptr[0] = 0x90;
}

void
DynParseCallback::interproc_cf(Function*f,Address addr,interproc_details*det)
{
    image_instPoint * p = NULL;
    switch(det->type) {
        case interproc_details::ret:
            p = new image_instPoint(
                    addr,
                    det->ibuf,
                    det->isize,
                    _img,
                    functionExit);
            break;
        case interproc_details::call:
            p = new image_instPoint(
                    addr,
                    det->ibuf,
                    det->isize,
                    _img,
                    det->data.call.target,
                    det->data.call.dynamic_call,
                    det->data.call.absolute_address,
                    callSite);                    
            break;
        case interproc_details::branch_interproc:
            p = new image_instPoint(
                    addr,
                    det->ibuf,
                    det->isize,
                    _img,
                    functionExit);
            break;
        default:
            assert(0);
    };

    if(p)
        _img->addInstPoint(p);

#if defined(ppc32_linux) || defined(ppc32_bgp)
    if(det->type == interproc_details::call) {
        image_func * ifunc = static_cast<image_func*>(f);
        _img->updatePltFunc(ifunc,det->data.call.target);
    }
#else
    f = f; // compiler warning
#endif
}

void
DynParseCallback::overlapping_blocks(Block*b1,Block*b2)
{
    parsing_printf("[%s:%d] blocks [%lx,%lx) and [%lx,%lx) overlap"
                   "inconsistently\n",
        FILE__,__LINE__,b1->start(),b1->end(),b2->start(),b2->end());
    static_cast<image_basicBlock*>(b1)->markAsNeedingRelocation();
    static_cast<image_basicBlock*>(b2)->markAsNeedingRelocation();
}
