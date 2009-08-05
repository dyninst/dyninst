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

#include "Annotatable.h"
#include "common/h/serialize.h"

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
    module_(NULL)
{
}

Aggregate::Aggregate(Symbol *sym) :
    module_(NULL)
{
    assert(sym);
    module_ = sym->getModule();
    symbols_.push_back(sym);
    mangledNames_.push_back(sym->getMangledName());
    prettyNames_.push_back(sym->getPrettyName());
    typedNames_.push_back(sym->getTypedName());
}


Offset Aggregate::getOffset() const 
{ 
	return getFirstSymbol()->getOffset();
}

unsigned Aggregate::getSize() const 
{ 
	return getFirstSymbol()->getSize(); 
}

Region * Aggregate::getRegion() const
{
   	return getFirstSymbol()->getRegion();
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

    symbols_.push_back(sym);

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
    if (!found) typedNames_.push_back(sym->getTypedName());;

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
    return true;
}

bool Aggregate::getSymbols(std::vector<Symbol *> &syms) const 
{
    syms = symbols_;
    return true;
}

Symbol * Aggregate::getFirstSymbol() const
{
    assert( symbols_.size()>0 );
    return symbols_[0];
}

SYMTAB_EXPORT bool Aggregate::addMangledName(string name, bool isPrimary) 
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

    if (addMangledNameInt(name, isPrimary) == false) return false;
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
    if (addPrettyNameInt(name, isPrimary) == false) return false;
     return true;
 }																	
 
 SYMTAB_EXPORT bool Aggregate::addTypedName(string name, bool isPrimary) 
 {
    // Check to see if we're duplicating
    for (unsigned i = 0; i < typedNames_.size(); i++) {
        if (typedNames_[i] == name)
            return false;
    }
    if (addTypedNameInt(name, isPrimary) == false) return false;

    if (isPrimary) {
        std::vector<std::string>::iterator iter = typedNames_.begin();
        typedNames_.insert(iter, name);
    }
    else
        typedNames_.push_back(name);
	return true;
 }

bool Aggregate::addMangledNameInt(string name, bool isPrimary) {
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
    return true;
}

bool Aggregate::addPrettyNameInt(string name, bool isPrimary) {
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
    return true;
}

bool Aggregate::addTypedNameInt(string name, bool isPrimary) {
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

    removeSymbolInt(sym);

    if (symbols_.empty()) 
	{
        // This was the only one; so add it back in and update our address
        // in the Symtab.
        symbols_.push_back(sym);
        module_->exec()->changeAggregateOffset(this, oldOffset, getOffset());
    }
    else 
	{
        module_->exec()->addSymbolToAggregates(sym);
    }
    return true;
}

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
		ScopedSerializerBase<Symtab> *ssb = dynamic_cast<ScopedSerializerBase<Symtab> *>(sb);

		if (!ssb)
		{
			fprintf(stderr, "%s[%d]:  SERIOUS:  FIXME\n", FILE__, __LINE__);
			SER_ERR("FIXME");
		}

		Symtab *st = ssb->getScope();

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
				fprintf(stderr, "%s[%d]:  module %s has %lu types\n", FILE__, __LINE__, mods[i]->fileName().c_str(), modtypes ? modtypes->size() : -1);
				if (mods[i]->getModuleTypesPrivate()->findType(t_id))
					fprintf(stderr, "%s[%d]:  found type %d in mod %s\n", FILE__, __LINE__, t_id, mods[i]->fileName().c_str());
			}
		}
	}
}

void Aggregate::restore_module_by_name(SerializerBase *sb,  
		std::string &mname) THROW_SPEC (SerializerError)
{

	if (!sb)
	{
		fprintf(stderr, "%s[%d]:  SERIOUS:  FIXME\n", FILE__, __LINE__);
		SER_ERR("FIXME");
	}

	Dyninst::ScopedSerializerBase<Dyninst::SymtabAPI::Symtab> *ssb = dynamic_cast<Dyninst::ScopedSerializerBase<Dyninst::SymtabAPI::Symtab> *>(sb);

	if (!ssb)
	{
		fprintf(stderr, "%s[%d]:  SERIOUS:  FIXME: %s\n", FILE__, __LINE__, typeid(sb).name());
		SER_ERR("FIXME");
	}

	Symtab *st = ssb->getScope();

	if (!st)
	{
		fprintf(stderr, "%s[%d]:  SERIOUS:  FIXME\n", FILE__, __LINE__);
		SER_ERR("FIXME");
	}

	if (!st->findModuleByName(module_, mname) || !(module_))
	{
		//  This should probably throw, but let's play nice for now
		fprintf(stderr, "%s[%d]:  FIXME: aggregate w/out module\n", FILE__, __LINE__);
	}
}

void Aggregate::rebuild_symbol_vector(SerializerBase *sb,  
		std::vector<Offset> *sym_offsets) THROW_SPEC (SerializerError)
{
	ScopedSerializerBase<Symtab> *ssb = dynamic_cast<ScopedSerializerBase<Symtab> *>(sb);

	if (!ssb)
	{
		//fprintf(stderr, "%s[%d]:  SERIOUS:  FIXME\n", FILE__, __LINE__);
		SER_ERR("FIXME");
	}

	Symtab *st = ssb->getScope();

	if (!st)
	{
		//fprintf(stderr, "%s[%d]:  SERIOUS:  FIXME\n", FILE__, __LINE__);
		SER_ERR("FIXME");
	}

	if (!sym_offsets)
	{
		//  can't have any aggregates w/out symbols
		//fprintf(stderr, "%s[%d]:  SERIOUS:  FIXME\n", FILE__, __LINE__);
		SER_ERR("FIXME");
	}

	if (!sym_offsets->size())
	{
		//  can't have any aggregates w/out symbols
		//fprintf(stderr, "%s[%d]:  SERIOUS:  FIXME\n", FILE__, __LINE__);
		SER_ERR("FIXME");
	}

	symbols_.resize(sym_offsets->size());

	for (unsigned int i = 0; i < sym_offsets->size(); ++i)
	{
		std::vector<Symbol *> *syms = st->findSymbolByOffset((*sym_offsets)[i]);

		if (!syms)
		{
			//  Should throw here, but for now let's just scream
			//fprintf(stderr, "%s[%d]:  SERIOUS:  FIXME\n", FILE__, __LINE__);
		}
		else
		{
			if (syms->size() > 1)
			{
				//fprintf(stderr, "%s[%d]:  SERIOUS:  FIXME\n", FILE__, __LINE__);
			}

			symbols_[i] = (*syms)[0];
		}

	}
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

void Aggregate::serialize_aggregate(SerializerBase * sb, const char * tag) THROW_SPEC (SerializerError)
{
	std::string modname = module_ ? module_->fullName() : std::string("");
	std::vector<Offset> sym_offsets;
	sym_offsets.resize(symbols_.size());

	//fprintf(stderr, "%s[%d]:  serialize_aggregate, module = %p\n", FILE__, __LINE__, module_);

	for (unsigned int i = 0; i < symbols_.size(); ++i)
	{
		assert(symbols_[i]);
		sym_offsets[i] = symbols_[i]->offset_;
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
		gtranslate(sb, sym_offsets, "symbolOffsetList");
		ifxml_end_element(sb, tag);

		if (sb->isBin() && sb->isInput())
			restore_module_by_name(sb, modname);

		if (sb->isBin() && sb->isInput())
			rebuild_symbol_vector(sb, &sym_offsets);
	}
	SER_CATCH(tag);

	//fprintf(stderr, "%s[%d]:  serialize_aggregate, module = %p\n", FILE__, __LINE__, module_);
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
