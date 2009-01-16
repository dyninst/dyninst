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

// $Id: Object.C,v 1.31 2008/11/03 15:19:25 jaw Exp $

#include "symutil.h"
#include "Annotatable.h"
#include "common/h/serialize.h"

#include "Symtab.h"
#include "Module.h"
#include "Collections.h"
#include "annotations.h"
#include "Symbol.h"

#include "symtabAPI/src/Object.h"

#include <iostream>

using namespace std;
using namespace Dyninst;
using namespace Dyninst::SymtabAPI;

string Symbol::emptyString("");

const char *Dyninst::SymtabAPI::supportedLanguages2Str(supportedLanguages s)
{
   switch(s) {
      CASE_RETURN_STR(lang_Unknown);
      CASE_RETURN_STR(lang_Assembly);
      CASE_RETURN_STR(lang_C);
      CASE_RETURN_STR(lang_CPlusPlus);
      CASE_RETURN_STR(lang_GnuCPlusPlus);
      CASE_RETURN_STR(lang_Fortran);
      CASE_RETURN_STR(lang_Fortran_with_pretty_debug);
      CASE_RETURN_STR(lang_CMFortran);
   };
   return "bad_language";
}


bool Dyninst::SymtabAPI::symbol_compare(const Symbol *s1, const Symbol *s2) 
{
    // select the symbol with the lowest address
    Offset s1_addr = s1->getAddr();
    Offset s2_addr = s2->getAddr();
    if (s1_addr > s2_addr)
    	return false;
    if (s1_addr < s2_addr)
    	return true;

    // symbols are co-located at the same address
    // select the symbol which is not a function
    if ((s1->getType() != Symbol::ST_FUNCTION) && (s2->getType() == Symbol::ST_FUNCTION))
    	return true;
    if ((s2->getType() != Symbol::ST_FUNCTION) && (s1->getType() == Symbol::ST_FUNCTION))
    	return false;
    
    // symbols are both functions
    // select the symbol which has GLOBAL linkage
    if ((s1->getLinkage() == Symbol::SL_GLOBAL) && (s2->getLinkage() != Symbol::SL_GLOBAL))
    	return true;
    if ((s2->getLinkage() == Symbol::SL_GLOBAL) && (s1->getLinkage() != Symbol::SL_GLOBAL))
    	return false;
	
    // neither function is GLOBAL
    // select the symbol which has LOCAL linkage
    if ((s1->getLinkage() == Symbol::SL_LOCAL) && (s2->getLinkage() != Symbol::SL_LOCAL))
    	return true;
    if ((s2->getLinkage() == Symbol::SL_LOCAL) && (s1->getLinkage() != Symbol::SL_LOCAL))
    	return false;
    
    // both functions are WEAK
    
    // Apparently sort requires a strict weak ordering
    // and fails for equality. our compare
    // function behaviour should be as follows
    // f(x,y) => !f(y,x)
    // f(x,y),f(y,z) => f(x,z)
    // f(x,x) = false. 
    // So return which ever is first in the array. May be that would help.
    return (s1 < s2);
}


bool AObject::needs_function_binding() const 
{
    return false;
}

bool AObject::get_func_binding_table(std::vector<relocationEntry> &) const 
{
    return false;
}

bool AObject::get_func_binding_table_ptr(const std::vector<relocationEntry> *&) const 
{
    return false;
}

bool AObject::addRelocationEntry(relocationEntry &)
{
    return true;
}

char *AObject::mem_image() const
{
	return NULL;
}

SYMTAB_EXPORT ExceptionBlock::~ExceptionBlock() 
{
}

SYMTAB_EXPORT ExceptionBlock::ExceptionBlock() : tryStart_(0), trySize_(0), 
								catchStart_(0), hasTry_(false) 
{
}

SYMTAB_EXPORT Offset ExceptionBlock::catchStart() const 
{
	return catchStart_;
}

SYMTAB_EXPORT relocationEntry::relocationEntry(const relocationEntry& ra) : 
   Serializable(),
   target_addr_(ra.target_addr_), 
   rel_addr_(ra.rel_addr_), 
   addend_ (ra.addend_),
   rtype_ (ra.rtype_),
   name_(ra.name_), 
   dynref_(ra.dynref_), relType_(ra.relType_) 
{
}

SYMTAB_EXPORT Offset relocationEntry::target_addr() const 
{
	return target_addr_;
}

SYMTAB_EXPORT Offset relocationEntry::rel_addr() const 
{
	return rel_addr_;
}

SYMTAB_EXPORT const string &relocationEntry::name() const 
{
	return name_;
}

SYMTAB_EXPORT Symbol *relocationEntry::getDynSym() const 
{
    return dynref_;
}

