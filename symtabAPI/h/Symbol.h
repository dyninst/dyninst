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

#if !defined(_Symbol_h_)
#define _Symbol_h_


/************************************************************************
 * header files.
************************************************************************/

#include "symutil.h"
#include "Annotatable.h"
#include <iosfwd>
#include <string>
#include <vector>
#include <boost/shared_ptr.hpp>

#ifndef CASE_RETURN_STR
#define CASE_RETURN_STR(x) case x: return #x
#endif


namespace Dyninst{
namespace SymtabAPI{

class Module;
class typeCommon;
class localVarCollection;
class Region;
class Aggregate;
class Function;
class Variable;
class Type;
class typeCollection;
class Symtab;

/************************************************************************
 * class Symbol
************************************************************************/

class SYMTAB_EXPORT Symbol : public AnnotatableSparse
{
   friend class typeCommon;
   friend class Symtab;
   friend class AObject;
   friend class Object;
   friend class Aggregate;
   friend class relocationEntry;

   public:
   struct Ptr
   {
   Ptr(Symbol* s) : m_this(s)
     {
     }
     ~Ptr() 
     {
     }
     Symbol* get() const
     {
       return m_this;
     }
     operator Symbol*() const
     {
       return m_this;
     }
     Symbol* operator->() const
     {
       return m_this;
       
     }
     
     Symbol* m_this;
     
   };
   
   
   

   enum SymbolType {
      ST_UNKNOWN,
      ST_FUNCTION,
      ST_OBJECT,
      ST_MODULE,
      ST_SECTION,
      ST_TLS,
      ST_DELETED,
      ST_INDIRECT,
      ST_NOTYPE
   };

   static const char *symbolType2Str(SymbolType t);

   enum SymbolLinkage {
      SL_UNKNOWN,
      SL_GLOBAL,
      SL_LOCAL,
      SL_WEAK,
      SL_UNIQUE
   };

   static const char *symbolLinkage2Str(SymbolLinkage t);

   enum SymbolTag {
      TAG_UNKNOWN,
      TAG_USER,
      TAG_LIBRARY,
      TAG_INTERNAL
   };

   static const char *symbolTag2Str(SymbolTag t);

   enum SymbolVisibility {
       SV_UNKNOWN,
       SV_DEFAULT,
       SV_INTERNAL,
       SV_HIDDEN,
       SV_PROTECTED
   };
   static const char *symbolVisibility2Str(SymbolVisibility t);

   Symbol();

   static Symbol *magicEmitElfSymbol();

   Symbol (const std::string& name,
                         SymbolType t,
                         SymbolLinkage l,
                         SymbolVisibility v,
                         Offset o,
                         Module *module = NULL, 
                         Region *r = NULL, 
                         unsigned s = 0,
                         bool d = false,
                         bool a = false,
			 int index= -1,
			 int strindex = -1,
                         bool cs = false);
   ~Symbol();
   Symbol(const Symbol&) = default;

   bool          operator== (const Symbol &) const;

   /***********************************************************
     Name Output Functions
    ***********************************************************/		
   std::string      getMangledName () const;
   std::string	 getPrettyName() const;
   std::string      getTypedName() const;

   Module *getModule() const { return module_; } 
   Symtab *getSymtab() const;
   SymbolType getType () const { return type_; }
   SymbolLinkage getLinkage () const { return linkage_; }
   Offset getOffset() const { return offset_; }
   Offset getPtrOffset() const { return ptr_offset_; }
   Offset getLocalTOC() const { return localTOC_; }
   unsigned getSize() const { return size_; }
   Region *getRegion() const { return region_; }

   bool isInDynSymtab() const { return (type_ != ST_DELETED) && isDynamic_ && !isDebug_; }
   bool isInSymtab() const { return (type_ != ST_DELETED) && !isDynamic_ && !isDebug_; }
   bool isAbsolute() const { return isAbsolute_; }
   bool isDebug() const { return isDebug_; }
   bool isCommonStorage() const { return isCommonStorage_; }

   bool              isFunction()            const;
   bool              setFunction(Function * func);
   Function *        getFunction()           const;

