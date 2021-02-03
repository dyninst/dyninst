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
#ifndef _PARSER_H_
#define _PARSER_H_

#include "ParseData.h"

#include <set>
#include <vector>
#include <queue>
#include <utility>

#include "dyntypes.h"
#include "IBSTree.h"

#include "LockFreeQueue.h"

#include "IA_IAPI.h"
#include "InstructionAdapter.h"

#include "CodeObject.h"
#include "CFG.h"
#include "ParseCallback.h"

#include "common/src/dthread.h"
#include <boost/thread/lockable_adapter.hpp>
#include <boost/thread/shared_mutex.hpp>
#include <unordered_map>

using namespace std;

typedef Dyninst::InsnAdapter::IA_IAPI InstructionAdapter_t;

namespace Dyninst {
    namespace ParseAPI {

        class CFGModifier;

/** This is the internal parser **/
        class Parser {
            // The CFG modifier needs to manipulate the lookup structures,
            // which are internal Parser data.
            friend class CFGModifier;

        private:

            // Owning code object
            CodeObject &_obj;

            // CFG object factory
            CFGFactory &_cfgfact;

            // Callback notifications
            ParseCallbackManager &_pcb;

            // region data store
            ParseData *_parse_data;

            // All allocated frames
            LockFreeQueue<ParseFrame *> frames;

            boost::atomic<bool> delayed_frames_changed;
            dyn_c_hash_map<Function*, std::set<ParseFrame*> > delayed_frames;

            // differentiate those provided via hints and
            // those found through RT or speculative parsing
            dyn_c_vector<Function *> hint_funcs;
            dyn_c_vector<Function *> discover_funcs;

            set<Function *, Function::less> sorted_funcs;
            set<Function*> deleted_func;

            // PLT, IAT entries
            dyn_hash_map<Address, string> plt_entries;

    // a sink block for unbound edges
    boost::atomic<Block *> _sink;
#ifdef ADD_PARSE_FRAME_TIMERS
    dyn_c_hash_map<unsigned int, unsigned int > time_histogram;
#endif
    enum ParseState {
        UNPARSED,       // raw state
        PARTIAL,        // parsing has started
        COMPLETE,       // full parsing -- range queries are invalid
        RETURN_SET,
        FINALIZED,
        UNPARSEABLE     // error condition
    };
    ParseState _parse_state;
        public:
            Parser(CodeObject &obj, CFGFactory &fact, ParseCallbackManager &pcb);

            ~Parser();

            /** Initialization & hints **/
            void add_hint(Function *f);

            // functions
            Function *findFuncByEntry(CodeRegion *cr, Address entry);

            int findFuncsByBlock(CodeRegion *cr, Block *b, set<Function*> &funcs);

            int findFuncs(CodeRegion *cr, Address addr, set<Function *> &funcs);

            int findFuncs(CodeRegion *cr, Address start, Address end, set<Function *> &funcs);

            // blocks
            Block *findBlockByEntry(CodeRegion *cr, Address entry);

            int findBlocks(CodeRegion *cr, Address addr, set<Block *> &blocks);

            // returns current blocks without parsing.
            int findCurrentBlocks(CodeRegion *cr, Address addr, std::set<Block *> &blocks);

            int findCurrentFuncs(CodeRegion *cr, Address addr, set<Function *> &funcs);

            Block *findNextBlock(CodeRegion *cr, Address addr);

            void parse();

            void parse_at(CodeRegion *cr, Address addr, bool recursive, FuncSource src);

            void parse_at(Address addr, bool recursive, FuncSource src);

            void parse_edges(vector<ParseWorkElem *> &work_elems);

            CFGFactory &factory() const { return _cfgfact; }

            CodeObject &obj() { return _obj; }

            // removal
            void remove_block(Block *);

            void remove_func(Function *);

            void move_func(Function *, Address new_entry, CodeRegion *new_reg);

        public:
            /** XXX all strictly internals below this point **/
            Block* record_block(Block *b);

            void record_func(Function *f);

            void init_frame(ParseFrame &frame);

            bool finalize(Function *f);

            ParseData *parse_data() { return _parse_data; }

        private:
            void parse_vanilla();
            void cleanup_frames();
            void parse_gap_heuristic(CodeRegion *cr);

