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

#include <iosfwd>
#include <stddef.h>
#include <string>
#include <utility>
#include <vector>
#include "Annotatable.h"
#include "Aggregate.h"
#include "Variable.h"
#include "VariableLocation.h"

SYMTAB_EXPORT std::ostream &operator<<(std::ostream &os, const Dyninst::SymtabAPI::Function &);

namespace Dyninst{
namespace SymtabAPI{

class Symbol;
class Type;
class FunctionBase;
class DwarfWalker;

class SYMTAB_EXPORT FuncRange {
  public:
   FuncRange(Dyninst::Offset off_, size_t size_, FunctionBase *cont_) :
     container(cont_),
     off(off_),
     size(size_)
   {
   }

   FunctionBase *container;
   Dyninst::Offset off;
   unsigned long size;

   //For interval tree
   Dyninst::Offset low() const { return off; }
   Dyninst::Offset high() const { return off + size; }
   typedef Dyninst::Offset type;
};

typedef std::vector<FuncRange> FuncRangeCollection;
typedef std::vector<FunctionBase *> InlineCollection;

class SYMTAB_EXPORT FunctionBase
{
   friend class InlinedFunction;
   friend class Function;
   friend class DwarfWalker;
  public:
   /***** Return Type Information *****/
   boost::shared_ptr<Type> getReturnType(Type::do_share_t) const;
   Type* getReturnType() const {
     return getReturnType(Type::share).get();
   }

   /***** Local Variable Information *****/
   bool findLocalVariable(std::vector<localVar *>&vars, std::string name);
   bool getLocalVariables(std::vector<localVar *>&vars);
   bool getParams(std::vector<localVar *>&params);

   bool operator==(const FunctionBase &);

   FunctionBase *getInlinedParent();
   const InlineCollection &getInlines();

   const FuncRangeCollection &getRanges();

   /***** Frame Pointer Information *****/
   bool setFramePtr(std::vector<VariableLocation> *locs);
   std::vector<VariableLocation> &getFramePtrRefForInit();
   std::vector<VariableLocation> &getFramePtr();
   dyn_mutex &getFramePtrLock();

   /***** Primary name *****/
   virtual std::string getName() const = 0;
   virtual bool addMangledName(std::string name, bool isPrimary, bool isDebug=false) = 0;
   virtual bool addPrettyName(std::string name, bool isPrimary, bool isDebug=false) = 0;

   /***** Opaque data object pointers, usable by user ****/
   void *getData();
   void setData(void *d);

   /* internal helper functions */
   bool addLocalVar(localVar *);
   bool addParam(localVar *);
   bool	setReturnType(boost::shared_ptr<Type>);

   virtual Offset getOffset() const = 0;
   virtual unsigned getSize() const = 0;
   virtual Module* getModule() const = 0;

   virtual ~FunctionBase();

  protected:
   FunctionBase();

   localVarCollection *locals;
   localVarCollection *params;

   mutable unsigned functionSize_;
   boost::shared_ptr<Type>          retType_;

   dyn_mutex inlines_lock;
   InlineCollection inlines;
   FunctionBase *inline_parent;

   FuncRangeCollection ranges;
   std::vector<VariableLocation> frameBase_;
   dyn_mutex frameBaseLock_;
   bool frameBaseExpanded_;
   void *data;
   void expandLocation(const VariableLocation &loc,
                       std::vector<VariableLocation> &ret);
};

/*
 *  `Function` can be derived from (e.g., ParseAPI::PLTFunction), but does not create an
 *  interface separate from FunctionBase.
 */
 class SYMTAB_EXPORT Function : public FunctionBase, public Aggregate
{
	friend std::ostream &::operator<<(std::ostream &os, const Dyninst::SymtabAPI::Function &);

 public:

   Function();
   Function(Symbol *sym);
   virtual ~Function();

   /* Symbol management */
   bool removeSymbol(Symbol *sym) override;

   /***** IA64-Specific Frame Pointer Information *****/
   bool  setFramePtrRegnum(int regnum);
   int   getFramePtrRegnum() const;

   /***** PPC64 Linux Specific Information *****/
   Offset getPtrOffset() const;
   Offset getTOCOffset() const;

   unsigned getSymbolSize() const;
   unsigned getSize() const override;
   std::string getName() const override;
   Offset getOffset() const override { return Aggregate::getOffset(); }
   bool addMangledName(std::string name, bool isPrimary, bool isDebug=false) override
   {return Aggregate::addMangledName(name, isPrimary, isDebug);}
   bool addPrettyName(std::string name, bool isPrimary, bool isDebug=false) override
   {return Aggregate::addPrettyName(name, isPrimary, isDebug);}

     Module * getModule() const override;
 };

class SYMTAB_EXPORT InlinedFunction : public FunctionBase
{
   friend class Symtab;
   friend class DwarfWalker;
   friend class Object;
  public:
   InlinedFunction(FunctionBase *parent);
   virtual ~InlinedFunction();

   Module* getModule() const override { return module_; }
   typedef std::vector<std::string>::const_iterator name_iter;
   std::pair<std::string, Dyninst::Offset> getCallsite();
   bool removeSymbol(Symbol *sym);
   bool addMangledName(std::string name, bool isPrimary, bool isDebug=false) override;
   bool addPrettyName(std::string name, bool isPrimary, bool isDebug=false) override;
   std::string getName() const override;
   Offset getOffset() const override;
   unsigned getSize() const override;
    void setFile(std::string filename);
  private:
   size_t callsite_file_number;
   Dyninst::Offset callsite_line;
   std::string name_;
   Module* module_;
   Dyninst::Offset offset_;
};

}
}

#endif
