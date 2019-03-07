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
#include "common/src/serialize.h"
#include "Object.h"
#include <boost/foreach.hpp>

#if defined(cap_dwarf)
#include "dwarfWalker.h"
#include "dwarf.h"
#endif

using namespace Dyninst;
using namespace Dyninst::SymtabAPI;
using namespace std;

static SymtabError serr;

StringTablePtr Statement::getStrings_() const {
    return strings_;
}

void Statement::setStrings_(StringTablePtr strings) {
    Statement::strings_ = strings;
}

void* Statement::getExtraStringTable_() const {
    return extra_string_table_;
}

void Statement::setExtraStringTable_(void* string_table) {
    Statement::extra_string_table_ = string_table;
} 

std::string& Statement::lookupExtraStringTable(int index) {
    uint32_t num_files = 0;
    char buf[512];
    memcpy(&num_files, extra_string_table_, sizeof(uint32_t));
    if (index >= num_files) {
       cerr << "Statement::lookupExtraStringTable query index " << index << " out of range " << num_files << endl;
       return ""; 
    }
    uint32_t offset = 0;
    uint32_t filename_length = 0;
    size_t header_offset = sizeof(uint32_t) + index * sizeof(uint32_t) * 2; 
    memcpy(&offset, (char*)extra_string_table_ + header_offset, sizeof(uint32_t));
    memcpy(&filename_length, (char*)extra_string_table_ + header_offset + sizeof(uint32_t), sizeof(uint32_t));
    memcpy(buf, (char*)extra_string_table_ + offset, filename_length + 1);
    std::stringstream ss;
    ss << buf;
    return ss.str();
}

const std::string& Statement::getFile() const {
    if(strings_) {
        if(file_index_ < strings_->size()) {
            // can't be ->[] on shared pointer to multi_index container or compiler gets confused
            return (*strings_)[file_index_].str;
        } else { // 
          if (file_index_ >= DYNINST_STR_TBL_FID_OFFSET) {
              cout << "getFile(), for dyninst file index " << file_index_  << endl;
              if (extra_string_table_ == NULL) {
                 cerr << "error, pointer to string table not set " << endl; 
              } else {
                 uint32_t real_index = (uint32_t)file_index_ - DYNINST_STR_TBL_FID_OFFSET; 
                 return lookupExtraStringTable(real_index);
              }
          }  
        }
    } 
    // This string will be pointed to, so it has to persist.
    static std::string emptyStr;
    return emptyStr;
}


string Module::getCompDir()
{
    if(!compDir_.empty()) return compDir_;

#if defined(cap_dwarf)
    if(info_.empty())
    {
        return "";
    }

    auto& cu = info_[0];
    if(!dwarf_hasattr(&cu, DW_AT_comp_dir))
    {
        return "";
    }

    Dwarf_Attribute attr;
    auto comp_dir = dwarf_formstring( dwarf_attr(&cu, DW_AT_comp_dir, &attr) );
    compDir_ = std::string( comp_dir ? comp_dir : "" );
    return compDir_;

#else
    // TODO Implement this for non-dwarf format
    return compDir_;
#endif
}


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
   return fullName_;
}

 Symtab *Module::exec() const
{
   return exec_;
}

supportedLanguages Module::language() const
{
   return language_;
}

bool Module::getAddressRanges(std::vector<AddressRange >&ranges,
      std::string lineSource, unsigned int lineNo)
{
   unsigned int originalSize = ranges.size();

   LineInformation *lineInformation = parseLineInformation();
   if (lineInformation)
      lineInformation->getAddressRanges( lineSource.c_str(), lineNo, ranges );

   if ( ranges.size() != originalSize )
      return true;

   return false;
}

bool Module::getSourceLines(std::vector<Statement::Ptr> &lines, Offset addressInRange)
{
   unsigned int originalSize = lines.size();

   LineInformation *lineInformation = parseLineInformation();
   if (lineInformation)
      lineInformation->getSourceLines( addressInRange, lines );

   if ( lines.size() != originalSize ) {
      auto stmt = lines[originalSize];  
      auto file_index = stmt->getFileIndex();
      if (file_index >= DYNINST_STR_TBL_FID_OFFSET) {
          cout << "we get the dyninst file index " << file_index << endl;    
          stmt->setExtraStringTable_(this->string_table); // set the pointer to string table chunk  
      } 
      return true;
   }

   return false;
}

bool Module::getSourceLines(std::vector<LineNoTuple> &lines, Offset addressInRange)
{
   unsigned int originalSize = lines.size();

    LineInformation *lineInformation = parseLineInformation();
   if (lineInformation) {
      lineInformation->getSourceLines( addressInRange, lines );
   }

   if ( lines.size() != originalSize ) {
      auto stmt = lines[originalSize]; 
      auto file_index = stmt.getFileIndex();
      if (file_index >= DYNINST_STR_TBL_FID_OFFSET) {
          cout << "we get the dyninst file index " << file_index << endl;
          lines[originalSize].setExtraStringTable_(this->string_table);  
      }
      return true;
   }
   return false;
}