SYMTAB_EXPORT bool relocationEntry::addDynSym(Symbol *dynref) 
{
    dynref_ = dynref;
    return true;
}

SYMTAB_EXPORT unsigned long relocationEntry::getRelType() const 
{
    return relType_;
}

SYMTAB_EXPORT Symbol::~Symbol ()
{
   std::string *sfa_p = NULL;

   if (getAnnotation(sfa_p, SymbolFileNameAnno))
   {
      if (!sfa_p) 
      {
         fprintf(stderr, "%s[%d]:  inconsistency here??\n", FILE__, __LINE__);
      }
      else
      {
         delete (sfa_p);
      }
   }
}

SYMTAB_EXPORT Symbol::Symbol(const Symbol& s) :
   Serializable(),
   AnnotatableSparse(),
   module_(s.module_), 
   type_(s.type_), linkage_(s.linkage_),
   addr_(s.addr_), sec_(s.sec_), size_(s.size_), 
   isInDynsymtab_(s.isInDynsymtab_), isInSymtab_(s.isInSymtab_), 
   isAbsolute_(s.isAbsolute_),
   function_(s.function_),
   variable_(s.variable_),
   mangledName_(s.mangledName_), 
   prettyName_(s.prettyName_), 
   typedName_(s.typedName_), 
   tag_(s.tag_), 
   framePtrRegNum_(s.framePtrRegNum_),
   retType_(s.retType_), 
   moduleName_(s.moduleName_) 
{
#if 0
   Annotatable <std::string, symbol_file_name_a> &sfa = *this;
   const Annotatable <std::string, symbol_file_name_a> &sfa_src = s;
   if (sfa_src.size())
      sfa.addAnnotation(sfa_src[0]);

   Annotatable <std::vector<std::string>, symbol_version_names_a> &sva = *this;
   const Annotatable <std::vector<std::string>, symbol_version_names_a> &sva_src = s;
   if (sva_src.size())
      sva.addAnnotation(sva_src[0]);

   Annotatable<localVarCollection, symbol_variables_a, true> &lvA = *this;
   const Annotatable<localVarCollection, symbol_variables_a, true> &lvA_src = s;
   if (lvA_src.size())
      lvA.addAnnotation(lvA_src[0]);

   Annotatable<localVarCollection, symbol_parameters_a, true> &pA = *this;
   const Annotatable<localVarCollection, symbol_parameters_a, true> &pA_src = s;
   if (pA_src.size())
      pA.addAnnotation(pA_src[0]);
   fprintf(stderr, "%s[%d]:  FIXME??  copy annotations here or not??\n", FILE__, __LINE__);
#endif

   std::string *sfa_p = NULL;
   if (s.getAnnotation(sfa_p, SymbolFileNameAnno))
   {
      if (!sfa_p) 
      {
         fprintf(stderr, "%s[%d]:  inconsistency here??\n", FILE__, __LINE__);
      }
      else
      {
         std::string *sfa_p2 = new std::string(*sfa_p);

         if (!addAnnotation(sfa_p2, SymbolFileNameAnno)) 
         {
            fprintf(stderr, "%s[%d]:  failed ot addAnnotation here\n", FILE__, __LINE__);
         }
      }
   }

   std::vector<std::string> *svn_p = NULL;
   if (s.getAnnotation(svn_p, SymbolVersionNamesAnno))
   {
      if (!svn_p) 
      {
         fprintf(stderr, "%s[%d]:  inconsistency here??\n", FILE__, __LINE__);
      }
      else
      {
         //  note:  in an older version we just copied one element from this
         // vector when the symbol got copied.  I think this is incorrect.
         //fprintf(stderr, "%s[%d]:  alloc'ing new vector for symbol versions\n", FILE__, __LINE__);

         //  if we alloc here, probably want to check in dtor to make
         //  sure that we are deleting this if it exists.
         //std::vector<std::string> *svn_p2 = new std::vector<std::string>();

         //for (unsigned int i = 0; i < svn_p->size(); ++i)
         //{
         //   svn_p2->push_back(std::string((*svn_p)[i]));
         //}

         if (!addAnnotation(svn_p, SymbolVersionNamesAnno))
         {
            fprintf(stderr, "%s[%d]:  failed ot addAnnotation here\n", FILE__, __LINE__);
         }
      }
   }

   localVarCollection *vars_p = NULL;
   if (s.getAnnotation(vars_p, FunctionLocalVariablesAnno))
   {
      if (!vars_p) 
      {
         fprintf(stderr, "%s[%d]:  inconsistency here??\n", FILE__, __LINE__);
      }
      else
      {
         if (!addAnnotation(vars_p, FunctionLocalVariablesAnno))
         {
            fprintf(stderr, "%s[%d]:  failed ot addAnnotation here\n", FILE__, __LINE__);
         }
      }
   }

   localVarCollection *params_p = NULL;
   if (s.getAnnotation(params_p, FunctionParametersAnno))
   {
      if (!params_p) 
      {
         fprintf(stderr, "%s[%d]:  inconsistency here??\n", FILE__, __LINE__);
      }
      else
      {
         if (!addAnnotation(params_p, FunctionParametersAnno))
         {
            fprintf(stderr, "%s[%d]:  failed ot addAnnotation here\n", FILE__, __LINE__);
         }
      }
   }
}

