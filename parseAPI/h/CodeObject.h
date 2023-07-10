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

#ifndef CODE_OBJECT_H
#define CODE_OBJECT_H

#include <set>
#include <string>
#include <vector>
#include <map>

#include "Symtab.h"
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
class ParseCallbackManager;
class CFGModifier;
class CodeSource;

typedef enum {
    PreambleMatching, IdiomMatching
} GapParsingType;

class CodeObject {
   friend class CFGModifier;
 public:
    PARSER_EXPORT static void version(int& major, int& minor, int& maintenance);
    typedef std::set<Function*,Function::less> funclist;

    PARSER_EXPORT CodeObject(CodeSource * cs, 
                             CFGFactory * fact = NULL, 
                             ParseCallback * cb = NULL,
                             bool defensiveMode = false,
                             bool ignoreParse = false);
    PARSER_EXPORT ~CodeObject();

    /** Parsing interface **/
    
    // `hint-based' parsing
    PARSER_EXPORT void parse();
    
    // `exact-target' parsing; optinally recursive
    PARSER_EXPORT void parse(Address target, bool recursive);

    // `even-more-exact-target' parsing; optinally recursive
    PARSER_EXPORT void parse(CodeRegion *cr, Address target, bool recursive);

    // parses new edges in already parsed functions
	struct NewEdgeToParse {
		Block *source;
		Address target;
		EdgeTypeEnum edge_type;
        bool checked; // true if call_ft edges have already had their callees checked
		NewEdgeToParse(Block *a, Address b, EdgeTypeEnum c) : source(a), target(b), edge_type(c), checked(false) {}
        NewEdgeToParse(Block* a, Address b, bool c, EdgeTypeEnum d) : source(a), target(b), edge_type(d), checked(c) { }
	};

    PARSER_EXPORT bool parseNewEdges( std::vector<NewEdgeToParse> & worklist ); 

    // `speculative' parsing
    PARSER_EXPORT void parseGaps(CodeRegion *cr, GapParsingType type=IdiomMatching);

    /** Lookup routines **/

    // functions
    PARSER_EXPORT Function * findFuncByEntry(CodeRegion * cr, Address entry);
    PARSER_EXPORT int findFuncsByBlock(CodeRegion *cr, Block* b, std::set<Function*> &funcs);
    PARSER_EXPORT int findFuncs(CodeRegion * cr, 
            Address addr, 
            std::set<Function*> & funcs);
      // Find functions overlapping the range [start,end)
    PARSER_EXPORT int findFuncs(CodeRegion * cr,
            Address start, Address end,
            std::set<Function*> & funcs);
    PARSER_EXPORT int findCurrentFuncs(CodeRegion * cr,
            Address addr,
            std::set<Function*> & funcs);


    PARSER_EXPORT const funclist & funcs() { return flist; }

    // blocks
    PARSER_EXPORT Block * findBlockByEntry(CodeRegion * cr, Address entry);
    PARSER_EXPORT int findBlocks(CodeRegion * cr, 
        Address addr, std::set<Block*> & blocks);
    // finds blocks without parsing. 
    PARSER_EXPORT int findCurrentBlocks(CodeRegion * cr, 
        Address addr, std::set<Block*> & blocks);
    PARSER_EXPORT Block * findNextBlock(CodeRegion * cr, Address addr);

    /* Misc */
    PARSER_EXPORT CodeSource * cs() const { return _cs; }
    PARSER_EXPORT CFGFactory * fact() const { return _fact; }
    PARSER_EXPORT bool defensiveMode() { return defensive; }

    PARSER_EXPORT bool isIATcall(Address insn, std::string &calleeName);

    // This is for callbacks; it is often much more efficient to 
    // batch callbacks and deliver them all at once than one at a time. 
    // Particularly if we're deleting code, it's better to get
    // "The following blocks were deleted" than "block 1 was deleted;
    // block 2 lost an edge; block 2 was deleted..."

    PARSER_EXPORT void startCallbackBatch();
    PARSER_EXPORT void finishCallbackBatch();
    PARSER_EXPORT void registerCallback(ParseCallback *cb);
    PARSER_EXPORT void unregisterCallback(ParseCallback *cb);

    /*
     * Calling finalize() forces completion of all on-demand
     * parsing operations for this object, if any remain.
     */
    PARSER_EXPORT void finalize();

    /*
     * Deletion support
     */
    PARSER_EXPORT void destroy(Edge *);
    PARSER_EXPORT void destroy(Block *);
    PARSER_EXPORT void destroy(Function *);

    /*
     * Hacky "for insertion" method
     */
    PARSER_EXPORT Address getFreeAddr() const;
    ParseData* parse_data();

 private:
    void process_hints();
    void add_edge(Block *src, Block *trg, EdgeTypeEnum et);
    // allows Functions to link up return edges after-the-fact
    friend void Function::delayed_link_return(CodeObject *,Block*);
    // allows Functions to finalize (need Parser access)
    friend void Function::finalize();
    // allows Function entry blocks to be moved to new regions
    friend void Function::setEntryBlock(Block *);

  private:
    CodeSource * _cs;
    CFGFactory * _fact;
    ParseCallbackManager * _pcb;

    Parser * parser; // parser implementation

    bool owns_factory;
    bool defensive;
    funclist& flist;
};

// We need CFG.h, which is included by this
template <class OutputIterator>
void Block::getFuncs(OutputIterator result) {
  std::set<Function *> stab;
  _obj->findFuncsByBlock(region(), this, stab);
  std::copy(stab.begin(), stab.end(), result);
}


}//namespace ParseAPI
}//namespace Dyninst

#endif
