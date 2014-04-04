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

    if (demangle) {
       Symtab *symt = module_->exec();
       string pretty, typed;
       bool result = symt->buildDemangledName(name, pretty, typed, 
                                              symt->isNativeCompiler(), module_->language());
       if (result) {
          prettyNames_.push_back(pretty);
          typedNames_.push_back(typed);
       }
       else {
          //If mangling failed, then assume mangled name is already pretty
          prettyNames_.push_back(name);
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
        module_->exec()->addSymbolToAggregates(sym);
    }
    return true;
}

#if !defined(SERIALIZATION_DISABLED)
void Aggregate::restore_type_by_id(SerializerBase *sb, Type *&t, 
		unsigned t_id) THROW_SPEC (SerializerError)
{
	if (module_)
	{
		typeCollection *tc = module_->getModuleTypesPrivate();
		if (tc)
		{
			t = tc->findType(t_id);
			if (!t)
			{
				//fprintf(stderr, "%s[%d]: failed to find type in module(%s) collection\n", 
				//		FILE__, __LINE__, module_->fileName().c_str());
			}
		}
		else
		{
			fprintf(stderr, "%s[%d]:  no types for module\n", FILE__, __LINE__);
		}
	}
	else
	{
		fprintf(stderr, "%s[%d]:  bad deserialization order??\n", FILE__, __LINE__);
		//SER_ERR("FIXME");
	}

	if (!t)
	{
		SerContextBase *scb = sb->getContext();
		if (!scb)
		{
			fprintf(stderr, "%s[%d]:  SERIOUS:  FIXME\n", FILE__, __LINE__);
			SER_ERR("FIXME");
		}

		SerContext<Symtab> *scs = dynamic_cast<SerContext<Symtab> *>(scb);

		if (!scs)
		{
			fprintf(stderr, "%s[%d]:  SERIOUS:  FIXME\n", FILE__, __LINE__);
			SER_ERR("FIXME");
		}

		Symtab *st = scs->getScope();

		if (!st)
		{
			fprintf(stderr, "%s[%d]:  SERIOUS:  FIXME\n", FILE__, __LINE__);
			SER_ERR("FIXME");
		}

		t = st->findType(t_id);

		if (!t)
		{
			//  This should probably throw, but let's play nice for now
			fprintf(stderr, "%s[%d]:  FIXME: cannot find type with id %d\n", FILE__, __LINE__, t_id);
			std::vector<Module *> mods;
			if (!st->getAllModules(mods))
			{
				fprintf(stderr, "%s[%d]:  failed to get all modules\n", FILE__, __LINE__);
			}
			for (unsigned int i = 0; i < mods.size(); ++i)
			{
				std::vector<Type *> *modtypes = mods[i]->getAllTypes();
				fprintf(stderr, "%s[%d]:  module %s has %ld types\n", FILE__, __LINE__, mods[i]->fileName().c_str(), (signed long) (modtypes ? modtypes->size() : -1));
				if (mods[i]->getModuleTypesPrivate()->findType(t_id))
					fprintf(stderr, "%s[%d]:  found type %d in mod %s\n", FILE__, __LINE__, t_id, mods[i]->fileName().c_str());
			}
		}
	}
}
#else
void Aggregate::restore_type_by_id(SerializerBase *, Type *&, 
                                   unsigned ) THROW_SPEC (SerializerError) 
{
}
#endif

#if !defined(SERIALIZATION_DISABLED)
void Aggregate::restore_module_by_name(SerializerBase *sb,  
		std::string &mname) THROW_SPEC (SerializerError)
{
	if (!sb)
	{
		fprintf(stderr, "%s[%d]:  SERIOUS:  FIXME\n", FILE__, __LINE__);
		SER_ERR("FIXME");
	}

		SerContextBase *scb = sb->getContext();
		if (!scb)
		{
			fprintf(stderr, "%s[%d]:  SERIOUS:  FIXME\n", FILE__, __LINE__);
			SER_ERR("FIXME");
		}

		SerContext<Symtab> *scs = dynamic_cast<SerContext<Symtab> *>(scb);

		if (!scs)
		{
			fprintf(stderr, "%s[%d]:  SERIOUS:  FIXME\n", FILE__, __LINE__);
			SER_ERR("FIXME");
		}

		Symtab *st = scs->getScope();


	if (!st)
	{
		fprintf(stderr, "%s[%d]:  SERIOUS:  FIXME\n", FILE__, __LINE__);
		SER_ERR("FIXME");
	}

	if (!st->findModuleByName(module_, mname) || !(module_))
	{
		//  This should probably throw, but let's play nice for now
		fprintf(stderr, "%s[%d]:  FIXME: aggregate w/out module: %s\n", FILE__, __LINE__, mname.c_str());
	}
}
#else
void Aggregate::restore_module_by_name(SerializerBase *, std::string &) THROW_SPEC (SerializerError)
{
}
#endif

extern Symbol * getSymForID(SerializerBase *sb, Address id);

