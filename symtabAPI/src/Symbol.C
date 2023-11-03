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

#include "symutil.h"
#include "Symbol.h"
#include "Module.h"
#include "Symtab.h"
#include "Aggregate.h"
#include "Function.h"
#include "Variable.h"
#include <string>
#include "annotations.h"

#include "common/src/headers.h"

#include <iostream>


using namespace Dyninst;
using namespace SymtabAPI;

Symbol *Symbol::magicEmitElfSymbol() {
	// I have no idea why this is the way it is,
	// but emitElf needs it...
	return new Symbol("",
			ST_NOTYPE,
                      SL_LOCAL,
                      SV_DEFAULT,
                      0,
                      NULL,
                      NULL,
                      0,
                      false,
                      false);
}
    
SYMTAB_EXPORT string Symbol::getMangledName() const 
{
    return mangledName_;
}

SYMTAB_EXPORT string Symbol::getPrettyName() const 
{
  return P_cplus_demangle(mangledName_, false);
}

SYMTAB_EXPORT string Symbol::getTypedName() const 
{
  return P_cplus_demangle(mangledName_, true);
}

bool Symbol::setOffset(Offset newOffset)
{
    offset_ = newOffset;
    return true;
}

bool Symbol::setPtrOffset(Offset newOffset)
{
    ptr_offset_ = newOffset;
    return true;
}

bool Symbol::setLocalTOC(Offset toc)
{
    localTOC_ = toc;
    return true;
}

SYMTAB_EXPORT bool Symbol::setModule(Module *mod) 
{
    assert(mod);
    module_ = mod; 
    return true;
}

SYMTAB_EXPORT bool Symbol::isFunction() const
{
    return (getFunction() != NULL);
}

SYMTAB_EXPORT bool Symbol::setFunction(Function *func)
{
    aggregate_ = func;
    return true;
}

SYMTAB_EXPORT Function * Symbol::getFunction() const
{
	if (aggregate_ == NULL) 
		return NULL;
    return dynamic_cast<Function *>(aggregate_);
}

SYMTAB_EXPORT bool Symbol::isVariable() const 
{
    return (getVariable() != NULL);
}

SYMTAB_EXPORT bool Symbol::setVariable(Variable *var) 
{
    aggregate_ = var;
    return true;
}

SYMTAB_EXPORT Variable * Symbol::getVariable() const
{
    return dynamic_cast<Variable *>(aggregate_);
}

SYMTAB_EXPORT bool Symbol::setSize(unsigned ns)
{
	size_ = ns;
	return true;
}

SYMTAB_EXPORT bool Symbol::setRegion(Region *r)
{
	region_ = r;
	return true;
}

SYMTAB_EXPORT Symbol::SymbolTag Symbol::tag() const 
{
    return tag_;
}


SYMTAB_EXPORT bool Symbol::setSymbolType(SymbolType sType)
{
    if ((sType != ST_UNKNOWN)&&
        (sType != ST_FUNCTION)&&
        (sType != ST_OBJECT)&&
        (sType != ST_MODULE)&&
        (sType != ST_NOTYPE) &&
	(sType != ST_INDIRECT))
        return false;
    
    SymbolType oldType = type_;	
    type_ = sType;
    if (module_ && module_->exec())
        module_->exec()->changeType(this, oldType);

    // TODO: update aggregate with information
    
    return true;
}

