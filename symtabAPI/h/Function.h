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
 * Symbol.h: symbol table objects.
************************************************************************/

#if !defined(_Function_h_)
#define _Function_h_

#include "Annotatable.h"
#include "Serialization.h"
#include "Aggregate.h"
#include "Variable.h"

SYMTAB_EXPORT std::ostream &operator<<(std::ostream &os, const Dyninst::SymtabAPI::Function &);

namespace Dyninst{
namespace SymtabAPI{

class Symbol;
class Type;


class Function : public Aggregate, public Serializable, public AnnotatableSparse
{
    friend class Symtab;
	friend std::ostream &::operator<<(std::ostream &os, const Dyninst::SymtabAPI::Function &);
   
 private:
   SYMTAB_EXPORT Function(Symbol *sym);
   
 public:
   
   SYMTAB_EXPORT Function();
   SYMTAB_EXPORT virtual ~Function();
   
   /* Symbol management */
   SYMTAB_EXPORT bool removeSymbol(Symbol *sym);      
   
   /***** Return Type Information *****/
   SYMTAB_EXPORT Type  * getReturnType() const;
   SYMTAB_EXPORT bool	setReturnType(Type *);
   
   /***** IA64-Specific Frame Pointer Information *****/
   SYMTAB_EXPORT bool  setFramePtrRegnum(int regnum);
   SYMTAB_EXPORT int   getFramePtrRegnum() const;
   
   /***** PPC64 Linux Specific Information *****/
   SYMTAB_EXPORT Offset getPtrOffset() const;
   SYMTAB_EXPORT Offset getTOCOffset() const;
   
   /***** Frame Pointer Information *****/
   SYMTAB_EXPORT bool  setFramePtr(std::vector<VariableLocation> *locs);
   SYMTAB_EXPORT std::vector<VariableLocation> &getFramePtr();
   
   /***** Local Variable Information *****/
   SYMTAB_EXPORT bool findLocalVariable(std::vector<localVar *>&vars, std::string name);
   SYMTAB_EXPORT bool getLocalVariables(std::vector<localVar *>&vars);
   SYMTAB_EXPORT bool getParams(std::vector<localVar *>&params);
   
   SYMTAB_EXPORT Serializable * serialize_impl(SerializerBase *sb, 
                                const char *tag = "Function") THROW_SPEC (SerializerError);

   SYMTAB_EXPORT unsigned getSize();
   
   SYMTAB_EXPORT bool operator==(const Function &);
   /* internal helper functions */
   bool addLocalVar(localVar *);
   bool addParam(localVar *);
   bool setupParams();
 private:
   
   Type          *retType_;
   int           framePtrRegNum_;
   std::vector<VariableLocation> *locs_;
   unsigned functionSize_;
};


}
}

#endif