SYMTAB_EXPORT Symbol& Symbol::operator=(const Symbol& s) 
{
   module_  = s.module_;
   type_    = s.type_;
   linkage_ = s.linkage_;
   addr_    = s.addr_;
   sec_     = s.sec_;
   size_    = s.size_;
   isInDynsymtab_ = s.isInDynsymtab_;
   isInSymtab_ = s.isInSymtab_;
   isAbsolute_ = s.isAbsolute_;
   function_ = s.function_;
   variable_ = s.variable_;
   tag_     = s.tag_;
   mangledName_ = s.mangledName_;
   prettyName_ = s.prettyName_;
   typedName_ = s.typedName_;
   framePtrRegNum_ = s.framePtrRegNum_;

   std::string *sfa_p = NULL;

   if (s.getAnnotation(sfa_p, SymbolFileNameAnno))
   {
      if (!sfa_p) 
      {
         fprintf(stderr, "%s[%d]:  inconsistency here??\n", FILE__, __LINE__);
      }
      else
      {
         std::string *sfa_p2 = new std::string(*sfa_p);

         if (!addAnnotation(sfa_p2, SymbolFileNameAnno))
         {
            fprintf(stderr, "%s[%d]:  failed ot addAnnotation here\n", FILE__, __LINE__);
         }
      }
   }

   std::vector<std::string> *svn_p = NULL;
   if (s.getAnnotation(svn_p, SymbolVersionNamesAnno))
   {
      if (!svn_p) 
      {
         fprintf(stderr, "%s[%d]:  inconsistency here??\n", FILE__, __LINE__);
      }
      else
      {
         //  note:  in an older version we just copied one element from this
         // vector when the symbol got copied.  I think this is incorrect.
         //fprintf(stderr, "%s[%d]:  alloc'ing new vector for symbol versions\n", FILE__, __LINE__);

         //  if we alloc here, probably want to check in dtor to make
         //  sure that we are deleting this if it exists.
         //std::vector<std::string> *svn_p2 = new std::vector<std::string>();

         //for (unsigned int i = 0; i < svn_p->size(); ++i)
         //{
         //   svn_p2->push_back(std::string((*svn_p)[i]));
         //}

         if (!addAnnotation(svn_p, SymbolVersionNamesAnno))
         {
            fprintf(stderr, "%s[%d]:  failed ot addAnnotation here\n", FILE__, __LINE__);
         }
      }
   }

   localVarCollection *vars_p = NULL;
   if (s.getAnnotation(vars_p, FunctionLocalVariablesAnno))
   {
      if (!vars_p) 
      {
         fprintf(stderr, "%s[%d]:  inconsistency here??\n", FILE__, __LINE__);
      }
      else
      {
         if (!addAnnotation(vars_p, FunctionLocalVariablesAnno))
         {
            fprintf(stderr, "%s[%d]:  failed ot addAnnotation here\n", FILE__, __LINE__);
         }
      }
   }

   localVarCollection *params_p = NULL;
   if (s.getAnnotation(params_p, FunctionParametersAnno))
   {
      if (!params_p) 
      {
         fprintf(stderr, "%s[%d]:  inconsistency here??\n", FILE__, __LINE__);
      }
      else
      {
         if (!addAnnotation(params_p, FunctionParametersAnno))
         {
            fprintf(stderr, "%s[%d]:  failed ot addAnnotation here\n", FILE__, __LINE__);
         }
      }
   }
#if 0
#if 1 
   fprintf(stderr, "%s[%d]:  WARNING:  assignment ctor not assigning local variables and parameters\n", FILE__, __LINE__);
#else
   Annotatable <std::string, symbol_file_name_a> &sfa = *this;
   const Annotatable <std::string, symbol_file_name_a> &sfa_src = s;
   if (sfa_src.size())
      sfa.addAnnotation(sfa_src[0]);

   Annotatable <std::vector<std::string>, symbol_version_names_a> &sva = *this;
   const Annotatable <std::vector<std::string>, symbol_version_names_a> &sva_src = s;
   if (sva_src.size())
      sva.addAnnotation(sva_src[0]);

   Annotatable<localVarCollection, symbol_variables_a, true> &lvA = *this;
   const Annotatable<localVarCollection, symbol_variables_a,true> &lvA_src = s;
   if (lvA_src.size())
      lvA.addAnnotation(lvA_src[0]);

   Annotatable<localVarCollection, symbol_parameters_a, true> &pA = *this;
   const Annotatable<localVarCollection, symbol_parameters_a,true> &pA_src = s;
   if (pA_src.size())
      pA.addAnnotation(pA_src[0]);
#endif
#endif
   return *this;
}

