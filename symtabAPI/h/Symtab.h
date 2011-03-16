/*
 * Copyright (c) 1996-2011 Barton P. Miller
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

#ifndef __SYMTAB_H__
#define __SYMTAB_H__

#include <set>

#include "Symbol.h"
#include "Module.h"
#include "Region.h"

#include "Annotatable.h"
#include "Serialization.h"

#include "ProcReader.h"

class MappedFile;

#define SYM_MAJOR 6
#define SYM_MINOR 2
#define SYM_BETA  1
 
namespace Dyninst {
namespace SymtabAPI {

class Archive;
class builtInTypeCollection;

class ExceptionBlock;
class Object;
class localVar;
class relocationEntry;
class Type;

typedef Dyninst::ProcessReader MemRegReader;

class Symtab : public LookupInterface,
               public Serializable,
               public AnnotatableSparse
{

   friend class Archive;
   friend class Symbol;
   friend class Function;
   friend class Variable;
   friend class Module;
   friend class Region;
   friend class emitElf;
   friend class emitElf64;
   friend class emitElfStatic;
   friend class emitWin;
   friend class Aggregate;
   friend class relocationEntry;

 public:

   /***** Public Member Functions *****/
   public:
   SYMTAB_EXPORT static void version(int& major, int& minor, int& maintenance);
   SYMTAB_EXPORT Symtab(MappedFile *);

   SYMTAB_EXPORT Symtab();

   SYMTAB_EXPORT Symtab(const Symtab& obj);
   SYMTAB_EXPORT Symtab(char *mem_image, size_t image_size, bool &err);

   SYMTAB_EXPORT static bool openFile(Symtab *&obj, std::string filename);
   SYMTAB_EXPORT static bool openFile(Symtab *&obj,char *mem_image, size_t size);
   SYMTAB_EXPORT static Symtab *findOpenSymtab(std::string filename);
   SYMTAB_EXPORT static bool closeSymtab(Symtab *);

   SYMTAB_EXPORT Serializable * serialize_impl(SerializerBase *sb, 
		   const char *tag = "Symtab") THROW_SPEC (SerializerError);
   void rebuild_symbol_hashes(SerializerBase *);
   void rebuild_funcvar_hashes(SerializerBase *);
   void rebuild_module_hashes(SerializerBase *);
   void rebuild_region_indexes(SerializerBase *) THROW_SPEC(SerializerError);
   static bool setup_module_up_ptrs(SerializerBase *,Symtab *st);
   static bool fixup_relocation_symbols(SerializerBase *,Symtab *st);

   SYMTAB_EXPORT bool exportXML(std::string filename);
   SYMTAB_EXPORT bool exportBin(std::string filename);
   static Symtab *importBin(std::string filename);
   SYMTAB_EXPORT bool getRegValueAtFrame(Address pc, 
                                     Dyninst::MachRegister reg, 
                                     Dyninst::MachRegisterVal &reg_result,
                                     MemRegReader *reader);
   SYMTAB_EXPORT bool hasStackwalkDebugInfo();

   /**************************************
    *** LOOKUP FUNCTIONS *****************
    **************************************/

   // Symbol

   SYMTAB_EXPORT virtual bool findSymbol(std::vector<Symbol *> &ret, 
                                         const std::string name,
                                         Symbol::SymbolType sType = Symbol::ST_UNKNOWN,
                                         NameType nameType = anyName,
                                         bool isRegex = false, 
                                         bool checkCase = false);
   SYMTAB_EXPORT virtual bool getAllSymbols(std::vector<Symbol *> &ret);
   SYMTAB_EXPORT virtual bool getAllSymbolsByType(std::vector<Symbol *> &ret, 
         Symbol::SymbolType sType);

   std::vector<Symbol *> *findSymbolByOffset(Offset);

   // Return all undefined symbols in the binary. Currently used for finding
   // the .o's in a static archive that have definitions of these symbols
   SYMTAB_EXPORT bool getAllUndefinedSymbols(std::vector<Symbol *> &ret);

   // Inversely, return all non-undefined symbols in the binary
   SYMTAB_EXPORT bool getAllDefinedSymbols(std::vector<Symbol *> &ret);

   // Function

   SYMTAB_EXPORT bool findFuncByEntryOffset(Function *&ret, const Offset offset);
   SYMTAB_EXPORT bool findFunctionsByName(std::vector<Function *> &ret, const std::string name,
                                          NameType nameType = anyName, 
                                          bool isRegex = false,
                                          bool checkCase = true);
   SYMTAB_EXPORT bool getAllFunctions(std::vector<Function *>&ret);
   SYMTAB_EXPORT bool getContainingFunction(Offset offset, Function* &func);

   // Variable
   SYMTAB_EXPORT bool findVariableByOffset(Variable *&ret, const Offset offset);
   SYMTAB_EXPORT bool findVariablesByName(std::vector<Variable *> &ret, const std::string name,
                                          NameType nameType = anyName, 
                                          bool isRegex = false, 
                                          bool checkCase = true);
   SYMTAB_EXPORT bool getAllVariables(std::vector<Variable *> &ret);

   // Module

   SYMTAB_EXPORT bool getAllModules(std::vector<Module *>&ret);
   SYMTAB_EXPORT bool findModuleByOffset(Module *&ret, Offset off);
   SYMTAB_EXPORT bool findModuleByName(Module *&ret, const std::string name);
   SYMTAB_EXPORT Module *getDefaultModule();

   // Region

   SYMTAB_EXPORT bool getCodeRegions(std::vector<Region *>&ret);
   SYMTAB_EXPORT bool getDataRegions(std::vector<Region *>&ret);
   SYMTAB_EXPORT bool getAllRegions(std::vector<Region *>&ret);
   SYMTAB_EXPORT bool getAllNewRegions(std::vector<Region *>&ret);
   //  change me to use a hash
   SYMTAB_EXPORT bool findRegion(Region *&ret, std::string regname);
   SYMTAB_EXPORT bool findRegion(Region *&ret, const Offset addr, const unsigned long size);
   SYMTAB_EXPORT bool findRegionByEntry(Region *&ret, const Offset offset);
   SYMTAB_EXPORT Region *findEnclosingRegion(const Offset offset);

   // Exceptions
   SYMTAB_EXPORT bool findException(ExceptionBlock &excp,Offset addr);
   SYMTAB_EXPORT bool getAllExceptions(std::vector<ExceptionBlock *> &exceptions);
   SYMTAB_EXPORT bool findCatchBlock(ExceptionBlock &excp, Offset addr, 
         unsigned size = 0);

   // Relocation entries
   SYMTAB_EXPORT bool getFuncBindingTable(std::vector<relocationEntry> &fbt) const;
   SYMTAB_EXPORT bool updateFuncBindingTable(Offset stub_addr, Offset plt_addr);

   /**************************************
    *** SYMBOL ADDING FUNCS **************
    **************************************/

   SYMTAB_EXPORT bool addSymbol(Symbol *newsym);
   SYMTAB_EXPORT bool addSymbol(Symbol *newSym, Symbol *referringSymbol);
   SYMTAB_EXPORT Function *createFunction(std::string name, Offset offset, size_t size, Module *mod = NULL);
   SYMTAB_EXPORT Variable *createVariable(std::string name, Offset offset, size_t size, Module *mod = NULL);

   SYMTAB_EXPORT bool deleteFunction(Function *func);
   SYMTAB_EXPORT bool deleteVariable(Variable *var);


   /*****Query Functions*****/
   SYMTAB_EXPORT bool isExec() const;
   SYMTAB_EXPORT bool isStripped();
   SYMTAB_EXPORT ObjectType getObjectType() const;
   SYMTAB_EXPORT Dyninst::Architecture getArchitecture();
   SYMTAB_EXPORT bool isCode(const Offset where) const;
   SYMTAB_EXPORT bool isData(const Offset where) const;
   SYMTAB_EXPORT bool isValidOffset(const Offset where) const;

   SYMTAB_EXPORT bool isNativeCompiler() const;
   SYMTAB_EXPORT bool getMappedRegions(std::vector<Region *> &mappedRegs) const;

   /***** Line Number Information *****/
   SYMTAB_EXPORT bool getAddressRanges(std::vector<std::pair<Offset, Offset> >&ranges,
         std::string lineSource, unsigned int LineNo);
   SYMTAB_EXPORT bool getSourceLines(std::vector<Statement *> &lines, 
         Offset addressInRange);
   SYMTAB_EXPORT bool getSourceLines(std::vector<LineNoTuple> &lines, 
                                     Offset addressInRange);
   SYMTAB_EXPORT bool addLine(std::string lineSource, unsigned int lineNo,
         unsigned int lineOffset, Offset lowInclAddr,
         Offset highExclAddr);
   SYMTAB_EXPORT bool addAddressRange(Offset lowInclAddr, Offset highExclAddr, std::string lineSource,
         unsigned int lineNo, unsigned int lineOffset = 0);
   SYMTAB_EXPORT void setTruncateLinePaths(bool value);
   SYMTAB_EXPORT bool getTruncateLinePaths();

   /***** Type Information *****/
   SYMTAB_EXPORT virtual bool findType(Type *&type, std::string name);
   SYMTAB_EXPORT virtual Type *findType(unsigned type_id);
   SYMTAB_EXPORT virtual bool findVariableType(Type *&type, std::string name);

   SYMTAB_EXPORT bool addType(Type *typ);

   SYMTAB_EXPORT static std::vector<Type *> *getAllstdTypes();
   SYMTAB_EXPORT static std::vector<Type *> *getAllbuiltInTypes();

   SYMTAB_EXPORT void parseTypesNow();

   /***** Local Variable Information *****/
   SYMTAB_EXPORT bool findLocalVariable(std::vector<localVar *>&vars, std::string name);

   /***** Relocation Sections *****/
   SYMTAB_EXPORT bool hasRel() const;
   SYMTAB_EXPORT bool hasRela() const;
   SYMTAB_EXPORT bool hasReldyn() const;
   SYMTAB_EXPORT bool hasReladyn() const;
   SYMTAB_EXPORT bool hasRelplt() const;
   SYMTAB_EXPORT bool hasRelaplt() const;
   
   SYMTAB_EXPORT bool isStaticBinary() const;

   /***** Write Back binary functions *****/
   SYMTAB_EXPORT bool emitSymbols(Object *linkedFile, std::string filename, unsigned flag = 0);
   SYMTAB_EXPORT bool addRegion(Offset vaddr, void *data, unsigned int dataSize, 
         std::string name, Region::RegionType rType_, bool loadable = false,
         unsigned long memAlign = sizeof(unsigned), bool tls = false);
   SYMTAB_EXPORT bool addRegion(Region *newreg);
   SYMTAB_EXPORT bool emit(std::string filename, unsigned flag = 0);

   SYMTAB_EXPORT void addDynLibSubstitution(std::string oldName, std::string newName);
   SYMTAB_EXPORT std::string getDynLibSubstitution(std::string name);

   SYMTAB_EXPORT bool getSegments(std::vector<Segment> &segs) const;
   
   SYMTAB_EXPORT void fixup_code_and_data(Offset newImageOffset,
                                          Offset newImageLength,
                                          Offset newDataOffset,
                                          Offset newDataLength);
   SYMTAB_EXPORT bool fixup_RegionAddr(const char* name, Offset memOffset, long memSize);
   SYMTAB_EXPORT bool fixup_SymbolAddr(const char* name, Offset newOffset);
   SYMTAB_EXPORT bool updateRegion(const char* name, void *buffer, unsigned size);
   SYMTAB_EXPORT bool updateCode(void *buffer, unsigned size);
   SYMTAB_EXPORT bool updateData(void *buffer, unsigned size);
   SYMTAB_EXPORT Offset getFreeOffset(unsigned size);

   SYMTAB_EXPORT bool addLibraryPrereq(std::string libname);
   SYMTAB_EXPORT bool addSysVDynamic(long name, long value);

   SYMTAB_EXPORT bool addLinkingResource(Archive *library);
   SYMTAB_EXPORT bool getLinkingResources(std::vector<Archive *> &libs);

   SYMTAB_EXPORT bool addExternalSymbolReference(Symbol *externalSym, Region *localRegion, relocationEntry localRel);

   /***** Data Member Access *****/
   SYMTAB_EXPORT std::string file() const;
   SYMTAB_EXPORT std::string name() const;
   SYMTAB_EXPORT std::string memberName() const;

   SYMTAB_EXPORT char *mem_image() const;

   SYMTAB_EXPORT Offset imageOffset() const;
   SYMTAB_EXPORT Offset dataOffset() const;
   SYMTAB_EXPORT Offset dataLength() const;
   SYMTAB_EXPORT Offset imageLength() const;
   //   SYMTAB_EXPORT char*  image_ptr ()  const;
   //   SYMTAB_EXPORT char*  data_ptr ()  const;
   SYMTAB_EXPORT Offset getInitOffset();
   SYMTAB_EXPORT Offset getFiniOffset();

   SYMTAB_EXPORT const char*  getInterpreterName() const;

   SYMTAB_EXPORT unsigned getAddressWidth() const;
   SYMTAB_EXPORT Offset getLoadOffset() const;
   SYMTAB_EXPORT Offset getEntryOffset() const;
   SYMTAB_EXPORT Offset getBaseOffset() const;
   SYMTAB_EXPORT Offset getTOCoffset() const;
   SYMTAB_EXPORT Address getLoadAddress();

   SYMTAB_EXPORT std::string getDefaultNamespacePrefix() const;

   SYMTAB_EXPORT unsigned getNumberofRegions() const;
   SYMTAB_EXPORT unsigned getNumberofSymbols() const;

   SYMTAB_EXPORT std::vector<std::string> &getDependencies();

   SYMTAB_EXPORT Archive *getParentArchive() const;

   /***** Error Handling *****/
   SYMTAB_EXPORT static SymtabError getLastSymtabError();
   SYMTAB_EXPORT static std::string printError(SymtabError serr);

   SYMTAB_EXPORT ~Symtab();

   bool delSymbol(Symbol *sym) { return deleteSymbol(sym); }
   bool deleteSymbol(Symbol *sym); 

   static builtInTypeCollection *builtInTypes;
   static typeCollection *stdTypes;

   Symbol *getSymbolByIndex(unsigned);
   protected:
   Symtab(std::string filename, std::string member_name, Offset offset, bool &err, void *base = NULL);
   Symtab(char *img, size_t size, std::string member_name, Offset offset, bool &err, void *base = NULL);

   /***** Private Member Functions *****/
   private:
   SYMTAB_EXPORT Symtab(std::string filename, bool &err); 

   SYMTAB_EXPORT bool extractInfo(Object *linkedFile);

   // Parsing code

   bool extractSymbolsFromFile(Object *linkedFile, std::vector<Symbol *> &raw_syms);
   bool fixSymModules(std::vector<Symbol *> &raw_syms);
   bool demangleSymbols(std::vector<Symbol *> &rawsyms);
   bool createIndices(std::vector<Symbol *> &raw_syms);
   bool createAggregates();

   bool fixSymModule(Symbol *&sym);
   bool demangleSymbol(Symbol *&sym);
   bool addSymbolToIndices(Symbol *&sym);
   bool addSymbolToAggregates(Symbol *&sym);
   bool doNotAggregate(Symbol *&sym);
   bool updateIndices(Symbol *sym, std::string newName, NameType nameType);


   void setModuleLanguages(dyn_hash_map<std::string, supportedLanguages> *mod_langs);

   void setupTypes();
   static void setupStdTypes();

   bool buildDemangledName( const std::string &mangled, 
         std::string &pretty,
         std::string &typed,
         bool nativeCompiler, 
         supportedLanguages lang );

   // Change the type of a symbol after the fact
   bool changeType(Symbol *sym, Symbol::SymbolType oldType);

   bool changeSymbolOffset(Symbol *sym, Offset newOffset);
   bool deleteSymbolFromIndices(Symbol *sym);

   bool changeAggregateOffset(Aggregate *agg, Offset oldOffset, Offset newOffset);
   bool deleteAggregate(Aggregate *agg);

   // Used by binaryEdit.C...
 public:
   SYMTAB_EXPORT bool canBeShared();
   SYMTAB_EXPORT Module *getOrCreateModule(const std::string &modName, 
                                           const Offset modAddr);

 public:
   //Only valid on ELF formats
   SYMTAB_EXPORT Offset getElfDynamicOffset();

 private:
   void createDefaultModule();

   Module *newModule(const std::string &name, const Offset addr, supportedLanguages lang);
   
   //bool buildFunctionLists(std::vector <Symbol *> &raw_funcs);
   //void enterFunctionInTables(Symbol *func, bool wasSymtab);


   bool addSymtabVariables();

   void checkPPC64DescriptorSymbols(Object *linkedFile);


   void parseLineInformation();
   void parseTypes();
   bool setDefaultNamespacePrefix(std::string &str);

   bool addUserRegion(Region *newreg);
   bool addUserType(Type *newtypeg);

   /***** Private Data Members *****/
   private:
   std::string member_name_;
   Offset member_offset_;
   Archive * parentArchive_;
   MappedFile *mf;
   MappedFile *mfForDebugInfo;

   Offset imageOffset_;
   unsigned imageLen_;
   Offset dataOffset_;
   unsigned dataLen_;

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
   std::vector<Segment> segments_;
   //  make sure is_a_out is set before calling symbolsToFunctions

   // A std::vector of all Symtabs. Used to avoid duplicating
   // a Symtab that already exists.
   static std::vector<Symtab *> allSymtabs;
   std::string defaultNamespacePrefix;

   //sections
   unsigned no_of_sections;
   std::vector<Region *> regions_;
   std::vector<Region *> codeRegions_;
   std::vector<Region *> dataRegions_;
   dyn_hash_map <Offset, Region *> regionsByEntryAddr;

   //Point where new loadable sections will be inserted
   unsigned newSectionInsertPoint;

   //symbols
   unsigned no_of_symbols;

   // Indices

   std::vector<Symbol *> everyDefinedSymbol;
   // Subset of the above
   std::vector<Symbol *> userAddedSymbols;
   // hashtable for looking up undefined symbols in the dynamic symbol
   // tale. Entries are referred by the relocation table entries
   // NOT a subset of everyDefinedSymbol
   std::map <std::string, std::vector<Symbol *> > undefDynSyms;

   // Symbols by offsets in the symbol table
   dyn_hash_map <Offset, std::vector<Symbol *> > symsByOffset;

   // The raw name from the symbol table
   dyn_hash_map <std::string, std::vector<Symbol *> > symsByMangledName;

   // The name after we've run it through the demangler
   dyn_hash_map <std::string, std::vector<Symbol *> > symsByPrettyName;

   // The name after we've derived the parameter types
   dyn_hash_map <std::string, std::vector<Symbol *> > symsByTypedName;

   // We also need per-Aggregate indices
   bool sorted_everyFunction;
   std::vector<Function *> everyFunction;
   // Since Functions are unique by address we require this structure to
   // efficiently track them.
   dyn_hash_map <Offset, Function *> funcsByOffset;

   // Similar for Variables
   std::vector<Variable *> everyVariable;
   dyn_hash_map <Offset, Variable *> varsByOffset;

   // For now, skip the index-by-name structures. We can use the Symbol
   // ones instead. 
   /*
   dyn_hash_map <std::string, std::vector<Function *> *> funcsByMangledName;
   dyn_hash_map <std::string, std::vector<Function *> *> funcsByPrettyName;
   dyn_hash_map <std::string, std::vector<Function *> *> funcsByTypedName;
   */

   //dyn_hash_map <Offset, std::vector<Function *> > funcsByEntryAddr;
   // note, a prettyName is not unique, it may map to a function appearing
   // in several modules.  Also only contains instrumentable functions....
   //dyn_hash_map <std::string, std::vector<Function *>*> funcsByPretty;
   // Hash table holding functions by mangled name.
   // Should contain same functions as funcsByPretty....
   //dyn_hash_map <std::string, std::vector<Function *>*> funcsByMangled;
   // A way to iterate over all the functions efficiently
   //std::vector<Symbol *> everyUniqueFunction;
   //std::vector<Function *> allFunctions;
   // And the counterpart "ones that are there right away"
   //std::vector<Symbol *> exportedFunctions;

   //dyn_hash_map <Address, Function *> funcsByAddr;
   dyn_hash_map <std::string, Module *> modsByFileName;
   dyn_hash_map <std::string, Module *> modsByFullName;
   std::vector<Module *> _mods;

   // Variables indexed by pretty (non-mangled) name
   /*
   dyn_hash_map <std::string, std::vector <Symbol *> *> varsByPretty;
   dyn_hash_map <std::string, std::vector <Symbol *> *> varsByMangled;
   dyn_hash_map <Offset, Symbol *> varsByAddr;
   std::vector<Symbol *> everyUniqueVariable;
   */

   //dyn_hash_map <std::string, std::vector <Symbol *> *> modsByName;
   //std::vector<Module *> _mods;


   std::vector<relocationEntry > relocation_table_;
   std::vector<ExceptionBlock *> excpBlocks;

   std::vector<std::string> deps_;

   // This set is used during static linking to satisfy dependencies
   std::vector<Archive *> linkingResources_;

   // This set represents Symtabs referenced by a new external Symbol
   bool getExplicitSymtabRefs(std::set<Symtab *> &refs);
   std::set<Symtab *> explicitSymtabRefs_;

   //Line Information valid flag;
   bool isLineInfoValid_;
   //type info valid flag
   bool isTypeInfoValid_;

   int nlines_;
   unsigned long fdptr_;
   char *lines_;
   char *stabstr_;
   int nstabs_;
   void *stabs_;
   char *stringpool_;

   //Relocation sections
   bool hasRel_;
   bool hasRela_;
   bool hasReldyn_;
   bool hasReladyn_;
   bool hasRelplt_;
   bool hasRelaplt_;

   bool isStaticBinary_;

   //Don't use obj_private, use getObject() instead.
 public:
   Object *getObject();
 private:
   Object *obj_private;

   // dynamic library name substitutions
   std::map <std::string, std::string> dynLibSubs;

   public:
   static Type *type_Error;
   static Type *type_Untyped;

 private:
    unsigned _ref_cnt;

 public:
   /********************************************************************/
   /**** DEPRECATED ****************************************************/
   /********************************************************************/
   dyn_hash_map <std::string, Module *> &getModsByFileName()
   {
      return modsByFileName;
   }
   dyn_hash_map <std::string, Module *> &getModsByFullName()
   {
      return modsByFullName;
   }
   
   SYMTAB_EXPORT bool findFuncByEntryOffset(std::vector<Symbol *>&ret, const Offset offset);
   SYMTAB_EXPORT virtual bool findSymbolByType(std::vector<Symbol *> &ret, 
                                               const std::string name,
                                               Symbol::SymbolType sType, 
                                               bool isMangled = false,
                                               bool isRegex = false, 
                                               bool checkCase = false);
};

