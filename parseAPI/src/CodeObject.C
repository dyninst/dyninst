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
#include "Parser.h"

#include "symtabAPI/h/Symtab.h"
#include "symtabAPI/h/Function.h"

#include "CodeObject.h"
#include "CFG.h"
#include "debug_parse.h"

#include "dyninstversion.h"

using namespace std;
using namespace Dyninst;
using namespace Dyninst::ParseAPI;

namespace {
    // initialization help
    static inline CFGFactory * __fact_init(CFGFactory * fact) {
        if(fact) return fact;
        return new CFGFactory();
    }
}

static const int ParseAPI_major_version = DYNINST_MAJOR_VERSION;
static const int ParseAPI_minor_version = DYNINST_MINOR_VERSION;
static const int ParseAPI_maintenance_version = DYNINST_PATCH_VERSION;

void CodeObject::version(int& major, int& minor, int& maintenance)
{
    major = ParseAPI_major_version;
    minor = ParseAPI_minor_version;
    maintenance = ParseAPI_maintenance_version;
}


CodeObject::CodeObject(CodeSource *cs, 
                       CFGFactory *fact, 
                       ParseCallback * cb, 
                       bool defMode,
                       bool ignoreParse) :
    _cs(cs),
    _fact(__fact_init(fact)),
    _pcb(new ParseCallbackManager(cb)),
    parser(new Parser(*this,*_fact,*_pcb) ),
    owns_factory(fact == NULL),
    defensive(defMode),
    flist(parser->sorted_funcs)
{
    process_hints(); // if any
    if (!ignoreParse)
      parse();
    else {
      // For cases where the user does not want to parse the CodeObject,
      // the user may still provides hints from external source,
      // we should report hints functions back.
      parser->record_hint_functions();
    }
}

void
CodeObject::process_hints()
{
    const dyn_c_vector<Hint> & hints = cs()->hints();
    int size = hints.size();
#pragma omp parallel for schedule(auto)
    for(int i = 0; i < size; ++i) {
        Function * f = NULL;
        CodeRegion * cr = hints[i]._reg;
        if(!cs()->regionsOverlap())
            f = parser->factory()._mkfunc(
               hints[i]._addr,HINT,hints[i]._name,this,cr,cs());
        else
            f = parser->factory()._mkfunc(
                hints[i]._addr,HINT,hints[i]._name,this,cr,cr);
        if(f) {
            parsing_printf("[%s] adding hint %lx\n",FILE__,f->addr());
            parser->add_hint(f);
        }
    }
}

CodeObject::~CodeObject() {
	// NB: We do not own _cs. It is only a polymorphic view.
    if(owns_factory)
        delete _fact;
    delete _pcb;
    if(parser)
        delete parser;
}

Function *
CodeObject::findFuncByEntry(CodeRegion * cr, Address entry)
{
    return parser->findFuncByEntry(cr,entry);
}

int
CodeObject::findFuncsByBlock(CodeRegion *cr, Block *b, set<Function*> &funcs)
{
    assert(parser);
    return parser->findFuncsByBlock(cr, b, funcs);
}

int
CodeObject::findFuncs(CodeRegion * cr, Address addr, set<Function*> & funcs)
{
    assert(parser);
    return parser->findFuncs(cr,addr,funcs);
}
int
CodeObject::findFuncs(CodeRegion * cr, Address start, Address end, set<Function*> & funcs)
{
    assert(parser);
	return parser->findFuncs(cr,start,end,funcs);
}

Block *
CodeObject::findBlockByEntry(CodeRegion * cr, Address addr)
{
    assert(parser);
    return parser->findBlockByEntry(cr, addr);
}

Block *
CodeObject::findNextBlock(CodeRegion * cr, Address addr)
{
    assert(parser);
    return parser->findNextBlock(cr, addr);
}

int
CodeObject::findBlocks(CodeRegion * cr, Address addr, set<Block*> & blocks)
{
    assert(parser);
    return parser->findBlocks(cr,addr,blocks);
}

// find without parsing.
int CodeObject::findCurrentBlocks(CodeRegion * cr, Address addr, set<Block*> & blocks)
{
    assert(parser);
    return parser->findCurrentBlocks(cr,addr,blocks);
}