SYMTAB_EXPORT const string& Symbol::getMangledName() const 
{
    return mangledName_;
}

SYMTAB_EXPORT const string& Symbol::getPrettyName() const 
{
    return prettyName_;
}

SYMTAB_EXPORT const string& Symbol::getTypedName() const 
{
    return typedName_;
}

SYMTAB_EXPORT const string& Symbol::getModuleName() const 
{
    if (module_)
        return module_->fullName();
    else
        return moduleName_;
}

SYMTAB_EXPORT Module* Symbol::getModule() const 
{
    return module_;
}

SYMTAB_EXPORT bool Symbol::setModule(Module *mod) 
{
	module_ = mod; 
	return true;
}

SYMTAB_EXPORT Symbol::SymbolType Symbol::getType() const 
{
    return type_;
}

SYMTAB_EXPORT Symbol::SymbolLinkage Symbol::getLinkage() const 
{
    return linkage_;
}

SYMTAB_EXPORT Offset Symbol::getAddr() const 
{
    return addr_;
}

SYMTAB_EXPORT Region *Symbol::getSec() const 
{
    return sec_;
}

SYMTAB_EXPORT bool Symbol::isInDynSymtab() const 
{
    return isInDynsymtab_;
}

SYMTAB_EXPORT bool Symbol::isInSymtab() const 
{
    return isInSymtab_;
}

SYMTAB_EXPORT bool Symbol::isAbsolute() const
{
    return isAbsolute_;
}

SYMTAB_EXPORT bool Symbol::isFunction() const
{
    return (function_ != NULL);
}

SYMTAB_EXPORT bool Symbol::setFunction(Function *func)
{
    function_ = func;
    return true;
}

SYMTAB_EXPORT Function * Symbol::getFunction() const
{
    return function_;
}

SYMTAB_EXPORT bool Symbol::isVariable() const 
{
    return variable_ != NULL;
}

SYMTAB_EXPORT bool Symbol::setVariable(Variable *var) 
{
    variable_ = var;
    return true;
}

SYMTAB_EXPORT Variable * Symbol::getVariable() const
{
    return variable_;
}

SYMTAB_EXPORT unsigned Symbol::getSize() const 
{
    return size_;
}

SYMTAB_EXPORT bool	Symbol::setSize(unsigned ns)
{
	size_ = ns;
	return true;
}

SYMTAB_EXPORT Symbol::SymbolTag Symbol::tag() const 
{
    return tag_;
}

SYMTAB_EXPORT bool Symbol::setModuleName(string module)
{
	moduleName_ = module;
	return true;
}

SYMTAB_EXPORT bool Symbol::setAddr (Offset newAddr) 
{
      addr_ = newAddr;
      return true;
}

SYMTAB_EXPORT bool Symbol::setDynSymtab() 
{
    isInDynsymtab_= true;
    return true;
}

SYMTAB_EXPORT bool Symbol::clearDynSymtab() 
{
    isInDynsymtab_ = false;
    return true;
}

SYMTAB_EXPORT bool Symbol::setIsInSymtab() 
{
    isInSymtab_= true;
    return true;
}

SYMTAB_EXPORT bool Symbol::clearIsInSymtab() 
{
    isInSymtab_= false;
    return true;
}

SYMTAB_EXPORT bool Symbol::setIsAbsolute()
{
    isAbsolute_= true;
    return true;
}

SYMTAB_EXPORT bool Symbol::clearIsAbsolute()
{
    isAbsolute_= false;
    return true;
}

SYMTAB_EXPORT Symbol::Symbol()
   : //name_("*bad-symbol*"), module_("*bad-module*"),
    module_(NULL), type_(ST_UNKNOWN), linkage_(SL_UNKNOWN), addr_(0), sec_(NULL), size_(0),
    isInDynsymtab_(false), isInSymtab_(true), isAbsolute_(false), 
    function_(NULL),
    variable_(NULL),
    tag_(TAG_UNKNOWN),
    framePtrRegNum_(-1), retType_(NULL), moduleName_("")
{
   // note: this ctor is called surprisingly often (when we have
   // vectors of Symbols and/or dictionaries of Symbols).  So, make it fast.
}

