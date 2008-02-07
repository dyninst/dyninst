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

#ifndef Symtab_h
#define Symtab_h
 
#include <sys/types.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <vector>
#include <string>

#include "Symbol.h"
#include "LineInformation.h"
#include "dynutil/h/Annotatable.h"

typedef struct {} user_funcs_a;
typedef struct {} user_sections_a;
typedef struct {} user_types_a;
typedef struct {} user_symbols_a;
typedef struct {} module_line_info_a;
typedef struct {} module_type_info_a;

class MappedFile;

namespace Dyninst{
namespace SymtabAPI{

class Archive;
class builtInTypeCollection;

class Symtab;
class ExceptionBlock;
class Object;
class relocationEntry;

//class lineDict;

 
class LookupInterface {
 public:
  	DLLEXPORT LookupInterface();
	DLLEXPORT virtual bool getAllSymbolsByType(std::vector<Symbol *> &ret, 
                                              Symbol::SymbolType sType) = 0;
	DLLEXPORT virtual bool findSymbolByType(std::vector<Symbol *> &ret, 
                                           const std::string name,
                                           Symbol::SymbolType sType, 
                                           bool isMangled = false,
                                           bool isRegex = false, 
                                           bool checkCase = false) = 0;
	DLLEXPORT virtual bool findType(Type *&type, std::string name) = 0;
	DLLEXPORT virtual bool findVariableType(Type *&type, std::string name)= 0;