int CodeObject::findCurrentFuncs(CodeRegion * cr, Address addr, set<Function*> & funcs)
{
    return parser->findCurrentFuncs(cr,addr,funcs);
}

void
CodeObject::parse() {
    if(!parser) {
        fprintf(stderr,"FATAL: internal parser undefined\n");
        return;
    }
    cs()->startTimer(PARSE_TOTAL_TIME);
    parser->parse();
    cs()->stopTimer(PARSE_TOTAL_TIME);

}

void
CodeObject::parse(Address target, bool recursive) {
    if(!parser) {
        fprintf(stderr,"FATAL: internal parser undefined\n");
        return;
    }
    parser->parse_at(target,recursive,ONDEMAND);
}

void
CodeObject::parse(CodeRegion *cr, Address target, bool recursive) {
   if (!parser) {
      fprintf(stderr, "FATAL: internal parser undefined\n");
      return;
   }
   parser->parse_at(cr, target, recursive, ONDEMAND);
}

void
CodeObject::parseGaps(CodeRegion *cr, GapParsingType type /* PreambleMatching 0 */) {
    if(!parser) {
        fprintf(stderr,"FATAL: internal parser undefined\n");
        return;
    }
    if (type == PreambleMatching) {
        parser->parse_gap_heuristic(cr);
    }
    else {
        //Try the probabilistic gap parsing
	parser->probabilistic_gap_parsing(cr);
    }
}

void
CodeObject::add_edge(Block * src, Block * trg, EdgeTypeEnum et)
{
    if (trg == NULL) {
        parser->link_block(src, parser->_sink, et, true);
    } else {
        parser->link_block(src,trg,et,false);
    }
}

void
CodeObject::finalize() {
    parser->finalize();
}

// Call this function on the CodeObject corresponding to the targets,
// not the sources, if the edges are inter-module ones
// 
// create work elements and pass them to the parser
bool 
CodeObject::parseNewEdges( vector<NewEdgeToParse> & worklist )
{
    vector< ParseWorkElem * > work_elems;
    vector<std::pair<Address,CodeRegion*> > parsedTargs;
    for (unsigned idx=0; idx < worklist.size(); idx++) {
        // see if the target block already exists, in which case we can use
        // add_edge
        set<CodeRegion*> regs;
        cs()->findRegions(worklist[idx].target,regs);
        assert(1 == regs.size()); // at present this function doesn't support 
                                  // ambiguous regions for the target address
        Block *trgB = findBlockByEntry(*(regs.begin()), worklist[idx].target);

        if (trgB) {
            // don't add edges that already exist 
            // (this could happen because of shared code)
            bool edgeExists = false;
            boost::lock_guard<Block> g(*worklist[idx].source);
            const Block::edgelist & existingTs = worklist[idx].source->targets();
            for (Block::edgelist::const_iterator tit = existingTs.begin();
                 tit != existingTs.end();
                 tit++)
            {
                if ((*tit)->trg() == trgB && 
                    (*tit)->type() == worklist[idx].edge_type) 
                {
                    edgeExists = true;
                }
            }
            if (!edgeExists) {
                add_edge(worklist[idx].source, trgB, worklist[idx].edge_type);
                if (CALL == worklist[idx].edge_type) {
                    // if it's a call edge, add it to Function::_call_edge_list
                    // since we won't re-finalize the function
                    vector<Function*> funcs;
                    worklist[idx].source->getFuncs(funcs);
                    for(vector<Function*>::iterator fit = funcs.begin();
                        fit != funcs.end();
                        fit++) 
                    {
                        boost::lock_guard<Block> blockGuard(*worklist[idx].source);
                        const Block::edgelist & tedges = worklist[idx].source->targets();
                        for(Block::edgelist::const_iterator eit = tedges.begin();
                            eit != tedges.end();
                            eit++)
                        {
                            if ((*eit)->trg() == trgB) {
                                (*fit)->_call_edge_list.insert(*eit);
                            }
                        }
                    }
                }
            }
        } 
        else {
            parsedTargs.push_back(pair<Address,CodeRegion*>(worklist[idx].target,
                                                            *regs.begin()));
            // FIXME this is a memory leak; bundles need to go in a frame or
            // they will never get reclaimed. Flag for Kevin or Drew
            //
            // The inline comment was added after I made this observation
            // but before the commit; I still don't see it, but if I'm
            // wrong ignore my warning. --nate
            //
            ParseWorkBundle *bundle = new ParseWorkBundle(); //parse_frames will delete when done
            ParseWorkElem *elem;
            // created checked_call_ft frames if appropriate.
            if (worklist[idx].checked && worklist[idx].edge_type == CALL_FT) {
                elem = bundle->add(new ParseWorkElem
                ( bundle, 
                  ParseWorkElem::checked_call_ft,
                  parser->link_tempsink(worklist[idx].source, worklist[idx].edge_type),
                  worklist[idx].source->last(),
                  worklist[idx].target,
                  true,
                  false ));
            } else {
                elem = bundle->add(new ParseWorkElem
                ( bundle, 
                  parser->link_tempsink(worklist[idx].source, worklist[idx].edge_type),
                  worklist[idx].source->last(),
                  worklist[idx].target,
                  true,
                  false ));
            }
            
            work_elems.push_back(elem);
        }
    }

    parser->_pcb.batch_begin(); // must batch callbacks and deliver after parsing structures are stable
    parser->parse_edges( work_elems );
    parser->_pcb.batch_end(_fact);

    if (defensiveMode()) {
        // update tampersStack for modified funcs
        for (unsigned idx=0; idx < parsedTargs.size(); idx++) {
            set<Function*> tfuncs;
            findFuncs(parsedTargs[idx].second, parsedTargs[idx].first, tfuncs);
            for (set<Function*>::iterator fit = tfuncs.begin();
                 fit != tfuncs.end();
                 fit++) 
            {
                (*fit)->tampersStack(true);
            }
        }
    }
    return true;
}

