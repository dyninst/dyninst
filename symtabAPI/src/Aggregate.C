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
#include "dyntypes.h"
#include "Annotatable.h"
#include "Serialization.h"
#include "common/src/serialize.h"

#include "Symtab.h"
#include "symutil.h"
#include "Module.h"
#include "Collections.h"

#include "Function.h"
#include "Variable.h"

#include "symtabAPI/src/Object.h"


#include "Aggregate.h"
#include "Symbol.h"

using namespace std;
using namespace Dyninst;
using namespace Dyninst::SymtabAPI;

Aggregate::Aggregate() :
    module_(NULL), firstSymbol(NULL), offset_(0L)
{
}

Aggregate::Aggregate(Symbol *sym) :
    module_(NULL), firstSymbol(NULL), offset_(0L)
{
    assert(sym);
    module_ = sym->getModule();
    symbols_.push_back(sym);
    firstSymbol = symbols_[0];
    offset_ = firstSymbol->getOffset();
    mangledNames_.push_back(sym->getMangledName());
    prettyNames_.push_back(sym->getPrettyName());
    typedNames_.push_back(sym->getTypedName());
}

Aggregate::Aggregate(Module *mod) :
   firstSymbol(NULL),
   offset_(0)
{
   module_ = mod;
}

Offset Aggregate::getOffset() const 
{ 
	if (!firstSymbol)
	{
		return (Offset) 0L;
	}
	return offset_;
}

unsigned Aggregate::getSize() const 
{ 
	if (!firstSymbol)
	{
		return (unsigned) 0;
	}
	return firstSymbol->getSize(); 
}

Region * Aggregate::getRegion() const
{
	if (!firstSymbol)
	{
		fprintf(stderr, "%s[%d]:  ERROR:  Aggregate w/out symbols\n", FILE__, __LINE__);
		return NULL;
	}
   	return firstSymbol->getRegion();
}

const vector<std::string> &Aggregate::getAllMangledNames() 
{
    return mangledNames_;
}

const vector<std::string> &Aggregate::getAllPrettyNames() 
{
    return prettyNames_;
}

const vector<std::string> &Aggregate::getAllTypedNames() 
{
    return typedNames_;
}

bool Aggregate::addSymbol(Symbol *sym) {

    // We keep a "primary" module, which is defined as "anything not DEFAULT_MODULE".
    if (module_ == NULL) {
        module_ = sym->getModule();
    }
    else if (module_->fileName() == "DEFAULT_MODULE") {
        module_ = sym->getModule();
    }
    // else keep current module.

    // No need to re-add symbols.
    for (unsigned i = 0; i < symbols_.size(); ++i)
        if (sym == symbols_[i]) return true;

    symbols_.push_back(sym);
    firstSymbol = symbols_[0];
    offset_ = firstSymbol->getOffset();

    // We need to add the symbol names (if they aren't there already)
    // We can have multiple identical names - for example, there are
    // often two symbols for main (static and dynamic symbol table)
    
    bool found = false;
    for (unsigned j = 0; j < mangledNames_.size(); j++) {
        if (sym->getMangledName() == mangledNames_[j]) {
            found = true;
            break;
        }
    }
    if (!found) mangledNames_.push_back(sym->getMangledName());

    found = false;

    for (unsigned j = 0; j < prettyNames_.size(); j++) {
        if (sym->getPrettyName() == prettyNames_[j]) {
            found = true;
            break;
        }
    }
    if (!found) prettyNames_.push_back(sym->getPrettyName());

    found = false;
    for (unsigned j = 0; j < typedNames_.size(); j++) {
        if (sym->getTypedName() == typedNames_[j]) {
            found = true;
            break;
        }
    }
    if (!found) typedNames_.push_back(sym->getTypedName());

    return true;
}

bool Aggregate::removeSymbolInt(Symbol *sym) {
    std::vector<Symbol *>::iterator iter;
    for (iter = symbols_.begin(); iter != symbols_.end(); iter++) {
        if ((*iter) == sym) {
            symbols_.erase(iter);
            break;
        }
    }
    if (symbols_.size() > 0) {
        firstSymbol = symbols_[0];
        offset_ = firstSymbol->getOffset();
    } else {
        firstSymbol = NULL;
        offset_ = 0L;
    }
    return true;
}