// parse the line information stored in .dyninstLineMap
bool Module::parseDyninstLineInformation()  
{
    Symtab* symObj = exec();
    if (symObj == NULL) {
        cerr << "Module::parseDyninstLineInformation - symtab is null " << endl;
        return false;
    }
    vector<LineMapInfoEntry> linemap = symObj->getAllRelocatedSymbols();
    string_table = symObj->getStringTable(); // get a copy of the string table
    // we still insert these to leverage the multi-index lookup data structure 
    for (int i = 0; i < linemap.size(); ++i) {
       lineInfo_->addLine(linemap[i].file_index,  // here the file index is with offset
                          linemap[i].line_number,
                          linemap[i].column_number,
                          linemap[i].low_addr_inc,
                          linemap[i].high_addr_exc);      
    }
    return linemap.size() > 0;
}


LineInformation *Module::parseLineInformation() {
    if (exec()->getArchitecture() != Arch_cuda &&
	(exec()->getObject()->hasDebugInfo() || !info_.empty())) {
        // Allocate if none
        if (!lineInfo_) {
            lineInfo_ = new LineInformation;
            // share our string table
            lineInfo_->setStrings(strings_);
        }

        // Parse any CUs that have been added to our list
        if(!info_.empty()) {
            for(auto cu = info_.begin();
                    cu != info_.end();
                    ++cu)
            {
                exec()->getObject()->parseLineInfoForCU(*cu, lineInfo_);
            }
        }

        // Before clearing the CU list (why is it even done anyway?), make sure to
        // call getCompDir so the comp_dir is stored in a static variable.
        getCompDir();

        // Clear list of work to do
        info_.clear();
    } else if (!lineInfo_) {
        objectLevelLineInfo = true;
        lineInfo_ = exec()->getObject()->parseLineInfoForObject(strings_);
    } 
    if (dyninst_linemap_parsed == false) {
        parseDyninstLineInformation(); // read the extra .dyninstLineMap section, propagate the line map info into the lineInfo_ that should have already been created
        dyninst_linemap_parsed = true;
    }
    cerr << "parseLineInformation: strings_ size: " << strings_->size() << " module name: " << fileName() << endl;
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

vector<Type *> *Module::getAllTypes()
{
	exec_->parseTypesNow();
	if(typeInfo_) return typeInfo_->getAllTypes();
	return NULL;
	
}

vector<pair<string, Type *> > *Module::getAllGlobalVars()
{
	exec_->parseTypesNow();
	if(typeInfo_) return typeInfo_->getAllGlobalVariables();
	return NULL;	
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

bool Module::findType(Type *&type, std::string name)
{
	typeCollection *tc = getModuleTypes();
	if (!tc) return false;

   type = tc->findType(name);

   if (type == NULL)
      return false;

   return true;
}

bool Module::findVariableType(Type *&type, std::string name)
{
	typeCollection *tc = getModuleTypes();
	if (!tc) return false;

	type = tc->findVariableType(name);

   if (type == NULL)
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
   lineInfo_(NULL),
   typeInfo_(NULL),
   fullName_(fullNm),
   compDir_(""),
   language_(lang),
   addr_(adr),
   exec_(img),
   strings_(new StringTable),
   ranges_finalized(false)
{
   fileName_ = extract_pathname_tail(fullNm);
}

Module::Module() :
   objectLevelLineInfo(false),
   lineInfo_(NULL),
   typeInfo_(NULL),
   fileName_(""),
   fullName_(""),
   compDir_(""),
   language_(lang_Unknown),
   addr_(0),
   exec_(NULL),
   strings_(new StringTable),
    ranges_finalized(false)
{
    dyninst_linemap_parsed = false;
}

Module::Module(const Module &mod) :
   LookupInterface(),
   objectLevelLineInfo(mod.objectLevelLineInfo),
   lineInfo_(mod.lineInfo_),
   typeInfo_(mod.typeInfo_),
   info_(mod.info_),
   fileName_(mod.fileName_),
   fullName_(mod.fullName_),
   compDir_(mod.compDir_),
   language_(mod.language_),
   addr_(mod.addr_),
   exec_(mod.exec_),
   strings_(mod.strings_),
    ranges_finalized(mod.ranges_finalized)

{
    dyninst_linemap_parsed = false;
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

bool Module::getAllFunctions(std::vector<Function *> &ret)
{
    return exec()->getAllFunctions(ret);
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
         && (fullName_==mod.fullName_)
         && (fileName_==mod.fileName_)
	 && (lineInfo_ == mod.lineInfo_)
         );
}

bool Module::setName(std::string newName)
{
   fullName_ = newName;
   fileName_ = extract_pathname_tail(fullName_);
   return true;
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

void Module::finalizeRanges()
{
    if(ranges.empty()) {
        return;
    }
    auto bit = ranges.begin();
    Address ext_s = bit->first;
    Address ext_e = ext_s;

    for( ; bit != ranges.end(); ++bit) {
        if(bit->first > ext_e) {
            finalizeOneRange(ext_s, ext_e);
            ext_s = bit->first;
        }
        ext_e = bit->second;
    }
    finalizeOneRange(ext_s, ext_e);
    ranges_finalized = true;
    ranges.clear();
}

void Module::finalizeOneRange(Address ext_s, Address ext_e) const {
    ModRange* r = new ModRange(ext_s, ext_e, const_cast<Module*>(this));
    ModRangeLookup* lookup = exec_->mod_lookup();
//    cout << "Inserting range " << std::hex << (*r) << std::dec << endl;
    lookup->insert(r);
}

void Module::addDebugInfo(Module::DebugInfoT info) {
//    cout << "Adding CU DIE to " << fileName() << endl;
    info_.push_back(info);

}

StringTablePtr & Module::getStrings() {
    return strings_;
}

void * Module::getStringTable() {
    return string_table;
}
