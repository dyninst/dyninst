/*
 * Copyright (c) 1996-2009 Barton P. Miller
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
#include "CodeObject.h"
#include "CFG.h"
#include "ParseCallback.h"

#include "Parser.h"
#include "ParserDetails.h"
#include "ParseData.h"
#include "debug_parse.h"
#include "util.h"

using namespace std;
using namespace Dyninst;
using namespace Dyninst::ParseAPI;

typedef std::pair< Address, EdgeTypeEnum > edge_pair_t;
typedef vector< edge_pair_t > Edges_t;

#define VERBOSE_EDGE_LOG

namespace {

inline void
#ifndef VERBOSE_EDGE_LOG                        
verbose_log(Address /* currAddr */, Edges_t::iterator & /* curEdge */)
{
#else
verbose_log(Address currAddr, Edges_t::iterator & curEdge)
{
  using namespace Dyninst::ParseAPI;
  
    switch(curEdge->second)
    {
        case CALL:
            parsing_printf("%s[%d]: adding call edge %x->%x\n",
                           FILE__, __LINE__, currAddr, curEdge->first);
        case CALL_FT:
            parsing_printf("%s[%d]: adding function fallthrough edge %x->%x\n",
                           FILE__, __LINE__, currAddr, curEdge->first);
            break;
        case FALLTHROUGH:
            parsing_printf("%s[%d]: adding fallthrough edge %x->%x\n",
                           FILE__, __LINE__, currAddr, curEdge->first);
            break;
        case COND_TAKEN:
            parsing_printf("%s[%d]: adding conditional taken edge %x->%x\n",
                           FILE__, __LINE__, currAddr, curEdge->first);
            break;
        case COND_NOT_TAKEN:
            parsing_printf("%s[%d]: adding conditional not taken edge %x->%x\n",
                           FILE__, __LINE__, currAddr, curEdge->first);
            break;
        case INDIRECT:
            parsing_printf("%s[%d]: adding indirect edge %x->%x\n",
                           FILE__, __LINE__, currAddr, curEdge->first);
            break;
        case DIRECT:
            parsing_printf("%s[%d]: adding direct edge %x->%x\n",
                           FILE__, __LINE__, currAddr, curEdge->first);
            break;
        case CATCH:
            parsing_printf("%s[%d]: adding catch block edge %x->%x\n",
                           FILE__, __LINE__, currAddr, curEdge->first);
            break;
        default:
            parsing_printf("%s[%d]: adding unknown edge type %d edge %x->%x\n",
                           FILE__, __LINE__, curEdge->second, currAddr, curEdge->first);
            break;
    }
#endif // VERBOSE_EDGE_LOG
}
} // anonymous namespace

static void 
getBlockInsns(Block &blk, std::set<Address> &addrs)
{
    unsigned bufSize = blk.size();
#if defined(cap_instruction_api)
    using namespace InstructionAPI;
    const unsigned char* bufferBegin = (const unsigned char *)
        (blk.obj()->cs()->getPtrToInstruction(blk.start()));
    InstructionDecoder dec = InstructionDecoder
        (bufferBegin, bufSize, blk.region()->getArch());
    InstructionAdapter_t ah = InstructionAdapter_t
        (dec, blk.start(), blk.obj(), blk.region(), blk.obj()->cs());
#else        
    InstrucIter iter(blk.start(), bufSize, blk.obj()->cs());
    InstructionAdapter_t ah(iter, 
        blk.obj(), blk.region(), obj()->cs());
#endif
    for (; ah.getAddr() < blk.end(); ah.advance()) {
        addrs.insert(ah.getAddr());
    } 
}

/*
 * Extra handling of return instructions
 */
inline
void Parser::ProcessReturnInsn(
    ParseFrame & frame,
    Block * cur,
    InstructionAdapter_t & ah)
{
    // returns always target the sink block
    link(cur,_sink,RET,true);

    ParseCallback::interproc_details det;
    det.ibuf = (unsigned char*)
        frame.func->isrc()->getPtrToInstruction(ah.getAddr());
    det.isize = ah.getSize();
    det.type = ParseCallback::interproc_details::ret;

    _pcb.interproc_cf(frame.func,ah.getAddr(),&det);
}