// set things up to pass through to IA_IAPI
bool CodeObject::isIATcall(Address insnAddr, std::string &calleeName)
{
   // find region
   std::set<CodeRegion*> regs;
   cs()->findRegions(insnAddr, regs);
   if (regs.size() != 1) {
      return false;
   }
   CodeRegion *reg = *regs.begin();

   // find block
   std::set<Block*> blocks;
   findBlocks(reg, insnAddr, blocks);
   if (blocks.empty()) {
      return false;
   }
   Block *blk = *blocks.begin();

   const unsigned char* bufferBegin = 
      (const unsigned char *)(cs()->getPtrToInstruction(insnAddr));
   using namespace InstructionAPI;
   InstructionDecoder dec = InstructionDecoder(bufferBegin,
      InstructionDecoder::maxInstructionLength, reg->getArch());
   InstructionAdapter_t* ah = InstructionAdapter_t::makePlatformIA_IAPI(
      cs()->getArch(), dec, insnAddr, this, reg, cs(), blk);
   bool ret = ah->isIATcall(calleeName);
   delete ah;
   return ret;
}

void CodeObject::startCallbackBatch() {
   _pcb->batch_begin();
}

void CodeObject::finishCallbackBatch() {
   _pcb->batch_end(_fact);
}

void CodeObject::destroy(Edge *e) {
   // The callback deletes the object so that we can
   // be sure to allow users to access its data before
   // its freed.
   // We hand in a CFGFactory so that we have customized
   // deletion methods.
   _pcb->destroy(e, _fact);
}

void CodeObject::destroy(Block *b) {
   parser->remove_block(b);
   _pcb->destroy(b, _fact);
}

void CodeObject::destroy(Function *f) {
   parser->remove_func(f);
   _pcb->destroy(f, _fact);
}

void CodeObject::registerCallback(ParseCallback *cb) {
   assert(_pcb);
   _pcb->registerCallback(cb);
}

void CodeObject::unregisterCallback(ParseCallback *cb) {
   _pcb->unregisterCallback(cb);
}

Address CodeObject::getFreeAddr() const {
   // Run over the code regions and return the highest address. We do this
   // so we can allocate more space...
   Address hi = 0;
   const std::vector<CodeRegion *> &regions = _cs->regions();
   for (std::vector<CodeRegion *>::const_iterator iter = regions.begin();
        iter != regions.end(); ++iter) {
      hi = (hi > (*iter)->high()) ? hi : (*iter)->high();
   }
   return hi;
}

ParseData *CodeObject::parse_data() { return parser->parse_data(); }
