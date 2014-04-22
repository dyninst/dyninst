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

#ifndef __SYMTAB_H__
#define __SYMTAB_H__

#include <set>

#include "Symbol.h"
#include "Module.h"
#include "Region.h"

#include "Annotatable.h"
#include "Serialization.h"
#include "ProcReader.h"
#include "IBSTree.h"

#include "boost/shared_ptr.hpp"

class MappedFile;

#define SYM_MAJOR 8
#define SYM_MINOR 2
#define SYM_BETA  0
 
namespace Dyninst {

   struct SymSegment;

namespace SymtabAPI {

class Archive;
class builtInTypeCollection;

class ExceptionBlock;
class Object;
class localVar;
class relocationEntry;
class Type;
class FunctionBase;
class FuncRange;

typedef IBSTree<FuncRange> FuncRangeLookup;
typedef Dyninst::ProcessReader MemRegReader;

class SYMTAB_EXPORT Symtab : public LookupInterface,
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
   static void version(int& major, int& minor, int& maintenance);
   Symtab(MappedFile *);

   Symtab();

   Symtab(const Symtab& obj);
   Symtab(unsigned char *mem_image, size_t image_size, 
                        const std::string &name, bool defensive_binary, bool &err);

   typedef enum {
      NotDefensive,
      Defensive} def_t; 

   static bool openFile(Symtab *&obj, std::string filename, 
                                      def_t defensive_binary = NotDefensive);
   static bool openFile(Symtab *&obj, void *mem_image, size_t size, 
                                      std::string name, def_t defensive_binary = NotDefensive);
   static Symtab *findOpenSymtab(std::string filename);
   static bool closeSymtab(Symtab *);

   Serializable * serialize_impl(SerializerBase *sb, 
		   const char *tag = "Symtab") THROW_SPEC (SerializerError);
   void rebuild_symbol_hashes(SerializerBase *);
   void rebuild_funcvar_hashes(SerializerBase *);
   void rebuild_module_hashes(SerializerBase *);
   void rebuild_region_indexes(SerializerBase *) THROW_SPEC(SerializerError);
   static bool setup_module_up_ptrs(SerializerBase *,Symtab *st);
   static bool fixup_relocation_symbols(SerializerBase *,Symtab *st);

   bool exportXML(std::string filename);
   bool exportBin(std::string filename);
   static Symtab *importBin(std::string filename);
   bool getRegValueAtFrame(Address pc, 
                                     Dyninst::MachRegister reg, 
                                     Dyninst::MachRegisterVal &reg_result,
                                     MemRegReader *reader);
   bool hasStackwalkDebugInfo();

   /**************************************
    *** LOOKUP FUNCTIONS *****************
    **************************************/

   // Symbol

   virtual bool findSymbol(std::vector<Symbol *> &ret, 
                                         const std::string& name,
                                         Symbol::SymbolType sType = Symbol::ST_UNKNOWN,
                                         NameType nameType = anyName,
                                         bool isRegex = false, 
                                         bool checkCase = false,
                                         bool includeUndefined = false);

   virtual bool getAllSymbols(std::vector<Symbol *> &ret);
   virtual bool getAllSymbolsByType(std::vector<Symbol *> &ret, 
         Symbol::SymbolType sType);

   std::vector<Symbol *> *findSymbolByOffset(Offset);

   // Return all undefined symbols in the binary. Currently used for finding
   // the .o's in a static archive that have definitions of these symbols
   bool getAllUndefinedSymbols(std::vector<Symbol *> &ret);

   // Inversely, return all non-undefined symbols in the binary
   bool getAllDefinedSymbols(std::vector<Symbol *> &ret);

   // Function

   bool findFuncByEntryOffset(Function *&ret, const Offset offset);
   bool findFunctionsByName(std::vector<Function *> &ret, const std::string name,
                                          NameType nameType = anyName, 
                                          bool isRegex = false,
                                          bool checkCase = true);
   bool getAllFunctions(std::vector<Function *>&ret);

   //Searches for functions without returning inlined instances
   bool getContainingFunction(Offset offset, Function* &func);
   //Searches for functions and returns inlined instances
   bool getContainingInlinedFunction(Offset offset, FunctionBase* &func);

   // Variable
   bool findVariableByOffset(Variable *&ret, const Offset offset);
   bool findVariablesByName(std::vector<Variable *> &ret, const std::string name,
                                          NameType nameType = anyName, 
                                          bool isRegex = false, 
                                          bool checkCase = true);
   bool getAllVariables(std::vector<Variable *> &ret);

   // Module

   bool getAllModules(std::vector<Module *>&ret);
   bool findModuleByOffset(Module *&ret, Offset off);
   bool findModuleByName(Module *&ret, const std::string name);
   Module *getDefaultModule();

