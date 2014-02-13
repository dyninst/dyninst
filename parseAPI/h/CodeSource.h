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
#ifndef _CODE_SOURCE_H_
#define _CODE_SOURCE_H_

#include <map>
#include <vector>
#include <utility>
#include <string>

#include "Symtab.h"
#include "IBSTree.h"
#include "dyntypes.h"

#include "InstructionSource.h"

class StatContainer;

namespace Dyninst {
namespace ParseAPI {

class CFGModifier;

/** A CodeSource is a very simple contract that allows a
    CodeObject to get the information it needs to pull code
    from some binary source
**/


class PARSER_EXPORT CodeRegion : public Dyninst::InstructionSource, public Dyninst::interval<Address> {
 public:

    /* Fills a vector with any names associated with the function at at 
       a given address in this code sources, e.g. symbol names in the
       case of a SymtabCodeSource.

       Optional
    */
    virtual void names(Address, vector<std::string> &) { return; }

    /* Finds the exception handler block for a given address
       in the region.

       Optional
    */
    virtual bool findCatchBlock(Address /* addr */, Address & /* catchStart */) 
    { return false; }

    /** interval implementation **/
    Address low() const =0;
    Address high() const =0;

    bool contains(Address) const;

    virtual bool wasUserAdded() const { return false; }

};

/* A starting point for parsing */
struct Hint {
    Hint() : _addr(0), _reg(NULL), _name("") { }
    Hint(Address a, CodeRegion * r, std::string s) :
        _addr(a), _reg(r), _name(s) { }

    Address _addr;
    CodeRegion * _reg;
    std::string _name;
};

class PARSER_EXPORT CodeSource : public Dyninst::InstructionSource {
   friend class CFGModifier;
 private:
    bool _regions_overlap;
    
 protected:
    /*
     * Implementers of CodeSource can fill the following
     * structures with available information. Some 
     * of this information is optional.
     */

    /*
     * Named external linkage table (e.g. PLT on ELF). Optional.
     */
    mutable std::map<Address, std::string> _linkage;

    /*
     * Table of Contents for position independent references. Optional.
     */
    Address _table_of_contents;

    /*
     * Code regions in the binary. At least one region is
     * required for parsing.
     */
    std::vector<CodeRegion *> _regions;

    /*
     * Code region lookup. Must be consistent with
     * the _regions vector. Mandatory.
     */
    Dyninst::IBSTree<CodeRegion> _region_tree;

    /*
     * Hints for where to begin parsing. Required for
     * the default parsing mode, but usage of one of
     * the direct parsing modes (parsing particular
     * locations or using speculative methods) is supported
     * without hints.
     */
    std::vector<Hint> _hints;
    

 public:
    /* Returns true if the function at an address is known to be
       non returning (e.g., is named `exit' on Linux/ELF, etc.).

       Optional.
    */
    virtual bool nonReturning(Address /*func_entry*/) { return false; }
    virtual bool nonReturning(std::string /* func_name */) { return false; }

    /*
     * If the binary file type supplies non-zero base
     * or load addresses (e.g. Windows PE), override.
     */
    virtual Address baseAddress() const { return 0; }
    virtual Address loadAddress() const { return 0; }

    std::map< Address, std::string > & linkage() const { return _linkage; }
    std::vector< Hint > const& hints() const { return _hints; } 
    std::vector<CodeRegion *> const& regions() const { return _regions; }
    int findRegions(Address addr, set<CodeRegion *> & ret) const;
    bool regionsOverlap() const { return _regions_overlap; }

    Address getTOC() const { return _table_of_contents; }
    /* If the binary file type supplies per-function
     * TOC's (e.g. ppc64 Linux), override.
     */
    virtual Address getTOC(Address) const { return _table_of_contents; }

    // statistics accessor
    virtual void print_stats() const { return; }
    virtual bool have_stats() const { return false; }

