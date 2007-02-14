/*
 * Copyright (c) 1996-2004 Barton P. Miller
 * 
 * We provide the Paradyn Parallel Performance Tools (below
 * described as "Paradyn") on an AS IS basis, and do not warrant its
 * validity or performance.  We reserve the right to update, modify,
 * or discontinue this software at any time.  We shall have no
 * obligation to supply such updates or modifications or any other
 * form of support to you.
 * 
 * This license is for research uses.  For such uses, there is no
 * charge. We define "research use" to mean you may freely use it
 * inside your organization for whatever purposes you see fit. But you
 * may not re-distribute Paradyn or parts of Paradyn, in any form
 * source or binary (including derivatives), electronic or otherwise,
 * to any other organization or entity without our permission.
 * 
 * (for other uses, please contact us at paradyn@cs.wisc.edu)
 * 
 * All warranties, including without limitation, any warranty of
 * merchantability or fitness for a particular purpose, are hereby
 * excluded.
 * 
 * By your use of Paradyn, you understand and agree that we (or any
 * other person or entity with proprietary rights in Paradyn) are
 * under no obligation to provide either maintenance services,
 * update services, notices of latent defects, or correction of
 * defects for Paradyn.
 * 
 * Even if advised of the possibility of such damages, under no
 * circumstances shall we (or any other person or entity with
 * proprietary rights in the software licensed hereunder) be liable
 * to you or any third party for direct, indirect, or consequential
 * damages of any character regardless of type of action, including,
 * without limitation, loss of profits, loss of use, loss of good
 * will, or computer failure or malfunction.  You agree to indemnify
 * us (and any other person or entity with proprietary rights in the
 * software licensed hereunder) for any and all liability it may
 * incur to third parties resulting from your use of Paradyn.
 */

// $Id: Object.C,v 1.4 2007/02/14 23:03:56 legendre Exp $

#include "symtabAPI/src/Object.h"
#include "symtabAPI/h/Dyn_Symtab.h"

string Dyn_Symbol::emptyString(string(""));

