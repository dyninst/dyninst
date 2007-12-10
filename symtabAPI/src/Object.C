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

// $Id: Object.C,v 1.12 2007/12/10 22:27:50 giri Exp $

#include "symtabAPI/src/Object.h"
#include "symtabAPI/h/Symtab.h"
#include "symtabAPI/src/Collections.h"

using namespace std;
using namespace Dyninst;
using namespace Dyninst::SymtabAPI;

string Symbol::emptyString("");

bool Dyninst::SymtabAPI::symbol_compare(const Symbol *s1, const Symbol *s2) {
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

bool AObject::needs_function_binding() const {
    return false;
}

bool AObject::get_func_binding_table(std::vector<relocationEntry> &) const {
    return false;
}

bool AObject::get_func_binding_table_ptr(const std::vector<relocationEntry> *&) const {
    return false;
}

bool AObject::addRelocationEntry(relocationEntry &){
    return true;
}

char *AObject::mem_image() const
{
	return NULL;
}

DLLEXPORT ExceptionBlock::~ExceptionBlock() {}

DLLEXPORT ExceptionBlock::ExceptionBlock() : tryStart_(0), trySize_(0), 
								catchStart_(0), hasTry_(false) {}

DLLEXPORT Offset ExceptionBlock::catchStart() const { 
	return catchStart_;
}

DLLEXPORT relocationEntry::relocationEntry(const relocationEntry& ra): target_addr_(ra.target_addr_), 
										rel_addr_(ra.rel_addr_), name_(ra.name_), dynref_(ra.dynref_), relType_(ra.relType_) {}

DLLEXPORT Offset relocationEntry::target_addr() const { 
	return target_addr_;
}

DLLEXPORT Offset relocationEntry::rel_addr() const { 
	return rel_addr_;
}

DLLEXPORT const string &relocationEntry::name() const {
	return name_;
}

DLLEXPORT Symbol *relocationEntry::getDynSym() const {
    return dynref_;
}

DLLEXPORT bool relocationEntry::addDynSym(Symbol *dynref) {
    dynref_ = dynref;
    return true;
}

DLLEXPORT unsigned long relocationEntry::getRelType() const {
    return relType_;
}

DLLEXPORT Symbol::~Symbol ()
{
    	mangledNames.clear();
    	prettyNames.clear();
    	typedNames.clear();
}

DLLEXPORT Symbol::Symbol(const Symbol& s)
    : module_(s.module_), type_(s.type_), linkage_(s.linkage_),
    addr_(s.addr_), sec_(s.sec_), size_(s.size_), upPtr_(s.upPtr_), isInDynsymtab_(s.isInDynsymtab_), isInSymtab_(s.isInSymtab_), 
    mangledNames(s.mangledNames), prettyNames(s.prettyNames), typedNames(s.typedNames), tag_(s.tag_), retType_(s.retType_), 
    vars_(s.vars_), params_(s.params_){
}

DLLEXPORT Symbol& Symbol::operator=(const Symbol& s) {
    module_  = s.module_;
    type_    = s.type_;
    linkage_ = s.linkage_;
    addr_    = s.addr_;
    sec_     = s.sec_;
    size_    = s.size_;
    upPtr_ = s.upPtr_;
    isInDynsymtab_ = s.isInDynsymtab_;
    isInSymtab_ = s.isInSymtab_;
    tag_     = s.tag_;
    mangledNames = s.mangledNames;
    prettyNames = s.prettyNames;
    typedNames = s.typedNames;
    vars_ = s.vars_;
    params_ = s.params_;

    return *this;
}

DLLEXPORT const string& Symbol::getName() const {
    if(mangledNames.size() > 0)
    	return mangledNames[0];
    return emptyString;
}

DLLEXPORT const string& Symbol::getPrettyName() const {
    if(prettyNames.size() > 0)
    	return prettyNames[0];
    return emptyString;
}

DLLEXPORT const string& Symbol::getTypedName() const {
    if(typedNames.size() > 0)
    	return typedNames[0];
    return emptyString;
}

DLLEXPORT const string& Symbol::getModuleName() const {
    return module_->fullName();
}

DLLEXPORT Module* Symbol::getModule() const {
    return module_;
}

DLLEXPORT bool 	Symbol::setModule(Module *mod) {
	module_ = mod; 
	return true;
}

DLLEXPORT Symbol::SymbolType Symbol::getType() const {
    return type_;
}

DLLEXPORT Symbol::SymbolLinkage Symbol::getLinkage() const {
    return linkage_;
}

DLLEXPORT Offset Symbol::getAddr() const {
    return addr_;
}

DLLEXPORT Section *Symbol::getSec() const {
    return sec_;
}

DLLEXPORT void *Symbol::getUpPtr() const {
	return upPtr_;
}

DLLEXPORT bool Symbol::isInDynSymtab() const {
    return isInDynsymtab_;
}

DLLEXPORT bool Symbol::isInSymtab() const {
    return isInSymtab_;
}

DLLEXPORT unsigned Symbol::getSize() const {
    return size_;
}

DLLEXPORT bool	Symbol::setSize(unsigned ns){
	size_ = ns;
	return true;
}

DLLEXPORT Symbol::SymbolTag Symbol::tag() const {
    return tag_;
}

DLLEXPORT bool Symbol::setModuleName(string module)
{
	module_->setName(module);
	return true;
}

DLLEXPORT bool Symbol::setAddr (Offset newAddr) {
      addr_ = newAddr;
      return true;
}

DLLEXPORT bool Symbol::setDynSymtab() {
    isInDynsymtab_= true;
    return true;
}

DLLEXPORT bool Symbol::clearDynSymtab() {
    isInDynsymtab_ = false;
    return true;
}

DLLEXPORT bool Symbol::setIsInSymtab() {
    isInSymtab_= true;
    return true;
}

DLLEXPORT bool Symbol::clearIsInSymtab() {
    isInSymtab_= false;
    return true;
}

DLLEXPORT Symbol::Symbol()
   : //name_("*bad-symbol*"), module_("*bad-module*"),
    module_(NULL), type_(ST_UNKNOWN), linkage_(SL_UNKNOWN), addr_(0), sec_(NULL), size_(0),
    upPtr_(NULL), isInDynsymtab_(false), isInSymtab_(true), tag_(TAG_UNKNOWN), retType_(NULL), vars_(NULL), 
    params_(NULL){
   // note: this ctor is called surprisingly often (when we have
   // vectors of Symbols and/or dictionaries of Symbols).  So, make it fast.
}

DLLEXPORT const std::vector<string>& Symbol::getAllMangledNames() const {
    return mangledNames;
}

DLLEXPORT const std::vector<string>& Symbol::getAllPrettyNames() const {
	return prettyNames;
}		
    
DLLEXPORT const std::vector<string>& Symbol::getAllTypedNames() const {
    	return typedNames;
}

DLLEXPORT Symbol::Symbol(const string iname, const string imodule,
    SymbolType itype, SymbolLinkage ilinkage, Offset iaddr,
    Section *isec, unsigned size, void *upPtr, bool isInDynSymtab, bool isInSymtab)
    : type_(itype),
    linkage_(ilinkage), addr_(iaddr), sec_(isec), size_(size), upPtr_(upPtr), isInDynsymtab_(isInDynSymtab),
    isInSymtab_(isInSymtab), tag_(TAG_UNKNOWN), retType_(NULL), vars_(NULL), params_(NULL) {
    	module_ = new Module();
    	module_->setName(imodule);
    	mangledNames.push_back(iname);
}

DLLEXPORT Symbol::Symbol(const string iname, Module *mod,
    SymbolType itype, SymbolLinkage ilinkage, Offset iaddr,
    Section *isec, unsigned size, void *upPtr, bool isInDynSymtab, bool isInSymtab)
    : module_(mod), type_(itype),
    linkage_(ilinkage), addr_(iaddr), sec_(isec), size_(size), upPtr_(upPtr), isInDynsymtab_(isInDynSymtab), 
    isInSymtab_(isInSymtab), tag_(TAG_UNKNOWN), retType_(NULL), vars_(NULL), params_(NULL) {
    	mangledNames.push_back(iname);
}

DLLEXPORT bool Symbol::addMangledName(string name, bool isPrimary) {
    std::vector<string> newMangledNames;

    // isPrimary defaults to false
    if (isPrimary)
            newMangledNames.push_back(name);
    bool found = false;
    for (unsigned i = 0; i < mangledNames.size(); i++) {
        if (mangledNames[i] == name) {
        	found = true;
        }
        else {
		newMangledNames.push_back(mangledNames[i]);
	}
    }
    if (!isPrimary)
        newMangledNames.push_back(name);
    mangledNames = newMangledNames;

    if(!found && module_->exec())
    {		//add it to the lookUps
    	if(type_ == ST_FUNCTION)
	    	module_->exec()->addFunctionName(this,name,true);
	else if(type_ == ST_OBJECT)
		module_->exec()->addVariableName(this,name,true);
	else if(type_ == ST_MODULE)
		module_->exec()->addModuleName(this,name);
    }
    
    // Bool: true if the name is new; AKA !found
    return (!found);
}																	

DLLEXPORT bool Symbol::addPrettyName(string name, bool isPrimary) {
    std::vector<string> newPrettyNames;

    // isPrimary defaults to false
    if (isPrimary)
            newPrettyNames.push_back(name);
    bool found = false;
    for (unsigned i = 0; i < prettyNames.size(); i++) {
        if (prettyNames[i] == name) {
        	found = true;
        }
        else {
		newPrettyNames.push_back(prettyNames[i]);
	}
    }
    if (!isPrimary)
        newPrettyNames.push_back(name);
    prettyNames = newPrettyNames;
    
    if(!found && module_->exec())
    {		//add it to the lookUps
    	if(type_ == ST_FUNCTION)
	    	module_->exec()->addFunctionName(this,name,false);
	else if(type_ == ST_OBJECT)
		module_->exec()->addVariableName(this,name,false);
	else if(type_ == ST_MODULE)
		module_->exec()->addModuleName(this,name);
    }
    
    // Bool: true if the name is new; AKA !found
    return (!found);
}

DLLEXPORT bool Symbol::addTypedName(string name, bool isPrimary) {
    std::vector<string> newTypedNames;

    // isPrimary defaults to false
    if (isPrimary)
            newTypedNames.push_back(name);
    bool found = false;
    for (unsigned i = 0; i < typedNames.size(); i++) {
        if (typedNames[i] == name) {
        	found = true;
        }
        else {
		newTypedNames.push_back(typedNames[i]);
	}
    }
    if (!isPrimary)
        newTypedNames.push_back(name);
    typedNames = newTypedNames;
    
    if(!found && module_->exec())
    {		//add it to the lookUps
    	if(type_ == ST_FUNCTION)
	    	module_->exec()->addFunctionName(this,name,false);
	else if(type_ == ST_OBJECT)
		module_->exec()->addVariableName(this,name,false);
    }
    
    // Bool: true if the name is new; AKA !found
    return (!found);
}

DLLEXPORT bool Symbol::setSymbolType(SymbolType sType){
        if((sType != ST_UNKNOWN)&&
           (sType != ST_FUNCTION)&&
           (sType != ST_OBJECT)&&
           (sType != ST_MODULE)&&
           (sType != ST_NOTYPE))
	        return false;
	SymbolType oldType = type_;	
	type_ = sType;
	if(module_->exec())
		module_->exec()->changeType(this, oldType);
        return true;
}

DLLEXPORT bool	Symbol::setUpPtr(void *newUpPtr)
{
	upPtr_ = newUpPtr;
	return true;
}

DLLEXPORT Type *Symbol::getReturnType(){
	return retType_;
}

DLLEXPORT bool  Symbol::setReturnType(Type *retType){
	retType_ = retType;
	return true;
}

DLLEXPORT bool Symbol::getLocalVariables(std::vector<localVar *>&vars)
{
	if(type_ != ST_FUNCTION || !module_ )
		return false;
	module_->exec()->parseTypesNow();	
	if(!vars_)
		return false;
	vars = *(vars_->getAllVars());
	if(vars.size())
		return true;
	return false;	
}

DLLEXPORT bool Symbol::getParams(std::vector<localVar *>&params)
{
	if(type_ != ST_FUNCTION || !module_ )
		return false;
	module_->exec()->parseTypesNow();	
	if(!params_)
		return false;
	params = *(params_->getAllVars());
	if(params.size())
		return true;
	return false;	
}

DLLEXPORT bool Symbol::findLocalVariable(std::vector<localVar *>&vars, string name){
	if(type_ != ST_FUNCTION || !module_ )
		return false;
	module_->exec()->parseTypesNow();	
	if(!vars_ && !params_)
		return false;
	unsigned origSize = vars.size();	
	if(vars_)
	{
		localVar *var = vars_->findLocalVar(name);
		if(var)
			vars.push_back(var);
	}
	if(params_)
	{
		localVar *var = params_->findLocalVar(name);
		if(var)
			vars.push_back(var);
	}
	if(vars.size() > origSize)
		return true;
	return false;
}

ostream& Dyninst::SymtabAPI::operator<< (ostream &os, const Symbol &s) {
          return os << "{"
                  << " mangled=" << s.getName()
                  << " pretty="  << s.getPrettyName()
                  << " module="  << s.module_
                  << " type="    << (unsigned) s.type_
                  << " linkage=" << (unsigned) s.linkage_
                  << " addr="    << s.addr_
                  << " tag="     << (unsigned) s.tag_
                  << " }" << endl;
}

#ifdef DEBUG 

ostream & relocationEntry::operator<< (ostream &s) const {
    s << "target_addr_ = " << target_addr_ << endl;
    s << "rel_addr_ = " << rel_addr_ << endl;
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

DLLEXPORT AObject::AObject()
{
}	

DLLEXPORT unsigned AObject::nsymbols () const 
{ 
   return symbols_.size(); 
}
    
DLLEXPORT bool AObject::get_symbols(string & name, 
                                    std::vector<Symbol *> &symbols ) 
{
   if( symbols_.find(name) == symbols_.end()) {
      return false;
   }
   symbols = symbols_[name];
   return true;
}

//    Offset getLoadAddress() const { return loadAddress_; }
//    Offset getEntryAddress() const { return entryAddress_; }
//    Offset getBaseAddress() const { return baseAddress_; }

DLLEXPORT char* AObject::code_ptr () const 
{ 
   return code_ptr_; 
}
 
DLLEXPORT Offset AObject::code_off () const 
{ 
   return code_off_; 
}

DLLEXPORT Offset AObject::code_len () const 
{ 
   return code_len_; 
}

DLLEXPORT char* AObject::data_ptr () const 
{ 
   return data_ptr_; 
}

DLLEXPORT Offset AObject::data_off () const 
{ 
   return data_off_; 
}

DLLEXPORT Offset AObject::data_len () const 
{ 
   return data_len_; 
}

DLLEXPORT Offset AObject::code_vldS() const 
{ 
   return code_vldS_; 
}

DLLEXPORT Offset AObject::code_vldE () const
{ 
   return code_vldE_;
}

DLLEXPORT Offset AObject::data_vldS() const { 
   return data_vldS_; 
}

DLLEXPORT Offset AObject::data_vldE () const { 
   return data_vldE_; 
}

DLLEXPORT bool AObject::is_aout() const { 
   return is_aout_;  
}

DLLEXPORT unsigned AObject::no_of_sections() const 
{ 
   return no_of_sections_; 
}

DLLEXPORT unsigned AObject::no_of_symbols() const 
{ 
   return no_of_symbols_;  
}

DLLEXPORT bool AObject::getAllExceptions(std::vector<ExceptionBlock *>&excpBlocks) const
{
   for(unsigned i=0;i<catch_addrs_.size();i++)
      excpBlocks.push_back(new ExceptionBlock(catch_addrs_[i]));
   return true;
}

DLLEXPORT std::vector<Section *> AObject::getAllSections() const
{
   return sections_;	
}

DLLEXPORT Offset AObject::loader_off() const 
{ 
   return loader_off_; 
}

DLLEXPORT unsigned AObject::loader_len() const 
{ 
   return loader_len_; 
}

DLLEXPORT int AObject::getAddressWidth() const 
{ 
   return addressWidth_nbytes; 
}

DLLEXPORT bool AObject::have_deferred_parsing(void) const
{ 
   return deferredParse;
}

DLLEXPORT void * AObject::getErrFunc() const { 
   return (void *) err_func_; 
}

DLLEXPORT hash_map< string, std::vector< Symbol *> > *AObject::getAllSymbols() 
{ 
   return &(symbols_);
}

DLLEXPORT AObject::~AObject() 
{
}

// explicitly protected
DLLEXPORT AObject::AObject(const string file , void (*err_func)(const char *)) 
   : file_(file), code_ptr_(0), code_off_(0),
     code_len_(0), data_ptr_(0), data_off_(0), data_len_(0),loader_off_(0),
     loader_len_(0), deferredParse(false), err_func_(err_func),
     addressWidth_nbytes(4) 
{
}

DLLEXPORT AObject::AObject(const AObject &obj)
   : file_(obj.file_), symbols_(obj.symbols_), 
     code_ptr_(obj.code_ptr_), code_off_(obj.code_off_), 
     code_len_(obj.code_len_), data_ptr_(obj.data_ptr_), 
     data_off_(obj.data_off_), data_len_(obj.data_len_), 
     loader_off_(obj.loader_off_), loader_len_(obj.loader_len_),
     deferredParse(false), err_func_(obj.err_func_), addressWidth_nbytes(4)
{
} 

DLLEXPORT AObject& AObject::operator=(const AObject &obj) 
{   
   if (this == &obj) {
      return *this;
   }
   file_      = obj.file_; 	symbols_   = obj.symbols_;
   code_ptr_  = obj.code_ptr_; 	code_off_  = obj.code_off_;
   code_len_  = obj.code_len_; 	data_ptr_  = obj.data_ptr_;
   data_off_  = obj.data_off_; 	data_len_  = obj.data_len_;
   err_func_  = obj.err_func_;
   loader_off_ = obj.loader_off_; 
   loader_len_ = obj.loader_len_;
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

void SymbolIter::reset () {
   currentPositionInVector = 0;
   symbolIterator = symbols->begin();
}

SymbolIter::operator bool() const {
   return (symbolIterator!=symbols->end());
}
	
void SymbolIter::operator++ ( int ) 
{
   if( currentPositionInVector + 1 < (symbolIterator->second).size())
   {
      currentPositionInVector++;
      return;
   }
		
   /* Otherwise, we need a new std::vector. */
   currentPositionInVector = 0;			
   symbolIterator++;
}
	
const string & SymbolIter::currkey() const {
   return symbolIterator->first;
}
    
/* If it's important that this be const, we could try to initialize
   currentVector to '& symbolIterator.currval()' in the constructor. */
Symbol *SymbolIter::currval() {
   return ((symbolIterator->second)[ currentPositionInVector ]);
}
