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
#include "symtabAPI/h/Symtab.h"
#include "symtabAPI/h/Function.h"

#include "CodeObject.h"
#include "CFG.h"
#include "Parser.h"
#include "debug_parse.h"

using namespace std;
using namespace Dyninst;
using namespace Dyninst::ParseAPI;

namespace {
    // initialization help
    static inline CFGFactory * __fact_init(CFGFactory * fact) {
        if(fact) return fact;
        return new CFGFactory();
    }
    static inline ParseCallback * __pcb_init(ParseCallback * cb) {
        if(cb) return cb;
        return new ParseCallback();
    }
}

CodeObject::CodeObject(CodeSource *cs, 
                       CFGFactory *fact, 
                       ParseCallback * cb, 
                       bool defMode) :
    _cs(cs),
    _fact(__fact_init(fact)),
    _pcb(__pcb_init(cb)),
    parser(new Parser(*this,*_fact,*_pcb) ),
    owns_factory(fact == NULL),
    owns_pcb(cb == NULL),
    defensive(defMode),
    flist(parser->sorted_funcs)
{
    process_hints(); // if any
}

void
CodeObject::process_hints()
{
    Function * f = NULL;
    const vector<Hint> & hints = cs()->hints();
    vector<Hint>::const_iterator hit;

    for(hit = hints.begin();hit!=hints.end();++hit) {
        CodeRegion * cr = (*hit)._reg;
        if(!cs()->regionsOverlap())
            f = parser->factory().mkfunc(
                (*hit)._addr,HINT,(*hit)._name,this,cr,cs());
        else
            f = parser->factory().mkfunc(
                (*hit)._addr,HINT,(*hit)._name,this,cr,cr);
        if(f) {
            parsing_printf("[%s] adding hint %lx\n",FILE__,f->addr());
            parser->add_hint(f);
        }
    }
}

CodeObject::~CodeObject() {
    if(owns_factory)
        delete _fact;
    if(owns_pcb)
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
CodeObject::findFuncs(CodeRegion * cr, Address addr, set<Function*> & funcs)
{
    return parser->findFuncs(cr,addr,funcs);
}

Block *
CodeObject::findBlockByEntry(CodeRegion * cr, Address addr)
{
    return parser->findBlockByEntry(cr, addr);
}

int
CodeObject::findBlocks(CodeRegion * cr, Address addr, set<Block*> & blocks)
{
    return parser->findBlocks(cr,addr,blocks);
}

void
CodeObject::parse() {
    if(!parser) {
        fprintf(stderr,"FATAL: internal parser undefined\n");
        return;
    }
    parser->parse();
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
CodeObject::parseGaps(CodeRegion *cr) {
    if(!parser) {
        fprintf(stderr,"FATAL: internal parser undefined\n");
        return;
    }
    parser->parse_gap_heuristic(cr);
}

void
CodeObject::add_edge(Block * src, Block * trg, EdgeTypeEnum et)
{
    parser->link(src,trg,et,false);
}

void
CodeObject::finalize() {
    parser->finalize();
}

void 
CodeObject::deleteFunc(Function *func)
{
    assert(func->_cache_valid);
    parser->remove_func(func);
    func->deleteBlocks(func->_blocks, NULL);
    fact()->free_func(func);
}

// create work elements and pass them to the parser
bool 
CodeObject::parseNewEdges( vector<Block*> & sources, 
                           vector<Address> & targets,
                           vector<EdgeTypeEnum> & edge_types )
{
    map< Function * , FuncReturnStatus > modfuncs;

    vector< ParseWorkElem * > work_elems;
    for (unsigned idx=0; idx < sources.size(); idx++) {
        ParseWorkElem *elem = new ParseWorkElem
            ( NULL, 
              parser->link_tempsink(sources[idx], edge_types[idx]),
              targets[idx],
              true,
              false );
        work_elems.push_back(elem);

        if (defensiveMode()) {
            vector< Function * > funcs;
            sources[idx]->getFuncs(funcs);
            for (unsigned fidx=0; fidx < funcs.size(); fidx++) {
                modfuncs[funcs[fidx]] = funcs[fidx]->retstatus();
            }
        }
    }

    parser->parse_edges( work_elems );

    return true;
}