bool symbol_compare(const Dyn_Symbol *s1, const Dyn_Symbol *s2) {
    // select the symbol with the lowest address
    Address s1_addr = s1->getAddr();
    Address s2_addr = s2->getAddr();
    if (s1_addr > s2_addr)
    	return false;
    if (s1_addr < s2_addr)
    	return true;

    // symbols are co-located at the same address
    // select the symbol which is not a function
    if ((s1->getType() != Dyn_Symbol::ST_FUNCTION) && (s2->getType() == Dyn_Symbol::ST_FUNCTION))
    	return true;
    if ((s2->getType() != Dyn_Symbol::ST_FUNCTION) && (s1->getType() == Dyn_Symbol::ST_FUNCTION))
    	return false;
    
    // symbols are both functions
    // select the symbol which has GLOBAL linkage
    if ((s1->getLinkage() == Dyn_Symbol::SL_GLOBAL) && (s2->getLinkage() != Dyn_Symbol::SL_GLOBAL))
    	return true;
    if ((s2->getLinkage() == Dyn_Symbol::SL_GLOBAL) && (s1->getLinkage() != Dyn_Symbol::SL_GLOBAL))
    	return false;
	
    // neither function is GLOBAL
    // select the symbol which has LOCAL linkage
    if ((s1->getLinkage() == Dyn_Symbol::SL_LOCAL) && (s2->getLinkage() != Dyn_Symbol::SL_LOCAL))
    	return true;
    if ((s2->getLinkage() == Dyn_Symbol::SL_LOCAL) && (s1->getLinkage() != Dyn_Symbol::SL_LOCAL))
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

bool AObject::get_func_binding_table(vector<relocationEntry> &) const {
    return false;
}

bool AObject::get_func_binding_table_ptr(const vector<relocationEntry> *&) const {
    return false;
}

char *AObject::mem_image() const
{
	return NULL;
}

DLLEXPORT Dyn_ExceptionBlock::~Dyn_ExceptionBlock() {}

DLLEXPORT Dyn_ExceptionBlock::Dyn_ExceptionBlock() : tryStart_(0), trySize_(0), 
								catchStart_(0), hasTry_(false) {}

DLLEXPORT OFFSET Dyn_ExceptionBlock::catchStart() const { 
	return catchStart_;
}

DLLEXPORT relocationEntry::~relocationEntry() {}

DLLEXPORT relocationEntry::relocationEntry(const relocationEntry& ra): target_addr_(ra.target_addr_), 
    											rel_addr_(ra.rel_addr_), name_(ra.name_) {}

DLLEXPORT OFFSET relocationEntry::target_addr() const { 
	return target_addr_;
}

DLLEXPORT OFFSET relocationEntry::rel_addr() const { 
	return rel_addr_;
}

DLLEXPORT const string &relocationEntry::name() const {
	return name_;
}

DLLEXPORT Dyn_Symbol::~Dyn_Symbol ()
{
    	mangledNames.clear();
    	prettyNames.clear();
    	typedNames.clear();
}

DLLEXPORT Dyn_Symbol::Dyn_Symbol(const Dyn_Symbol& s)
    : module_(s.module_), type_(s.type_), linkage_(s.linkage_),
    addr_(s.addr_), sec_(s.sec_), size_(s.size_), upPtr_(s.upPtr_), mangledNames(s.mangledNames), prettyNames(s.prettyNames),
    typedNames(s.typedNames), tag_(s.tag_){
}

DLLEXPORT Dyn_Symbol& Dyn_Symbol::operator=(const Dyn_Symbol& s) {
    module_  = s.module_;
    type_    = s.type_;
    linkage_ = s.linkage_;
    addr_    = s.addr_;
    sec_     = s.sec_;
    size_    = s.size_;
    upPtr_ = s.upPtr_;
    tag_     = s.tag_;
    mangledNames = s.mangledNames;
    prettyNames = s.prettyNames;
    typedNames = s.typedNames;

    return *this;
}

DLLEXPORT const string& Dyn_Symbol::getName() const {
    if(mangledNames.size() > 0)
    	return mangledNames[0];
    return emptyString;
}

DLLEXPORT const string& Dyn_Symbol::getPrettyName() const {
    if(prettyNames.size() > 0)
    	return prettyNames[0];
    return emptyString;
}

DLLEXPORT const string& Dyn_Symbol::getTypedName() const {
    if(typedNames.size() > 0)
    	return typedNames[0];
    return emptyString;
}

DLLEXPORT const string& Dyn_Symbol::getModuleName() const {
    return module_->fullName();
}

DLLEXPORT Dyn_Module* Dyn_Symbol::getModule() const {
    return module_;
}

DLLEXPORT Dyn_Symbol::SymbolType Dyn_Symbol::getType() const {
    return type_;
}

DLLEXPORT Dyn_Symbol::SymbolLinkage Dyn_Symbol::getLinkage() const {
    return linkage_;
}

DLLEXPORT Address Dyn_Symbol::getAddr() const {
    return addr_;
}

DLLEXPORT Dyn_Section *Dyn_Symbol::getSec() const {
    return sec_;
}

DLLEXPORT void *Dyn_Symbol::getUpPtr() const {
	return upPtr_;
}

DLLEXPORT unsigned Dyn_Symbol::getSize() const {
    return size_;
}

DLLEXPORT Dyn_Symbol::SymbolTag Dyn_Symbol::tag() const {
    return tag_;
}

DLLEXPORT void Dyn_Symbol::setModuleName(string module)
{
	module_->setName(module);
}

DLLEXPORT void Dyn_Symbol::setAddr (Address newAddr) {
      addr_ = newAddr;
}

DLLEXPORT Dyn_Symbol::Dyn_Symbol()
   : //name_("*bad-symbol*"), module_("*bad-module*"),
    module_(NULL), type_(ST_UNKNOWN), linkage_(SL_UNKNOWN), addr_(0), sec_(NULL), size_(0),
    upPtr_(NULL), tag_(TAG_UNKNOWN) {
   // note: this ctor is called surprisingly often (when we have
   // vectors of Symbols and/or dictionaries of Symbols).  So, make it fast.
}

DLLEXPORT const vector<string>& Dyn_Symbol::getAllMangledNames() const {
    return mangledNames;
}

DLLEXPORT const vector<string>& Dyn_Symbol::getAllPrettyNames() const {
	return prettyNames;
}		
    
DLLEXPORT const vector<string>& Dyn_Symbol::getAllTypedNames() const {
    	return typedNames;
}

DLLEXPORT Dyn_Symbol::Dyn_Symbol(const string& iname, const string& imodule,
    SymbolType itype, SymbolLinkage ilinkage, Address iaddr,
    Dyn_Section *isec, unsigned size, void *upPtr)
    : type_(itype),
    linkage_(ilinkage), addr_(iaddr), 
    sec_(isec), size_(size), upPtr_(upPtr), tag_(TAG_UNKNOWN) {
    	module_ = new Dyn_Module();
	module_->setName(imodule);
    	mangledNames.push_back(iname);
}

DLLEXPORT Dyn_Symbol::Dyn_Symbol(const string& iname, Dyn_Module *mod,
    SymbolType itype, SymbolLinkage ilinkage, Address iaddr,
    Dyn_Section *isec, unsigned size, void *upPtr)
    : module_(mod), type_(itype),
    linkage_(ilinkage), addr_(iaddr), 
    sec_(isec), size_(size), upPtr_(upPtr), tag_(TAG_UNKNOWN) {
    	mangledNames.push_back(iname);
}

DLLEXPORT bool Dyn_Symbol::addMangledName(string name, bool isPrimary) {
    vector<string> newMangledNames;

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
    }
    
    // Bool: true if the name is new; AKA !found
    return (!found);
}																	