SYMTAB_EXPORT Symbol::Symbol(const string iname, const string imodule,
    SymbolType itype, SymbolLinkage ilinkage, Offset iaddr,
    Region *isec, unsigned size,  bool isInDynSymtab, bool isInSymtab,
    bool isAbsolute)
    : type_(itype),
    linkage_(ilinkage), addr_(iaddr), sec_(isec), size_(size), isInDynsymtab_(isInDynSymtab),
    isInSymtab_(isInSymtab), isAbsolute_(isAbsolute), tag_(TAG_UNKNOWN), framePtrRegNum_(-1),
    retType_(NULL)
{
        module_ = NULL;
    	moduleName_ = imodule;
        mangledName_ = iname;
}

SYMTAB_EXPORT Symbol::Symbol(const string iname, Module *mod,
    SymbolType itype, SymbolLinkage ilinkage, Offset iaddr,
    Region *isec, unsigned size,  bool isInDynSymtab, bool isInSymtab,
    bool isAbsolute)
    : module_(mod), type_(itype),
    linkage_(ilinkage), addr_(iaddr), sec_(isec), size_(size),  isInDynsymtab_(isInDynSymtab), 
    isInSymtab_(isInSymtab), isAbsolute_(isAbsolute), tag_(TAG_UNKNOWN), framePtrRegNum_(-1),
    retType_(NULL)
{
    mangledName_ = iname;
}


SYMTAB_EXPORT bool Symbol::setSymbolType(SymbolType sType)
{
    if ((sType != ST_UNKNOWN)&&
        (sType != ST_FUNCTION)&&
        (sType != ST_OBJECT)&&
        (sType != ST_MODULE)&&
        (sType != ST_NOTYPE))
        return false;
    
    SymbolType oldType = type_;	
    type_ = sType;
    if (module_->exec())
        module_->exec()->changeType(this, oldType);
    
    return true;
}

SYMTAB_EXPORT bool Symbol::setVersionFileName(std::string &fileName)
{
   std::string *fn_p = NULL;
   if (getAnnotation(fn_p, SymbolFileNameAnno)) 
   {
      if (!fn_p) 
      {
         fprintf(stderr, "%s[%d]:  inconsistency here\n", FILE__, __LINE__);
      }
      else
      {
         fprintf(stderr, "%s[%d]:  WARNING, already have filename set for symbol %s\n", 
                 FILE__, __LINE__, getMangledName().c_str());
      }
      return false;
   }
   else
   {
      //  not sure if we need to copy here or not, so let's do it...
      std::string *fn = new std::string(fileName);
      if (!addAnnotation(fn, SymbolFileNameAnno)) 
      {
         fprintf(stderr, "%s[%d]:  failed to add anno here\n", FILE__, __LINE__);
         return false;
      }
      return true;
   }

   return false;
}

SYMTAB_EXPORT bool Symbol::setVersions(std::vector<std::string> &vers)
{
   std::vector<std::string> *vn_p = NULL;
   if (getAnnotation(vn_p, SymbolVersionNamesAnno)) 
   {
      if (!vn_p) 
      {
         fprintf(stderr, "%s[%d]:  inconsistency here\n", FILE__, __LINE__);
      }
      else
         fprintf(stderr, "%s[%d]:  WARNING, already have versions set for symbol %s\n", FILE__, __LINE__, getMangledName().c_str());
      return false;
   }
   else
   {
      if (!addAnnotation(&vers, SymbolVersionNamesAnno)) 
      {
         fprintf(stderr, "%s[%d]:  failed to add anno here\n", FILE__, __LINE__);
      }
   }

   return true;
}

SYMTAB_EXPORT bool Symbol::getVersionFileName(std::string &fileName)
{
   std::string *fn_p = NULL;

   if (getAnnotation(fn_p, SymbolFileNameAnno)) 
   {
      if (!fn_p) 
      {
         fprintf(stderr, "%s[%d]:  inconsistency here\n", FILE__, __LINE__);
      }
      else
         fileName = *fn_p;

      return true;
   }

   return false;

#if 0
   Annotatable<std::string, symbol_file_name_a> &fn = *this;
   if (!fn.size()) {
      return false;
   }
   fileName = fn[0];
   return true;
#endif
}

SYMTAB_EXPORT bool Symbol::getVersions(std::vector<std::string> *&vers)
{
   std::vector<std::string> *vn_p = NULL;

   if (getAnnotation(vn_p, SymbolVersionNamesAnno)) 
   {
      if (!vn_p) 
      {
         fprintf(stderr, "%s[%d]:  inconsistency here\n", FILE__, __LINE__);
      }
      else
      {
         vers = vn_p;
         return true;
      } 
   }

   return false;

#if 0
   Annotatable<std::vector<std::string>, symbol_version_names_a> &sv = *this;
   if (!sv.size()) {
      return false;
   }
   vers = &(sv[0]);
   return true;
#endif
}