   bool              isVariable()            const;
   bool              setVariable(Variable *var);
   Variable *        getVariable()           const;

   SymbolVisibility getVisibility() const { return visibility_; }

   int getIndex() const { return index_; }
   bool setIndex(int index) { index_ = index; return true; }
   int getStrIndex() const { return strindex_; }
   bool setStrIndex(int strindex) { strindex_ = strindex; return true; }
   void setReferringSymbol (Symbol *referringSymbol);
   Symbol* getReferringSymbol () const;

   //////////////// Modification
   bool setOffset (Offset newOffset);
   bool setPtrOffset (Offset newOffset);
   bool setLocalTOC (Offset localTOC);
   bool setSize(unsigned ns);
   bool setRegion(Region *r);
   bool setModule(Module *mod);

   bool setMangledName(std::string name);

   bool setSymbolType(SymbolType sType);

   SymbolTag            tag ()               const;
   bool  setDynamic(bool d) { isDynamic_ = d; return true;}
   bool  setAbsolute(bool a) { isAbsolute_ = a; return true; }
   bool  setDebug(bool dbg) { isDebug_ = dbg; return true; }
   bool  setCommonStorage(bool cs) { isCommonStorage_ = cs; return true; }

   bool  setVersionFileName(std::string &fileName);
   bool  setVersions(std::vector<std::string> &vers);
   bool  setVersionNum(unsigned verNum);
   void setVersionHidden() { versionHidden_ = true; }

   bool  getVersionFileName(std::string &fileName) const;
   bool  getVersions(std::vector<std::string> *&vers) const;
   bool  getVersionNum(unsigned &verNum) const;
   bool  getVersionHidden() const { return versionHidden_; }

   friend
      std::ostream& operator<< (std::ostream &os, const Symbol &s);

   public:
   int getInternalType() const { return internal_type_; }
   void setInternalType(int i) { internal_type_ = i; }

   private:

   Module*       module_;
   SymbolType    type_;
   int           internal_type_;
   SymbolLinkage linkage_;
   SymbolVisibility visibility_;
   Offset        offset_;
   Offset        ptr_offset_;  // Function descriptor offset.  Not available on all platforms.
   Offset        localTOC_;
   Region*       region_;
   Symbol* 	 referring_;
   unsigned      size_;  // size of this symbol. This is NOT available on all platforms.

   bool          isDynamic_;
   bool          isAbsolute_;
   bool          isDebug_;

   Aggregate *   aggregate_; // Pointer to Function or Variable container, if appropriate.

   std::string mangledName_;

   SymbolTag     tag_;
   int index_;
   int strindex_;

   bool          isCommonStorage_;

   std::vector<std::string> verNames_;

   bool versionHidden_;
};

class SYMTAB_EXPORT LookupInterface 
{
   public:
      LookupInterface();
      virtual bool getAllSymbolsByType(std::vector<Symbol *> &ret,
            Symbol::SymbolType sType) = 0;
      virtual bool findSymbol(std::vector<Symbol *> &ret,
                                            const std::string& name,
                                            Symbol::SymbolType sType = Symbol::ST_UNKNOWN,
                                            NameType nameType = anyName,
                                            bool isRegex = false,
                                            bool checkCase = false,
                                            bool includeUndefined = false) = 0;
      virtual bool findType(boost::shared_ptr<Type>& type, std::string name) = 0;
      bool findType(Type*& t, std::string n) {
        boost::shared_ptr<Type> tp;
        auto r = findType(tp, n);
        t = tp.get();
        return r;
      }
      virtual bool findVariableType(boost::shared_ptr<Type>& type, std::string name)= 0;
      bool findVariableType(Type*& t, std::string n) {
        boost::shared_ptr<Type> tp;
        auto r = findVariableType(tp, n);
        t = tp.get();
        return r;
      }

      virtual ~LookupInterface();
};

SYMTAB_EXPORT std::ostream& operator<< (std::ostream &os, const Symbol &s);

}//namespace SymtabAPI
}//namespace Dyninst

#endif /* !defined(_Symbol_h_) */