   // Region

   bool getCodeRegions(std::vector<Region *>&ret);
   bool getDataRegions(std::vector<Region *>&ret);
   bool getAllRegions(std::vector<Region *>&ret);
   bool getAllNewRegions(std::vector<Region *>&ret);
   //  change me to use a hash
   bool findRegion(Region *&ret, std::string regname);
   bool findRegion(Region *&ret, const Offset addr, const unsigned long size);
   bool findRegionByEntry(Region *&ret, const Offset offset);
   Region *findEnclosingRegion(const Offset offset);

   // Exceptions
   bool findException(ExceptionBlock &excp,Offset addr);
   bool getAllExceptions(std::vector<ExceptionBlock *> &exceptions);
   bool findCatchBlock(ExceptionBlock &excp, Offset addr, 
         unsigned size = 0);

   // Relocation entries
   bool getFuncBindingTable(std::vector<relocationEntry> &fbt) const;
   bool updateFuncBindingTable(Offset stub_addr, Offset plt_addr);

   /**************************************
    *** SYMBOL ADDING FUNCS **************
    **************************************/

   bool addSymbol(Symbol *newsym);
   bool addSymbol(Symbol *newSym, Symbol *referringSymbol);
   Function *createFunction(std::string name, Offset offset, size_t size, Module *mod = NULL);
   Variable *createVariable(std::string name, Offset offset, size_t size, Module *mod = NULL);

   bool deleteFunction(Function *func);
   bool deleteVariable(Variable *var);


   /*****Query Functions*****/
   bool isExec() const;
   bool isStripped();
   ObjectType getObjectType() const;
   Dyninst::Architecture getArchitecture();
   bool isCode(const Offset where) const;
   bool isData(const Offset where) const;
   bool isValidOffset(const Offset where) const;

   bool isNativeCompiler() const;
   bool getMappedRegions(std::vector<Region *> &mappedRegs) const;

   /***** Line Number Information *****/
   bool getAddressRanges(std::vector<std::pair<Offset, Offset> >&ranges,
         std::string lineSource, unsigned int LineNo);
   bool getSourceLines(std::vector<Statement *> &lines, 
         Offset addressInRange);
   bool getSourceLines(std::vector<LineNoTuple> &lines, 
                                     Offset addressInRange);
   bool addLine(std::string lineSource, unsigned int lineNo,
         unsigned int lineOffset, Offset lowInclAddr,
         Offset highExclAddr);
   bool addAddressRange(Offset lowInclAddr, Offset highExclAddr, std::string lineSource,
         unsigned int lineNo, unsigned int lineOffset = 0);
   void setTruncateLinePaths(bool value);
   bool getTruncateLinePaths();

   /***** Type Information *****/
   virtual bool findType(Type *&type, std::string name);
   virtual Type *findType(unsigned type_id);
   virtual bool findVariableType(Type *&type, std::string name);

   bool addType(Type *typ);

   static boost::shared_ptr<builtInTypeCollection> builtInTypes();
   static boost::shared_ptr<typeCollection> stdTypes();

   static std::vector<Type *> *getAllstdTypes();
   static std::vector<Type *> *getAllbuiltInTypes();

   void parseTypesNow();

   /***** Local Variable Information *****/
   bool findLocalVariable(std::vector<localVar *>&vars, std::string name);

   /***** Relocation Sections *****/
   bool hasRel() const;
   bool hasRela() const;
   bool hasReldyn() const;
   bool hasReladyn() const;
   bool hasRelplt() const;
   bool hasRelaplt() const;
   
   bool isStaticBinary() const;

   /***** Write Back binary functions *****/
   bool emitSymbols(Object *linkedFile, std::string filename, unsigned flag = 0);
   bool addRegion(Offset vaddr, void *data, unsigned int dataSize, 
         std::string name, Region::RegionType rType_, bool loadable = false,
         unsigned long memAlign = sizeof(unsigned), bool tls = false);
   bool addRegion(Region *newreg);
   bool emit(std::string filename, unsigned flag = 0);

   void addDynLibSubstitution(std::string oldName, std::string newName);
   std::string getDynLibSubstitution(std::string name);

   bool getSegments(std::vector<Segment> &segs) const;
   
   void fixup_code_and_data(Offset newImageOffset,
                                          Offset newImageLength,
                                          Offset newDataOffset,
                                          Offset newDataLength);
   bool fixup_RegionAddr(const char* name, Offset memOffset, long memSize);
   bool fixup_SymbolAddr(const char* name, Offset newOffset);
   bool updateRegion(const char* name, void *buffer, unsigned size);
   bool updateCode(void *buffer, unsigned size);
   bool updateData(void *buffer, unsigned size);
   Offset getFreeOffset(unsigned size);