void Symbol::serialize(SerializerBase *s, const char *tag) 
{
   try {
      ifxml_start_element(s, tag);
      gtranslate(s, type_, symbolType2Str, "type");
      gtranslate(s, linkage_, symbolLinkage2Str, "linkage");
      gtranslate(s, tag_, symbolTag2Str, "tag");
      gtranslate(s, addr_, "addr");
      gtranslate(s, size_, "size");
      gtranslate(s, isInDynsymtab_, "isInDynsymtab");
      gtranslate(s, isInSymtab_, "isInSymtab");
      gtranslate(s, isAbsolute_, "isAbsolute");
      gtranslate(s, prettyName_, "prettyName", "prettyName");
      gtranslate(s, mangledName_, "mangledName", "mangledName");
      gtranslate(s, typedName_, "typedName", "typedName");
      gtranslate(s, framePtrRegNum_, "framePtrRegNum");
      //  Note:  have to deal with retType_ here?? Probably use type id.
      gtranslate(s, moduleName_, "moduleName");
      ifxml_end_element(s, "Symbol");
#if 0
      symbol_start(param);
      translate(param.type_, "type");
      translate(param.linkage_, "linkage");
      translate(param.tag_, "tag");
      getSD().translate(param.addr_, "addr");
      getSD().translate(param.size_, "size");
      getSD().translate(param.isInDynsymtab_, "isInDynsymtab");
      getSD().translate(param.isInSymtab_, "isInSymtab");
      getSD().translate(param.prettyName_, "prettyName", "prettyName");
      getSD().translate(param.mangledName_, "mangledName", "mangledName");
      getSD().translate(param.typedName_, "typedName", "typedName");
      getSD().translate(param.framePtrRegNum_, "framePtrRegNum");
      //  Note:  have to deal with retType_ here?? Probably use type id.
      getSD().translate(param.moduleName_, "moduleName");
      symbol_end(param);
#endif
   } SER_CATCH("Symbol");
}

ostream& Dyninst::SymtabAPI::operator<< (ostream &os, const Symbol &s) 
{
   return os << "{"
      << " mangled=" << s.getMangledName()
      << " pretty="  << s.getPrettyName()
      << " module="  << s.module_
      //<< " type="    << (unsigned) s.type_
      << " type="    << s.symbolType2Str(s.type_)
      //<< " linkage=" << (unsigned) s.linkage_
      << " linkage=" << s.symbolLinkage2Str(s.linkage_)
      << " addr=0x"    << hex << s.addr_ << dec
      //<< " tag="     << (unsigned) s.tag_
      << " tag="     << s.symbolTag2Str(s.tag_)
      << " isAbs="   << s.isAbsolute_
      << (s.function_!=NULL ? " [FUNC]" : "")
      << (s.isInSymtab_ ? " [STA]" : "")
      << (s.isInDynsymtab_ ? " [DYN]" : "")
      << " }" << endl;
}

#ifdef DEBUG 

ostream & relocationEntry::operator<< (ostream &s) const {
   s << "target_addr_ = " << target_addr_ << endl;
   s << "rel_addr_ = " << rel_addr_ << endl;
   s << "addend_ = " << addend_ << endl;
   s << "rtype_ = " << rtype_ << endl;
   s << "name_ = " << name_ << endl;
   return s; 
}

ostream &operator<<(ostream &os, relocationEntry &q) {
   return q.operator<<(os);
}

/**************************************************
 *
 *  Stream based debuggering output - for regreesion testing.
 *  Dump info on state of object *this....
 *
 **************************************************/


const ostream &AObject::dump_state_info(ostream &s) {

   // key and value for distc hash iter.... 
   string str;
   Symbol sym;
   hash_map<string, std::vector <Symbol> >::iterator symbols_iter = symbols_.begin();

   s << "Debugging Info for AObject (address) : " << this << endl;

   s << " file_ = " << file_ << endl;
   s << " symbols_ = " << endl;

   // and loop over all the symbols, printing symbol name and data....
   //  or try at least....
   for(;symbols_iter!=symbols_.end();symbols_iter++)
   {
      str = symbols_iter->first
         for(int i = 0; i<symbols_iter->second->size(); i++)
         {
            sym = (*(symbols_iter->second))[i];
            s << "  key = " << str << " val " << sym << endl;
         }
   }
   s << " code_ptr_ = " << code_ptr_ << endl;
   s << " code_off_ = " << code_off_ << endl;
   s << " code_len_ = " << code_len_ << endl;
   s << " data_ptr_ = " << data_ptr_ << endl;
   s << " data_off_ = " << data_off_ << endl;
   s << " data_len_ = " << data_len_ << endl;
   return s;
}

#endif

SYMTAB_EXPORT AObject::AObject()
{
}	

SYMTAB_EXPORT unsigned AObject::nsymbols () const 
{ 
   return symbols_.size(); 
}

