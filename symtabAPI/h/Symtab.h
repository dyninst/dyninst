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

#include <iosfwd>
#include <map>
#include <stddef.h>
#include <utility>
#include <string>
#include <vector>
#include <set>
#include <memory>

#include "Symbol.h"
#include "Module.h"
#include "Region.h"
#include "Function.h"
#include "Annotatable.h"
#include "ProcReader.h"
#include "Type.h"
#include "compiler_annotations.h"
#include "relocationEntry.h"
#include "ExceptionBlock.h"
#include "dyninstversion.h"

#include "boost/shared_ptr.hpp"

class MappedFile;

#define SYM_MAJOR DYNINST_MAJOR_VERSION
#define SYM_MINOR DYNINST_MINOR_VERSION
#define SYM_BETA  DYNINST_PATCH_VERSION
 
namespace Dyninst {

   struct SymSegment;

namespace SymtabAPI {

class Archive;
class builtInTypeCollection;

class Object;
class localVar;
class Type;
struct symtab_impl;

typedef Dyninst::ProcessReader MemRegReader;

class SYMTAB_EXPORT Symtab : public LookupInterface,
               public AnnotatableSparse
{
   friend class Archive;
   friend class Symbol;
   friend class Function;
   friend class Variable;
   friend class Module;
   friend class Region;
   friend class emitElfStatic;
   friend class emitWin;
   friend class Aggregate;
   friend class relocationEntry;
   friend class Object;

   // Hide implementation details that are complex or add large dependencies
   const std::unique_ptr<symtab_impl> impl;

 public:


   /***** Public Member Functions *****/
   public:
   static void version(int& major, int& minor, int& maintenance);

   Symtab();

   Symtab(unsigned char *mem_image, size_t image_size, 
                        const std::string &name, bool defensive_binary, bool &err);

   ~Symtab();

   Symtab(Symtab const&) = delete;
   Symtab& operator=(Symtab const&) = delete;
   Symtab(Symtab&&) = delete;
   Symtab& operator=(Symtab&&) = delete;

   typedef enum {
      NotDefensive,
      Defensive} def_t; 

   static bool openFile(Symtab *&obj, std::string filename, 
                                      def_t defensive_binary = NotDefensive);
   static bool openFile(Symtab *&obj, void *mem_image, size_t size, 
                                      std::string name, def_t defensive_binary = NotDefensive);
   static Symtab *findOpenSymtab(std::string filename);
   static bool closeSymtab(Symtab *);

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

   std::vector<Symbol *> findSymbolByOffset(Offset);

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
   const std::vector<Function*>& getAllFunctionsRef() const { return everyFunction; }

   //Searches for functions without returning inlined instances
   bool getContainingFunction(Offset offset, Function* &func);
   //Searches for functions and returns inlined instances
   bool getContainingInlinedFunction(Offset offset, FunctionBase* &func);

   // Variable
   bool findVariablesByOffset(std::vector<Variable *> &ret, const Offset offset);
   bool findVariablesByName(std::vector<Variable *> &ret, const std::string name,
                                          NameType nameType = anyName, 
                                          bool isRegex = false, 
                                          bool checkCase = true);
   bool getAllVariables(std::vector<Variable *> &ret);

   // Module

   bool getAllModules(std::vector<Module *>&ret);
   DYNINST_DEPRECATED("Use findModulesByOffset(Offset)") bool findModuleByOffset(Module *& ret, Offset off);
   Module* findModuleByOffset(Offset offset) const;
   std::vector<Module*> findModulesByName(std::string const& name) const;
   Module *getDefaultModule() const;
   Module* getContainingModule(Offset offset) const;

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
   bool findPltEntryByTarget(Address target_address, relocationEntry &result) const;
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
   bool isExecutable() const;
   bool isSharedLibrary() const;
   bool isStripped();
   ObjectType getObjectType() const;
   Dyninst::Architecture getArchitecture() const;
   bool isCode(const Offset where) const;
   bool isData(const Offset where) const;
   bool isValidOffset(const Offset where) const;

   bool getMappedRegions(std::vector<Region *> &mappedRegs) const;

   /***** Line Number Information *****/
   bool getAddressRanges(std::vector<AddressRange> &ranges,
                         std::string lineSource, unsigned int LineNo);
   bool getSourceLines(std::vector<Statement::Ptr> &lines,
                       Offset addressInRange);
   bool getSourceLines(std::vector<LineNoTuple> &lines,
                                     Offset addressInRange);
   void setTruncateLinePaths(bool value);
   bool getTruncateLinePaths();
   
   /***** Type Information *****/
   virtual bool findType(boost::shared_ptr<Type>& type, std::string name);
   bool findType(Type*& t, std::string n) {
     boost::shared_ptr<Type> tp;
     auto r = findType(tp, n);
     t = tp.get();
     return r;
   }
   virtual boost::shared_ptr<Type> findType(unsigned type_id, Type::do_share_t);
   Type* findType(unsigned i) { return findType(i, Type::share).get(); }
   virtual bool findVariableType(boost::shared_ptr<Type>& type, std::string name);
   bool findVariableType(Type*& t, std::string n) {
     boost::shared_ptr<Type> tp;
     auto r = findVariableType(tp, n);
     t = tp.get();
     return r;
   }

   bool addType(Type *typ);

   static boost::shared_ptr<builtInTypeCollection>& builtInTypes();
   static boost::shared_ptr<typeCollection>& stdTypes();

   static void getAllstdTypes(std::vector<boost::shared_ptr<Type>>&);
   static std::vector<Type*>* getAllstdTypes() {
     std::vector<boost::shared_ptr<Type>> v;
     getAllstdTypes(v);
     auto r = new std::vector<Type*>(v.size());
     for(std::size_t i = 0; i < v.size(); i++) (*r)[i] = v[i].get();
     return r;
   }
   static void getAllbuiltInTypes(std::vector<boost::shared_ptr<Type>>&);
   static std::vector<Type*>* getAllbuiltInTypes() {
     std::vector<boost::shared_ptr<Type>> v;
     getAllbuiltInTypes(v);
     auto r = new std::vector<Type*>(v.size());
     for(std::size_t i = 0; i < v.size(); i++) (*r)[i] = v[i].get();
     return r;
   }

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

   Offset preferedBase() const;
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
   bool isBigEndianDataEncoding() const;
   bool getABIVersion(int &major, int &minor) const;

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

   unsigned getNumberOfRegions() const;
   unsigned getNumberOfSymbols() const;

   std::vector<std::string> &getDependencies();
   bool removeLibraryDependency(std::string lib);

   Archive *getParentArchive() const;

   /***** Error Handling *****/
   static SymtabError getLastSymtabError();
   static void setSymtabError(SymtabError new_err);
   static std::string printError(SymtabError serr);

   bool delSymbol(Symbol *sym) { return deleteSymbol(sym); }
   bool deleteSymbol(Symbol *sym); 

   /***** Private Member Functions *****/
   private:

   Symtab(std::string filename, bool defensive_bin, bool &err);

   bool extractInfo(Object *linkedFile);

   // Parsing code

   bool extractSymbolsFromFile(Object *linkedFile, std::vector<Symbol *> &raw_syms);

   bool fixSymRegion(Symbol *sym);

   bool fixSymModules(std::vector<Symbol *> &raw_syms);
   bool createIndices(std::vector<Symbol *> &raw_syms, bool undefined);
   bool createAggregates();

   bool fixSymModule(Symbol *&sym);
   bool addSymbolToIndices(Symbol *&sym, bool undefined);
   bool addSymbolToAggregates(const Symbol *sym);
   bool doNotAggregate(const Symbol *sym);


   void setModuleLanguages(dyn_hash_map<std::string, supportedLanguages> *mod_langs);

   // Change the type of a symbol after the fact
   bool changeType(Symbol *sym, Symbol::SymbolType oldType);

   bool deleteSymbolFromIndices(Symbol *sym);

   bool changeAggregateOffset(Aggregate *agg, Offset oldOffset, Offset newOffset);
   bool deleteAggregate(Aggregate *agg);

   bool addFunctionRange(FunctionBase *fbase, Dyninst::Offset next_start);

   // Used by binaryEdit.C...
 public:


   bool canBeShared();
   DYNINST_DEPRECATED("Use getContainingModule")
   Module *getOrCreateModule(const std::string &modName, const Offset modAddr);
   bool parseFunctionRanges();

   //Only valid on ELF formats
   Offset getElfDynamicOffset();
   // SymReader interface
   void getSegmentsSymReader(std::vector<SymSegment> &segs);
   void rebase(Offset offset);

 private:
   void createDefaultModule();
   void addModule(Module *m);
   
   //bool buildFunctionLists(std::vector <Symbol *> &raw_funcs);
   //void enterFunctionInTables(Symbol *func, bool wasSymtab);


   bool addSymtabVariables();


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
   dyn_rwlock symbols_rwlock{};
   // boost::mutex symbols_mutex;

   std::string member_name_{};
   Offset member_offset_{};
   Archive * parentArchive_{};
   MappedFile *mf{};

   Offset preferedBase_{};
   Offset imageOffset_{};
   unsigned imageLen_{};
   Offset dataOffset_{};
   unsigned dataLen_{};

   bool is_a_out{false};
   Offset main_call_addr_{}; // address of call to main()

   unsigned address_width_{sizeof(int)};
   std::string interpreter_name_{};
   Offset entry_address_{};
   Offset base_address_{};
   Offset load_address_{};
   ObjectType object_type_{obj_Unknown};
   bool is_eel_{false};
   std::vector<Segment> segments_{};

   //  make sure is_a_out is set before calling symbolsToFunctions

   // A std::vector of all Symtabs. Used to avoid duplicating
   // a Symtab that already exists.
   static std::vector<Symtab *> allSymtabs;
   std::string defaultNamespacePrefix{};

   //sections
   unsigned no_of_sections{};
   std::vector<Region *> regions_{};
   std::vector<Region *> codeRegions_{};
   std::vector<Region *> dataRegions_{};
   dyn_hash_map <Offset, Region *> regionsByEntryAddr{};

   //Point where new loadable sections will be inserted
   unsigned newSectionInsertPoint{};

   //symbols
   unsigned no_of_symbols{};
   
   // We also need per-Aggregate indices
   bool sorted_everyFunction{false};
   std::vector<Function *> everyFunction{};

   // Similar for Variables
   std::vector<Variable *> everyVariable{};

   std::vector<relocationEntry > relocation_table_{};
   std::vector<ExceptionBlock *> excpBlocks{};

   std::vector<std::string> deps_{};

   // This set is used during static linking to satisfy dependencies
   std::vector<Archive *> linkingResources_{};

   // This set represents Symtabs referenced by a new external Symbol
   bool getExplicitSymtabRefs(std::set<Symtab *> &refs);
   std::set<Symtab *> explicitSymtabRefs_{};

   //Relocation sections
   bool hasRel_{false};
   bool hasRela_{false};
   bool hasReldyn_{false};
   bool hasReladyn_{false};
   bool hasRelplt_{false};
   bool hasRelaplt_{false};

   bool isStaticBinary_{false};
   bool isDefensiveBinary_{false};

   //Don't use obj_private, use getObject() instead.
 public:
   Object *getObject();
   const Object *getObject() const;
   void dumpModRanges();
   void dumpFuncRanges();

 private:
   Object *obj_private{};

   // dynamic library name substitutions
   std::map <std::string, std::string> dynLibSubs{};

   public:
   static boost::shared_ptr<Type>& type_Error();
   static boost::shared_ptr<Type>& type_Untyped();

 private:
    unsigned _ref_cnt{1};
};

}//namespace SymtabAPI

}//namespace Dyninst
#endif