   bool addLibraryPrereq(std::string libname);
   bool addSysVDynamic(long name, long value);

   bool addLinkingResource(Archive *library);
   bool getLinkingResources(std::vector<Archive *> &libs);

   bool addExternalSymbolReference(Symbol *externalSym, Region *localRegion, relocationEntry localRel);
   bool addTrapHeader_win(Address ptr);

   bool updateRelocations(Address start, Address end, Symbol *oldsym, Symbol *newsym);

   /***** Data Member Access *****/
   std::string file() const;
   std::string name() const;
   std::string memberName() const;

   char *mem_image() const;

   Offset imageOffset() const;
   Offset dataOffset() const;
   Offset dataLength() const;
   Offset imageLength() const;
   //   char*  image_ptr ()  const;
   //   char*  data_ptr ()  const;
   Offset getInitOffset();
   Offset getFiniOffset();

   const char*  getInterpreterName() const;

   unsigned getAddressWidth() const;
   Offset getLoadOffset() const;
   Offset getEntryOffset() const;
   Offset getBaseOffset() const;
   Offset getTOCoffset(Function *func = NULL) const;
   Offset getTOCoffset(Offset off) const;
   Address getLoadAddress();
   bool isDefensiveBinary() const; 
   Offset fileToDiskOffset(Dyninst::Offset) const;
   Offset fileToMemOffset(Dyninst::Offset) const;


   std::string getDefaultNamespacePrefix() const;

   unsigned getNumberofRegions() const;
   unsigned getNumberofSymbols() const;

   std::vector<std::string> &getDependencies();
   bool removeLibraryDependency(std::string lib);

   Archive *getParentArchive() const;

   /***** Error Handling *****/
   static SymtabError getLastSymtabError();
   static std::string printError(SymtabError serr);

   ~Symtab();

   bool delSymbol(Symbol *sym) { return deleteSymbol(sym); }
   bool deleteSymbol(Symbol *sym); 

   Symbol *getSymbolByIndex(unsigned);

   /***** Private Member Functions *****/
   private:

   Symtab(std::string filename, bool defensive_bin, bool &err);

   bool extractInfo(Object *linkedFile);

   // Parsing code

   bool extractSymbolsFromFile(Object *linkedFile, std::vector<Symbol *> &raw_syms);

   bool fixSymRegion(Symbol *sym);

   bool fixSymModules(std::vector<Symbol *> &raw_syms);
   bool demangleSymbols(std::vector<Symbol *> &rawsyms);
   bool createIndices(std::vector<Symbol *> &raw_syms, bool undefined);
   bool createAggregates();

   bool fixSymModule(Symbol *&sym);
   bool demangleSymbol(Symbol *&sym);
   bool addSymbolToIndices(Symbol *&sym, bool undefined);
   bool addSymbolToAggregates(Symbol *&sym);
   bool doNotAggregate(Symbol *&sym);
   bool updateIndices(Symbol *sym, std::string newName, NameType nameType);


   void setModuleLanguages(dyn_hash_map<std::string, supportedLanguages> *mod_langs);

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

   bool parseFunctionRanges();
   bool addFunctionRange(FunctionBase *fbase, Dyninst::Offset next_start);

   // Used by binaryEdit.C...
 public:
   bool canBeShared();
   Module *getOrCreateModule(const std::string &modName, 
                                           const Offset modAddr);

 public:
   //Only valid on ELF formats
   Offset getElfDynamicOffset();
   // SymReader interface
   void getSegmentsSymReader(std::vector<SymSegment> &segs);
   void rebase(Offset offset);

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

   void setTOCOffset(Offset offset);
   /***** Private Data Members *****/
   private:

   static boost::shared_ptr<typeCollection> setupStdTypes();
   static boost::shared_ptr<builtInTypeCollection> setupBuiltinTypes();


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
   // hashtable for looking up undefined symbols in the dynamic symbol
   // tale. Entries are referred by the relocation table entries
   // NOT a subset of everyDefinedSymbol
   std::vector<Symbol *> undefDynSyms;
   std::map <std::string, std::vector<Symbol *> > undefDynSymsByMangledName;
   std::map <std::string, std::vector<Symbol *> > undefDynSymsByPrettyName;
   std::map <std::string, std::vector<Symbol *> > undefDynSymsByTypedName;

   
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
   bool isDefensiveBinary_;

   FuncRangeLookup *func_lookup;

   // Line information map from module name to line info
   dyn_hash_map<std::string, LineInformation> *lineInfo;

   //Don't use obj_private, use getObject() instead.
 public:
   Object *getObject();
 private:
   Object *obj_private;

