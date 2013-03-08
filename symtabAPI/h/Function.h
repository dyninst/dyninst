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
class FunctionBase;
class DwarfWalker;

struct FuncRange {
   Dyninst::Offset off;
   unsigned long size;
};

typedef std::vector<FuncRange> FuncRangeCollection;
typedef std::vector<FunctionBase *> InlineCollection;
typedef std::vector<FuncRange> FuncRangeCollection;

class FunctionBase : public Aggregate
{
   friend class InlinedFunction;
   friend class Function;
  public:
   /***** Return Type Information *****/
   SYMTAB_EXPORT Type  * getReturnType() const;
   
   /***** Local Variable Information *****/
   SYMTAB_EXPORT bool findLocalVariable(std::vector<localVar *>&vars, std::string name);
   SYMTAB_EXPORT bool getLocalVariables(std::vector<localVar *>&vars);
   SYMTAB_EXPORT bool getParams(std::vector<localVar *>&params);

   SYMTAB_EXPORT bool operator==(const FunctionBase &);

   SYMTAB_EXPORT FunctionBase *getInlinedParent();
   SYMTAB_EXPORT const InlineCollection &getInlines();
   
   SYMTAB_EXPORT const FuncRangeCollection &getRanges();
   
   /***** Frame Pointer Information *****/
   SYMTAB_EXPORT bool setFramePtr(std::vector<VariableLocation> *locs);
   SYMTAB_EXPORT std::vector<VariableLocation> &getFramePtrRefForInit();
   SYMTAB_EXPORT std::vector<VariableLocation> &getFramePtr();   

   /***** Opaque data object pointers, usable by user ****/
   void *getData();
   void setData(void *d);

   /* internal helper functions */
   bool addLocalVar(localVar *);
   bool addParam(localVar *);
   SYMTAB_EXPORT bool	setReturnType(Type *);

  protected:
   SYMTAB_EXPORT FunctionBase(Symbol *);
   SYMTAB_EXPORT FunctionBase(Module *);
   SYMTAB_EXPORT FunctionBase();
   SYMTAB_EXPORT ~FunctionBase();

   localVarCollection *locals;
   localVarCollection *params;

   Type          *retType_;
   unsigned functionSize_;

   InlineCollection inlines;
   FunctionBase *inline_parent;

   FuncRangeCollection ranges;
   std::vector<VariableLocation> frameBase_;
   bool frameBaseExpanded_;
   void *data;

   void expandLocation(const VariableLocation &loc,
                       std::vector<VariableLocation> &ret);
};

class Function : public FunctionBase
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
   
   /***** IA64-Specific Frame Pointer Information *****/
   SYMTAB_EXPORT bool  setFramePtrRegnum(int regnum);
   SYMTAB_EXPORT int   getFramePtrRegnum() const;
   
   /***** PPC64 Linux Specific Information *****/
   SYMTAB_EXPORT Offset getPtrOffset() const;
   SYMTAB_EXPORT Offset getTOCOffset() const;
   
   SYMTAB_EXPORT Serializable * serialize_impl(SerializerBase *sb, 
                                const char *tag = "Function") THROW_SPEC (SerializerError);

   SYMTAB_EXPORT unsigned getSize();
};

class InlinedFunction : public FunctionBase
{
   friend class Symtab;
   friend class DwarfWalker;
  protected:
   InlinedFunction(FunctionBase *parent);
   ~InlinedFunction();
  public:
   SYMTAB_EXPORT std::pair<std::string, Dyninst::Offset> getCallsite();
   SYMTAB_EXPORT virtual bool removeSymbol(Symbol *sym);
  private:
   std::string callsite_file;
   Dyninst::Offset callsite_line;
};

}
}

#endif