SYMTAB_EXPORT bool Symbol::setVersionFileName(std::string &fileName)
{
   std::string *fn_p = NULL;
   if (getAnnotation(fn_p, SymbolFileNameAnno)) 
   {
      return false;
   }
   else
   {
      //  not sure if we need to copy here or not, so let's do it...
      std::string *fn = new std::string(fileName);
      if (!addAnnotation(fn, SymbolFileNameAnno)) 
      {
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
      return false;
   }
   else
   {
      addAnnotation(&vers, SymbolVersionNamesAnno);
   }

   return true;
}

SYMTAB_EXPORT bool Symbol::getVersionFileName(std::string &fileName) const
{
   std::string *fn_p = NULL;

   if (getAnnotation(fn_p, SymbolFileNameAnno)) 
   {
      if (fn_p) 
         fileName = *fn_p;

      return true;
   }

   return false;
}

SYMTAB_EXPORT bool Symbol::getVersions(std::vector<std::string> *&vers) const
{
   std::vector<std::string> *vn_p = NULL;

   if (getAnnotation(vn_p, SymbolVersionNamesAnno)) 
   {
      if (vn_p) 
      {
         vers = vn_p;
         return true;
      } 
   }

   return false;
}

SYMTAB_EXPORT bool Symbol::setMangledName(std::string name)
{
   mangledName_ = name;
   setStrIndex(-1);
   return true;
}

std::ostream& Dyninst::SymtabAPI::operator<< (ostream &os, const Symbol &s) 
{
	return os << "{"
                  << " mangled=" << s.getMangledName()
                  << " pretty="  << s.getPrettyName()
                  << " module="  << s.module_
           //<< " type="    << (unsigned) s.type_
                  << " type="    << s.symbolType2Str(s.type_)
           //<< " linkage=" << (unsigned) s.linkage_
                  << " linkage=" << s.symbolLinkage2Str(s.linkage_)
                  << " offset=0x"    << hex << s.offset_ << dec
                  << " size=0x" << hex << s.size_ << dec
                  << " ptr_offset=0x"    << hex << s.ptr_offset_ << dec
                  << " localTOC=0x"    << hex << s.localTOC_ << dec
        //<< " tag="     << (unsigned) s.tag_
                  << " tag="     << s.symbolTag2Str(s.tag_)
                  << " isAbs="   << s.isAbsolute_
                  << " isDbg="   << s.isDebug_
                  << " isCommon=" << s.isCommonStorage_
                  << (s.isFunction() ? " [FUNC]" : "")
                  << (s.isVariable() ? " [VAR]" : "")
                  << (s.isInSymtab() ? "[STA]" : "")
                  << (s.isInDynSymtab() ? "[DYN]" : "")
                  << " }";
}



ostream & Dyninst::SymtabAPI::operator<< (ostream &s, const ExceptionBlock &eb) 
{
	s << "tryStart=" << eb.tryStart_ 
	  << ", trySize=" << eb.trySize_ 
	  << ", catchStart=" << eb.catchStart_ 
	  << ", hasTry=" << eb.trySize_ 
	  << ", tryStart_ptr=" << eb.tryStart_ptr
	  << ", tryEnd_ptr=" << eb.tryEnd_ptr
	  << ", catchStart_ptr=" << eb.catchStart_ptr;
	
	return s; 
}

bool Symbol::operator==(const Symbol& s) const
{
	// explicitly ignore tags when comparing symbols

	//  compare sections by offset, not pointer
	if (!region_ && s.region_) return false;
	if (region_ && !s.region_) return false;
	if (region_)
	{
		if (region_->getDiskOffset() != s.region_->getDiskOffset())
			return false;
	}

	// compare modules by name, not pointer
	if (!module_ && s.module_) return false;
	if (module_ && !s.module_) return false;
	if (module_)
	{
		if (module_->fileName() != s.module_->fileName())
			return false;
	}

	return (   (type_    == s.type_)
			&& (linkage_ == s.linkage_)
			&& (offset_    == s.offset_)
			&& (size_    == s.size_)
			&& (isDynamic_ == s.isDynamic_)
			&& (isAbsolute_ == s.isAbsolute_)
			&& (isDebug_ == s.isDebug_)
                        && (isCommonStorage_ == s.isCommonStorage_)
		   && (versionHidden_ == s.versionHidden_)
		   && (mangledName_ == s.mangledName_));
		   //			&& (prettyName_ == s.prettyName_)
		   //	&& (typedName_ == s.typedName_));
}

Symtab *Symbol::getSymtab() const { 
  return module_ ? module_->exec() : NULL; 
}

Symbol::Symbol () :
  module_(NULL),
  type_(ST_NOTYPE),
  internal_type_(0),
  linkage_(SL_UNKNOWN),
  visibility_(SV_UNKNOWN),
  offset_(0),
  ptr_offset_(0),
  localTOC_(0),
  region_(NULL),
  referring_(NULL),
  size_(0),
  isDynamic_(false),
  isAbsolute_(false),
  isDebug_(false),
  aggregate_(NULL),
  tag_(TAG_UNKNOWN) ,
  index_(-1),
  strindex_(-1),
  isCommonStorage_(false),
  versionHidden_(false)
{
}

Symbol::Symbol(const std::string& name,
	       SymbolType t,
	       SymbolLinkage l,
	       SymbolVisibility v,
	       Offset o,
	       Module *module,
	       Region *r,
	       unsigned s,
	       bool d,
	       bool a,
	       int index,
	       int strindex,
               bool cs):
  module_(module),
  type_(t),
  internal_type_(0),
  linkage_(l),
  visibility_(v),
  offset_(o),
  ptr_offset_(0),
  localTOC_(0),
  region_(r),
  referring_(NULL),
  size_(s),
  isDynamic_(d),
  isAbsolute_(a),
  isDebug_(false),
  aggregate_(NULL),
  mangledName_(name),
  tag_(TAG_UNKNOWN),
  index_(index),
  strindex_(strindex),
  isCommonStorage_(cs),
  versionHidden_(false)
{
}

Symbol::~Symbol ()
{
	std::string *sfa_p = NULL;

	if (getAnnotation(sfa_p, SymbolFileNameAnno))
	{
           removeAnnotation(SymbolFileNameAnno);
           delete (sfa_p);
	}
}

void Symbol::setReferringSymbol(Symbol* referringSymbol) 
{
	referring_= referringSymbol;
}

Symbol* Symbol::getReferringSymbol() const
{
	return referring_;
}

const char *Symbol::symbolType2Str(SymbolType t)
{
   switch (t)
   {
      CASE_RETURN_STR(ST_UNKNOWN);
      CASE_RETURN_STR(ST_FUNCTION);
      CASE_RETURN_STR(ST_OBJECT);
      CASE_RETURN_STR(ST_MODULE);
      CASE_RETURN_STR(ST_SECTION);
      CASE_RETURN_STR(ST_TLS);
      CASE_RETURN_STR(ST_DELETED);
      CASE_RETURN_STR(ST_NOTYPE);
      CASE_RETURN_STR(ST_INDIRECT);
   };

   return "invalid symbol type";
}

const char *Symbol::symbolLinkage2Str(SymbolLinkage t)
{
   switch (t)
   {
      CASE_RETURN_STR(SL_UNKNOWN);
      CASE_RETURN_STR(SL_GLOBAL);
      CASE_RETURN_STR(SL_LOCAL);
      CASE_RETURN_STR(SL_WEAK);
      CASE_RETURN_STR(SL_UNIQUE);
   };

   return "invalid symbol linkage";
}

const char *Symbol::symbolTag2Str(SymbolTag t)
{
   switch (t)
   {
      CASE_RETURN_STR(TAG_UNKNOWN);
      CASE_RETURN_STR(TAG_USER);
      CASE_RETURN_STR(TAG_LIBRARY);
      CASE_RETURN_STR(TAG_INTERNAL);
   };

   return "invalid symbol tag";
}

const char *Symbol::symbolVisibility2Str(SymbolVisibility t)
{
   switch(t) {
      CASE_RETURN_STR(SV_UNKNOWN);
      CASE_RETURN_STR(SV_DEFAULT);
      CASE_RETURN_STR(SV_INTERNAL);
      CASE_RETURN_STR(SV_HIDDEN);
      CASE_RETURN_STR(SV_PROTECTED);
   }
   return "invalid symbol visibility";
}
