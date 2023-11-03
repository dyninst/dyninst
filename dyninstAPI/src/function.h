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

// $Id: function.h,v 1.52 2008/09/08 16:44:02 bernat Exp $

#ifndef FUNCTION_H
#define FUNCTION_H

#include <string>
#include <list>
#include <map>
#include <set>
#include <stddef.h>
#include <vector>
#include "codegen.h"
#include "codeRange.h"
#include "util.h"
#include "parse-cfg.h"

#include "bitArray.h"

#include "block.h"
#include "instPoint.h"
#include "PatchCFG.h"
#include "Point.h"

#include "Variable.h"
#include "stackanalysis.h"
#if defined(cap_stack_mods)
#include "StackMod.h"
#include "StackMod/OffsetVector.h"
#include "StackMod/StackAccess.h"
#include "StackMod/StackLocation.h"
#include "StackMod/TMap.h"
#endif
#include "Relocation/DynCommon.h"

class PCProcess;
class mapped_module;
class mapped_object;

class func_instance;
class block_instance;
class edge_instance;
class instPoint;

typedef enum callType {
  unknown_call,
  cdecl_call,
  stdcall_call,
  fastcall_call,
  thiscall_call
} callType;

using Dyninst::PatchAPI::Point;

class func_instance : public patchTarget, public Dyninst::PatchAPI::PatchFunction {
  friend class block_instance;
  friend class edge_instance;
  friend class instPoint;
  public:
    // Almost everythcing gets filled in later.
    func_instance(parse_func *f,
                  Address baseAddr,
                  mapped_module *mod);

    func_instance(const func_instance *parent,
                  mapped_module *child_mod);

    ~func_instance();

  ////////////////////////////////////////////////
  // Passthrough functions.
  ////////////////////////////////////////////////
  // To minimize wasted memory (since there will be many copies of
  // this function) we make most methods passthroughs to the original
  // parsed version.

  string symTabName() const { return ifunc()->symTabName(); }
  string prettyName() const { return ifunc()->prettyName(); }
  string typedName() const { return ifunc()->typedName(); }
  string name() const { return symTabName(); }

  SymtabAPI::Aggregate::name_iter symtab_names_begin() const 
   {
     return ifunc()->symtab_names_begin();
   }
   SymtabAPI::Aggregate::name_iter symtab_names_end() const 
   {
     return ifunc()->symtab_names_end();
   }
   SymtabAPI::Aggregate::name_iter pretty_names_begin() const 
   {
     return ifunc()->pretty_names_begin();
   }
   SymtabAPI::Aggregate::name_iter pretty_names_end() const 
   {
     return ifunc()->pretty_names_end();
   }
   SymtabAPI::Aggregate::name_iter typed_names_begin() const 
   {
     return ifunc()->typed_names_begin();
   }
   SymtabAPI::Aggregate::name_iter typed_names_end() const 
   {
     return ifunc()->typed_names_end();
   }
   //vector<string> symTabNameVector() const { return ifunc()->symTabNameVector(); }
   //vector<string> prettyNameVector() const { return ifunc()->prettyNameVector(); }
   //vector<string> typedNameVector() const { return ifunc()->typedNameVector(); }

  // Debuggering functions
  void debugPrint() const;

  // And add...
  // Don't make the std::string a reference; we want a copy.
  void addSymTabName(const std::string name, bool isPrimary = false);
  void addPrettyName(const std::string name, bool isPrimary = false);

  Address getPtrAddress() const {return ptrAddr_;}

  // Not defined here so we don't have to play header file magic
  // Not const; we can add names via the Dyninst layer
  parse_func *ifunc() const { return SCAST_PF(func_); }
  mapped_module *mod() const { return mod_; }
  mapped_object *obj() const;

  AddressSpace *proc() const;
  std::string format() const;

  ////////////////////////////////////////////////
  // CFG and other function body methods
  ////////////////////////////////////////////////
  typedef AddrOrderedBlockSet BlockSet;

  block_instance *entryBlock();