DLLEXPORT bool Dyn_Symbol::addPrettyName(string name, bool isPrimary) {
    vector<string> newPrettyNames;

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
    }
    
    // Bool: true if the name is new; AKA !found
    return (!found);
}

DLLEXPORT bool Dyn_Symbol::addTypedName(string name, bool isPrimary) {
    vector<string> newTypedNames;

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

DLLEXPORT bool Dyn_Symbol::setType(SymbolType sType){
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

DLLEXPORT void	Dyn_Symbol::setUpPtr(void *newUpPtr)
{
	upPtr_ = newUpPtr;
	return;
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
    Dyn_Symbol sym;
	hash_map<string, vector <Dyn_Symbol> >::iterator symbols_iter = symbols_.begin();

    s << "Debugging Info for AObject (address) : " << this << endl;

    s << " file_ = " << file_ << endl;
    s << " symbols_ = " << endl;

    // and loop over all the symbols, printing symbol name and data....
    //  or try at least....
	for(;symbols_iter=symbols_.end();symbols_iter++)
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
                                    vector<Dyn_Symbol *> &symbols ) 
{
   if( symbols_.find(name) == symbols_.end()) {
      return false;
   }
   symbols = symbols_[name];
   return true;
}

//    OFFSET getLoadAddress() const { return loadAddress_; }
//    OFFSET getEntryAddress() const { return entryAddress_; }
//    OFFSET getBaseAddress() const { return baseAddress_; }

DLLEXPORT Word* AObject::code_ptr () const 
{ 
   return code_ptr_; 
}
 
DLLEXPORT OFFSET AObject::code_off () const 
{ 
   return code_off_; 
}

DLLEXPORT OFFSET AObject::code_len () const 
{ 
   return code_len_; 
}

DLLEXPORT Word* AObject::data_ptr () const 
{ 
   return data_ptr_; 
}

DLLEXPORT OFFSET AObject::data_off () const 
{ 
   return data_off_; 
}

DLLEXPORT OFFSET AObject::data_len () const 
{ 
   return data_len_; 
}

DLLEXPORT OFFSET AObject::code_vldS() const 
{ 
   return code_vldS_; 
}

DLLEXPORT OFFSET AObject::code_vldE () const
{ 
   return code_vldE_;
}

DLLEXPORT OFFSET AObject::data_vldS() const { 
   return data_vldS_; 
}

DLLEXPORT OFFSET AObject::data_vldE () const { 
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

DLLEXPORT bool AObject::getAllExceptions(vector<Dyn_ExceptionBlock *>&excpBlocks) const
{
   for(unsigned i=0;i<catch_addrs_.size();i++)
      excpBlocks.push_back(new Dyn_ExceptionBlock(catch_addrs_[i]));
   return true;
}

DLLEXPORT vector<Dyn_Section *> AObject::getAllSections() const
{
   return sections_;	
}

DLLEXPORT OFFSET AObject::loader_off() const 
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

DLLEXPORT hash_map< string, vector< Dyn_Symbol *> > *AObject::getAllSymbols() 
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
		
   /* Otherwise, we need a new vector. */
   currentPositionInVector = 0;			
   symbolIterator++;
}
	
const string & SymbolIter::currkey() const {
   return symbolIterator->first;
}
    
/* If it's important that this be const, we could try to initialize
   currentVector to '& symbolIterator.currval()' in the constructor. */
Dyn_Symbol *SymbolIter::currval() {
   return ((symbolIterator->second)[ currentPositionInVector ]);
}