bool Aggregate::getSymbols(std::vector<Symbol *> &syms) const 
{
    syms = symbols_;
    return true;
}

Symbol * Aggregate::getFirstSymbol() const
{
    return firstSymbol;
}

bool Aggregate::addMangledNameInternal(std::string name, bool isPrimary, bool demangle)
{
    // Check to see if we're duplicating
    for (unsigned i = 0; i < mangledNames_.size(); i++) {
        if (mangledNames_[i] == name)
            return false;
    }

    if (isPrimary) {
        std::vector<std::string>::iterator iter = mangledNames_.begin();
        mangledNames_.insert(iter, name);
    }
    else
        mangledNames_.push_back(name);

    if (0 && demangle) {
       Symtab *symt = module_->exec();
       string pretty, typed;
       bool result = symt->buildDemangledName(name, pretty, typed, 
                                              symt->isNativeCompiler(), module_->language());
       if (result) {
	 if(std::find(prettyNames_.begin(), prettyNames_.end(), pretty) == prettyNames_.end()) 
	 {
	   prettyNames_.push_back(pretty);
	 }
	 if(std::find(typedNames_.begin(), typedNames_.end(), typed) == typedNames_.end()) 
	 {
	   typedNames_.push_back(typed);
	 }
       }
       else {
          //If mangling failed, then assume mangled name is already pretty
	 if(std::find(prettyNames_.begin(), prettyNames_.end(), name) == prettyNames_.end()) 
	 {
	   prettyNames_.push_back(name);
	 }
       }
    }
    return true;
}

SYMTAB_EXPORT bool Aggregate::addMangledName(string name, bool isPrimary) 
{
   if (!addMangledNameInternal(name, isPrimary, false))
      return false;

    Symbol *staticSym = NULL;
    Symbol *dynamicSym = NULL;
    for (unsigned i = 0; i < symbols_.size(); ++i) {
      if (symbols_[i]->isInDynSymtab()) {
         dynamicSym = symbols_[i];
      }
      if (symbols_[i]->isInSymtab()) {
         staticSym = symbols_[i];
      }
    }

    // Add a symbol representing this name
    // We only do this for mangled names since we don't have access
    // to a name mangler
    if (staticSym) {
      Symbol *newSym = new Symbol(*staticSym);
      newSym->setMangledName(name);
      module_->exec()->demangleSymbol(newSym);
      newSym->isDynamic_ = false;
      module_->exec()->addSymbol(newSym);
    }
    if (dynamicSym) {
      Symbol *newSym = new Symbol(*dynamicSym);
      newSym->setMangledName(name);
      module_->exec()->demangleSymbol(newSym);
      newSym->isDynamic_ = true;
      module_->exec()->addSymbol(newSym);
    }

    return true;
 }

SYMTAB_EXPORT bool Aggregate::addPrettyName(string name, bool isPrimary) 
 {
    // Check to see if we're duplicating
    for (unsigned i = 0; i < prettyNames_.size(); i++) {
        if (prettyNames_[i] == name)
            return false;
    }

    if (isPrimary) {
        std::vector<std::string>::iterator iter = prettyNames_.begin();
        prettyNames_.insert(iter, name);
    }
    else
        prettyNames_.push_back(name);

    if (mangledNames_.empty()) {
       //Can happen with inlined (symbolless) functions that
       // only specify a demangled name.
       mangledNames_.push_back(name);
    }
    return true;
 }

SYMTAB_EXPORT bool Aggregate::addTypedName(string name, bool isPrimary) 
{
  // Check to see if we're duplicating
  for (unsigned i = 0; i < typedNames_.size(); i++) {
    if (typedNames_[i] == name)
      return false;
  }
  
  if (isPrimary) {
    std::vector<std::string>::iterator iter = typedNames_.begin();
    typedNames_.insert(iter, name);
  }
  else
    typedNames_.push_back(name);
  return true;
}