	DLLEXPORT virtual ~LookupInterface();
};
 
class Module : public LookupInterface,
               public Annotatable<LineInformation *, module_line_info_a>,
               public Annotatable<typeCollection *, module_type_info_a> {
 public:
    DLLEXPORT Module();
    DLLEXPORT Module(supportedLanguages lang, Offset adr, std::string fullNm,
                            Symtab *img);
    DLLEXPORT Module(const Module &mod);
    DLLEXPORT bool operator==(const Module &mod) const;
    
    DLLEXPORT const std::string &fileName() const;
    DLLEXPORT const std::string &fullName() const;
    DLLEXPORT bool setName(std::string newName);
    
    DLLEXPORT supportedLanguages language() const;
    DLLEXPORT void setLanguage(supportedLanguages lang);
    
    DLLEXPORT Offset addr() const;
    DLLEXPORT Symtab *exec() const;
    
    DLLEXPORT virtual bool getAllSymbolsByType(std::vector<Symbol *> &ret, 
                                            Symbol::SymbolType sType);
    DLLEXPORT virtual bool findSymbolByType(std::vector<Symbol *> &ret, 
                                           const std::string name,
                                           Symbol::SymbolType sType, 
                                           bool isMangled = false,
                                           bool isRegex = false, 
                                           bool checkCase = false);
	
    DLLEXPORT bool isShared() const;
    DLLEXPORT ~Module();
    
    /***** Type Information *****/
    DLLEXPORT virtual bool findType(Type *&type, std::string name);
    DLLEXPORT virtual bool findVariableType(Type *&type, std::string name);

    DLLEXPORT std::vector<Type *> *getAllTypes();
    DLLEXPORT std::vector<std::pair<std::string, Type *> > *getAllGlobalVars();
    DLLEXPORT typeCollection *getModuleTypes();

    /***** Local Variable Information *****/
    DLLEXPORT bool findLocalVariable(std::vector<localVar *>&vars, std::string name);

    /***** Line Number Information *****/
	DLLEXPORT bool getAddressRanges(std::vector<std::pair<Offset, Offset> >&ranges,
					std::string lineSource, unsigned int LineNo);
    DLLEXPORT bool getSourceLines(std::vector<LineInformationImpl::LineNoTuple> &lines, Offset addressInRange);
    DLLEXPORT bool setLineInfo(LineInformation *lineInfo);
    DLLEXPORT LineInformation *getLineInformation();
    DLLEXPORT bool hasLineInformation();
    DLLEXPORT bool setDefaultNamespacePrefix(std::string str);

private:
    std::string fileName_;                   // short file 
    std::string fullName_;                   // full path to file 
    supportedLanguages language_;
    Offset addr_;                      // starting address of module
    Symtab *exec_;
#if 0
    LineInformation *lineInfo_;
    typeCollection *moduleTypes_;	//type information
#endif
};
 


class Symtab : public LookupInterface,
              public Annotatable<Symbol *, user_funcs_a>, 
              public Annotatable<Section *, user_sections_a>, 
              public Annotatable<Type *, user_types_a>, 
              public Annotatable<Symbol *, user_symbols_a> 
{
    
    friend class Archive;
    friend class Symbol;
    friend class Module;
    friend class emitElf;
    friend class emitElf64;
	 
   /***** Public Member Functions *****/
 public:
   	DLLEXPORT Symtab();

	DLLEXPORT Symtab(const Symtab& obj);
	
   DLLEXPORT static bool openFile(Symtab *&obj, std::string filename);
#if 0  
	DLLEXPORT static bool openFile(Symtab *&obj,char *mem_image, size_t size);
#endif
	
	DLLEXPORT bool exportXML(std::string filename);

	/***** Lookup Functions *****/
	DLLEXPORT virtual bool findSymbolByType(std::vector<Symbol *> &ret, 
                                           const std::string name,
                                           Symbol::SymbolType sType, 
                                           bool isMangled = false,
                                           bool isRegex = false, 
                                           bool checkCase = false);
	
	DLLEXPORT bool findFuncByEntryOffset(std::vector<Symbol *>&ret, const Offset offset);
	
	DLLEXPORT virtual bool getAllSymbolsByType(std::vector<Symbol *> &ret, 
                                              Symbol::SymbolType sType);
	DLLEXPORT bool getAllModules(std::vector<Module *>&ret);
	DLLEXPORT bool getAllSections(std::vector<Section *>&ret);
	DLLEXPORT bool getAllNewSections(std::vector<Section *>&ret);
	
	DLLEXPORT bool findModule(Module *&ret, const std::string name);
	DLLEXPORT bool findSection(Section *&ret, std::string secname);
	DLLEXPORT bool findSectionByEntry(Section *&ret, const Offset offset);

	DLLEXPORT bool addSymbol(Symbol *newsym, bool isDynamic = false);
    DLLEXPORT bool addSymbol(Symbol *newSym, Symbol *referringSymbol);
	
	DLLEXPORT bool findException(ExceptionBlock &excp,Offset addr);
	DLLEXPORT bool getAllExceptions(std::vector<ExceptionBlock *> &exceptions);
	DLLEXPORT bool findCatchBlock(ExceptionBlock &excp, Offset addr, 
                                 unsigned size = 0);

	DLLEXPORT bool getFuncBindingTable(std::vector<relocationEntry> &fbt) const;
	
	/*****Query Functions*****/
	DLLEXPORT bool isExec() const;
    DLLEXPORT bool isStripped();
    DLLEXPORT ObjectType getObjectType() const;
 
	DLLEXPORT bool isCode(const Offset where) const;
	DLLEXPORT bool isData(const Offset where) const;
	DLLEXPORT bool isValidOffset(const Offset where) const;

	DLLEXPORT bool isNativeCompiler() const;
    DLLEXPORT bool getMappedRegions(std::vector<Region> &mappedregs) const;
	
	/***** Line Number Information *****/
	DLLEXPORT bool getAddressRanges(std::vector<std::pair<Offset, Offset> >&ranges,
						std::string lineSource, unsigned int LineNo);
	DLLEXPORT bool getSourceLines(std::vector<LineInformationImpl::LineNoTuple> &lines, Offset addressInRange);
	DLLEXPORT bool addLine(std::string lineSource, unsigned int lineNo,
					unsigned int lineOffset, Offset lowInclAddr,
					Offset highExclAddr);
	DLLEXPORT bool addAddressRange(Offset lowInclAddr, Offset highExclAddr, std::string lineSource,
					unsigned int lineNo, unsigned int lineOffset = 0);
	
	/***** Type Information *****/
	DLLEXPORT virtual bool findType(Type *&type, std::string name);
	DLLEXPORT virtual bool findVariableType(Type *&type, std::string name);

	DLLEXPORT bool addType(Type *typ);
    	
	DLLEXPORT static std::vector<Type *> *getAllstdTypes();
	DLLEXPORT static std::vector<Type *> *getAllbuiltInTypes();
 	
	DLLEXPORT void parseTypesNow();

	/***** Local Variable Information *****/
	DLLEXPORT bool findLocalVariable(std::vector<localVar *>&vars, std::string name);

	/***** Write Back binary functions *****/
	DLLEXPORT bool emitSymbols(Object *linkedFile, std::string filename, unsigned flag = 0);
	DLLEXPORT bool addSection(Offset vaddr, void *data, unsigned int dataSize, std::string name, unsigned long flags, bool loadable = false);
	DLLEXPORT bool addSection(Section *newScn);
	DLLEXPORT bool emit(std::string filename, unsigned flag = 0);

	DLLEXPORT bool getSegments(std::vector<Segment> &segs) const;
	DLLEXPORT bool updateCode(void *buffer, unsigned size);
	DLLEXPORT bool updateData(void *buffer, unsigned size);
	DLLEXPORT Offset getFreeOffset(unsigned size);
	
	/***** Data Member Access *****/
	DLLEXPORT std::string file() const;
	DLLEXPORT std::string name() const;

	DLLEXPORT char *mem_image() const;
	
	DLLEXPORT Offset imageOffset() const;
	DLLEXPORT Offset dataOffset() const;
	DLLEXPORT Offset dataLength() const;
   DLLEXPORT Offset imageLength() const;
   DLLEXPORT char*  image_ptr ()  const;
   DLLEXPORT char*  data_ptr ()  const;

   DLLEXPORT const char*  getInterpreterName() const;

   DLLEXPORT unsigned getAddressWidth() const;
   DLLEXPORT Offset getLoadOffset() const;
   DLLEXPORT Offset getEntryOffset() const;
   DLLEXPORT Offset getBaseOffset() const;
   DLLEXPORT Offset getTOCoffset() const;

   DLLEXPORT std::string getDefaultNamespacePrefix() const;
	
	DLLEXPORT unsigned getNumberofSections() const;
   DLLEXPORT unsigned getNumberofSymbols() const;

#if defined (os_aix)
      //  These will go away.... (get_stab_info) and get_line_info, etc
	void get_stab_info(char *&stabstr, int &nstabs, void *&stabs, 
                      char *&stringpool) const;
	void get_line_info(int& nlines, char*& lines,unsigned long& fdptr) const;

	void get_stab_info(Object *linkedFile, char *&stabstr, int &nstabs, void *&stabs, 
                      char *&stringpool) const;
	void get_line_info(Object *linkedFile, int& nlines, char*& lines,unsigned long& fdptr) const;
   int nlines_;
   unsigned long fdptr_;
   char *lines_;
   char *stabstr_;
   int nstabs_;
   void *stabs_;
   char *stringpool_;
#endif
   
	/***** Error Handling *****/
	DLLEXPORT static SymtabError getLastSymtabError();
	DLLEXPORT static std::string printError(SymtabError serr);

	DLLEXPORT ~Symtab();
	
	void addFunctionName(Symbol *func,
                        const std::string newName,
                        bool isMangled /*=false*/);
	void addVariableName(Symbol *var,
                        const std::string newName,
                        bool isMangled /*=false*/);
	void addModuleName(Symbol *mod,
                        const std::string newName);

	bool delSymbol(Symbol *sym); 
	
	static builtInTypeCollection *builtInTypes;
	static typeCollection *stdTypes;
	
 protected:
   Symtab(std::string &filename, std::string &member_name, Offset offset, bool &err);
#if 0 
	Symtab(char *mem_image, size_t size, std::string &member_name, 
              Offset offset, bool &err);
#endif

   /***** Private Member Functions *****/
 private:
	DLLEXPORT Symtab(std::string &filename, bool &err); 
#if 0 
	DLLEXPORT Symtab(char *mem_image, size_t image_size, bool &err);
#endif
	DLLEXPORT bool extractInfo(Object *linkedFile);

	void setupTypes();
	static void setupStdTypes();

	bool buildDemangledName( const std::string &mangled, 
                            std::string &pretty,
                            std::string &typed,
                            bool nativeCompiler, 
                            supportedLanguages lang );
	bool symbolsToFunctions(Object *linkedFile, std::vector<Symbol *> *raw_funcs);
	bool changeType(Symbol *sym, Symbol::SymbolType oldType);
			       
	void setModuleLanguages(hash_map<std::string, supportedLanguages> *mod_langs);
	Module *getOrCreateModule(const std::string &modName, 
                                 const Offset modAddr);
	Module *newModule(const std::string &name, const Offset addr, supportedLanguages lang);
	bool buildFunctionLists(std::vector <Symbol *> &raw_funcs);
	void enterFunctionInTables(Symbol *func, bool wasSymtab);
	bool addSymtabVariables();
	bool addSymbolInt(Symbol *newsym, bool from_user, bool isDynamic = false);
	
	bool findFunction(std::vector <Symbol *> &ret, const std::string &name, 
                     bool isMangled=false, bool isRegex = false,
                     bool checkCase = false);
	bool findVariable(std::vector <Symbol *> &ret, const std::string &name,
                     bool isMangled=false, bool isRegex = false,
                     bool checkCase = false);
	bool findMod(std::vector <Symbol *> &ret, const std::string &name, 
                     bool isMangled=false, bool isRegex = false,
                     bool checkCase = false);
	bool findFuncVectorByPretty(const std::string &name, std::vector<Symbol *> &ret);
	bool findFuncVectorByMangled(const std::string &name, std::vector<Symbol *> &ret);
	bool findVarVectorByPretty(const std::string &name, std::vector<Symbol *> &ret);
	bool findVarVectorByMangled(const std::string &name, std::vector<Symbol *> &ret);

	bool findFuncVectorByMangledRegex(const std::string &rexp, bool checkCase,
                                     std::vector<Symbol *>&ret);
	bool findFuncVectorByPrettyRegex(const std::string &rexp, bool checkCase,
                                    std::vector<Symbol *>&ret);
	bool findVarVectorByMangledRegex(const std::string &rexp, bool checkCase,
                                    std::vector<Symbol *>&ret);
	bool findVarVectorByPrettyRegex(const std::string &rexp, bool checkCase,
                                   std::vector<Symbol *>&ret);
	bool findModByRegex(const std::string &rexp, bool checkCase,
                                     std::vector<Symbol *>&ret);
	bool getAllFunctions(std::vector<Symbol *> &ret);
	bool getAllVariables(std::vector<Symbol *> &ret);
	bool getAllSymbols(std::vector<Symbol *> &ret);

	void checkPPC64DescriptorSymbols(Object *linkedFile);
	   

	void parseLineInformation();
	void parseTypes();
   bool setDefaultNamespacePrefix(std::string &str);

   /***** Private Data Members *****/
 private:
   std::string member_name_;
   MappedFile *mf;

   Offset imageOffset_;
   unsigned imageLen_;
   Offset dataOffset_;
   unsigned dataLen_;

   Offset codeValidStart_;
   Offset codeValidEnd_;
   Offset dataValidStart_;
   Offset dataValidEnd_;

   bool is_a_out;
   Offset main_call_addr_; // address of call to main()

   bool nativeCompiler;

   unsigned address_width_;
   char *code_ptr_;
   char *data_ptr_;
   std::string interpreter_name_;
   Offset entry_address_;
   Offset base_address_;
   Offset load_address_;
   Offset toc_offset_;
   ObjectType object_type_;
   bool is_eel_;
   std::vector<Region> mapped_regions_;
   std::vector<Segment> segments_;
   //  make sure is_a_out is set before calling symbolsToFunctions

   // A std::vector of all Symtabs. Used to avoid duplicating
   // a Symtab that already exists.
   static std::vector<Symtab *> allSymtabs;
   std::string defaultNamespacePrefix;
	
   //sections
   unsigned no_of_sections;
   std::vector<Section *> sections_;
   hash_map <Offset, Section *> secsByEntryAddr;

   //Point where new loadable sections will be inserted
   unsigned newSectionInsertPoint;
	
   //symbols
   unsigned no_of_symbols;
	
   hash_map <Offset, std::vector<Symbol *> > funcsByEntryAddr;
	// note, a prettyName is not unique, it may map to a function appearing
   // in several modules.  Also only contains instrumentable functions....
   hash_map <std::string, std::vector<Symbol *>*> funcsByPretty;
   // Hash table holding functions by mangled name.
   // Should contain same functions as funcsByPretty....
   hash_map <std::string, std::vector<Symbol *>*> funcsByMangled;
   // A way to iterate over all the functions efficiently
   std::vector<Symbol *> everyUniqueFunction;
   // And the counterpart "ones that are there right away"
   std::vector<Symbol *> exportedFunctions;

   hash_map <std::string, Module *> modsByFileName;
   hash_map <std::string, Module *> modsByFullName;
   	
	// Variables indexed by pretty (non-mangled) name
   hash_map <std::string, std::vector <Symbol *> *> varsByPretty;
   hash_map <std::string, std::vector <Symbol *> *> varsByMangled;
   hash_map <Offset, Symbol *> varsByAddr;
   std::vector<Symbol *> everyUniqueVariable;
   std::vector<Symbol *> modSyms;
   hash_map <std::string, std::vector <Symbol *> *> modsByName;
   std::vector<Symbol *> notypeSyms;
   std::vector<Module *> _mods;

   // hashtable for looking up undefined symbols in the dynamic symbol
   // tale. Entries are referred by the relocation table entries
   std::map <std::string, Symbol *> undefDynSyms;
   std::vector<relocationEntry > relocation_table_;
   std::vector<ExceptionBlock *> excpBlocks;
    
    //Line Information valid flag;
    bool isLineInfoValid_;
    //type info valid flag
    bool isTypeInfoValid_;

public:
    Type *type_Error;
    Type *type_Untyped;
	    
};

/**
 * Used to represent something like a C++ try/catch block.  
 * Currently only used on Linux/x86
 **/
class ExceptionBlock {
 public:
   DLLEXPORT ExceptionBlock(Offset tStart, unsigned tSize, Offset cStart);
   DLLEXPORT ExceptionBlock(Offset cStart);
   DLLEXPORT ExceptionBlock(const ExceptionBlock &eb);
	DLLEXPORT ~ExceptionBlock();
	DLLEXPORT ExceptionBlock();

