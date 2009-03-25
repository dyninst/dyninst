/*
 * Copyright (c) 1996-2007 Barton P. Miller
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */

/************************************************************************
 * $Id: Symbol.h,v 1.20 2008/11/03 15:19:24 jaw Exp $

************************************************************************/

// Present a common superclass for all Symbol aggregates. 
// We never create an Aggregate directly, but only via a child class.

#if !defined(_Aggregate_h_)
#define _Aggregate_h_

#include "Annotatable.h"

namespace Dyninst{
namespace SymtabAPI{

class Symbol;
class Module;
class Symtab; 

class Aggregate : public AnnotatableSparse 
{
    friend class Symtab;
 protected:
      SYMTAB_EXPORT Aggregate(Symbol *sym);
   public:
      
      virtual ~Aggregate() {};

      SYMTAB_EXPORT Offset   getOffset() const { return getFirstSymbol()->getOffset(); }
      SYMTAB_EXPORT unsigned getSize() const { return getFirstSymbol()->getSize(); }
      SYMTAB_EXPORT Module * getModule() const { return module_; }
      SYMTAB_EXPORT Region * getRegion() const { return getFirstSymbol()->getRegion(); }

      /***** Symbol Collection Management *****/
      SYMTAB_EXPORT bool addSymbol(Symbol *sym);
      SYMTAB_EXPORT bool removeSymbol(Symbol *sym);
      SYMTAB_EXPORT bool getSymbols(std::vector<Symbol *>&syms) const;
      SYMTAB_EXPORT Symbol * getFirstSymbol() const;

      /***** Symbol naming *****/
      SYMTAB_EXPORT const std::vector<std::string> &getAllMangledNames();
      SYMTAB_EXPORT const std::vector<std::string> &getAllPrettyNames();
      SYMTAB_EXPORT const std::vector<std::string> &getAllTypedNames();

      /***** Aggregate updating *****/
      SYMTAB_EXPORT virtual bool addMangledName(std::string name, bool isPrimary);
      SYMTAB_EXPORT virtual bool addPrettyName(std::string name, bool isPrimary);
      SYMTAB_EXPORT virtual bool addTypedName(std::string name, bool isPrimary);

      SYMTAB_EXPORT bool setModule(Module *mod);
      SYMTAB_EXPORT bool setSize(unsigned size);
      SYMTAB_EXPORT bool setOffset(unsigned offset);
      
   protected:

      bool addMangledNameInt(std::string name, bool isPrimary);
      bool addPrettyNameInt(std::string name, bool isPrimary);
      bool addTypedNameInt(std::string name, bool isPrimary);


      // Offset comes from a symbol
      // Module we keep here so we can have the correct "primary"
      // (AKA 'not DEFAULT_MODULE') module
      Module *module_;

      std::vector<Symbol *> symbols_;

      std::vector<std::string> mangledNames_;
      std::vector<std::string> prettyNames_;
      std::vector<std::string> typedNames_;
};

}
}

#endif
