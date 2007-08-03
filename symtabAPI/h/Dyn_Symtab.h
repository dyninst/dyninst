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

#ifndef Dyn_Symtab_h
#define Dyn_Symtab_h
 
#include <sys/types.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <vector>
#include <string>

#include "util.h"
#include "Dyn_Symbol.h"

typedef unsigned long OFFSET;

class Dyn_Archive;
typedef enum { lang_Unknown,
               lang_Assembly,
               lang_C,
               lang_CPlusPlus,
               lang_GnuCPlusPlus,
               lang_Fortran,
               lang_Fortran_with_pretty_debug,
               lang_CMFortran
} supportedLanguages;

#if 0 
 // To be used when flags will be used instead of booleans to specify paramaters for lookups
 #define IS_MANGLED 		4
 #define IS_REGEX   		2
 #define IS_CASE_SENSITIVE 	1
 #define IS_PRETTY		0
 #define NOT_REGEX		0
 #define NOT_CASE_SENSITIVE	0
#endif

typedef enum { Obj_Parsing,
               Syms_To_Functions,
               Build_Function_Lists,
               No_Such_Function,
               No_Such_Variable,
               No_Such_Module,
               No_Such_Section,
               No_Such_Symbol,
               No_Such_Member,
               Not_A_File,
               Not_An_Archive,
	       Export_Error, 
               Invalid_Flags
} SymtabError;

typedef enum {
   obj_Unknown,
   obj_SharedLib,
   obj_Executable
} ObjectType;

class Dyn_Symtab;
class Dyn_ExceptionBlock;
class Object;
class relocationEntry;

//class lineDict;

 
class Dyn_LookupInterface {
 public:
  	DLLEXPORT Dyn_LookupInterface();
	DLLEXPORT virtual bool getAllSymbolsByType(vector<Dyn_Symbol *> &ret, 
                                              Dyn_Symbol::SymbolType sType) = 0;
	DLLEXPORT virtual bool findSymbolByType(vector<Dyn_Symbol *> &ret, 
                                           const string &name,
                                           Dyn_Symbol::SymbolType sType, 
                                           bool isMangled = false,
                                           bool isRegex = false, 
                                           bool checkCase = false) = 0;
	DLLEXPORT virtual ~Dyn_LookupInterface();
};
 
class Dyn_Module : public Dyn_LookupInterface {
 public:
   DLLEXPORT Dyn_Module();
   DLLEXPORT Dyn_Module(supportedLanguages lang, OFFSET adr, string fullNm,
                        Dyn_Symtab *img);
	DLLEXPORT Dyn_Module(const Dyn_Module &mod);
	DLLEXPORT bool operator==(const Dyn_Module &mod) const;
   DLLEXPORT const string &fileName() const;
   DLLEXPORT const string &fullName() const;
	DLLEXPORT bool setName(string newName);
   DLLEXPORT supportedLanguages language() const;
   DLLEXPORT void setLanguage(supportedLanguages lang);
   DLLEXPORT OFFSET addr() const;
	DLLEXPORT Dyn_Symtab *exec() const;
	
	DLLEXPORT virtual bool getAllSymbolsByType(vector<Dyn_Symbol *> &ret, 
                                              Dyn_Symbol::SymbolType sType);
	DLLEXPORT virtual bool findSymbolByType(vector<Dyn_Symbol *> &ret, 
                                           const string &name,
                                           Dyn_Symbol::SymbolType sType, 
                                           bool isMangled = false,
                                           bool isRegex = false, 
                                           bool checkCase = false);
	
   DLLEXPORT bool isShared() const;

	//bool parseFileLineInfo();
	//OFFSET& getAddrFromLine(unsigned linenum) const;
	DLLEXPORT ~Dyn_Module();

 private:
   string fileName_;                   // short file 
   string fullName_;                   // full path to file 
   supportedLanguages language_;
   OFFSET addr_;                      // starting address of module
	Dyn_Symtab *exec_;
	//lineDict *lineInfo;		    // address - line number mapping for the module
};
 
class Dyn_Symtab : public Dyn_LookupInterface {
	 
   /***** Public Member Functions *****/
 public:
   DLLEXPORT Dyn_Symtab();

	DLLEXPORT Dyn_Symtab(const Dyn_Symtab& obj);
	
   	DLLEXPORT static bool openFile(string &filename, Dyn_Symtab *&obj);
	
	DLLEXPORT static bool openFile(char *mem_image, 
                                  size_t size, 
                                  Dyn_Symtab *&obj);
	
	DLLEXPORT bool exportXML(string filename);
	DLLEXPORT bool emitSymbols(string filename);