   DLLEXPORT bool hasTry() const;
   DLLEXPORT Offset tryStart() const;
   DLLEXPORT Offset tryEnd() const;
   DLLEXPORT Offset trySize() const;
	DLLEXPORT Offset catchStart() const;
   DLLEXPORT bool contains(Offset a) const;

 private:
   Offset tryStart_;
   unsigned trySize_;
   Offset catchStart_;
   bool hasTry_;
};
 
class Section {
   public:
      DLLEXPORT Section();
      DLLEXPORT Section(unsigned sidnumber, std::string sname, Offset saddr, 
            unsigned long ssize, void *secPtr, 
            unsigned long sflags = 0, bool isLoadable = false);
      DLLEXPORT Section(unsigned sidnumber, std::string sname, unsigned long ssize,
            void *secPtr, unsigned long sflags= 0, bool isLoadable = false);
      DLLEXPORT Section(const Section &sec);
      DLLEXPORT Section& operator=(const Section &sec);
      DLLEXPORT std::ostream& operator<< (std::ostream &os);
      DLLEXPORT bool operator== (const Section &sec);

      DLLEXPORT ~Section();

      DLLEXPORT unsigned getSecNumber() const;
      DLLEXPORT bool setSecNumber(unsigned sidnumber);
      DLLEXPORT std::string getSecName() const;
      DLLEXPORT Offset getSecAddr() const;
      DLLEXPORT void *getPtrToRawData() const;
      DLLEXPORT bool setPtrToRawData(void *, unsigned long); 
      DLLEXPORT unsigned long getSecSize() const;
      DLLEXPORT bool isBSS() const;
      DLLEXPORT bool isText() const;
      DLLEXPORT bool isData() const;
      DLLEXPORT unsigned getFlags() const;
      DLLEXPORT bool isOffsetInSection(const Offset &offset) const;
      DLLEXPORT bool isLoadable() const;
      DLLEXPORT unsigned long flags() const;
      DLLEXPORT bool isDirty() const;
      DLLEXPORT std::vector<relocationEntry> &getRelocations();
      DLLEXPORT bool patchData(Offset off, void *buf, unsigned size);