  // Kevin's defensive mode shtuff
  // Blocks that have a sink target, essentially.
  const BlockSet &unresolvedCF();// Blocks that have a sink target, essentially
  const BlockSet &abruptEnds(); // Blocks where we provisionally stopped 
                                // parsing because things looked weird.
  block_instance * setNewEntry(block_instance *def, // if no better choice
                               std::set<block_instance*> &deadBlocks);
  // kevin signal-handler information
  bool isSignalHandler() {return handlerFaultAddr_ != 0;}
  Address getHandlerFaultAddr() {return handlerFaultAddr_;}
  Address getHandlerFaultAddrAddr() {return handlerFaultAddrAddr_;}
  void setHandlerFaultAddr(Address fa);
  void setHandlerFaultAddrAddr(Address faa, bool set);
  void triggerModified();

  block_instance *getBlockByEntry(const Address addr);
  // get all blocks that contain the given address
  bool getBlocks(const Address addr, std::set<block_instance*> &blks);
  // Get the block with an instruction that starts at addr
  block_instance *getBlock(const Address addr);

  Offset addrToOffset(const Address addr) const;

  bool hasNoStackFrame() const {return ifunc()->hasNoStackFrame();}
  bool savesFramePointer() const {return ifunc()->savesFramePointer();}

  func_instance* getNoPowerPreambleFunc() { return _noPowerPreambleFunc; }
  void setNoPowerPreambleFunc(func_instance* f) { _noPowerPreambleFunc = f; }
  func_instance* getPowerPreambleFunc() { return _powerPreambleFunc; }
  void setPowerPreambleFunc(func_instance* f) { _powerPreambleFunc = f; }

  ////////////////////////////////////////////////
  // Legacy/inter-module calls. Arguably should be an
  // interprocedural edge, but I expect that would
  // break all manner of things
  ////////////////////////////////////////////////
  func_instance *findCallee(block_instance *callBlock);

  bool isInstrumentable();

  Address get_address() const;
  unsigned get_size() const;
  unsigned footprint(); // not const, calls ifunc()->extents()
  std::string get_name() const;

#if defined(arch_x86) || defined(arch_x86_64)
  //Replaces the function with a 'return val' statement.
  // currently needed only on Linux/x86
  // Defined in inst-x86.C
  bool setReturnValue(int val);

#endif

  ////////////////////////////////////////////////
  // Code overlapping
  ////////////////////////////////////////////////
  // Get all functions that "share" the block. Actually, the
  // block_instance will not be shared (they are per function),
  // but the underlying parse_block records the sharing status.
  // So dodge through to the image layer and find out that info.
  // Returns true if such functions exist.

  bool getSharingFuncs(block_instance *b,
                       std::set<func_instance *> &funcs);

  // The same, but for any function that overlaps with any of
  // our basic blocks.
  // OPTIMIZATION: we're not checking all blocks, only an exit
  // point; this _should_ work :) but needs to change if we
  // ever do flow-sensitive parsing
  bool getSharingFuncs(std::set<func_instance *> &funcs);

  // Slower version of the above that also finds functions that occupy
  // the same address range, even if they do not share blocks - this can
  // be caused by overlapping but disjoint assembly sequences
  bool getOverlappingFuncs(std::set<func_instance *> &funcs);
  bool getOverlappingFuncs(block_instance *b, std::set<func_instance *> &funcs);

  ////////////////////////////////////////////////
  // Misc
  ////////////////////////////////////////////////


  const std::vector< int_parRegion* > &parRegions();

  bool containsSharedBlocks() const { return ifunc()->containsSharedBlocks(); }
  unsigned getNumDynamicCalls();

  // Fill the <callers> vector with pointers to the statically-determined
  // list of functions that call this function.
  template <class OutputIterator>
    void getCallerBlocks(OutputIterator result);
  template <class OutputIterator>
    void getCallerFuncs(OutputIterator result);
  bool getLiveCallerBlocks(const std::set<block_instance*> &deadBlocks,
                           const std::list<func_instance*> &deadFuncs,
                           std::map<Address,vector<block_instance*> > & output_stubs);



#if defined(arch_power) || defined(arch_aarch64)
  bool savesReturnAddr() const { return ifunc()->savesReturnAddr(); }
#endif

#if defined(os_windows)
  //Calling convention for this function
  callType func_instance::getCallingConvention();
  int getParamSize() { return paramSize; }
  void setParamSize(int s) { paramSize = s; }
#endif