    // manage statistics
    virtual void incrementCounter(const std::string& /*name*/) const { return; } 
    virtual void addCounter(const std::string& /*name*/, int /*num*/) const { return; }
    virtual void decrementCounter(const std::string& /*name*/) const { return; }
    
 protected:
    CodeSource() : _regions_overlap(false),
                   _table_of_contents(0) {}
    virtual ~CodeSource() {}

    void addRegion(CodeRegion *);
   
 private: 
    // statistics
    virtual bool init_stats() { return false; }

};

/** SymtabCodeRegion and SymtabCodeSource implement CodeSource for program
    binaries supported by the SymtabAPI 
**/

class PARSER_EXPORT SymtabCodeRegion : public CodeRegion {
 private:
    SymtabAPI::Symtab * _symtab;
    SymtabAPI::Region * _region;
 public:
    SymtabCodeRegion(SymtabAPI::Symtab *, SymtabAPI::Region *);
    ~SymtabCodeRegion();

    void names(Address, vector<std::string> &);
    bool findCatchBlock(Address addr, Address & catchStart);

    /** InstructionSource implementation **/
    bool isValidAddress(const Address) const;
    void* getPtrToInstruction(const Address) const;
    void* getPtrToData(const Address) const;
    unsigned int getAddressWidth() const;
    bool isCode(const Address) const;
    bool isData(const Address) const;
    Address offset() const;
    Address length() const;
    Architecture getArch() const;

    /** interval **/
    Address low() const { return offset(); }
    Address high() const { return offset() + length(); }

    SymtabAPI::Region * symRegion() const { return _region; }
};

class PARSER_EXPORT SymtabCodeSource : public CodeSource {
 private:
    SymtabAPI::Symtab * _symtab;
    bool owns_symtab;
    mutable CodeRegion * _lookup_cache;

    static dyn_hash_map<std::string, bool> non_returning_funcs;
    
    // Stats information
    StatContainer * stats_parse;
    bool _have_stats;
    
 public:
    struct hint_filt {
        virtual ~hint_filt() { }
        virtual bool operator()(SymtabAPI::Function * f) =0;
    };

    SymtabCodeSource(SymtabAPI::Symtab *, 
                                   hint_filt *, 
                                   bool allLoadedRegions=false);
    SymtabCodeSource(SymtabAPI::Symtab *);
    SymtabCodeSource(char *);

    ~SymtabCodeSource();

    bool nonReturning(Address func_entry);
    bool nonReturning(std::string func_name);

    bool resizeRegion(SymtabAPI::Region *, Address newDiskSize);

    Address baseAddress() const;
    Address loadAddress() const;
    Address getTOC(Address addr) const;
    SymtabAPI::Symtab * getSymtabObject() {return _symtab;} 

    /** InstructionSource implementation **/
    bool isValidAddress(const Address) const;
    void* getPtrToInstruction(const Address) const;
    void* getPtrToData(const Address) const;
    unsigned int getAddressWidth() const;
    bool isCode(const Address) const;
    bool isData(const Address) const;
    Address offset() const;
    Address length() const;
    Architecture getArch() const;

    void removeHint(Hint);

    static void addNonReturning(std::string func_name);
    
    // statistics accessor
    void print_stats() const;
    bool have_stats() const { return _have_stats; }

    // manage statistics
    void incrementCounter(const std::string& name) const;
    void addCounter(const std::string& name, int num) const; 
    void decrementCounter(const std::string& name) const;

 private:
    void init(hint_filt *, bool);
    void init_regions(hint_filt *, bool);
    void init_hints(dyn_hash_map<void*, CodeRegion*> &, hint_filt*);
    void init_linkage();

    CodeRegion * lookup_region(const Address addr) const;
    void removeRegion(CodeRegion &); // removes from region tree

    void overlapping_warn(const char * file, unsigned line) const;
    
    // statistics
    bool init_stats();
 
};

inline bool CodeRegion::contains(const Address addr) const
{
    return addr >= offset() && addr < (offset() + length());
}

}
}

#endif 
