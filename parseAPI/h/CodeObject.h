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

#ifndef CODE_OBJECT_H
#define CODE_OBJECT_H

#include <map>

#include <Symtab.h>
#include "IBSTree.h"

#include "CodeSource.h"
#include "CFGFactory.h"
#include "CFG.h"
#include "ParseContainers.h"

namespace Dyninst {
namespace ParseAPI {

/** A CodeObject defines a collection of binary code, for example a binary,
    dynamic library, archive, memory snapshot, etc. In the context of
    Dyninst, it maps to an image object.
**/

class Parser;   // internals
class ParseCallback;

class CodeObject {
 public:
    PARSER_EXPORT static void version(int& major, int& minor, int& maintenance);
    typedef ContainerWrapper<
        std::set<Function*,Function::less>,
        Function*,
        Function*
    > funclist;

    PARSER_EXPORT CodeObject(CodeSource * cs, 
               CFGFactory * fact = NULL, 
               ParseCallback * cb = NULL,
               bool defensiveMode = false);
    PARSER_EXPORT ~CodeObject();

    /** Parsing interface **/
    
    // `hint-based' parsing
    PARSER_EXPORT void parse();
    
    // `exact-target' parsing; optinally recursive
    PARSER_EXPORT void parse(Address target, bool recursive);

    // adds new edges to parsed functions
    PARSER_EXPORT bool parseNewEdges( vector<Block*> & sources, 
                                      vector<Address> & targets, 
                                      vector<EdgeTypeEnum> & edge_types);

    // `speculative' parsing
    PARSER_EXPORT void parseGaps(CodeRegion *cr);

    /** Lookup routines **/

    // functions
    PARSER_EXPORT Function * findFuncByEntry(CodeRegion * cr, Address entry);
    PARSER_EXPORT int findFuncs(CodeRegion * cr, 
            Address addr, 
            std::set<Function*> & funcs);
      // Find functions overlapping the range [start,end)
    PARSER_EXPORT int findFuncs(CodeRegion * cr,
            Address start, Address end,
            std::set<Function*> & funcs);
    PARSER_EXPORT funclist & funcs() { return flist; }

    // blocks
    PARSER_EXPORT Block * findBlockByEntry(CodeRegion * cr, Address entry);
    PARSER_EXPORT int findBlocks(CodeRegion * cr, 
        Address addr, std::set<Block*> & blocks);

    /* Misc */
    PARSER_EXPORT CodeSource * cs() const { return _cs; }
    PARSER_EXPORT CFGFactory * fact() const { return _fact; }
    PARSER_EXPORT bool defensiveMode() { return defensive; }
    PARSER_EXPORT void deleteFunc(Function *);

    /*
     * Calling finalize() forces completion of all on-demand
     * parsing operations for this object, if any remain.
     */
    PARSER_EXPORT void finalize();

 private:
    void process_hints();
    void add_edge(Block *src, Block *trg, EdgeTypeEnum et);
    // allows Function to (re-)finalize
    friend void Function::deleteBlocks(vector<Block*> &, Block *);
    // allows Functions to link up return edges after-the-fact
    friend void Function::delayed_link_return(CodeObject *,Block*);
    // allows Functions to finalize (need Parser access)
    friend void Function::finalize();

 private:
    CodeSource * _cs;
    CFGFactory * _fact;
    ParseCallback * _pcb;

    Parser * parser; // parser implementation

    bool owns_factory;
    bool owns_pcb;
    bool defensive;
    funclist flist;

};

}//namespace ParseAPI
}//namespace Dyninst

#endif
