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

// $Id: binaryEdit.h,v 1.15 2008/10/28 18:42:45 bernat Exp $

#ifndef BINARY_H
#define BINARY_H

#include <string>
#include <string.h>
#include <vector>
#include <map>
#include <memory>

#include "infHeap.h"
#include "addressSpace.h"
#include "codeRange.h"
#include "ast.h"

#include "parseAPI/h/InstructionSource.h"
#include "PatchMgr.h"

class fileDescriptor;
class func_instance;
class memoryTracker;
class depRelocation;

class BinaryEdit : public AddressSpace {
 public:
    bool readDataSpace(const void *inOther, 
                       u_int amount, 
                       void *inSelf, 
                       bool showError);
    bool readTextSpace(const void *inOther, 
                       u_int amount, 
                       void *inSelf);

    bool writeDataSpace(void *inOther,
                        u_int amount,
                        const void *inSelf);
    bool writeTextSpace(void *inOther,
                        u_int amount,
                        const void *inSelf);

    bool readDataWord(const void *inOther, 
                      u_int amount, 
                      void *inSelf, 
                      bool showError);
    bool readTextWord(const void *inOther, 
                      u_int amount, 
                      void *inSelf);

    bool writeDataWord(void *inOther,
                       u_int amount,
                       const void *inSelf);
    bool writeTextWord(void *inOther,
                       u_int amount,
                       const void *inSelf);

    Address inferiorMalloc(unsigned size, 
                           inferiorHeapType type=anyHeap,
                           Address near = 0, 
                           bool *err = NULL);
    virtual void inferiorFree(Address item);
    virtual bool inferiorRealloc(Address item, unsigned newSize);

    Address offset() const;
    Address length() const;
    Architecture getArch() const;
    /*
    // Until we need these different from AddressSpace,
    // I'm not implementing.
    void *getPtrToInstruction(Address) const;
    bool isValidAddress(const Address) const;
    */

    bool multithread_capable(bool ignore_if_mt_not_set = false);
    
    bool multithread_ready(bool ignore_if_mt_not_set = false);

    virtual bool hasBeenBound(const SymtabAPI::relocationEntry &, 
                              func_instance *&, 
                              Address) { return false; }

    bool needsPIC();

    BinaryEdit();
    ~BinaryEdit();

    static BinaryEdit *openFile(const std::string &file,
                                Dyninst::PatchAPI::PatchMgrPtr mgr = Dyninst::PatchAPI::PatchMgrPtr(),
                                Dyninst::PatchAPI::Patcher::Ptr patch = Dyninst::PatchAPI::Patcher::Ptr(),
                                const std::string &member = "");

    bool writeFile(const std::string &newFileName);
    
    virtual func_instance *findOnlyOneFunction(const std::string &name,
                                              const std::string &libname = "",
                                              bool search_rt_lib = true);

    bool openSharedLibrary(const std::string &file, bool openDependencies = true);

    void addDependentRelocation(Address to, SymtabAPI::Symbol *referring);

    Address getDependentRelocationAddr(SymtabAPI::Symbol *referring);

    void addLibraryPrereq(std::string libname);

   void setupRTLibrary(std::vector<BinaryEdit *> &r);
   std::vector<BinaryEdit *> &rtLibrary();
   bool getAllDependencies(std::map<std::string, BinaryEdit* > &deps);

   void markDirty();
   bool isDirty();

   mapped_object *getMappedObject();
   
   void setMultiThreadCapable(bool b);

   void addSibling(BinaryEdit *);
   std::vector<BinaryEdit *> &getSiblings();

   bool replaceTrapHandler();
   bool usedATrap();
   bool isMultiThreadCapable();
   mapped_object *openResolvedLibraryName(std::string filename, 
                                          std::map<std::string, BinaryEdit*> &allOpened);

   bool writing() { return writing_; }

   void addDyninstSymbol(SymtabAPI::Symbol *sym) { newDyninstSyms_.push_back(sym); }

   virtual void addTrap(Address from, Address to, codeGen &gen);
   virtual void removeTrap(Address /*from*/) {}
    static bool getResolvedLibraryPath(const std::string &filename, std::vector<std::string> &paths);

 private:
    Address highWaterMark_;
    Address lowWaterMark_;
    bool isDirty_;

    static bool getStatFileDescriptor(const std::string &file,
                                      fileDescriptor &desc);


    bool inferiorMallocStatic(unsigned size);

    Address maxAllocedAddr();

    bool createMemoryBackingStore(mapped_object *obj);

    bool initialize();

    void makeInitAndFiniIfNeeded();

    bool archSpecificMultithreadCapable();

   bool doStaticBinarySpecialCases();
    
    codeRangeTree memoryTracker_;

    mapped_object * addSharedObject(const std::string *fullPath);

    std::vector<depRelocation *> dependentRelocations;

    void buildDyninstSymbols(std::vector<SymtabAPI::Symbol *> &newSyms, 
                             SymtabAPI::Region *newSec,
                             SymtabAPI::Module *newMod);

    mapped_object *mobj;
    std::vector<BinaryEdit *> rtlib;
    std::vector<BinaryEdit *> siblings;
    bool multithread_capable_;
    bool writing_;

    std::vector<SymtabAPI::Symbol *> newDyninstSyms_;

};

class depRelocation {
	public:
 depRelocation(Address a, SymtabAPI::Symbol *r) : to(a), referring(r) { }
  Address getAddress() const { return to; }
  SymtabAPI::Symbol *getReferring() const { return referring; }

 private:
  Address to;
  SymtabAPI::Symbol *referring;
};

class memoryTracker : public codeRange {
public:
  memoryTracker(Address a, unsigned s) : memoryTracker(a, s, nullptr) {}

  memoryTracker(Address a, unsigned s, void *b)
      : a_(a), s_(s) {
    b_.reset(new char[s_]);
    if (b) {
      memcpy(b_.get(), b, s_);
    }
  }
  ~memoryTracker() = default;

  memoryTracker(memoryTracker const &) = delete;
  memoryTracker &operator=(memoryTracker const &) = delete;

  memoryTracker(memoryTracker &&) = default;
  memoryTracker &operator=(memoryTracker &&) = default;

  Address get_address() const { return a_; }
  unsigned get_size() const { return s_; }
  void *get_local_ptr() const { return static_cast<void*>(b_.get()); }
  void realloc(unsigned newsize) {
    if(newsize <= s_) {
    	// No need to fiddle with the data, just change the size
    	s_ = newsize;
    	return;
    }
    auto *ptr = new char[newsize];
    std::copy(b_.get(), b_.get()+s_, ptr);
    b_.reset(ptr);
    s_ = newsize;
  }

  bool alloced{false};
  bool dirty{false};

private:
  Address a_;
  unsigned s_;
  std::unique_ptr<char[]> b_;
};

#endif // BINARY_H
