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

class Aggregate : public AnnotatableSparse 
{
   public:
      DLLEXPORT Aggregate();
      
      virtual ~Aggregate() {};

      DLLEXPORT Offset   getAddress() const;
      DLLEXPORT Module * getModule() const;

      /***** Symbol Collection Management *****/
      DLLEXPORT bool addSymbol(Symbol *sym);
      DLLEXPORT bool removeSymbol(Symbol *sym);
      DLLEXPORT bool getAllSymbols(std::vector<Symbol *>&syms) const;
      DLLEXPORT Symbol * getFirstSymbol() const;

      /***** Symbol naming *****/
      DLLEXPORT const vector<std::string> &getAllMangledNames();
      DLLEXPORT const vector<std::string> &getAllPrettyNames();
      DLLEXPORT const vector<std::string> &getAllTypedNames();

   protected:

      bool addMangledNameInt(string name, bool isPrimary);
      bool addPrettyNameInt(string name, bool isPrimary);
      bool addTypedNameInt(string name, bool isPrimary);


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