	/***** Lookup Functions *****/
	DLLEXPORT virtual bool findSymbolByType(vector<Dyn_Symbol *> &ret, 
                                           const string &name,
                                           Dyn_Symbol::SymbolType sType, 
                                           bool isMangled = false,
                                           bool isRegex = false, 
                                           bool checkCase = false);
	DLLEXPORT bool findFuncByEntryOffset(const OFFSET &offset, vector<Dyn_Symbol *>&ret);	

	DLLEXPORT bool findModule(const string &name, Dyn_Module *&ret);
	DLLEXPORT bool findSection(const string &secName, Dyn_Section *&ret);
	DLLEXPORT bool findSectionByEntry(const OFFSET &offset, Dyn_Section *&ret);

	DLLEXPORT bool addSymbol(Dyn_Symbol *newsym);

	DLLEXPORT virtual bool getAllSymbolsByType(vector<Dyn_Symbol *> &ret, 
                                              Dyn_Symbol::SymbolType sType);
	
	DLLEXPORT bool getAllModules(vector<Dyn_Module *>&ret);
	DLLEXPORT bool getAllSections(vector<Dyn_Section *>&ret);
	
	DLLEXPORT bool findException(OFFSET &addr, Dyn_ExceptionBlock &excp);
	DLLEXPORT bool getAllExceptions(vector<Dyn_ExceptionBlock *> &exceptions);
	DLLEXPORT bool findCatchBlock(Dyn_ExceptionBlock &excp, OFFSET addr, 
                                 unsigned size = 0);

	DLLEXPORT bool getFuncBindingTable(vector<relocationEntry> &fbt) const;
	
	DLLEXPORT bool isExec() const;
 
	DLLEXPORT bool isCode(const OFFSET &where) const;
	DLLEXPORT bool isData(const OFFSET &where) const;
	DLLEXPORT bool isValidOffset(const OFFSET &where) const;

	DLLEXPORT bool isNativeCompiler() const;
	
	/***** Data Member Access *****/
	DLLEXPORT string file() const;
	DLLEXPORT string name() const;

	DLLEXPORT char *  mem_image() const;
	
	DLLEXPORT OFFSET codeOffset() const;
	DLLEXPORT OFFSET dataOffset() const;
	DLLEXPORT OFFSET dataLength() const;
	DLLEXPORT OFFSET codeLength() const;
   DLLEXPORT void*  code_ptr ()  const;
   DLLEXPORT void*  data_ptr ()  const;

   DLLEXPORT const char*  getInterpreterName() const;

	DLLEXPORT unsigned getAddressWidth() const;
	DLLEXPORT OFFSET getLoadAddress() const;
	DLLEXPORT OFFSET getEntryAddress() const;
	DLLEXPORT OFFSET getBaseAddress() const;
	DLLEXPORT OFFSET getTOCoffset() const;
	
	DLLEXPORT unsigned getNumberofSections() const;
   DLLEXPORT unsigned getNumberofSymbols() const;
   
   DLLEXPORT ObjectType getObjectType() const;

	void get_stab_info(char *&stabstr, int &nstabs, void *&stabs, 
                      char *&stringpool) const;
	void get_line_info(int& nlines, char*& lines,unsigned long& fdptr) const;
   
	/***** Error Handling *****/
	DLLEXPORT static SymtabError getLastSymtabError();
	DLLEXPORT static string printError(SymtabError serr);

	DLLEXPORT ~Dyn_Symtab();
	
	void addFunctionName(Dyn_Symbol *func,
                        const string newName,
                        bool isMangled /*=false*/);
	void addVariableName(Dyn_Symbol *var,
                        const string newName,
                        bool isMangled /*=false*/);
	void addModuleName(Dyn_Symbol *mod,
                        const string newName);

	bool delSymbol(Dyn_Symbol *sym); 
	
 protected:
   Dyn_Symtab(string &filename, string &member_name, OFFSET offset, bool &err);
	Dyn_Symtab(char *mem_image, size_t size, string &member_name, 
              OFFSET offset, bool &err);

   /***** Private Member Functions *****/
 private:
	DLLEXPORT Dyn_Symtab(string &filename, bool &err); 
	DLLEXPORT Dyn_Symtab(char *mem_image, size_t image_size, bool &err);
	DLLEXPORT bool extractInfo();

	bool buildDemangledName( const string &mangled, 
                            string &pretty,
                            string &typed,
                            bool nativeCompiler, 
                            supportedLanguages lang );
	bool symbolsToFunctions(vector<Dyn_Symbol *> *raw_funcs);
	bool changeType(Dyn_Symbol *sym, Dyn_Symbol::SymbolType oldType);
			       
