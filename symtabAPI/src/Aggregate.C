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

#include "Symtab.h"
#include "symutil.h"
#include "Module.h"
#include "Collections.h"

#include "Function.h"
#include "Variable.h"

#include "symtabAPI/src/Object.h"

#include "debug.h"
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
    if(sym)
    {
        module_ = sym->getModule();
        symbols_.push_back(sym);
        firstSymbol = symbols_[0];
        offset_ = firstSymbol->getOffset();
    }
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
    boost::unique_lock<dyn_mutex> l(lock_);
	if (!firstSymbol)
	{
           create_printf("%s[%d]:  ERROR:  Aggregate w/out symbols\n", FILE__, __LINE__);
           return NULL;
	}
   	return firstSymbol->getRegion();
}

bool Aggregate::addSymbol(Symbol *sym) {

    // We keep a "primary" module.
    if (module_ == NULL) {
        module_ = sym->getModule();
    }

    boost::unique_lock<dyn_mutex> l(lock_);

    // No need to re-add symbols.
    for (unsigned i = 0; i < symbols_.size(); ++i)
        if (sym == symbols_[i]) return true;

    symbols_.push_back(sym);
    firstSymbol = symbols_[0];
    offset_ = firstSymbol->getOffset();

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

bool Aggregate::addMangledNameInternal(std::string name, bool /*isPrimary*/, bool /*demangle*/)
{
    // Check to see if we're duplicating
    boost::unique_lock<dyn_mutex> l(lock_);
    for (auto i = mangled_names_begin();
         i != mangled_names_end();
         ++i)
    {
        if (i->find(name) != string::npos)
            return false;
    }
    return true;
}

SYMTAB_EXPORT bool Aggregate::addMangledName(string name, bool isPrimary, bool isDebug)
{
   if (!addMangledNameInternal(name, isPrimary, false))
      return false;

    Symbol *staticSym = NULL;
    Symbol *dynamicSym = NULL;
    {
      boost::unique_lock<dyn_mutex> l(lock_);
      for (unsigned i = 0; i < symbols_.size(); ++i) {
        if (symbols_[i]->isInDynSymtab()) {
           dynamicSym = symbols_[i];
        }
        if (symbols_[i]->isInSymtab()) {
           staticSym = symbols_[i];
        }
      }
    }

    // Add a symbol representing this name
    // We only do this for mangled names since we don't have access
    // to a name mangler
    if (staticSym) {
      Symbol *newSym = new Symbol(*staticSym);
      newSym->setMangledName(name);
      newSym->isDynamic_ = false;
      newSym->isDebug_ = isDebug;
      module_->exec()->addSymbol(newSym);
    }
    if (dynamicSym) {
      Symbol *newSym = new Symbol(*dynamicSym);
      newSym->setMangledName(name);
      newSym->isDynamic_ = true;
      newSym->isDebug_ = isDebug;
      module_->exec()->addSymbol(newSym);
    }
    return true;
 }

SYMTAB_EXPORT bool Aggregate::addPrettyName(string name, bool isPrimary, bool isDebug)
{
    // Check to see if we're duplicating
   {
       boost::unique_lock<dyn_mutex> l(lock_);
       for (auto i = pretty_names_begin();
            i != pretty_names_end();
            i++) {
           if (i->find(name) != string::npos)
	       return false;
       }
   }
   return addMangledName(name, isPrimary, isDebug);
}

SYMTAB_EXPORT bool Aggregate::addTypedName(string name, bool isPrimary, bool isDebug)
{
    // Check to see if we're duplicating
   {
       boost::unique_lock<dyn_mutex> l(lock_);
       for (auto i = typed_names_begin();
            i != typed_names_end();
	    i++) {
           if (i->find(name) != string::npos)
               return false;
       }
   }
   return addMangledName(name, isPrimary, isDebug);
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

void Aggregate::print(std::ostream &os) const {
  std::string modname = module_ ? module_->fileName() : std::string("no_mod");
  os   << "Aggregate{"
       << " Module=" << modname
       << " MangledNames=[";
  ostream_iterator<string> out_iter(std::cout, ", ");
  std::copy(mangled_names_begin(), mangled_names_end(), out_iter);
  os << "]";
  
  os << " PrettyNames=["; 
  std::copy(pretty_names_begin(), pretty_names_end(), out_iter);
  os << "]";
  os << " TypedNames=["; 
  std::copy(typed_names_begin(), typed_names_end(), out_iter);
  
  os << "]";
  os << " }";
}

bool Aggregate::operator==(const Aggregate &a)
{
	if (symbols_.size() != a.symbols_.size()) return false;
	if (module_ && !a.module_) return false;
	if (!module_ && a.module_) return false;
	if (module_ && (module_->fileName() != a.module_->fileName())) return false;

	for (unsigned int i = 0; i < symbols_.size(); ++i)
	{
		Symbol *s1 = symbols_[i];
		Symbol *s2 = a.symbols_[i];
		if (s1 && !s2) return false;
		if (!s1 && s2) return false;
		if (!s1)
                   create_printf("%s[%d]:  WARN:  NULL Symbol pointer here\n", FILE__, __LINE__);
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

Aggregate::name_iter Aggregate::mangled_names_begin() const
{
  return boost::make_transform_iterator(symbols_.cbegin(), std::mem_fn(&Symbol::getMangledName));
}

Aggregate::name_iter Aggregate::mangled_names_end() const
{
  return boost::make_transform_iterator(symbols_.cend(), std::mem_fn(&Symbol::getMangledName));
}
Aggregate::name_iter Aggregate::pretty_names_begin() const
{
  return boost::make_transform_iterator(symbols_.cbegin(), std::mem_fn(&Symbol::getPrettyName));
}
Aggregate::name_iter Aggregate::pretty_names_end() const
{
  return boost::make_transform_iterator(symbols_.cend(), std::mem_fn(&Symbol::getPrettyName));
}
Aggregate::name_iter Aggregate::typed_names_begin() const
{
  return boost::make_transform_iterator(symbols_.cbegin(), std::mem_fn(&Symbol::getTypedName));
}
Aggregate::name_iter Aggregate::typed_names_end() const
{
  return boost::make_transform_iterator(symbols_.cend(), std::mem_fn(&Symbol::getTypedName));
}
