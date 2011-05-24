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

/************************************************************************
 * $Id: Symbol.h,v 1.20 2008/11/03 15:19:24 jaw Exp $

************************************************************************/

// Present a common superclass for all Symbol aggregates. 
// We never create an Aggregate directly, but only via a child class.

#if !defined(_Aggregate_h_)
#define _Aggregate_h_

#include <iostream>
#include "Annotatable.h"

SYMTAB_EXPORT std::ostream &operator<<(std::ostream &os, const Dyninst::SymtabAPI::Aggregate &);

namespace Dyninst{
namespace SymtabAPI{

class Symbol;
class Module;
class Symtab; 
class Region; 
class Aggregate; 
struct SymbolCompareByAddr;

class Aggregate /*: public AnnotatableSparse  */
{
    friend class Symtab;
    friend struct SymbolCompareByAddr;
	friend std::ostream &::operator<<(std::ostream &os, const Dyninst::SymtabAPI::Aggregate &);

 protected:
      SYMTAB_EXPORT Aggregate();
      SYMTAB_EXPORT Aggregate(Symbol *sym);

   public:
      
      virtual ~Aggregate() {};

      SYMTAB_EXPORT Offset   getOffset() const;
      SYMTAB_EXPORT unsigned getSize() const;
      SYMTAB_EXPORT Module * getModule() const { return module_; }
      SYMTAB_EXPORT Region * getRegion() const;

      /***** Symbol Collection Management *****/
      SYMTAB_EXPORT bool addSymbol(Symbol *sym);
      SYMTAB_EXPORT virtual bool removeSymbol(Symbol *sym) = 0;
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
      
	  bool operator==(const Aggregate &a);


   protected:

      bool addMangledNameInt(std::string name, bool isPrimary);
      bool addPrettyNameInt(std::string name, bool isPrimary);
      bool addTypedNameInt(std::string name, bool isPrimary);

      SYMTAB_EXPORT bool removeSymbolInt(Symbol *sym);
      SYMTAB_EXPORT virtual bool changeSymbolOffset(Symbol *sym);

      // Offset comes from a symbol
      // Module we keep here so we can have the correct "primary"
      // (AKA 'not DEFAULT_MODULE') module
      Module *module_;

      std::vector<Symbol *> symbols_;
      Symbol *firstSymbol;  // cached for speed
      Offset offset_;       // cached for speed

      std::vector<std::string> mangledNames_;
      std::vector<std::string> prettyNames_;
      std::vector<std::string> typedNames_;

	  void restore_type_by_id(SerializerBase *, Type *&, unsigned) THROW_SPEC (SerializerError);
	  void restore_module_by_name(SerializerBase *, std::string &) THROW_SPEC (SerializerError);
	  //void rebuild_symbol_vector(SerializerBase *, std::vector<Offset> *) THROW_SPEC (SerializerError);
	  void rebuild_symbol_vector(SerializerBase *, std::vector<Address> &) THROW_SPEC (SerializerError);
	  SYMTAB_EXPORT void serialize_aggregate(SerializerBase *, const char * = "Aggregate") THROW_SPEC (SerializerError);
};

}
}

#endif