  void getReachableBlocks(const std::set<block_instance*> &exceptBlocks,
                          const std::list<block_instance*> &seedBlocks,
                          std::set<block_instance*> &reachBlocks);//output


  // So we can assert(consistency());
  bool consistency() const;

  // Wrappers for patchapi findPoints to find a single instPoint
  instPoint *funcEntryPoint(bool create);
  instPoint *funcExitPoint(block_instance* blk, bool create);
  instPoint *preCallPoint(block_instance* blk, bool create);
  instPoint *postCallPoint(block_instance* blk, bool create);
  instPoint *blockEntryPoint(block_instance* blk, bool create);
  instPoint *blockExitPoint(block_instance* b, bool create);
  instPoint *preInsnPoint(block_instance *b, Address a,
                          InstructionAPI::Instruction insn,
                          bool trusted, bool create);
  instPoint *postInsnPoint(block_instance *b, Address a,
                           InstructionAPI::Instruction insn,
                           bool trusted, bool create);
  instPoint *edgePoint(edge_instance* eg, bool create);

  // Wrappers for patchapi findPoints to find all instPoints w/ certain type
  typedef std::vector<instPoint*> Points;
  void funcExitPoints(Points*);
  void callPoints(Points*);
  void blockInsnPoints(block_instance*, Points*);
  void edgePoints(Points*);

  // Function wrapping
  bool addSymbolsForCopy();
  bool updateRelocationsToSym(Dyninst::SymtabAPI::Symbol *oldsym, 
			      Dyninst::SymtabAPI::Symbol *newsym);
  Dyninst::SymtabAPI::Symbol *getWrapperSymbol();
  Dyninst::SymtabAPI::Symbol *getRelocSymbol();
  void createWrapperSymbol(Address entry, std::string name);

  static void destroy(func_instance *f);
  void removeBlock(block_instance *block);

  void split_block_cb(block_instance *b1, block_instance *b2);
  void add_block_cb(block_instance *block);

  virtual void markModified();

#if defined(cap_stack_mods)
  // Stack modification
  void addParam(Dyninst::SymtabAPI::localVar* p) { _params.insert(p); }
  void addVar(Dyninst::SymtabAPI::localVar* v) { _vars.insert(v); }
  std::set<Dyninst::SymtabAPI::localVar*> getParams() const { return _params; }
  std::set<Dyninst::SymtabAPI::localVar*> getVars() const { return _vars; }

  void setStackMod(bool b) { _hasStackMod = b; }
  bool hasStackMod() const { return _hasStackMod; }

  void addMod(StackMod* m, TMap* tMap);
  void removeMod(StackMod* m);
  std::set<StackMod*>* getMods() const { return _modifications; }
  void printMods() const;

  Accesses* getAccesses(Address addr);

  void setCanary(bool b) { _hasCanary = b; }
  bool hasCanary() { return _hasCanary; }

  bool hasRandomize() { return _randomizeStackFrame; }

  bool hasOffsetVector() const { return _processedOffsetVector; }
  bool hasValidOffsetVector() const { return _validOffsetVector; }
  bool createOffsetVector();
  OffsetVector* getOffsetVector() const { return _offVec; }

  TMap* getTMap() const { return _tMap; }
  void replaceTMap(TMap* newTMap) { _tMap = newTMap; }

  std::map<Address, StackAccess *> *getDefinitionMap() {
    return _definitionMap;
  }

  bool randomize(TMap* tMap, bool seeded = false, int seed = -1);
  void freeStackMod();

#endif

  bool operator<(func_instance& rhs) {
      return addr() < rhs.addr();
  }

 private:

  // helper func for block_instance::setNotAbruptEnd(), do not call directly 
  void removeAbruptEnd(const block_instance *); 

  ///////////////////// Basic func info
  //Address addr_; // Absolute address of the start of the function
  Address ptrAddr_; // Absolute address of the function descriptor, if exists

  // parse_func *ifunc_;
  mapped_module *mod_; // This is really a dodge; translate a list of
  // parse_funcs to int_funcs

  ///////////////////// CFG and function body
  // Defensive mode
  BlockSet unresolvedCF_;
  BlockSet abruptEnds_;
  size_t prevBlocksAbruptEnds_; // num func blocks when calculated


  Address handlerFaultAddr_; /* if this is a signal handler, faultAddr_ is
                                set to -1, or to the address of the fault
                                that last caused the handler to be invoked. */
  Address handlerFaultAddrAddr_;

  //////////////////////////  Parallel Regions
  std::vector<int_parRegion*> parallelRegions_; /* pointer to the parallel regions */

  void addblock_instance(block_instance *instance);

#if defined(os_windows)
  callType callingConv;
  int paramSize;
#endif

   Dyninst::SymtabAPI::Symbol *wrapperSym_;

#if defined(cap_stack_mods)
  // Stack modification
  bool createOffsetVector_Symbols();

  bool createOffsetVector_Analysis(ParseAPI::Function *func,
                                   ParseAPI::Block *block,
                                   InstructionAPI::Instruction insn,
                                   Address addr);

  bool addToOffsetVector(StackAnalysis::Height off,
          int size,
          StackAccess::StackAccessType type,
          bool isRegisterHeight,
          ValidPCRange* valid,
          MachRegister reg = MachRegister());

  void createTMap_internal(StackMod* mod, StackLocation* loc, TMap* tMap);
  void createTMap_internal(StackMod* mod, TMap* tMap);

  std::set<Dyninst::SymtabAPI::localVar*> _params;
  std::set<Dyninst::SymtabAPI::localVar*> _vars;
  bool _hasDebugSymbols;

  bool _hasStackMod;
  std::set<StackMod*>* _modifications;

  bool _seeded;
  int _seed;
  bool _randomizeStackFrame;
  bool _hasCanary;

  bool _processedOffsetVector;
  bool _validOffsetVector;
  OffsetVector* _offVec;
  set<tmpObject, less_tmpObject >* _tmpObjects;

  // Records transformations to known stack locations and stack pointers due to
  // stack modifications.
  TMap* _tMap;

  // Records known accesses to stack locations (so we can determine how to
  // modify the accesses for stack modifications).
  std::map<Address, Accesses*>* _accessMap;

  // Records stack pointer definitions that need to be modified for stack
  // modifications.
  std::map<Address, StackAccess *> *_definitionMap;
#endif
  func_instance* _noPowerPreambleFunc;
  func_instance* _powerPreambleFunc;
};

template <class OutputIterator>
void func_instance::getCallerBlocks(OutputIterator result)
{
  if(!ifunc() || !ifunc()->entryBlock())
    return;
  /*
  const block_instance::edgelist &ins = entryBlock()->sources();
  for (block_instance::edgelist::const_iterator iter = ins.begin();
       iter != ins.end(); ++iter) {
  */
  const PatchBlock::edgelist &ins = entryBlock()->sources();
  for (PatchBlock::edgelist::const_iterator iter = ins.begin();
       iter != ins.end(); ++iter) 
  {
      if ((*iter)->type() == ParseAPI::CALL) {
        *result = SCAST_EI(*iter)->src();
        ++result;
      }
  }
}

template <class OutputIterator>
void func_instance::getCallerFuncs(OutputIterator result)
{
  std::set<block_instance *> callerBlocks;
  getCallerBlocks(std::inserter(callerBlocks, callerBlocks.end()));
  for (std::set<block_instance *>::iterator iter = callerBlocks.begin();
       iter != callerBlocks.end(); ++iter) {
    (*iter)->getFuncs(result);
  }
}

#endif /* FUNCTION_H */