/**
 * Used to represent something like a C++ try/catch block.  
 * Currently only used on Linux/x86
 **/
SYMTAB_EXPORT  std::ostream &operator<<(std::ostream &os, const ExceptionBlock &q);

class ExceptionBlock : public Serializable, public AnnotatableSparse {

   public:
	  SYMTAB_EXPORT Serializable * serialize_impl(SerializerBase *sb, 
			  const char *tag = "exceptionBlock") THROW_SPEC (SerializerError);
      SYMTAB_EXPORT ExceptionBlock(Offset tStart, unsigned tSize, Offset cStart);
      SYMTAB_EXPORT ExceptionBlock(Offset cStart);
      SYMTAB_EXPORT ExceptionBlock(const ExceptionBlock &eb);
      SYMTAB_EXPORT ~ExceptionBlock();
      SYMTAB_EXPORT ExceptionBlock();

      SYMTAB_EXPORT bool hasTry() const;
      SYMTAB_EXPORT Offset tryStart() const;
      SYMTAB_EXPORT Offset tryEnd() const;
      SYMTAB_EXPORT Offset trySize() const;
      SYMTAB_EXPORT Offset catchStart() const;
      SYMTAB_EXPORT bool contains(Offset a) const;

      friend SYMTAB_EXPORT std::ostream &operator<<(std::ostream &os, const ExceptionBlock &q);
   private:
      Offset tryStart_;
      unsigned trySize_;
      Offset catchStart_;
      bool hasTry_;
};