      DLLEXPORT bool addRelocationEntry(Offset relocationAddr, Symbol *dynref, unsigned long relType);

      enum {textSection = 1, dataSection = 2,
         relocationSection = 4, symtabSection = 8,
         stringSection = 16, dynsymtabSection = 32,
         dynamicSection = 64, versymSection = 128,
         verneedSection = 256, verdefSection = 512 } sectionType;

   private:	
      unsigned sidnumber_;
      std::string sname_;
      Offset saddr_;
      unsigned long ssize_;
      void *rawDataPtr_;
      unsigned long sflags_;  //holds the type of section(text/data/bss/except etc)
      bool isLoadable_;
      bool isDirty_;
      std::vector<relocationEntry> rels_;
      void *buffer_;
};

// relocation information for calls to functions not in this image
// on sparc-solaris: target_addr_ = rel_addr_ = PLT entry addr
// on x86-solaris: target_addr_ = PLT entry addr
//		   rel_addr_ =  GOT entry addr  corr. to PLT_entry
class relocationEntry {
   public:
      DLLEXPORT relocationEntry();
      DLLEXPORT relocationEntry(Offset ta,Offset ra, std::string n, Symbol *dynref = NULL, unsigned long relType = 0);
      DLLEXPORT relocationEntry(Offset ra, std::string n, Symbol *dynref = NULL, unsigned long relType = 0);

      DLLEXPORT relocationEntry(const relocationEntry& ra);

      DLLEXPORT const relocationEntry& operator= (const relocationEntry &ra);
      DLLEXPORT Offset target_addr() const;
      DLLEXPORT Offset rel_addr() const;
      DLLEXPORT const std::string &name() const;
      DLLEXPORT Symbol *getDynSym() const;
      DLLEXPORT bool addDynSym(Symbol *dynref);
      DLLEXPORT unsigned long getRelType() const;

      // dump output.  Currently setup as a debugging aid, not really
      //  for object persistance....
      DLLEXPORT std::ostream & operator<<(std::ostream &s) const;
      friend std::ostream &operator<<(std::ostream &os, relocationEntry &q);

      enum {pltrel = 1, dynrel = 2} relocationType;

   private:
      Offset target_addr_;	// target address of call instruction 
      Offset rel_addr_;		// address of corresponding relocation entry 
      std::string  name_;
      Symbol *dynref_;
      unsigned long relType_;
};

}//namespace SymtabAPI

}//namespace Dyninst
#endif