            bool getGapRange(CodeRegion*, Address, Address&, Address&);
            void probabilistic_gap_parsing(CodeRegion *cr);
            //void parse_sbp();

            ParseFrame::Status frame_status(CodeRegion *cr, Address addr);

            /** CFG structure manipulations **/
            void end_block(Block *b, InstructionAdapter_t *ah);

            Block *block_at(ParseFrame &frame,
                            Function *owner,
                            Address addr,
                            Block *&split);

            pair<Block *, Edge *> add_edge(
                    ParseFrame &frame,
                    Function *owner,
                    Block *src,
                    Address src_addr,
                    Address dst,
                    EdgeTypeEnum et,
                    Edge *exist);

            Block *follow_fallthrough(Block *b, Address addr);

            Edge *link_addr(Address src, Block *dst, EdgeTypeEnum et, bool sink, Function* func);
            Edge *link_block(Block* src, Block *dst, EdgeTypeEnum et, bool sink);


            Edge *link_tempsink(Block *src, EdgeTypeEnum et);

            void relink(Edge *exist, Block *src, Block *dst);

            pair<Function *, Edge *> bind_call(
                    ParseFrame &frame, Address target, Block *cur, Edge *exist);

    void parse_frames(LockFreeQueue<ParseFrame *> &, bool);
    void parse_frame(ParseFrame & frame,bool);
    bool parse_frame_one_iteration(ParseFrame & frame, bool);
    bool inspect_value_driven_jump_tables(ParseFrame &);

    void resumeFrames(Function * func, LockFreeQueue<ParseFrame *> & work);

    // defensive parsing details
    void tamper_post_processing(LockFreeQueue<ParseFrame *>&, ParseFrame *);
    ParseFrame * getTamperAbsFrame(Function *tamperFunc);

            /* implementation of the parsing loop */
            void ProcessUnresBranchEdge(
                    ParseFrame &,
                    Block *,
                    InstructionAdapter_t *,
                    Address target);

            void ProcessCallInsn(
                    ParseFrame &,
                    Block *,
                    InstructionAdapter_t *,
                    bool,
                    bool,
                    bool,
                    Address);

            void ProcessReturnInsn(
                    ParseFrame &,
                    Block *,
                    InstructionAdapter_t *);

            bool ProcessCFInsn(
                    ParseFrame &,
                    Block *,
                    InstructionAdapter_t *);

            void finalize();

            void finalize_funcs(dyn_c_vector<Function *> &funcs);
	    void clean_bogus_funcs(dyn_c_vector<Function*> &funcs);
            void finalize_ranges();
            void finalize_jump_tables();
            void delete_bogus_blocks(Edge*);
            bool set_edge_parsing_status(ParseFrame&, Address addr, Block *b);
            void update_function_ret_status(ParseFrame &, Function*, ParseWorkElem* );
            void record_hint_functions();



            void invalidateContainingFuncs(Function *, Block *);

            bool getSyscallNumber(Function *, Block *, Address, Architecture, long int &);

            friend class CodeObject;

    Mutex<true> parse_mutex;

    struct NewFrames : public std::set<ParseFrame*>, public boost::lockable_adapter<boost::mutex> {};

    LockFreeQueueItem<ParseFrame *> *ProcessOneFrame(ParseFrame *pf, bool recursive);

    void SpawnProcessFrame(ParseFrame *frame, bool recursive);

    void ProcessFrames(LockFreeQueue<ParseFrame *> *work_queue, bool recursive);

    void LaunchWork(LockFreeQueueItem<ParseFrame*> *frame_list, bool recursive);


    void processCycle(LockFreeQueue<ParseFrame *> &work, bool recursive);

    void processFixedPoint(LockFreeQueue<ParseFrame *> &work, bool recursive);

    LockFreeQueueItem<ParseFrame *> *postProcessFrame(ParseFrame *pf, bool recursive);

            void updateBlockEnd(Block *b, Address addr, Address previnsn, region_data *rd) const;

            // Range data is initialized through writing to interval trees.
            // This is intrinsitcally mutual exclusive. So we delay this initialization until
            // someone actually needs this.
            //
            // Note: this has to be run in a single thread.
            vector<Function*> funcs_to_ranges;            

            dyn_c_hash_map<Block*, std::set<Function* > > funcsByBlockMap;
        };

    }
}


#endif
