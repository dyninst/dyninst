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

#include "common/src/vgannotations.h"
#include <string.h>
#include <common/src/debug_common.h>
#include "debug.h"

#include "Annotatable.h"
#include "Module.h"
#include "Symtab.h"
#include "Collections.h"
#include "Function.h"
#include "Variable.h"
#include "LineInformation.h"
#include "symutil.h"
#include "annotations.h"

#include "common/src/pathName.h"
#include "Object.h"
#include <boost/foreach.hpp>
#include <algorithm>

#if defined(cap_dwarf)
#include "dwarfWalker.h"
#include "dwarf.h"
#endif

#include <vector>

using namespace Dyninst;
using namespace Dyninst::SymtabAPI;
using namespace std;

static SymtabError serr;

bool Module::findSymbol(std::vector<Symbol *> &found,
                        const std::string& name,
                        Symbol::SymbolType sType, 
                        NameType nameType,
                        bool isRegex,
                        bool checkCase,
                        bool includeUndefined) {
    unsigned orig_size = found.size();
    std::vector<Symbol *> obj_syms;
    
    if (exec()->findSymbol(obj_syms, name, sType, nameType, isRegex, checkCase, includeUndefined)) {
        return false;
    }
    
    for (unsigned i = 0; i < obj_syms.size(); i++) {
        if (obj_syms[i]->getModule() == this)
            found.push_back(obj_syms[i]);
    }
    
    if (found.size() > orig_size) 
        return true;
    
    return false;        
}

bool Module::getAllSymbols(std::vector<Symbol *> &found) {
    unsigned orig_size = found.size();
    std::vector<Symbol *> obj_syms;
    
    if (!exec()->getAllSymbols(obj_syms)) {
        return false;
    }
    
    for (unsigned i = 0; i < obj_syms.size(); i++) {
        if (obj_syms[i]->getModule() == this)
            found.push_back(obj_syms[i]);
    }
    
    if (found.size() > orig_size) 
        return true;
    
    return false;        
}

const std::string &Module::fileName() const
{
   return fileName_;
}

const std::string &Module::fullName() const
{
   return fileName();
}

 Symtab *Module::exec() const
{
   return exec_;
}

supportedLanguages Module::language() const
{
   return language_;
}

bool Module::getAddressRanges(std::vector<AddressRange >&ranges_,
      std::string lineSource, unsigned int lineNo)
{
   unsigned int originalSize = ranges_.size();

   LineInformation *lineInformation = parseLineInformation();
   if (lineInformation)
      lineInformation->getAddressRanges( lineSource.c_str(), lineNo, ranges_ );

   if ( ranges_.size() != originalSize )
      return true;

   return false;
}

bool Module::getSourceLines(std::vector<Statement::Ptr> &lines, Offset addressInRange)
{
   unsigned int originalSize = lines.size();

   LineInformation *lineInformation = parseLineInformation();
   if (lineInformation)
      lineInformation->getSourceLines( addressInRange, lines );

   if ( lines.size() != originalSize )
      return true;

   return false;
}

bool Module::getSourceLines(std::vector<LineNoTuple> &lines, Offset addressInRange)
{
   unsigned int originalSize = lines.size();

    LineInformation *lineInformation = parseLineInformation();

//    cout << "Module " << fileName() << " searching for line info in " << lineInformation << endl;
   if (lineInformation)
      lineInformation->getSourceLines( addressInRange, lines );

   if ( lines.size() != originalSize )
      return true;
   
   return false;
}

LineInformation *Module::parseLineInformation() {
    if(lineInfo_) return lineInfo_;

    const bool is_cuda = exec()->getArchitecture() == Arch_cuda;
    const bool debug_info = exec()->getObject()->hasDebugInfo();

    if (!debug_info || is_cuda) {
	objectLevelLineInfo = true;
	lineInfo_ = exec()->getObject()->parseLineInfoForObject(strings_);
	return lineInfo_;
    }

    lineInfo_ = new LineInformation;
    lineInfo_->setStrings(strings_);

    exec()->getObject()->parseLineInfoForCU(addr(), lineInfo_);
    return lineInfo_;
}

