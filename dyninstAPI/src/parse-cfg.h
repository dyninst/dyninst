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
 
// $Id: parse-cfg.h,v 1.37 2008/09/03 06:08:44 jaw Exp $

#ifndef IMAGE_FUNC_H
#define IMAGE_FUNC_H

#include "codeRange.h"
#include "Parsing.h"
#include "Symbol.h"
#include "Function.h"

#include <string>

class pdmodule;

class parse_block : public codeRange, public Dyninst::ParseAPI::Block  {
 public:
    parse_block(parse_func*, Dyninst::ParseAPI::CodeRegion*, Dyninst::Address);
    parse_block(Dyninst::ParseAPI::CodeObject *, Dyninst::ParseAPI::CodeRegion*, Dyninst::Address);

    ~parse_block() = default;

    // just pass through to Block
    Dyninst::Address firstInsnOffset() const;
    Dyninst::Address lastInsnOffset() const;
    Dyninst::Address endOffset() const;
    Dyninst::Address getSize() const;

    // cfg access & various predicates 
    bool isShared() const { return containingFuncs() > 1; }
    bool isExitBlock();
    bool isCallBlock();
    bool isIndirectTailCallBlock();
    bool isEntryBlock(parse_func * f) const;
    parse_func *getEntryFunc() const;  // func starting with this bock

    bool unresolvedCF() const { return unresolvedCF_; }
    bool abruptEnd() const { return abruptEnd_; }
    void setUnresolvedCF(bool newVal);
    void setAbruptEnd(bool newVal) { abruptEnd_ = newVal; }

    // misc utility
    int id() const { return blockNumber_; }
    
    // Returns the address of our callee (if we're a call block, of course)
    std::pair<bool, Dyninst::Address> callTarget();

    // instrumentation-related
    bool needsRelocation() const { return needsRelocation_; }
    void markAsNeedingRelocation() { needsRelocation_ = true; }

    // codeRange implementation
    void *getPtrToInstruction(Dyninst::Address addr) const;
    Dyninst::Address get_address() const { return firstInsnOffset(); }
    unsigned get_size() const { return getSize(); }

 private:
    bool needsRelocation_;
    int blockNumber_;

    bool unresolvedCF_;
    bool abruptEnd_;
};

inline Dyninst::Address 
parse_block::firstInsnOffset() const {
    return Dyninst::ParseAPI::Block::start(); 
}
inline Dyninst::Address 
parse_block::lastInsnOffset() const {
    return Dyninst::ParseAPI::Block::lastInsnAddr();
}
inline Dyninst::Address 
parse_block::endOffset() const {
    return Dyninst::ParseAPI::Block::end();
}
inline Dyninst::Address 
parse_block::getSize() const {
    return Dyninst::ParseAPI::Block::size();
}

class image_edge : public Dyninst::ParseAPI::Edge {
    friend class parse_block;
 public:
    image_edge(parse_block *source, 
               parse_block *target, 
               EdgeTypeEnum type) :
    Dyninst::ParseAPI::Edge(source,target,type)
   { }

    // MSVC++ 2003 does not properly support covariant return types
    // in overloaded methods
#if !defined _MSC_VER || _MSC_VER > 1310 
   virtual parse_block * src() const { return (parse_block*)Dyninst::ParseAPI::Edge::src(); }
   virtual parse_block * trg() const { return (parse_block*)Dyninst::ParseAPI::Edge::trg(); }
#endif

   const char * getTypeString();
};

class parse_func : public Dyninst::ParseAPI::Function
{
   enum UnresolvedCF {
      UNSET_CF,
      HAS_UNRESOLVED_CF,
      NO_UNRESOLVED_CF
   };

  friend class DynCFGFactory;
  friend class DynParseCallback;
  public:
   /* Annotatable requires a default constructor */
   parse_func() { }
  public:
   parse_func(Dyninst::SymtabAPI::Function *func, 
        pdmodule *m, 
        image *i, 
        Dyninst::ParseAPI::CodeObject * obj,
        Dyninst::ParseAPI::CodeRegion * reg,
        Dyninst::InstructionSource * isrc,
        Dyninst::ParseAPI::FuncSource src);

   ~parse_func();

   Dyninst::SymtabAPI::Function* getSymtabFunction() const{
      return  func_; 
   }	

   /*** Function naming ***/
   std::string symTabName() const { 
       return func_->getFirstSymbol()->getMangledName();
   }
   std::string prettyName() const {
       return func_->getFirstSymbol()->getPrettyName();
   }
   std::string typedName() const {
       return func_->getFirstSymbol()->getTypedName();
   }
   Dyninst::SymtabAPI::Aggregate::name_iter symtab_names_begin() const 
   {
     return func_->mangled_names_begin();
   }
   Dyninst::SymtabAPI::Aggregate::name_iter symtab_names_end() const 
   {
     return func_->mangled_names_end();
   }
   Dyninst::SymtabAPI::Aggregate::name_iter pretty_names_begin() const 
   {
     return func_->pretty_names_begin();
   }
   Dyninst::SymtabAPI::Aggregate::name_iter pretty_names_end() const 
   {
     return func_->pretty_names_end();
   }
   Dyninst::SymtabAPI::Aggregate::name_iter typed_names_begin() const 
   {
     return func_->typed_names_begin();
   }
   Dyninst::SymtabAPI::Aggregate::name_iter typed_names_end() const 
   {
     return func_->typed_names_end();
   }
   