inline 
static
InstructionAdapter_t * getNewAdapter(Function *func, Address addr)
{
    unsigned bufSize = func->region()->offset() 
                     + func->region()->length() 
                     - addr;
#if defined(cap_instruction_api)
    using namespace InstructionAPI;
    const unsigned char* bufferBegin = (const unsigned char *)
        (func->isrc()->getPtrToInstruction(addr));
    InstructionDecoder * dec = new InstructionDecoder
        (bufferBegin, bufSize, func->region()->getArch());
    InstructionAdapter_t * ah = new InstructionAdapter_t
        (*dec, addr, func->obj(), func->region(), func->isrc());
#else        
    InstrucIter *iter = new InstrucIter(addr, bufSize, func->isrc());
    InstructionAdapter_t *ah = new InstructionAdapter_t(*iter, 
        func->obj(), func->region(), func->isrc());
#endif
    return ah;
}

/*
 * Extra handling for literal call instructions
 * as well as other instructions treated as calls
 */
inline
void Parser::ProcessCallInsn(
    ParseFrame & frame,
    Block * /* cur */,
    InstructionAdapter_t & ah,
    bool isDynamic,
    bool isAbsolute,
    bool isResolved,
    Address target)
{
    ParseCallback::interproc_details det;
    det.ibuf = (unsigned char*)
       frame.func->isrc()->getPtrToInstruction(ah.getAddr());
    det.isize = ah.getSize();
    
    if(ah.isCall()) {
        det.data.call.absolute_address = isAbsolute;
        det.data.call.dynamic_call = isDynamic;
        det.data.call.target = target;
        if (likely(isResolved))
            det.type = ParseCallback::interproc_details::call; 
        else
            det.type = ParseCallback::interproc_details::unres_call; 
    }
    else
        det.type = ParseCallback::interproc_details::branch_interproc;
    
    _pcb.interproc_cf(frame.func,ah.getAddr(),&det);
}