bool Module::getStatements(std::vector<LineInformation::Statement_t> &statements)
{
	unsigned initial_size = statements.size();
	LineInformation *li = parseLineInformation();
    if(!li) return false;

    std::copy(li->begin(), li->end(), std::back_inserter(statements));

	return (statements.size() > initial_size);
}

void Module::getAllTypes(vector<boost::shared_ptr<Type>>& v)
{
	exec_->parseTypesNow();
	if(typeInfo_) typeInfo_->getAllTypes(v);	
}

void Module::getAllGlobalVars(vector<pair<string, boost::shared_ptr<Type>>>& v)
{
	exec_->parseTypesNow();
	if(typeInfo_) typeInfo_->getAllGlobalVariables(v);
}

typeCollection *Module::getModuleTypes()
{
	exec_->parseTypesNow();
	return getModuleTypesPrivate();
}

typeCollection *Module::getModuleTypesPrivate()
{
  return typeInfo_;
}

bool Module::findType(boost::shared_ptr<Type> &type, std::string name)
{
	typeCollection *tc = getModuleTypes();
	if (!tc) return false;

   type = tc->findType(name, Type::share);

   if (!type)
      return false;

   return true;
}

bool Module::findVariableType(boost::shared_ptr<Type> &type, std::string name)
{
	typeCollection *tc = getModuleTypes();
	if (!tc) return false;

	type = tc->findVariableType(name, Type::share);

   if (!type)
      return false;

   return true;
}


bool Module::setLineInfo(LineInformation *lineInfo)
{
    assert(!lineInfo_);
    //delete lineInfo_;
    lineInfo_ = lineInfo;
    return true;
}

LineInformation *Module::getLineInformation()
{
  return lineInfo_;
}

bool Module::findLocalVariable(std::vector<localVar *>&vars, std::string name)
{
	std::vector<Function *>mod_funcs;

	if (!exec_->getAllFunctions(mod_funcs))
	{
		return false;
	}

	unsigned origSize = vars.size();

	for (unsigned int i = 0; i < mod_funcs.size(); i++)
	{
		mod_funcs[i]->findLocalVariable(vars, name);
	}

	if (vars.size() > origSize)
		return true;

	return false;
}

Module::Module(supportedLanguages lang, Offset adr,
      std::string fullNm, Symtab *img) :
   objectLevelLineInfo(false),
   lineInfo_(NULL),
   typeInfo_(NULL),
   fileName_(fullNm),
   compDir_(""),
   language_(lang),
   addr_(adr),
   exec_(img),
   strings_(new StringTable)
{}

Module::Module() :
   objectLevelLineInfo(false),
   lineInfo_(NULL),
   typeInfo_(NULL),
   fileName_(""),
   language_(lang_Unknown),
   addr_(0),
   exec_(NULL),
   strings_(new StringTable)
{
}

Module::Module(const Module &mod) :
   LookupInterface(),
   objectLevelLineInfo(mod.objectLevelLineInfo),
   lineInfo_(mod.lineInfo_),
   typeInfo_(mod.typeInfo_),
   fileName_(mod.fileName_),
   language_(mod.language_),
   addr_(mod.addr_),
   exec_(mod.exec_),
   strings_(mod.strings_)

{
}

Module::~Module()
{
  if (!objectLevelLineInfo)
    delete lineInfo_;
  delete typeInfo_;
  
}

bool Module::isShared() const
{
   return exec_->getObjectType() == obj_SharedLib;
}

bool Module::getAllSymbolsByType(std::vector<Symbol *> &found, Symbol::SymbolType sType)
{
   unsigned orig_size = found.size();
   std::vector<Symbol *> obj_syms;

   if (!exec()->getAllSymbolsByType(obj_syms, sType))
      return false;

   for (unsigned i = 0; i < obj_syms.size(); i++) 
   {
      if (obj_syms[i]->getModule() == this)
         found.push_back(obj_syms[i]);
   }

   if (found.size() > orig_size)
   {
      return true;
   }

   serr = No_Such_Symbol;
   return false;
}