	void setModuleLanguages(hash_map<string, supportedLanguages> *mod_langs);
	Dyn_Module *getOrCreateModule(const string &modName, 
                                 const OFFSET modAddr);
	Dyn_Module *newModule(const string &name, const OFFSET addr, supportedLanguages lang);
	bool buildFunctionLists(vector <Dyn_Symbol *> &raw_funcs);
	void enterFunctionInTables(Dyn_Symbol *func, bool wasSymtab);
	bool addSymtabVariables();
	
	bool findFunction(vector <Dyn_Symbol *> &ret, const string &name, 
                     bool isMangled=false, bool isRegex = false,
                     bool checkCase = false);
	bool findVariable(vector <Dyn_Symbol *> &ret, const string &name,
                     bool isMangled=false, bool isRegex = false,
                     bool checkCase = false);
	bool findMod(vector <Dyn_Symbol *> &ret, const string &name, 
                     bool isMangled=false, bool isRegex = false,
                     bool checkCase = false);
	bool findFuncVectorByPretty(const string &name, vector<Dyn_Symbol *> &ret);
	bool findFuncVectorByMangled(const string &name, vector<Dyn_Symbol *> &ret);
	bool findVarVectorByPretty(const string &name, vector<Dyn_Symbol *> &ret);
	bool findVarVectorByMangled(const string &name, vector<Dyn_Symbol *> &ret);

	bool findFuncVectorByMangledRegex(const string &rexp, bool checkCase,
                                     vector<Dyn_Symbol *>&ret);
	bool findFuncVectorByPrettyRegex(const string &rexp, bool checkCase,
                                    vector<Dyn_Symbol *>&ret);
	bool findVarVectorByMangledRegex(const string &rexp, bool checkCase,
                                    vector<Dyn_Symbol *>&ret);
	bool findVarVectorByPrettyRegex(const string &rexp, bool checkCase,
                                   vector<Dyn_Symbol *>&ret);
	bool findModByRegex(const string &rexp, bool checkCase,
                                     vector<Dyn_Symbol *>&ret);
	bool getAllFunctions(vector<Dyn_Symbol *> &ret);
	bool getAllVariables(vector<Dyn_Symbol *> &ret);
	bool getAllSymbols(vector<Dyn_Symbol *> &ret);
	
	void checkPPC64DescriptorSymbols();

   /***** Private Data Members *****/
 private:
   string filename_;
	string member_name_;
	string name_;

   OFFSET codeOffset_;
   unsigned codeLen_;
   OFFSET dataOffset_;
   unsigned dataLen_;

   OFFSET codeValidStart_;
   OFFSET codeValidEnd_;
   OFFSET dataValidStart_;
   OFFSET dataValidEnd_;

   bool is_a_out;
   OFFSET main_call_addr_; // address of call to main()

   bool nativeCompiler;

   // data from the symbol table 
   Object *linkedFile;

	// A vector of all Dyn_Symtabs. Used to avoid duplicating
   // a Dyn_Symtab that already exists.
	static vector<Dyn_Symtab *> allSymtabs;
	
	//sections
	unsigned no_of_sections;
	vector<Dyn_Section *> sections_;
	hash_map <OFFSET, Dyn_Section *> secsByEntryAddr;
	
	//symbols
	unsigned no_of_symbols;
	
   hash_map <OFFSET, vector<Dyn_Symbol *> > funcsByEntryAddr;
	// note, a prettyName is not unique, it may map to a function appearing
   // in several modules.  Also only contains instrumentable functions....
   hash_map <string, vector<Dyn_Symbol *>*> funcsByPretty;
   // Hash table holding functions by mangled name.
   // Should contain same functions as funcsByPretty....
   hash_map <string, vector<Dyn_Symbol *>*> funcsByMangled;
   // A way to iterate over all the functions efficiently
   vector<Dyn_Symbol *> everyUniqueFunction;
   // We make an initial list of functions based off the symbol table,
   // and may create more when we actually analyze. Keep track of
   // those created ones so we can distinguish them if necessary
   vector<Dyn_Symbol *> createdFunctions;
   // And the counterpart "ones that are there right away"
   vector<Dyn_Symbol *> exportedFunctions;

	hash_map <string, Dyn_Module *> modsByFileName;
   hash_map <string, Dyn_Module *> modsByFullName;
   	