// relocation information for calls to functions not in this image
// on sparc-solaris: target_addr_ = rel_addr_ = PLT entry addr
// on x86-solaris: target_addr_ = PLT entry addr
//		   rel_addr_ =  GOT entry addr  corr. to PLT_entry
SYMTAB_EXPORT std::ostream &operator<<(std::ostream &os, const relocationEntry &q);

class relocationEntry : public Serializable, public AnnotatableSparse {
   public:

      SYMTAB_EXPORT relocationEntry();
      SYMTAB_EXPORT relocationEntry(Offset ta, Offset ra, Offset add, 
			  std::string n, Symbol *dynref = NULL, unsigned long relType = 0);
      SYMTAB_EXPORT relocationEntry(Offset ta, Offset ra, std::string n, 
			  Symbol *dynref = NULL, unsigned long relType = 0);
      SYMTAB_EXPORT relocationEntry(Offset ra, std::string n, Symbol *dynref = NULL, 
			  unsigned long relType = 0, Region::RegionType rtype = Region::RT_REL);
      SYMTAB_EXPORT relocationEntry(Offset ta, Offset ra, Offset add,
                          std::string n, Symbol *dynref = NULL, unsigned long relType = 0,
                          Region::RegionType rtype = Region::RT_REL);

      SYMTAB_EXPORT const relocationEntry& operator= (const relocationEntry &ra);