std::vector<Function*> Module::getAllFunctions() const
{
    auto const& all_funcs = exec()->getAllFunctionsRef();
    std::vector<Function*> funcs;
    std::copy_if(all_funcs.begin(), all_funcs.end(), std::back_inserter(funcs),
	[this] (Function *f) {
	  return f->getModule() == this;
        }
    );
    return funcs;
}

bool Module::operator==(Module &mod) 
{
   if (exec_ && !mod.exec_) return false;
   if (!exec_ && mod.exec_) return false;
   if (exec_)
   {
	   if (exec_->file() != mod.exec_->file()) return false;
	   if (exec_->name() != mod.exec_->name()) return false;
   }

   return (
         (language_==mod.language_)
         && (addr_==mod.addr_)
         && (fileName_==mod.fileName_)
	 && (lineInfo_ == mod.lineInfo_)
         );
}

void Module::setLanguage(supportedLanguages lang)
{
   language_ = lang;
}

Offset Module::addr() const
{
   return addr_;
}

bool Module::setDefaultNamespacePrefix(string str)
{
    return exec_->setDefaultNamespacePrefix(str);
}

bool Module::findVariablesByOffset(std::vector<Variable *> &ret, const Offset offset)
{
    std::vector<Variable *> tmp;
    if (!exec()->findVariablesByOffset(tmp, offset))  {
        return false;
    }

    bool succ = false;
    for (auto v: tmp)  {
        if (v->getModule() == this)  {
            ret.push_back(v);
            succ = true;
        }
    }

    return succ;
}

bool Module::findVariablesByName(std::vector<Variable *> &ret, const std::string& name,
				 NameType nameType,
				 bool isRegex,
				 bool checkCase) {
  bool succ = false;
  std::vector<Variable *> tmp;

  if (!exec()->findVariablesByName(tmp, name, nameType, isRegex, checkCase)) {
    return false;
  }
  for (unsigned i = 0; i < tmp.size(); i++) {
    if (tmp[i]->getModule() == this) {
      ret.push_back(tmp[i]);
      succ = true;
    }
  }
  return succ;
}

void Module::addRange(Dyninst::Address low, Dyninst::Address high)
{
    dwarf_printf("Adding range [%lx, %lx) to %s\n", low, high, fileName().c_str());
    std::set<AddressRange>::iterator lb = ranges.lower_bound(AddressRange(low, high));
    if(lb != ranges.end() && lb->first <= low)
    {
        if(lb->second >= high)
        {
            return;
        }
        ranges.insert(AddressRange(lb->first, high));
//        printf("Actual is [%lx, %lx) due to overlap with [%lx, %lx)\n", lb->first, high, lb->first, lb->second);
        ranges.erase(lb);
    }
    else
    {
        ranges.insert(AddressRange(low, high));
    }

//    ranges.push_back(std::make_pair(low, high));
//    exec_->mod_lookup()->insert(new ModRange(low, high, this));
}

std::vector<ModRange*> Module::finalizeRanges()
{
    if(ranges.empty()) {
        return {};
    }

    std::vector<ModRange*> mod_ranges;
    mod_ranges.reserve(ranges.size());

    auto bit = ranges.begin();
    Address ext_s = bit->first;
    Address ext_e = ext_s;

    for( ; bit != ranges.end(); ++bit) {
        if(bit->first > ext_e) {
            mod_ranges.push_back(new ModRange(ext_s, ext_e, this));
            ext_s = bit->first;
        }
        ext_e = bit->second;
    }
    mod_ranges.push_back(new ModRange(ext_s, ext_e, this));
    ranges.clear();

    return mod_ranges;
}

StringTablePtr & Module::getStrings() {
    return strings_;
}