   void copyNames(parse_func *duplicate);

   // return true if the name is new (and therefore added)
   bool addSymTabName(std::string name, bool isPrimary = false);
   bool addPrettyName(std::string name, bool isPrimary = false);
   bool addTypedName(std::string name, bool isPrimary = false);

   /*** Location queries ***/
   Dyninst::Address getOffset() const;
   Dyninst::Address getPtrOffset() const;
   unsigned getSymTabSize() const;
   Dyninst::Address getEndOffset(); // May trigger parsing

   void *getPtrToInstruction(Dyninst::Address addr) const;

   /*** misc. accessors ***/
   pdmodule *pdmod() const { return mod_;}
   image *img() const { return image_; }

   ////////////////////////////////////////////////
   // CFG and other function body methods
   ////////////////////////////////////////////////

   ////////////////////////////////////////////////
   // Instpoints!
   ////////////////////////////////////////////////

   // Initiate parsing on this function
   bool parse();
 
   bool isInstrumentable();
   bool hasUnresolvedCF();

   // ----------------------------------------------------------------------


   ///////////////////////////////////////////////////
   // Mutable function code, used for hybrid analysis
   ///////////////////////////////////////////////////
   Dyninst::ParseAPI::FuncReturnStatus init_retstatus() const; // only call on defensive binaries
   void setinit_retstatus(Dyninst::ParseAPI::FuncReturnStatus rs); //also sets retstatus
   bool hasWeirdInsns() { return hasWeirdInsns_; } // true if we stopped the 
                                // parse at a weird instruction (e.g., arpl)
   void setHasWeirdInsns(bool wi);
   void setPrevBlocksUnresolvedCF(size_t newVal) { prevBlocksUnresolvedCF_ = newVal; }
   size_t getPrevBlocksUnresolvedCF() const { return prevBlocksUnresolvedCF_; }


   // ----------------------------------------------------------------------


   ////////////////////////////////////////////////
   // Misc
   ////////////////////////////////////////////////

    struct compare {
        bool operator()(parse_func * const &f1,
                        parse_func * const &f2) const {
            return (f1->getOffset() < f2->getOffset());
        }
    };

#if defined(DYNINST_HOST_ARCH_POWER) || defined(DYNINST_HOST_ARCH_AARCH64)
   bool savesReturnAddr() const { return saves_return_addr_; }
#endif

   parse_block * entryBlock();

   bool isPLTFunction();

   bool isLeafFunc();

   const Dyninst::SymtabAPI::Function *func() const { return func_; }

   bool containsPowerPreamble() { return containsPowerPreamble_; }
   void setContainsPowerPreamble(bool c) { containsPowerPreamble_ = c; }
   parse_func* getNoPowerPreambleFunc() { return noPowerPreambleFunc_; }
   void setNoPowerPreambleFunc(parse_func* f) { noPowerPreambleFunc_ = f; }
   Dyninst::Address getPowerTOCBaseAddress() { return baseTOC_; }
   void setPowerTOCBaseAddress(Dyninst::Address addr) { baseTOC_ = addr; }


 private:
   ///////////////////// Basic func info
   Dyninst::SymtabAPI::Function *func_{nullptr};		/* pointer to the underlying symtab Function */

   pdmodule *mod_{nullptr};	/* pointer to file that defines func. */
   image *image_{nullptr};

   bool hasWeirdInsns_{false};    // true if we stopped the parse at a
								  // weird instruction (e.g., arpl)
   size_t prevBlocksUnresolvedCF_{}; // num func blocks when calculated

   // Some functions are known to be unparesable by name
   bool isInstrumentableByFunctionName();
   UnresolvedCF unresolvedCF_{UNSET_CF};

   Dyninst::ParseAPI::FuncReturnStatus init_retstatus_{Dyninst::ParseAPI::FuncReturnStatus::UNSET};

   // Architecture specific data
   bool saves_return_addr_{false};

   bool isPLTFunction_{false};

   bool containsPowerPreamble_{false};
   // If the function contains the power preamble, this field points the corresponding function that does not contain the preamble
   parse_func* noPowerPreambleFunc_{nullptr};
   Dyninst::Address baseTOC_{};
};

inline Dyninst::Address 
parse_func::getOffset() const {
    return Dyninst::ParseAPI::Function::addr();
}
inline Dyninst::Address 
parse_func::getPtrOffset() const {
    return func_->getFirstSymbol()->getPtrOffset();
}
inline unsigned 
parse_func::getSymTabSize() const { 
    return func_->getFirstSymbol()->getSize();
}



#endif /* FUNCTION_H */