	  SYMTAB_EXPORT Serializable * serialize_impl(SerializerBase *sb, 
			  const char *tag = "relocationEntry") THROW_SPEC (SerializerError);

      SYMTAB_EXPORT Offset target_addr() const;
      SYMTAB_EXPORT Offset rel_addr() const;
      SYMTAB_EXPORT Offset addend() const;
      SYMTAB_EXPORT Region::RegionType regionType() const;
      SYMTAB_EXPORT const std::string &name() const;
      SYMTAB_EXPORT Symbol *getDynSym() const;
      SYMTAB_EXPORT bool addDynSym(Symbol *dynref);
      SYMTAB_EXPORT unsigned long getRelType() const;

      SYMTAB_EXPORT void setTargetAddr(const Offset);
      SYMTAB_EXPORT void setRelAddr(const Offset);
      SYMTAB_EXPORT void setAddend(const Offset);
      SYMTAB_EXPORT void setRegionType(const Region::RegionType);
      SYMTAB_EXPORT void setName(const std::string &newName);

      // dump output.  Currently setup as a debugging aid, not really
      //  for object persistance....
      //SYMTAB_EXPORT std::ostream & operator<<(std::ostream &s) const;
      friend SYMTAB_EXPORT std::ostream &operator<<(std::ostream &os, const relocationEntry &q);