void Parser::ProcessCFInsn(
    ParseFrame & frame,
    Block * cur,
    InstructionAdapter_t *& ah)
{
    FuncReturnStatus insn_ret;
    Edges_t edges_out;
    ParseWorkBundle * bundle = NULL;

    // terminate the block at this address
    end_block(cur,*ah);
    
    // Instruction adapter provides edge estimates from an instruction
    ah->getNewEdges(edges_out, frame.func, cur, frame.num_insns, &plt_entries); 

    if (unlikely(_obj.defensiveMode() && !ah->isCall() && edges_out.size())) {
        // only parse branch edges that align with existing blocks
        bool hasUnalignedEdge = false;
        set<CodeRegion*> tregs;
        set<Block*> tblocks;
        set<Address> insns_cur;
        for(Edges_t::iterator curEdge = edges_out.begin();
            !hasUnalignedEdge && curEdge != edges_out.end(); 
            ++curEdge)
        {
            if (cur->end() <= curEdge->first) {
                continue;
            }
            _obj.cs()->findRegions(curEdge->first,tregs);
            if (!tregs.empty()) {
                _parse_data->findBlocks(*tregs.begin(),curEdge->first, tblocks);
                for (set<Block*>::iterator bit= tblocks.begin(); 
                     !hasUnalignedEdge && bit != tblocks.end(); 
                     bit++) 
                {
                    if ((*bit)->end() <= cur->start() ||
                        (*bit) == cur) 
                    {
                        continue;
                    }
                    set<Address> insns_tblk;
                    getBlockInsns(**bit,insns_tblk);
                    if (insns_cur.empty()) {
                        getBlockInsns(*cur,insns_cur);
                    }
                    if ((*bit)->start() < cur->start()) {
                        if (insns_tblk.end() == insns_tblk.find(cur->start())) {
                            hasUnalignedEdge = true;
                        } 
                    } else if (insns_cur.end() == insns_cur.find((*bit)->start())) {
                        hasUnalignedEdge = true;
                    }
                    if (hasUnalignedEdge) {
                        mal_printf("Found unaligned blocks [%lx %lx) [%lx %lx), adding abruptEnd "
                                   "point and killing out edges\n", cur->start(), cur->end(), 
                                   (*bit)->start(), (*bit)->end());
                    }
                }
            }
        }
        if (true == hasUnalignedEdge) {
            edges_out.clear();
            ParseCallback::default_details det(
                (unsigned char*) cur->region()->getPtrToInstruction(cur->lastInsnAddr()),
                cur->end() - cur->lastInsnAddr(),
                true);
            _pcb.abruptEnd_cf(cur->lastInsnAddr(),&det);
        }
    }

    insn_ret = ah->getReturnStatus(frame.func,frame.num_insns); 

    // Update function return status if possible
    if(unlikely(insn_ret != UNSET && frame.func->_rs < RETURN))
        frame.func->_rs = insn_ret; 

    // Return instructions need extra processing
    if(insn_ret == RETURN)
        ProcessReturnInsn(frame,cur,*ah);

    bool dynamic_call = ah->isDynamicCall();
    bool absolute_call = ah->isAbsoluteCall();
    // unresolved is true for indirect calls, unresolved indirect branches, 
    // and later on is set set to true for transfers to bad addresses
    bool has_unres = ah->hasUnresolvedControlFlow(frame.func,frame.num_insns);

    parsing_printf("\t\t%d edges:\n",edges_out.size());
    for(Edges_t::iterator curEdge = edges_out.begin();
        curEdge != edges_out.end(); ++curEdge)
    {
        Edge * newedge = NULL;
        bool resolvable_edge = true;
        bool tailcall = false;

        if(!is_code(frame.func,curEdge->first) &&
           !HASHDEF(plt_entries,curEdge->first))
        {
            if(curEdge->second != NOEDGE || !dynamic_call) {
                has_unres = true;
                resolvable_edge = false;
                if (curEdge->second != -1 && _obj.defensiveMode()) 
                    mal_printf("bad edge target at %lx type %d\n",
                               curEdge->first, curEdge->second);
            }
        }

        /*
         * Call case 
         */ 
        if(curEdge->second == NOEDGE)
        {
            // call callback
            resolvable_edge = resolvable_edge && !dynamic_call;
            ProcessCallInsn(frame,cur,*ah,dynamic_call,
                absolute_call,resolvable_edge,curEdge->first);

            tailcall = !dynamic_call && 
                ah->isTailCall(frame.func,frame.num_insns);
            if(resolvable_edge)
                newedge = link_tempsink(cur,CALL);
            else {
                newedge = link(cur,_sink,CALL,true);
            }
            if(!ah->isCall())
                newedge->_type._interproc = true;
        }
        /*
         * All other edge types are handled identically
         */
        else
        {
            if(resolvable_edge) {
                newedge = link_tempsink(cur,curEdge->second);
            }
            else
                newedge = link(cur,_sink,curEdge->second,true);
        }

        if(!bundle) {
            bundle = new ParseWorkBundle();
            frame.work_bundles.push_back(bundle);
        }

        verbose_log(ah->getAddr(),curEdge);

        ParseWorkElem * we = 
          bundle->add(
            new ParseWorkElem(
                bundle,
                newedge,
                curEdge->first,
                resolvable_edge,
                tailcall)
          );

        // We will not attempt to further process
        // unresolvable edges; they stay sinks
        if(resolvable_edge) {
            parsing_printf("[%s:%d] pushing %lx onto worklist\n",
                FILE__,__LINE__,we->target());
            frame.pushWork(we);

            if (unlikely(_obj.defensiveMode())) {
                // update the underlying code bytes for CF targets
                if (  CALL == curEdge->second
                      || DIRECT == curEdge->second
                      || COND_TAKEN == curEdge->second )
                {
                    _pcb.updateCodeBytes(curEdge->first);
                }
            }
        } 
        else if( unlikely(_obj.defensiveMode() && NOEDGE != curEdge->second) )
        {   
            // invoke callback for: 
            // unresolved indirect branches and direct ctrl transfers 
            // with bad targets (calls will have been taken care of)
            ParseCallback::interproc_details det;
            det.ibuf = (unsigned char*)
               frame.func->isrc()->getPtrToInstruction(ah->getAddr());
            det.isize = ah->getSize();
            det.data.unres.target = curEdge->first;
            det.type = ParseCallback::interproc_details::unres_branch;
            if (-1 == det.data.unres.target) {
                det.data.unres.target = 0;
            }
            bool valid; Address addr;
            boost::tie(valid, addr) = ah->getCFT();
            if (!valid) {
                det.data.unres.dynamic = true;
                det.data.unres.absolute_address = true;
            } else {
                det.data.unres.dynamic = false;
                det.data.unres.absolute_address = false;
            }
            _pcb.interproc_cf(frame.func,ah->getAddr(),&det);
        }
    }

    if(ah->isDelaySlot())
        ah->advance();

    if(!frame.func->_cleans_stack && ah->cleansStack())
        frame.func->_cleans_stack = true;
}
