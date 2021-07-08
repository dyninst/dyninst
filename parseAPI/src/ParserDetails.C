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

#include "CodeObject.h"
#include "CFG.h"
#include "ParseCallback.h"

#include "Parser.h"
#include "ParserDetails.h"
#include "debug_parse.h"
#include "util.h"

using namespace std;
using namespace Dyninst;
using namespace Dyninst::ParseAPI;

typedef std::pair<Address, EdgeTypeEnum> edge_pair_t;
typedef vector<edge_pair_t> Edges_t;

#define VERBOSE_EDGE_LOG

namespace {

    inline void
#ifndef VERBOSE_EDGE_LOG
    verbose_log(Address /* currAddr */, Edges_t::iterator & /* curEdge */)
    {
#else
    verbose_log(Address currAddr, Edges_t::iterator &curEdge) {
        using namespace Dyninst::ParseAPI;

        switch (curEdge->second) {
            case CALL:
                parsing_printf("%s[%d]: adding call edge %lx->%lx\n",
                               FILE__, __LINE__, currAddr, curEdge->first);
                break;
            case CALL_FT:
                parsing_printf("%s[%d]: adding function fallthrough edge %lx->%lx\n",
                               FILE__, __LINE__, currAddr, curEdge->first);
                break;
            case FALLTHROUGH:
                parsing_printf("%s[%d]: adding fallthrough edge %lx->%lx\n",
                               FILE__, __LINE__, currAddr, curEdge->first);
                break;
            case COND_TAKEN:
                parsing_printf("%s[%d]: adding conditional taken edge %lx->%lx\n",
                               FILE__, __LINE__, currAddr, curEdge->first);
                break;
            case COND_NOT_TAKEN:
                parsing_printf("%s[%d]: adding conditional not taken edge %lx->%lx\n",
                               FILE__, __LINE__, currAddr, curEdge->first);
                break;
            case INDIRECT:
                parsing_printf("%s[%d]: adding indirect edge %lx->%lx\n",
                               FILE__, __LINE__, currAddr, curEdge->first);
                break;
            case DIRECT:
                parsing_printf("%s[%d]: adding direct edge %lx->%lx\n",
                               FILE__, __LINE__, currAddr, curEdge->first);
                break;
            case CATCH:
                parsing_printf("%s[%d]: adding catch block edge %lx->%lx\n",
                               FILE__, __LINE__, currAddr, curEdge->first);
                break;
            default:
                parsing_printf("%s[%d]: adding unknown edge type %d edge %lx->%lx\n",
                               FILE__, __LINE__, curEdge->second, currAddr, curEdge->first);
                break;
        }
#endif // VERBOSE_EDGE_LOG
    }
} // anonymous namespace

static void
getBlockInsns(Block &blk, std::set<Address> &addrs) {
    unsigned bufSize = blk.size();
    using namespace InstructionAPI;
    const unsigned char *bufferBegin = (const unsigned char *)
            (blk.region()->getPtrToInstruction(blk.start()));
    InstructionDecoder dec = InstructionDecoder
            (bufferBegin, bufSize, blk.region()->getArch());
    InstructionAdapter_t *ah = InstructionAdapter_t::makePlatformIA_IAPI(blk.obj()->cs()->getArch(), dec, blk.start(),
                                                                         blk.obj(), blk.region(), blk.obj()->cs(),
                                                                         &blk);

    for (; ah->getAddr() < blk.end(); ah->advance()) {
        addrs.insert(ah->getAddr());
    }
    delete ah;
}

/* called in defensive mode to create parseFrames at tampered addresses 
   for functions that return TAMPER_ABS. */
ParseFrame *
Parser::getTamperAbsFrame(Function *tamperFunc) {
    assert(TAMPER_ABS == tamperFunc->tampersStack());
    Function *targFunc = NULL;

    // get the binary's load address and subtract it

    CodeSource *cs = tamperFunc->obj()->cs();
    set<CodeRegion *> targRegs;

    Address loadAddr = cs->loadAddress();
    Address target = tamperFunc->_tamper_addr - loadAddr;
    if (loadAddr < tamperFunc->_tamper_addr) {
        cs->findRegions(target, targRegs);
    }
    if (targRegs.empty()) {
        mal_printf("ERROR: could not create function at tamperAbs "
                           "addr, which is in another object %lx\n", target);
        return NULL;
    }

    assert(1 == targRegs.size()); // we don't do analyze stack tampering on 
    // platforms that use archive files
    targFunc = _parse_data->createAndRecordFunc
            (*(targRegs.begin()),
             target,
             tamperFunc->src());

    if (!targFunc) {
        targFunc = _parse_data->createAndRecordFunc(*(targRegs.begin()), target, RT);
    }
    if (!targFunc) 
        targFunc = _parse_data->findFunc(*(targRegs.begin()), target);

    if (!targFunc) {
        mal_printf("ERROR: could not create function at tamperAbs addr %lx\n",
                   tamperFunc->_tamper_addr);
        return NULL;
    }

    ParseFrame *pf = NULL;
    CodeRegion *reg = targFunc->region();

    ParseFrame::Status exist = _parse_data->frameStatus(reg, target);
    switch (exist) {
        case ParseFrame::FRAME_ERROR:
        case ParseFrame::PROGRESS:
            fprintf(stderr, "ERROR: function frame at %lx in bad state, can't "
                    "add edge; status=%d\n", target, exist);
            return NULL;
            break;
        case ParseFrame::PARSED:
            fprintf(stderr, "ERROR: function frame at %lx already parsed, can't "
                    "add edge; status=%d\n", target, exist);
            return NULL;
            break;
        case ParseFrame::BAD_LOOKUP:
            // create new frame
            pf = _parse_data->findFrame(reg, target);
            assert(!pf);
            pf = new ParseFrame(targFunc, _parse_data);
            break;
        case ParseFrame::UNPARSED:
        case ParseFrame::CALL_BLOCKED:
            pf = _parse_data->findFrame(reg, target);
            if (!pf) {
                fprintf(stderr, "ERROR: no function frame at %lx for frame "
                                "that should exist, can't add edge; status=%d\n",
                        target, exist);
                return NULL;
            }
            break;
        default:
            assert(0);
    }

    // make a temp edge
    Function::const_blocklist ret_blks = tamperFunc->returnBlocks();
    for (auto bit = ret_blks.begin();
         bit != ret_blks.end();
         ++bit) {
        Edge *edge = link_tempsink(*bit, CALL);

        // create new bundle since we're not adding CALL,CALL_FT edge pairs
        /* FIXME flag for Drew or Kevin:
             this code (see original in comment below) does not add
            the new work element to the worklist; I preserved this
            behavior while changing code to fit the new, 
            leak-free idiom, but I don't know whether this is intended
            or a bug. You might want to wrap the call in a frame.pushWork.
         */
        (void) pf->mkWork(NULL, edge, edge->src()->last(), target, true, true);
        /*
        ParseWorkBundle *bundle = new ParseWorkBundle();
        pf->work_bundles.push_back(bundle);
        bundle->add(
            new ParseWorkElem(
                bundle,
                edge,
                target,
                true,
                true)
          );
        */
    }

    return pf;
}

// Param pf tampers with its stack by a relative or absolute amount. 
// In the first case, adjust CALL_FT target edge if there is one
// In the second case, add a new ParseFrame to the worklist or 
// trigger parsing in the target object. 
void
Parser::tamper_post_processing(LockFreeQueue<ParseFrame *> &work_queue, ParseFrame *pf) {
    vector<ParseFrame *> work;
    std::copy(work_queue.begin(), work_queue.end(), std::back_inserter(work));
    // tampers with stack by relative amount: 
    // adjust CALL_FT target edge if there is one
    for (auto widx = work.begin();
         pf->func->tampersStack() == TAMPER_REL &&
         widx != work.end();
         widx++) {
        if ((*widx)->status() == ParseFrame::CALL_BLOCKED &&
            pf->func == (*widx)->call_target) {
            for (unsigned bidx = 0;
                 bidx < (*widx)->work_bundles.size();
                 bidx++) {
                const vector<ParseWorkElem *> &elems =
                        (*widx)->work_bundles[bidx]->elems();
                bool rightBundle = false;
                ParseWorkElem *ftEdge = NULL;
                for (unsigned eix = 0; eix < elems.size(); eix++) {
                    if (NULL == elems[eix]->edge()) {
                        continue;
                    }
                    if (elems[eix]->edge()->type() == CALL &&
                        elems[eix]->target() == pf->func->addr()) {
                        rightBundle = true;
                    } else if (elems[eix]->edge()->type() == CALL_FT) {
                        ftEdge = elems[eix];
                    }
                }
                if (rightBundle && ftEdge) {
                    ftEdge->setTarget(ftEdge->target() +
                                      pf->func->_tamper_addr);
                }
            }
        }
    }
    // create frame for TAMPER_ABS target in this object or parse
    // in target object 
    /*
    if (pf->func->tampersStack() == TAMPER_ABS) {
        Address objLoad = 0;
        CodeObject *targObj = NULL;
        if (_pcb.absAddr(pf->func->_tamper_addr,
                         objLoad,
                         targObj)) {
            if (targObj == &_obj) { // target is in this object, add frame
                ParseFrame *tf = getTamperAbsFrame(pf->func);
                if (tf && !_parse_data->findFrame(tf->func->region(),
                                                  tf->func->addr())) {
                    init_frame(*tf);
                    frames.insert(tf);
                    _parse_data->registerFrame(tf);
                    _pcb.updateCodeBytes(pf->func->_tamper_addr - objLoad);
                }
                if (tf) {
                    mal_printf("adding TAMPER_ABS target %lx frame\n",
                               pf->func->_tamper_addr);
                    work_queue.insert(tf);
                }
            } else { // target is in another object, parse there
                mal_printf("adding TAMPER_ABS target %lx "
                                   "in separate object at %lx\n",
                           pf->func->_tamper_addr, objLoad);
                _obj.parse(pf->func->_tamper_addr - objLoad, true);
            }
        } else {
            mal_printf("discarding invalid TAMPER_ABS target %lx\n",
                       pf->func->_tamper_addr);
        }
    }
    */
}


/*
 * Extra handling for bad jump instructions
 */
inline
void Parser::ProcessUnresBranchEdge(
        ParseFrame &frame,
        Block *cur,
        InstructionAdapter_t *ah,
        Address target) {
    ParseCallback::interproc_details det;
    det.ibuf = (unsigned char *)
            frame.func->isrc()->getPtrToInstruction(ah->getAddr());
    det.isize = ah->getSize();
    if (((Address) -1) == target) {
        det.data.unres.target = 0;
    } else {
        det.data.unres.target = target;
    }
    det.type = ParseCallback::interproc_details::unresolved;
    det.data.unres.target = target;

    bool valid;
    Address addr;
    boost::tie(valid, addr) = ah->getCFT();
    if (!valid) {
        det.data.unres.dynamic = true;
        det.data.unres.absolute_address = true;
    } else {
        det.data.unres.dynamic = false;
        det.data.unres.absolute_address = false;
    }
    _pcb.interproc_cf(frame.func, cur, ah->getAddr(), &det);
}

/*
 * Extra handling of return instructions
 */
inline
void Parser::ProcessReturnInsn(
        ParseFrame &frame,
        Block *cur,
        InstructionAdapter_t *ah) {
    // returns always target the sink block
    link_block(cur, _sink, RET, true);

    ParseCallback::interproc_details det;
    det.ibuf = (unsigned char *)
            frame.func->isrc()->getPtrToInstruction(ah->getAddr());
    det.isize = ah->getSize();
    det.type = ParseCallback::interproc_details::ret;

    _pcb.interproc_cf(frame.func, cur, ah->getAddr(), &det);
}


/*
 * Extra handling for literal call instructions
 * as well as other instructions treated as calls
 */
inline
void Parser::ProcessCallInsn(
        ParseFrame &frame,
        Block *cur,
        InstructionAdapter_t *ah,
        bool isDynamic,
        bool isAbsolute,
        bool isResolved,
        Address target) {
    ParseCallback::interproc_details det;
    det.ibuf = (unsigned char *)
            frame.func->isrc()->getPtrToInstruction(ah->getAddr());
    det.isize = ah->getSize();

    if (ah->isCall()) {
        det.data.call.absolute_address = isAbsolute;
        det.data.call.dynamic_call = isDynamic;
        det.data.call.target = target;
        if (likely(isResolved))
            det.type = ParseCallback::interproc_details::call;
        else
            det.type = ParseCallback::interproc_details::unresolved;
    } else
        det.type = ParseCallback::interproc_details::branch_interproc;

    _pcb.interproc_cf(frame.func, cur, ah->getAddr(), &det);
}

bool Parser::ProcessCFInsn(
        ParseFrame &frame,
        Block *cur,
        InstructionAdapter_t *ah)
{
    FuncReturnStatus insn_ret;
    Edges_t edges_out;
    ParseWorkBundle *bundle = NULL;
    bool set_func_to_return = false;

    region_data::edge_data_map::accessor a;
    region_data::edge_data_map* edm = _parse_data->get_edge_data_map(frame.func->region());
    assert(edm->find(a, ah->getAddr()));
    cur = a->second.b;


    // Instruction adapter provides edge estimates from an instruction
    parsing_printf("Getting edges\n");
    ah->getNewEdges(edges_out, frame.func, cur, frame.num_insns, &plt_entries, frame.knownTargets);
    parsing_printf("Returned %lu edges\n", edges_out.size());
    if (unlikely(_obj.defensiveMode() && !ah->isCall() && edges_out.size())) {
        // only parse branch edges that align with existing blocks
        //
        // Xiaozhu: The person who works on defensive mode needs to
        // revisit this code. In parallel parsing, the block boundary (cur->end())
        // is not reliable because it can be split by another thread
        bool hasUnalignedEdge = false;
        set<CodeRegion *> tregs;
        set<Block *> tblocks;
        set<Address> insns_cur;
        for (Edges_t::iterator curEdge = edges_out.begin();
             !hasUnalignedEdge && curEdge != edges_out.end();
             ++curEdge) {
            if (cur->end() <= curEdge->first) {
                parsing_printf("%s[%d] skipping edge\n", FILE__, __LINE__);
                continue;
            }
            _obj.cs()->findRegions(curEdge->first, tregs);
            if (!tregs.empty()) {
                _parse_data->findBlocks(*tregs.begin(), curEdge->first, tblocks);
                for (set<Block *>::iterator bit = tblocks.begin();
                     !hasUnalignedEdge && bit != tblocks.end();
                     bit++) {
                    if ((*bit)->end() <= cur->start() ||
                        (*bit) == cur) {
                        parsing_printf("%s[%d] skipping edge\n", FILE__, __LINE__);
                        continue;
                    }
                    set<Address> insns_tblk;
                    getBlockInsns(**bit, insns_tblk);
                    if (insns_cur.empty()) {
                        getBlockInsns(*cur, insns_cur);
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
            parsing_printf("Has unaligned edge, clearing and treating as abrupt end\n");
            edges_out.clear();
            ParseCallback::default_details det(
                    (unsigned char *) cur->region()->getPtrToInstruction(cur->lastInsnAddr()),
                    cur->end() - cur->lastInsnAddr(),
                    true);
            _pcb.abruptEnd_cf(cur->lastInsnAddr(), cur, &det);
        }
    }

    insn_ret = ah->getReturnStatus(frame.func, frame.num_insns);

    // Update function return status if possible
    if (unlikely(insn_ret != UNSET && frame.func->_rs < RETURN) && !HASHDEF(plt_entries, frame.func->addr())) {
        // insn_ret can only be UNSET, UNKNOWN, or RETURN
        // UNKNOWN means that there is an unresolved undirect control flow,
        // such as unresolve jump tables or indirect tail calls.
        // In such cases, we do not have concrete evidence that
        // the function cannot not return, so we mark this function as RETURN.
        frame.func->set_retstatus(RETURN);
        set_func_to_return = true;
    }

    // Return instructions need extra processing
    if (insn_ret == RETURN)
        ProcessReturnInsn(frame, cur, ah);

    bool dynamic_call = ah->isDynamicCall();
    bool absolute_call = ah->isAbsoluteCall();
    // unresolved is true for indirect calls, unresolved indirect branches, 
    // and later on is set set to true for transfers to bad addresses
    bool has_unres = ah->hasUnresolvedControlFlow(frame.func, frame.num_insns);

    parsing_printf("\t\t%lu edges:\n", edges_out.size());
    for (Edges_t::iterator curEdge = edges_out.begin();
         curEdge != edges_out.end(); ++curEdge) {
        Edge *newedge = NULL;
        bool resolvable_edge = true;
        bool tailcall = false;
        parsing_printf("\t\t\tedge target %lx, is_code %d, is_plt %d, edge type %d, dynamic_call %d\n", 
                curEdge->first, is_code(frame.func, curEdge->first), HASHDEF(plt_entries, curEdge->first),
                curEdge->second, dynamic_call);
        if (!is_code(frame.func, curEdge->first) &&
            !HASHDEF(plt_entries, curEdge->first)) {
            if (curEdge->second != CALL || !dynamic_call) {
                has_unres = true;
                resolvable_edge = false;
                if ((int) curEdge->second != -1 && _obj.defensiveMode())
                    mal_printf("bad edge target at %lx type %d\n",
                               curEdge->first, curEdge->second);
            }
        }

        /*
         * Call case 
         */
        if (curEdge->second == CALL) {
            // call callback
            resolvable_edge = resolvable_edge && !dynamic_call;
            ProcessCallInsn(frame, cur, ah, dynamic_call,
                            absolute_call, resolvable_edge, curEdge->first);

            if (resolvable_edge) {
                newedge = link_tempsink(cur, CALL);
            } else {
                newedge = link_block(cur, _sink, CALL, true);
            }
            if (!ah->isCall()) {
                parsing_printf("Setting edge 0x%p (0x%lx/0x%lx) to interproc\n",
                               (void*)newedge,
                               newedge->src()->start(),
                               newedge->trg_addr());
                newedge->_type._interproc = true;
            }
        }
            /*
             * All other edge types are handled identically
             */
        else {
            if (resolvable_edge) {
                newedge = link_tempsink(cur, curEdge->second);
            } else
                newedge = link_block(cur, _sink, curEdge->second, true);
        }

        if (ah->isTailCall(frame.func, curEdge->second, frame.num_insns, frame.knownTargets)) {
            tailcall = true;
            parsing_printf("Setting edge 0x%p (0x%lx/0x%lx) to interproc (tail call)\n",
                           (void*)newedge,
                           newedge->src()->start(),
                           newedge->trg_addr());
            newedge->_type._interproc = true;
        }

        if (!bundle) {
            bundle = new ParseWorkBundle();
            frame.work_bundles.push_back(bundle);
        }

        verbose_log(ah->getAddr(), curEdge);
        parsing_printf("resolveable_edge: %d, tailcall: %d, target: %lx\n", resolvable_edge, tailcall, curEdge->first);
        ParseWorkElem *we =
                bundle->add(
                        new ParseWorkElem(
                                bundle,
                                newedge,
                                ah->getAddr(),
                                curEdge->first,
                                resolvable_edge,
                                tailcall)
                );
        frame.knownTargets.insert(curEdge->first);

        // We will not attempt to further process
        // unresolvable edges; they stay sinks
        if (resolvable_edge) {
            parsing_printf("[%s:%d] pushing %lx onto worklist\n",
                           FILE__, __LINE__, we->target());
            parsing_printf("[%s:%d] new edge is %p\n", FILE__, __LINE__, (void*)newedge);
            frame.pushWork(we);

            if (unlikely(_obj.defensiveMode())) {
                // update the underlying code bytes for CF targets
                if (CALL == curEdge->second
                    || DIRECT == curEdge->second
                    || COND_TAKEN == curEdge->second) {

                    _pcb.updateCodeBytes(curEdge->first);
                }
            }
        } else if (unlikely(_obj.defensiveMode())) {
            ProcessUnresBranchEdge(frame, cur, ah, curEdge->first);
        }
    }

    if (unlikely(has_unres && edges_out.empty())) {
        link_block(cur, _sink, INDIRECT, true);
        ProcessUnresBranchEdge(frame, cur, ah, -1);
    }

    if (ah->isDelaySlot())
        ah->advance();

    if (!frame.func->_cleans_stack && ah->cleansStack()) {
        frame.func->_cleans_stack = true;
    }
    return set_func_to_return;
}