      enum {pltrel = 1, dynrel = 2} relocationType;
      SYMTAB_EXPORT bool operator==(const relocationEntry &) const;

      // Architecture-specific functions
      SYMTAB_EXPORT static unsigned long getGlobalRelType(unsigned addressWidth);
      static const char *relType2Str(unsigned long r, unsigned addressWidth = sizeof(Address));

   private:
      Offset target_addr_;	// target address of call instruction 
      Offset rel_addr_;		// address of corresponding relocation entry 
      Offset addend_;       // addend (from RELA entries)
      Region::RegionType rtype_;        // RT_REL vs. RT_RELA
      std::string  name_;
      Symbol *dynref_;
      unsigned long relType_;
};

#if 1
#if 1
SYMTAB_EXPORT SerializerBase *nonpublic_make_bin_symtab_serializer(Symtab *t, std::string file);
SYMTAB_EXPORT SerializerBase *nonpublic_make_bin_symtab_deserializer(Symtab *t, std::string file);
SYMTAB_EXPORT void nonpublic_free_bin_symtab_serializer(SerializerBase *sb);
#else

template <class T>
SerializerBase *nonpublic_make_bin_serializer(T *t, std::string file)
{
	SerializerBin<T> *ser;
	ser = new SerializerBin<T>(t, "SerializerBin", file, sd_serialize, true);
	T *test_st = ser->getScope();
	assert(test_st == t);
	return ser;
}

template <class T>
SerializerBase *nonpublic_make_bin_deserializer(T *t, std::string file)
{
	SerializerBin<T> *ser;
	ser = new SerializerBin<T>(t, "DeserializerBin", file, sd_deserialize, true);
	T *test_st = ser->getScope();
	assert(test_st == t);
	return ser;
}

template <class T>
void nonpublic_free_bin_serializer(SerializerBase *sb)
{
	SerializerBin<T> *sbin = dynamic_cast<SerializerBin<T> *>(sb);
	if (sbin)
	{
		delete(sbin);
	}
	else
		fprintf(stderr, "%s[%d]:  FIXME\n", FILE__, __LINE__);

}
#endif
#endif

}//namespace SymtabAPI

}//namespace Dyninst
#endif