   // dynamic library name substitutions
   std::map <std::string, std::string> dynLibSubs;

   public:
   static boost::shared_ptr<Type> type_Error();
   static boost::shared_ptr<Type> type_Untyped();

 private:
    unsigned _ref_cnt;
};

/**
 * Used to represent something like a C++ try/catch block.  
 * Currently only used on Linux/x86
 **/
SYMTAB_EXPORT  std::ostream &operator<<(std::ostream &os, const ExceptionBlock &q);

class SYMTAB_EXPORT ExceptionBlock : public Serializable, public AnnotatableSparse {
  // Accessors provide consistent access to the *original* offsets.
  // We allow this to be updated (e.g. to account for relocated code
   public:
	  Serializable * serialize_impl(SerializerBase *sb, 
			  const char *tag = "exceptionBlock") THROW_SPEC (SerializerError);
      ExceptionBlock(Offset tStart, unsigned tSize, Offset cStart);
      ExceptionBlock(Offset cStart);
      ExceptionBlock(const ExceptionBlock &eb);
      ~ExceptionBlock();
      ExceptionBlock();

      bool hasTry() const;
      Offset tryStart() const;
      Offset tryEnd() const;
      Offset trySize() const;
      Offset catchStart() const;
      bool contains(Offset a) const;
      void setTryStart(Offset ts) 
      {
	tryStart_ptr = ts;
      }
      void setTryEnd(Offset te)
      {
	tryEnd_ptr = te;
      }

      void setCatchStart(Offset cs)
      {
	catchStart_ptr = cs;
      }

      void setFdeStart(Offset fs)
      {
	fdeStart_ptr = fs;
      }
      
      void setFdeEnd(Offset fe)
      {
	fdeEnd_ptr = fe;
      }      	


      friend SYMTAB_EXPORT std::ostream &operator<<(std::ostream &os, const ExceptionBlock &q);
   private:
      Offset tryStart_;
      unsigned trySize_;
      Offset catchStart_;
      bool hasTry_;
      Offset tryStart_ptr;
      Offset tryEnd_ptr;
      Offset catchStart_ptr;
      Offset fdeStart_ptr;
      Offset fdeEnd_ptr;
};

// relocation information for calls to functions not in this image
SYMTAB_EXPORT std::ostream &operator<<(std::ostream &os, const relocationEntry &q);

class SYMTAB_EXPORT relocationEntry : public Serializable, public AnnotatableSparse {
   public:

      relocationEntry();
      relocationEntry(Offset ta, Offset ra, Offset add, 
			  std::string n, Symbol *dynref = NULL, unsigned long relType = 0);
      relocationEntry(Offset ta, Offset ra, std::string n, 
			  Symbol *dynref = NULL, unsigned long relType = 0);
      relocationEntry(Offset ra, std::string n, Symbol *dynref = NULL, 
			  unsigned long relType = 0, Region::RegionType rtype = Region::RT_REL);
      relocationEntry(Offset ta, Offset ra, Offset add,
                          std::string n, Symbol *dynref = NULL, unsigned long relType = 0,
                          Region::RegionType rtype = Region::RT_REL);

	  Serializable * serialize_impl(SerializerBase *sb, 
			  const char *tag = "relocationEntry") THROW_SPEC (SerializerError);

      Offset target_addr() const;
      Offset rel_addr() const;
      Offset addend() const;
      Region::RegionType regionType() const;
      const std::string &name() const;
      Symbol *getDynSym() const;
      bool addDynSym(Symbol *dynref);
      unsigned long getRelType() const;

      void setTargetAddr(const Offset);
      void setRelAddr(const Offset);
      void setAddend(const Offset);
      void setRegionType(const Region::RegionType);
      void setName(const std::string &newName);
      void setRelType(unsigned long relType) { relType_ = relType; }

      // dump output.  Currently setup as a debugging aid, not really
      //  for object persistance....
      //std::ostream & operator<<(std::ostream &s) const;
      friend SYMTAB_EXPORT std::ostream & operator<<(std::ostream &os, const relocationEntry &q);

      enum {pltrel = 1, dynrel = 2};
      bool operator==(const relocationEntry &) const;

      // Architecture-specific functions
      static unsigned long getGlobalRelType(unsigned addressWidth, Symbol *sym = NULL);
      static const char *relType2Str(unsigned long r, unsigned addressWidth = sizeof(Address));

   private:
      Offset target_addr_;	// target address of call instruction 
      Offset rel_addr_;		// address of corresponding relocation entry 
      Offset addend_;       // addend (from RELA entries)
      Region::RegionType rtype_;        // RT_REL vs. RT_RELA
      std::string  name_;
      Symbol *dynref_;
      unsigned long relType_;
      Offset rel_struct_addr_;
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