	// Variables indexed by pretty (non-mangled) name
   hash_map <string, vector <Dyn_Symbol *> *> varsByPretty;
   hash_map <string, vector <Dyn_Symbol *> *> varsByMangled;
   hash_map <OFFSET, Dyn_Symbol *> varsByAddr;
   vector<Dyn_Symbol *> everyUniqueVariable;
   //vector<Dyn_Symbol *> createdVariables;
   //vector<Dyn_Symbol *> exportedVariables;
   vector<Dyn_Symbol *> modSyms;
   hash_map <string, vector <Dyn_Symbol *> *> modsByName;
   vector<Dyn_Symbol *> notypeSyms;
	vector<Dyn_Module *> _mods;

	//vector<relocationEntry *> relocation_table_;
	vector<Dyn_ExceptionBlock *> excpBlocks;
 private:
  	friend class Dyn_Archive;
  	friend class Dyn_Symbol;
};

/**
 * Used to represent something like a C++ try/catch block.  
 * Currently only used on Linux/x86
 **/
class Dyn_ExceptionBlock {
 public:
   DLLEXPORT Dyn_ExceptionBlock(OFFSET tStart, unsigned tSize, OFFSET cStart);
   DLLEXPORT Dyn_ExceptionBlock(OFFSET cStart);
   DLLEXPORT Dyn_ExceptionBlock(const Dyn_ExceptionBlock &eb);
	DLLEXPORT ~Dyn_ExceptionBlock();
	DLLEXPORT Dyn_ExceptionBlock();

   DLLEXPORT bool hasTry() const;
   DLLEXPORT OFFSET tryStart() const;
   DLLEXPORT OFFSET tryEnd() const;
   DLLEXPORT OFFSET trySize() const;
	DLLEXPORT OFFSET catchStart() const;
   DLLEXPORT bool contains(OFFSET a) const;

 private:
   OFFSET tryStart_;
   unsigned trySize_;
   OFFSET catchStart_;
   bool hasTry_;
};
 
class Dyn_Section {
 public:
  	DLLEXPORT Dyn_Section();
  	DLLEXPORT Dyn_Section(unsigned sidnumber, string sname, OFFSET saddr, 
                         unsigned long ssize, void *secPtr, 
                         unsigned long sflags = 0);
	DLLEXPORT Dyn_Section(unsigned sidnumber, string sname, unsigned long ssize,
                         void *secPtr, unsigned long sflags= 0);
	DLLEXPORT Dyn_Section(const Dyn_Section &sec);
	DLLEXPORT Dyn_Section& operator=(const Dyn_Section &sec);
	DLLEXPORT ostream& operator<< (ostream &os);
	DLLEXPORT bool operator== (const Dyn_Section &sec);

	DLLEXPORT ~Dyn_Section();
	
 	DLLEXPORT unsigned getSecNumber() const;
	DLLEXPORT string getSecName() const;
	DLLEXPORT OFFSET getSecAddr() const;
	DLLEXPORT void *getPtrToRawData() const;
	DLLEXPORT unsigned long getSecSize() const;
	DLLEXPORT bool isBSS() const;
	DLLEXPORT bool isText() const;
	DLLEXPORT bool isData() const;
	DLLEXPORT bool isOffsetInSection(const OFFSET &offset) const;

 private:	
	unsigned sidnumber_;
	string sname_;
	OFFSET saddr_;
	unsigned long ssize_;
	void *rawDataPtr_;
	unsigned long sflags_;  //holds the type of section(text/data/bss/except etc)
};

// relocation information for calls to functions not in this image
// on sparc-solaris: target_addr_ = rel_addr_ = PLT entry addr
// on x86-solaris: target_addr_ = PLT entry addr
//		   rel_addr_ =  GOT entry addr  corr. to PLT_entry
class relocationEntry {
public:
   DLLEXPORT relocationEntry();
   DLLEXPORT relocationEntry(OFFSET ta,OFFSET ra, string n);
   DLLEXPORT relocationEntry(const relocationEntry& ra);
   DLLEXPORT ~relocationEntry();
   
   DLLEXPORT const relocationEntry& operator= (const relocationEntry &ra);
   DLLEXPORT OFFSET target_addr() const;
   DLLEXPORT OFFSET rel_addr() const;
   DLLEXPORT const string &name() const;
   
   // dump output.  Currently setup as a debugging aid, not really
   //  for object persistance....
   DLLEXPORT ostream & operator<<(ostream &s) const;
   friend ostream &operator<<(ostream &os, relocationEntry &q);
   
private:
   OFFSET target_addr_;	// target address of call instruction 
   OFFSET rel_addr_;		// address of corresponding relocation entry 
   string  name_;
};

#endif