bool Aggregate::changeSymbolOffset(Symbol *sym) 
{
    Offset oldOffset = getOffset();
    unsigned int old_count = symbols_.size();

    removeSymbolInt(sym);
    if (old_count == symbols_.size()) return true;

    if (symbols_.empty()) {
        // This was the only one; so add it back in and update our address
        // in the Symtab.
        symbols_.push_back(sym);
        firstSymbol = symbols_[0];
        offset_ = firstSymbol->getOffset();
        module_->exec()->changeAggregateOffset(this, oldOffset, getOffset());

    } else {
      module_->exec()->addSymbolToAggregates(const_cast<const Symbol*>(sym));
    }
    return true;
}

void Aggregate::restore_type_by_id(SerializerBase *, Type *&, 
                                   unsigned ) THROW_SPEC (SerializerError) 
{
}

void Aggregate::restore_module_by_name(SerializerBase *, std::string &) THROW_SPEC (SerializerError)
{
}

extern Symbol * getSymForID(SerializerBase *sb, Address id);

void Aggregate::rebuild_symbol_vector(SerializerBase *, std::vector<Address> &) THROW_SPEC (SerializerError)
{
}

std::ostream &operator<<(std::ostream &os, const Dyninst::SymtabAPI::Aggregate &a)
{
	std::string modname = a.module_ ? a.module_->fullName() : std::string("no_mod");
	os   << "Aggregate{"
		<< " Module=" << modname
		<< " MangledNames=["; 
		for (unsigned int i = 0; i < a.mangledNames_.size(); ++i)
		{
			os << a.mangledNames_[i];
			if ((i + 1) < a.mangledNames_.size())
				os << ", ";
		}
		os << "]";

		os << " PrettyNames=["; 
		for (unsigned int i = 0; i < a.prettyNames_.size(); ++i)
		{
			os << a.prettyNames_[i];
			if ((i + 1) < a.prettyNames_.size())
				os << ", ";
		}
		os << "]";

		os << " TypedNames=["; 
		for (unsigned int i = 0; i < a.typedNames_.size(); ++i)
		{
			os << a.typedNames_[i];
			if ((i + 1) < a.typedNames_.size())
				os << ", ";
		}
		os << "]";
		os << " }";

		return os;
}

void Aggregate::serialize_aggregate(SerializerBase *, const char *) THROW_SPEC (SerializerError)
{
}

bool Aggregate::operator==(const Aggregate &a)
{
	if (mangledNames_.size() != a.mangledNames_.size()) return false;
	if (prettyNames_.size() != a.prettyNames_.size()) return false;
	if (typedNames_.size() != a.typedNames_.size()) return false;
	if (symbols_.size() != a.symbols_.size()) return false;
	if (module_ && !a.module_) return false;
	if (!module_ && a.module_) return false;
	if (module_ && (module_->fullName() != a.module_->fullName())) return false;

	for (unsigned int i = 0; i < mangledNames_.size(); ++i)
	{
		if (mangledNames_[i] != a.mangledNames_[i]) return false;
	}
	for (unsigned int i = 0; i < prettyNames_.size(); ++i)
	{
		if (prettyNames_[i] != a.prettyNames_[i]) return false;
	}
	for (unsigned int i = 0; i < typedNames_.size(); ++i)
	{
		if (typedNames_[i] != a.typedNames_[i]) return false;
	}
	for (unsigned int i = 0; i < symbols_.size(); ++i)
	{
		Symbol *s1 = symbols_[i];
		Symbol *s2 = a.symbols_[i];
		if (s1 && !s2) return false;
		if (!s1 && s2) return false;
		if (!s1)
			fprintf(stderr, "%s[%d]:  WARN:  NULL Symbol pointer here\n", FILE__, __LINE__);
		else
		{
			//  just compare offset and a couple other params
			if (s1->getOffset() != s2->getOffset()) return false;
			if (s1->getType() != s2->getType()) return false;
			if (s1->getSize() != s2->getSize()) return false;
		}
	}

	return true;
}
