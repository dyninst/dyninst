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

#ifndef __SYMTAB_H__
#define __SYMTAB_H__
 
#include "Symbol.h"
#include "Region.h"
#include "LineInformation.h"

#include "Annotatable.h"
#include "Serialization.h"


int symtab_printf(const char *format, ...);

typedef struct {} user_funcs_a;
typedef struct {} user_regions_a;
typedef struct {} user_types_a;
typedef struct {} user_symbols_a;

class MappedFile;

namespace Dyninst{
namespace SymtabAPI{

class Archive;
class builtInTypeCollection;

class ExceptionBlock;
class Object;
class relocationEntry;

class Symtab : public LookupInterface,
               public Serializable,
               public AnnotatableSparse
{

   friend class Archive;
   friend class Symbol;
   friend class Module;
   friend class Region;
   friend class emitElf;
   friend class emitElf64;

 public:
   typedef enum {
       mangledName = 1,
       prettyName = 2,
       typedName = 4,
       anyName = 7 } nameType_t;

   /***** Public Member Functions *****/
   public:
   SYMTABEXPORT Symtab(MappedFile *);

   SYMTABEXPORT Symtab();

   SYMTABEXPORT Symtab(const Symtab& obj);

   SYMTABEXPORT static bool openFile(Symtab *&obj, std::string filename);
   SYMTABEXPORT static bool openFile(Symtab *&obj,char *mem_image, size_t size);

   SYMTABEXPORT void serialize(SerializerBase *sb, const char *tag = "Symtab");
   static bool setup_module_up_ptrs(SerializerBase *,Symtab *st);
   static bool fixup_relocation_symbols(SerializerBase *,Symtab *st);

   SYMTABEXPORT bool exportXML(std::string filename);
   SYMTABEXPORT bool exportBin(std::string filename);
   static Symtab *importBin(std::string filename);


   /**************************************
    *** LOOKUP FUNCTIONS *****************
    **************************************/

   // Symbol

   SYMTABEXPORT virtual bool findSymbolByType(std::vector<Symbol *> &ret, 
                                           const std::string name,
                                           Symbol::SymbolType sType, 
                                           nameType_t nameType,
                                           bool isRegex = false, 
                                           bool checkCase = false);
   SYMTABEXPORT virtual bool findAllSymbols(std::vector<Symbol *> &ret);

   SYMTABEXPORT virtual bool getAllSymbolsByType(std::vector<Symbol *> &ret, 
         Symbol::SymbolType sType);

   // Return all undefined symbols in the binary. Currently used for finding
   // the .o's in a static archive that have definitions of these symbols
   SYMTABEXPORT bool getAllUndefinedSymbols(std::vector<Symbol *> &ret);

   // Function

   SYMTABEXPORT bool findFuncByEntryOffset(Function *&ret, const Offset offset);
   SYMTABEXPORT bool findFunctionsByName(std::vector<Function *> &ret, const std::string name,
                                      nameType_t nameType = anyName, 
                                      bool isRegex = false,
                                      bool checkCase = true);
   SYMTABEXPORT bool getAllFunctions(std::vector<Function *>&ret);

   // Variable
   SYMTABEXPORT bool findVariableByOffset(Variable *&ret, const Offset offset);
   SYMTABEXPORT bool findVariablesByName(std::vector<Variable *> &ret, const std::string name,
                                      nameType_t nameType = anyName, 
                                      bool isRegex = false, 
                                      bool checkCase = true);
   SYMTABEXPORT bool getAllVariables(std::vector<Variable *> &ret);

   // Module

   SYMTABEXPORT bool getAllModules(std::vector<Module *>&ret);
   SYMTABEXPORT bool findModuleByOffset(Module *&ret, Offset off);
   SYMTABEXPORT bool findModuleByName(Module *&ret, const std::string name);


   // Region

   SYMTABEXPORT bool getCodeRegions(std::vector<Region *>&ret);
   SYMTABEXPORT bool getDataRegions(std::vector<Region *>&ret);
   SYMTABEXPORT bool getAllRegions(std::vector<Region *>&ret);
   SYMTABEXPORT bool getAllNewRegions(std::vector<Region *>&ret);
   //  change me to use a hash
   SYMTABEXPORT bool findRegion(Region *&ret, std::string regname);
   SYMTABEXPORT bool findRegionByEntry(Region *&ret, const Offset offset);
   SYMTABEXPORT Region *findEnclosingRegion(const Offset offset);

   // Exceptions
   SYMTABEXPORT bool findException(ExceptionBlock &excp,Offset addr);
   SYMTABEXPORT bool getAllExceptions(std::vector<ExceptionBlock *> &exceptions);
   SYMTABEXPORT bool findCatchBlock(ExceptionBlock &excp, Offset addr, 
         unsigned size = 0);

   // Relocation entries
   SYMTABEXPORT bool getFuncBindingTable(std::vector<relocationEntry> &fbt) const;

   /**************************************
    *** SYMBOL ADDING FUNCS **************
    **************************************/

   SYMTABEXPORT bool addSymbol(Symbol *newsym, bool isDynamic = false);
   SYMTABEXPORT bool addSymbol(Symbol *newSym, Symbol *referringSymbol);


   /*****Query Functions*****/
   SYMTABEXPORT bool isExec() const;
   SYMTABEXPORT bool isStripped();
   SYMTABEXPORT ObjectType getObjectType() const;

   SYMTABEXPORT bool isCode(const Offset where) const;
   SYMTABEXPORT bool isData(const Offset where) const;
   SYMTABEXPORT bool isValidOffset(const Offset where) const;

   SYMTABEXPORT bool isNativeCompiler() const;
   SYMTABEXPORT bool getMappedRegions(std::vector<Region *> &mappedRegs) const;

   /***** Line Number Information *****/
   SYMTABEXPORT bool getAddressRanges(std::vector<std::pair<Offset, Offset> >&ranges,
         std::string lineSource, unsigned int LineNo);
   SYMTABEXPORT bool getSourceLines(std::vector<LineInformationImpl::LineNoTuple> &lines, 
         Offset addressInRange);
   SYMTABEXPORT bool addLine(std::string lineSource, unsigned int lineNo,
         unsigned int lineOffset, Offset lowInclAddr,
         Offset highExclAddr);
   SYMTABEXPORT bool addAddressRange(Offset lowInclAddr, Offset highExclAddr, std::string lineSource,
         unsigned int lineNo, unsigned int lineOffset = 0);

   /***** Type Information *****/
   SYMTABEXPORT virtual bool findType(Type *&type, std::string name);
   SYMTABEXPORT virtual bool findVariableType(Type *&type, std::string name);

   SYMTABEXPORT bool addType(Type *typ);

   SYMTABEXPORT static std::vector<Type *> *getAllstdTypes();
   SYMTABEXPORT static std::vector<Type *> *getAllbuiltInTypes();

   SYMTABEXPORT void parseTypesNow();

   /***** Local Variable Information *****/
   SYMTABEXPORT bool findLocalVariable(std::vector<localVar *>&vars, std::string name);

   /***** Relocation Sections *****/
   SYMTABEXPORT bool hasRel() const;
   SYMTABEXPORT bool hasRela() const;

   /***** Write Back binary functions *****/
   SYMTABEXPORT bool emitSymbols(Object *linkedFile, std::string filename, unsigned flag = 0);
   SYMTABEXPORT bool addRegion(Offset vaddr, void *data, unsigned int dataSize, 
         std::string name, Region::RegionType rType_, bool loadable = false);
   SYMTABEXPORT bool addRegion(Region *newreg);
   SYMTABEXPORT bool emit(std::string filename, unsigned flag = 0);

   SYMTABEXPORT void addDynLibSubstitution(std::string oldName, std::string newName);
   SYMTABEXPORT std::string getDynLibSubstitution(std::string name);

   SYMTABEXPORT bool getSegments(std::vector<Segment> &segs) const;
   SYMTABEXPORT bool updateCode(void *buffer, unsigned size);
   SYMTABEXPORT bool updateData(void *buffer, unsigned size);
   SYMTABEXPORT Offset getFreeOffset(unsigned size);

   /***** Data Member Access *****/
   SYMTABEXPORT std::string file() const;
   SYMTABEXPORT std::string name() const;

   SYMTABEXPORT char *mem_image() const;

   SYMTABEXPORT Offset imageOffset() const;
   SYMTABEXPORT Offset dataOffset() const;
   SYMTABEXPORT Offset dataLength() const;
   SYMTABEXPORT Offset imageLength() const;
   //   SYMTABEXPORT char*  image_ptr ()  const;
   //   SYMTABEXPORT char*  data_ptr ()  const;

   SYMTABEXPORT const char*  getInterpreterName() const;

   SYMTABEXPORT unsigned getAddressWidth() const;
   SYMTABEXPORT Offset getLoadOffset() const;
   SYMTABEXPORT Offset getEntryOffset() const;
   SYMTABEXPORT Offset getBaseOffset() const;
   SYMTABEXPORT Offset getTOCoffset() const;

   SYMTABEXPORT std::string getDefaultNamespacePrefix() const;

   SYMTABEXPORT unsigned getNumberofRegions() const;
   SYMTABEXPORT unsigned getNumberofSymbols() const;

   SYMTABEXPORT std::vector<std::string> &getDependencies();

   /***** Error Handling *****/
   SYMTABEXPORT static SymtabError getLastSymtabError();
   SYMTABEXPORT static std::string printError(SymtabError serr);

   SYMTABEXPORT ~Symtab();

   bool delSymbol(Symbol *sym); 

   static builtInTypeCollection *builtInTypes;
   static typeCollection *stdTypes;

   protected:
   Symtab(std::string filename, std::string member_name, Offset offset, bool &err, void *base = NULL);
   Symtab(char *img, size_t size, std::string member_name, Offset offset, bool &err, void *base = NULL);

   /***** Private Member Functions *****/
   private:
   SYMTABEXPORT Symtab(std::string filename, bool &err); 
   SYMTABEXPORT Symtab(char *mem_image, size_t image_size, bool &err);

   SYMTABEXPORT bool extractInfo(Object *linkedFile);

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
   bool updateIndices(Symbol *sym, std::string newName, nameType_t nameType);


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

   Module *getOrCreateModule(const std::string &modName, 
         const Offset modAddr);
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
   std::map <std::string, Symbol *> undefDynSyms;

   // Symbols by offsets in the symbol table
   dyn_hash_map <Offset, std::vector<Symbol *> > symsByOffset;

   // The raw name from the symbol table
   dyn_hash_map <std::string, std::vector<Symbol *> > symsByMangledName;

   // The name after we've run it through the demangler
   dyn_hash_map <std::string, std::vector<Symbol *> > symsByPrettyName;

   // The name after we've derived the parameter types
   dyn_hash_map <std::string, std::vector<Symbol *> > symsByTypedName;

   // We also need per-Aggregate indices
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

   //Don't use obj_private, use getObject() instead.
   Object *getObject();
   Object *obj_private;

   // dynamic library name substitutions
   std::map <std::string, std::string> dynLibSubs;

   public:
   Type *type_Error;
   Type *type_Untyped;

 public:
   /********************************************************************/
   /**** DEPRECATED ****************************************************/
   /********************************************************************/
   
   SYMTABEXPORT bool findFuncByEntryOffset(std::vector<Symbol *>&ret, const Offset offset);
   SYMTABEXPORT virtual bool findSymbolByType(std::vector<Symbol *> &ret, 
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
class ExceptionBlock : public Serializable, public AnnotatableSparse {

   public:
      SYMTABEXPORT void serialize(SerializerBase *sb, const char *tag = "exceptionBlock");
      SYMTABEXPORT ExceptionBlock(Offset tStart, unsigned tSize, Offset cStart);
      SYMTABEXPORT ExceptionBlock(Offset cStart);
      SYMTABEXPORT ExceptionBlock(const ExceptionBlock &eb);
      SYMTABEXPORT ~ExceptionBlock();
      SYMTABEXPORT ExceptionBlock();

      SYMTABEXPORT bool hasTry() const;
      SYMTABEXPORT Offset tryStart() const;
      SYMTABEXPORT Offset tryEnd() const;
      SYMTABEXPORT Offset trySize() const;
      SYMTABEXPORT Offset catchStart() const;
      SYMTABEXPORT bool contains(Offset a) const;

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

class relocationEntry : public Serializable, public AnnotatableSparse {
   public:

      SYMTABEXPORT relocationEntry();
      SYMTABEXPORT relocationEntry(Offset ta, Offset ra, Offset add, std::string n, Symbol *dynref = NULL, unsigned long relType = 0);
      SYMTABEXPORT relocationEntry(Offset ta, Offset ra, std::string n, Symbol *dynref = NULL, unsigned long relType = 0);
      SYMTABEXPORT relocationEntry(Offset ra, std::string n, Symbol *dynref = NULL, unsigned long relType = 0, Region::RegionType rtype = Region::RT_REL);

      SYMTABEXPORT relocationEntry(const relocationEntry& ra);

      SYMTABEXPORT const relocationEntry& operator= (const relocationEntry &ra);
      SYMTABEXPORT void serialize(SerializerBase *sb, const char *tag = "relocationEntry");

      SYMTABEXPORT Offset target_addr() const;
      SYMTABEXPORT Offset rel_addr() const;
      SYMTABEXPORT Offset addend() const;
      SYMTABEXPORT Region::RegionType regionType() const;
      SYMTABEXPORT void setAddend(const Offset);
      SYMTABEXPORT void setRegionType(const Region::RegionType);
      SYMTABEXPORT const std::string &name() const;
      SYMTABEXPORT Symbol *getDynSym() const;
      SYMTABEXPORT bool addDynSym(Symbol *dynref);
      SYMTABEXPORT unsigned long getRelType() const;

      // dump output.  Currently setup as a debugging aid, not really
      //  for object persistance....
      SYMTABEXPORT std::ostream & operator<<(std::ostream &s) const;
      friend std::ostream &operator<<(std::ostream &os, relocationEntry &q);

      enum {pltrel = 1, dynrel = 2} relocationType;

   private:
      Offset target_addr_;	// target address of call instruction 
      Offset rel_addr_;		// address of corresponding relocation entry 
      Offset addend_;       // addend (from RELA entries)
      Region::RegionType rtype_;        // RT_REL vs. RT_RELA
      std::string  name_;
      Symbol *dynref_;
      unsigned long relType_;
};

}//namespace SymtabAPI

}//namespace Dyninst
#endif