SYMTAB_EXPORT bool AObject::get_symbols(string & name, 
      std::vector<Symbol *> &symbols ) 
{
   if ( symbols_.find(name) == symbols_.end()) {
      return false;
   }

   symbols = symbols_[name];
   return true;
}

SYMTAB_EXPORT char* AObject::code_ptr () const 
{ 
   return code_ptr_; 
}

SYMTAB_EXPORT Offset AObject::code_off () const 
{ 
   return code_off_; 
}

SYMTAB_EXPORT Offset AObject::code_len () const 
{ 
   return code_len_; 
}

SYMTAB_EXPORT char* AObject::data_ptr () const 
{ 
   return data_ptr_; 
}

SYMTAB_EXPORT Offset AObject::data_off () const 
{ 
   return data_off_; 
}

SYMTAB_EXPORT Offset AObject::data_len () const 
{ 
   return data_len_; 
}

SYMTAB_EXPORT bool AObject::is_aout() const 
{
   return is_aout_;  
}

SYMTAB_EXPORT bool AObject::isDynamic() const 
{
   return is_dynamic_;  
}

SYMTAB_EXPORT unsigned AObject::no_of_sections() const 
{ 
   return no_of_sections_; 
}

SYMTAB_EXPORT unsigned AObject::no_of_symbols() const 
{ 
   return no_of_symbols_;  
}

SYMTAB_EXPORT bool AObject::getAllExceptions(std::vector<ExceptionBlock *>&excpBlocks) const
{
   for (unsigned i=0;i<catch_addrs_.size();i++)
      excpBlocks.push_back(new ExceptionBlock(catch_addrs_[i]));

   return true;
}

SYMTAB_EXPORT std::vector<Region *> AObject::getAllRegions() const
{
   return regions_;	
}

SYMTAB_EXPORT Offset AObject::loader_off() const 
{ 
   return loader_off_; 
}

SYMTAB_EXPORT unsigned AObject::loader_len() const 
{ 
   return loader_len_; 
}


SYMTAB_EXPORT int AObject::getAddressWidth() const 
{ 
   return addressWidth_nbytes; 
}

SYMTAB_EXPORT bool AObject::have_deferred_parsing(void) const
{ 
   return deferredParse;
}

SYMTAB_EXPORT void * AObject::getErrFunc() const 
{
   return (void *) err_func_; 
}

SYMTAB_EXPORT dyn_hash_map< string, std::vector< Symbol *> > *AObject::getAllSymbols() 
{ 
   return &(symbols_);
}

SYMTAB_EXPORT AObject::~AObject() 
{
}

// explicitly protected
SYMTAB_EXPORT AObject::AObject(MappedFile *mf_, MappedFile *mfd, void (*err_func)(const char *)) 
: mf(mf_), mfForDebugInfo(mfd), code_ptr_(0), code_off_(0),
   code_len_(0), data_ptr_(0), data_off_(0), data_len_(0),loader_off_(0),
   loader_len_(0), is_dynamic_(false), deferredParse(false), err_func_(err_func),
   addressWidth_nbytes(4) 
{
}

SYMTAB_EXPORT AObject::AObject(const AObject &obj)
: mf(obj.mf), mfForDebugInfo(obj.mfForDebugInfo), symbols_(obj.symbols_), 
   code_ptr_(obj.code_ptr_), code_off_(obj.code_off_), 
   code_len_(obj.code_len_), data_ptr_(obj.data_ptr_), 
   data_off_(obj.data_off_), data_len_(obj.data_len_), 
   loader_off_(obj.loader_off_), loader_len_(obj.loader_len_), is_dynamic_(obj.is_dynamic_),
   deferredParse(false), err_func_(obj.err_func_), addressWidth_nbytes(4)
{
} 

SYMTAB_EXPORT AObject& AObject::operator=(const AObject &obj) 
{   
   if (this == &obj) {
      return *this;
   }

   mf = obj.mf;
   mfForDebugInfo = obj.mfForDebugInfo;
   symbols_   = obj.symbols_;
   code_ptr_  = obj.code_ptr_;
   code_off_  = obj.code_off_;
   code_len_  = obj.code_len_;
   data_ptr_  = obj.data_ptr_;
   data_off_  = obj.data_off_;
   data_len_  = obj.data_len_;
   err_func_  = obj.err_func_;
   loader_off_ = obj.loader_off_; 
   loader_len_ = obj.loader_len_;
   is_dynamic_ = obj.is_dynamic_;
   addressWidth_nbytes = obj.addressWidth_nbytes;
   return *this;
}