#if !defined(SERIALIZATION_DISABLED)
void Aggregate::rebuild_symbol_vector(SerializerBase *sb, std::vector<Address> &symids) THROW_SPEC (SerializerError)
{
	Offset off_accum = 0;
	for (unsigned long i = 0; i < symids.size(); ++i)
	{
		Symbol *sym = getSymForID(sb, symids[i]);
		if (!sym)
		{
			fprintf(stderr, "%s[%d]:  ERROR rebuilding aggregate: ", __FILE__, __LINE__);
			fprintf(stderr, "cannot find symbol for id %p\n", (void *) symids[i]);
			continue;
		}

		symbols_.push_back(sym);
        firstSymbol = symbols_[0];
        offset_ = firstSymbol->getOffset();

		//  sanity check to make sure that all our symbols share the same offset
		if (serializer_debug_flag())
		{
			if (!off_accum) 
				off_accum = sym->getOffset();
			else
			{
				if (sym->getOffset() != off_accum)
				{
					fprintf(stderr, "%s[%d]:  INTERNAL ERROR:  mismatch offsets: %p--%p\n", FILE__, __LINE__, (void *)off_accum, (void *)sym->getOffset());
				}
			}
		}

		//  This sucks, but apparently there are symbols that are somehow
		//  not getting their aggregate fields set properly (before serialize
		//  presumably), strangely only affects 64bit cases.  Here we try
		//  to correct for this by setting the aggregate values of all symbols
		//  at this Offset.  This lookup should be avoided by solving the problem
		//  somewhere else (at the source, wherever it is that the rogue symbols
		//  are being created and/or lost)
		//  
		//  Maybe it is also somehow possible that spurious symbols are being
		//  created and indexed during deserialize.
		SerContextBase *scb = sb->getContext();
		if (!scb)
		{
			fprintf(stderr, "%s[%d]:  SERIOUS:  FIXME\n", FILE__, __LINE__);
			SER_ERR("FIXME");
		}

		SerContext<Symtab> *scs = dynamic_cast<SerContext<Symtab> *>(scb);

		if (!scs)
		{
			fprintf(stderr, "%s[%d]:  SERIOUS:  FIXME\n", FILE__, __LINE__);
			SER_ERR("FIXME");
		}

		Symtab *st = scs->getScope();

	if (!st)
	{
		fprintf(stderr, "%s[%d]:  SERIOUS:  FIXME\n", FILE__, __LINE__);
		SER_ERR("FIXME");
	}
		std::vector<Symbol *> *syms = st->findSymbolByOffset(sym->getOffset());
		for (unsigned long j = 0; j < syms->size(); ++j)
		{
			(*syms)[j]->aggregate_ = this;
		}
	}
}
#else
void Aggregate::rebuild_symbol_vector(SerializerBase *, std::vector<Address> &) THROW_SPEC (SerializerError)
{
}
#endif

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

#if !defined(SERIALIZATION_DISABLED)
void Aggregate::serialize_aggregate(SerializerBase * sb, const char * tag) THROW_SPEC (SerializerError)
{
	std::string modname = module_ ? module_->fullName() : std::string("");
	std::vector<Address> symids;
	for (unsigned long i = 0; i < symbols_.size(); ++i)
	{
		assert(symbols_[i]);
		assert(sizeof(Address) == sizeof(void *));
		symids.push_back((Address) symbols_[i]);
	}

	try
	{
		ifxml_start_element(sb, tag);
		gtranslate(sb, modname, "moduleName");
		//  Arguably we should be able to reconstruct these name lists from the set of 
		//  symbols, right?  
		gtranslate(sb, mangledNames_, "mangledNameList");
		gtranslate(sb, prettyNames_, "prettyNameList");
		gtranslate(sb, typedNames_, "typedNameList");
#if 0
		gtranslate(sb, symbols_, "aggregatedSymbols", "aggregateSymbol");
		gtranslate(sb, sym_offsets, "symbolOffsetList");
#endif
		gtranslate(sb, symids, "symbolIDList");
		ifxml_end_element(sb, tag);

		if (sb->isBin() && sb->isInput())
		{
			restore_module_by_name(sb, modname);
			rebuild_symbol_vector(sb, symids);
		}

#if 0
		if (sb->isBin() && sb->isInput())
		{
			fprintf(stderr, "%s[%d]:  DESERIALIZE AGGREGATE %s, %lu offsets\n", FILE__, __LINE__, prettyNames_.size() ? prettyNames_[0].c_str() : "no_names", sym_offsets.size());
			rebuild_symbol_vector(sb, &sym_offsets);
		}
#endif
	}
	SER_CATCH(tag);

	serialize_printf("%s[%d]:  %sSERIALIZE AGGREGATE, nsyms = %lu\n", FILE__, __LINE__, 
			sb->isInput() ? "DE" : "", symbols_.size());
}
#else
void Aggregate::serialize_aggregate(SerializerBase *, const char *) THROW_SPEC (SerializerError)
{
}
#endif
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