//  a helper routine that selects a language based on information from the symtab
supportedLanguages AObject::pickLanguage(string &working_module, char *working_options,
      supportedLanguages working_lang)
{
   supportedLanguages lang = lang_Unknown;
   static int sticky_fortran_modifier_flag = 0;
   // (2) -- check suffixes -- try to keep most common suffixes near the top of the checklist
   string::size_type len = working_module.length();
   if((len>2) && (working_module.substr(len-2,2) == string(".c"))) lang = lang_C;
   else if ((len>2) && (working_module.substr(len-2,2) == string(".C"))) lang = lang_CPlusPlus;
   else if ((len>4) && (working_module.substr(len-4,4) == string(".cpp"))) lang = lang_CPlusPlus;
   else if ((len>2) && (working_module.substr(len-2,2) == string(".F"))) lang = lang_Fortran;
   else if ((len>2) && (working_module.substr(len-2,2) == string(".f"))) lang = lang_Fortran;
   else if ((len>3) && (working_module.substr(len-3,3) == string(".cc"))) lang = lang_C;
   else if ((len>2) && (working_module.substr(len-2,2) == string(".a"))) lang = lang_Assembly; // is this right?
   else if ((len>2) && (working_module.substr(len-2,2) == string(".S"))) lang = lang_Assembly;
   else if ((len>2) && (working_module.substr(len-2,2) == string(".s"))) lang = lang_Assembly;
   else
   {
      //(3) -- try to use options string -- if we have 'em
      if (working_options)
      {
         //  NOTE:  a binary is labeled "gcc2_compiled" even if compiled w/g77 -- thus this is
         //  quite inaccurate to make such assumptions
         if (strstr(working_options, "gcc"))
            lang = lang_C;
         else if (strstr(working_options, "g++"))
            lang = lang_CPlusPlus;
      }
   }
   //  This next section tries to determine the version of the debug info generator for a
   //  Sun fortran compiler.  Some leave the underscores on names in the debug info, and some
   //  have the "pretty" names, we need to detect this in order to properly read the debug.
   if (working_lang == lang_Fortran)
   {
      if (sticky_fortran_modifier_flag)
      {
         //cerr << FILE__ << __LINE__ << ": UPDATE: lang_Fortran->lang_Fortran_with_pretty_debug." << endl;
         working_lang = lang_Fortran_with_pretty_debug;
      }
      else if (working_options)
      {
         char *dbg_gen = NULL;
         //cerr << FILE__ << __LINE__ << ":  OPT: " << working_options << endl;			
         if (NULL != (dbg_gen = strstr(working_options, "DBG_GEN=")))
         {
            //cerr << __FILE__ << __LINE__ << ":  OPT: " << dbg_gen << endl;
            // Sun fortran compiler (probably), need to examine version
            char *dbg_gen_ver_maj = dbg_gen + strlen("DBG_GEN=");
            //cerr << __FILE__ << __LINE__ << ":  OPT: " << dbg_gen_ver_maj << endl;
            char *next_dot = strchr(dbg_gen_ver_maj, '.');
            if (NULL != next_dot)
            {
               next_dot = '\0';  //terminate major version number string
               int ver_maj = atoi(dbg_gen_ver_maj);
               //cerr <<"Major Debug Ver. "<<ver_maj<< endl;
               if (ver_maj < 3)
               {
                  working_lang = lang_Fortran_with_pretty_debug;
                  sticky_fortran_modifier_flag = 1;
                  //cerr << __FILE__ << __LINE__ << ": UPDATE: lang_Fortran->lang_Fortran_with_pretty_debug.  " << "Major Debug Ver. "<<ver_maj<<endl;
               }
            }
         }
      }
   }
   return lang;
}

SymbolIter::SymbolIter( Object & obj ) 
: symbols(obj.getAllSymbols()), currentPositionInVector(0) 
{
   symbolIterator = obj.getAllSymbols()->begin();
}

SymbolIter::SymbolIter( const SymbolIter & src ) 
: symbols(src.symbols),currentPositionInVector(0),
   symbolIterator( src.symbolIterator ) 
{
}

SymbolIter::~SymbolIter () 
{
}


void SymbolIter::reset () 
{
   currentPositionInVector = 0;
   symbolIterator = symbols->begin();
}

SymbolIter::operator bool() const
{
   return (symbolIterator!=symbols->end());
}

void SymbolIter::operator++ ( int ) 
{
   if ( currentPositionInVector + 1 < (symbolIterator->second).size())
   {
      currentPositionInVector++;
      return;
   }

   /* Otherwise, we need a new std::vector. */
   currentPositionInVector = 0;			
   symbolIterator++;
}

const string & SymbolIter::currkey() const 
{
   return symbolIterator->first;
}

/* If it's important that this be const, we could try to initialize
   currentVector to '& symbolIterator.currval()' in the constructor. */

Symbol *SymbolIter::currval() 
{
   return ((symbolIterator->second)[ currentPositionInVector ]);
}

