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

// $Id: binaryEdit.C,v 1.26 2008/10/28 18:42:44 bernat Exp $

#include "binaryEdit.h"
#include "common/src/headers.h"
#include "mapped_object.h"
#include "mapped_module.h"
#include "debug.h"
#include "os.h"
#include "instPoint.h"
#include "function.h"
#include "Object.h"

using namespace Dyninst::SymtabAPI;

// Reading and writing get somewhat interesting. We are building
// a false address space - that of the "inferior" binary we're editing. 
// However, that address space doesn't exist - and so we must overlay
// it on ours. In the dynamic case this is easy, of course. So what
// we have is a mapping. An "Address" that we get (inOther), can
// map to one of the following:
//  1) An area created with inferiorMalloc
//  2) A section of the binary that is original
//  3) A section of the binary that was modified

bool BinaryEdit::readTextSpace(const void *inOther,
                               u_int size,
                               void *inSelf) {
    Address addr = (Address) inOther;
    
    // Look up this address in the code range tree of memory
    codeRange *range = NULL;
    if (!memoryTracker_.find(addr, range))
        return false;
    assert(addr >= range->get_address());

    Address offset = addr - range->get_address();
    assert(offset < range->get_size());

    void *local_ptr = ((void *) (offset + (Address)range->get_local_ptr()));
    memcpy(inSelf, local_ptr, size);

    return true;
}

bool BinaryEdit::writeTextSpace(void *inOther,
                            u_int size,
                            const void *inSelf) {
    // This assumes we already have a memory tracker; inefficient, but
    // it works. 
    Address addr = (Address) inOther;
    unsigned int to_do = size;
    Address local = (Address) inSelf;
    markDirty();

    while (to_do) {
       // Look up this address in the code range tree of memory
       codeRange *range = NULL;
       if (!memoryTracker_.find(addr, range)) {
          return false;
       }
       
       // We might (due to fragmentation) be overlapping multiple backing
       // store "chunks", so this has to be iterative rather than a one-shot.
       
       Address chunk_start = range->get_address();
       Address chunk_end = range->get_address() + range->get_size();
       
       assert (addr >= chunk_start);
       
       unsigned chunk_size = 0;
       if ((addr + to_do) <= chunk_end) {
          chunk_size = to_do;
       }
       else {
          chunk_size = chunk_end - addr;
       }
       
       Address offset = addr - range->get_address();
       assert(offset < range->get_size());
       
       void *local_ptr = ((void *) (offset + (Address)range->get_local_ptr()));
       inst_printf("Copying to 0x%p [base=0x%p] from 0x%lx (%u bytes)  target=0x%lx  offset=0x%lx\n", 
              local_ptr, range->get_local_ptr(), local, chunk_size, addr, offset);
       memcpy(local_ptr, (void *)local, chunk_size);
       memoryTracker* mt = dynamic_cast<memoryTracker*>(range);
       assert(mt);
       mt->dirty = true;
       
       to_do -= chunk_size;
       addr += chunk_size;
       local += chunk_size;
    }

    return true;
}    

bool BinaryEdit::readDataSpace(const void *inOther,
                           u_int amount,
                           void *inSelf,
                           bool) {
    return readTextSpace(inOther, amount, inSelf);
}

bool BinaryEdit::writeDataSpace(void *inOther,
                            u_int amount,
                            const void *inSelf) {
    return writeTextSpace(inOther, amount, inSelf);
}

bool BinaryEdit::readTextWord(const void *inOther,
                              u_int size,
                              void *inSelf)
{ return readTextSpace(inOther, size, inSelf); }

bool BinaryEdit::writeTextWord(void *inOther,
                               u_int size,
                               const void *inSelf)
{ return writeTextSpace(inOther, size, inSelf); }

bool BinaryEdit::readDataWord(const void *inOther,
                              u_int amount,
                              void *inSelf,
                              bool)
{ return readTextSpace(inOther, amount, inSelf); }

bool BinaryEdit::writeDataWord(void *inOther,
                               u_int amount,
                               const void *inSelf)
{ return writeTextSpace(inOther, amount, inSelf); }

const Address ADDRESS_LO = (Address)0;
const Address ADDRESS_HI = (Address)(~(Address)0);

Address BinaryEdit::inferiorMalloc(unsigned size,
                               inferiorHeapType /*ignored*/,
                               Address /*near*/,
                               bool *err) {
    // It looks like we're ignoring near...
    Address ret = 0;

    Address lo = ADDRESS_LO;
    Address hi = ADDRESS_HI;

    if (err) *err = false;
   
    inferiorMallocAlign(size); // align size

    int freeIndex = -1;
    int ntry = 0;
    for (ntry = 0; freeIndex == -1; ntry++) {
        switch(ntry) {
        case 0: 
            // See if we have available memory
            break;
        case 1:
            inferiorFreeCompact();
            break;
        case 2:
            inferiorMallocStatic(size);
            break;
        default:
            return 0;
        }
        ret = inferiorMallocInternal(size, lo, hi, anyHeap);
        if (ret) {
	  memoryTracker *newTracker = new memoryTracker(ret, size);
	  newTracker->alloced = true;
	  memoryTracker_.insert(newTracker);
	  break;
	}
    }

    return ret;
}

void BinaryEdit::inferiorFree(Address item)
{
  inferiorFreeInternal(item);

  codeRange *obj{};
  if(memoryTracker_.find(item, obj)) delete obj;

  // Remove it from the tree
  memoryTracker_.remove(item);
}

bool BinaryEdit::inferiorRealloc(Address item, unsigned newsize)
{
  bool result = inferiorReallocInternal(item, newsize);
  if (!result)
    return false;

  maxAllocedAddr();

  codeRange *obj;
  result = memoryTracker_.find(item, obj);
  assert(result);

  memoryTracker_.remove(item);

  memoryTracker *mem_track = dynamic_cast<memoryTracker *>(obj);
  assert(mem_track);

  mem_track->realloc(newsize);

  memoryTracker_.insert(obj);
  return true;
}

Architecture BinaryEdit::getArch() const {
  assert(mapped_objects.size());
    // XXX presumably all of the objects in the BinaryEdit collection
    //     must be the same architecture.
  return mapped_objects[0]->parse_img()->codeObject()->cs()->getArch();
}

Address BinaryEdit::offset() const {
    fprintf(stderr,"error BinaryEdit::offset() unimpl\n");
    return 0;
}
Address BinaryEdit::length() const {
    fprintf(stderr,"error BinaryEdit::length() unimpl\n");
    return 0;
}

bool BinaryEdit::multithread_capable(bool) {
   return multithread_capable_;
}

bool BinaryEdit::multithread_ready(bool) {
    return multithread_capable();
}

BinaryEdit::BinaryEdit() : 
   highWaterMark_(0),
   lowWaterMark_(0),
   isDirty_(false),
   memoryTracker_{},
   mobj(NULL),
   multithread_capable_(false),
   writing_(false)
{
   trapMapping.shouldBlockFlushes(true);
}

BinaryEdit::~BinaryEdit()
{
	/*
	 * NB: We do not own the objects contained in
	 * 	   newDyninstSyms_, rtlib, or siblings, so
	 * 	   do not ::delete them
	*/
    for(auto *rel : dependentRelocations) {
        delete rel;
    }

    std::vector<codeRange*> x;
    memoryTracker_.elements(x);
    for(auto const *c : x) {
    	delete c;
    }
}

BinaryEdit *BinaryEdit::openFile(const std::string &file, 
                                 PatchMgrPtr mgr, 
                                 Dyninst::PatchAPI::Patcher::Ptr patch,
                                 const std::string &member) {
    if (!OS::executableExists(file)) {
        startup_printf("%s[%d]:  failed to read file %s\n", FILE__, __LINE__, file.c_str());
        std::string msg = std::string("Can't read executable file ") + file + (": ") + strerror(errno);
        showErrorCallback(68, msg.c_str());
        return NULL;
    }
    
    fileDescriptor desc;
    if (!getStatFileDescriptor(file, desc)) {
        startup_printf("%s[%d]: failed to create file descriptor for %s!\n",
                       FILE__, __LINE__, file.c_str());
        return NULL;
    }
    
    // Open the mapped object as an archive member
    if( !member.empty() ) {
        desc.setMember(member);
    }

    BinaryEdit *newBinaryEdit = new BinaryEdit();
    if (!newBinaryEdit) {
        startup_printf("%s[%d]: failed to create binary representation for %s!\n",
                       FILE__, __LINE__, file.c_str());
    }

    newBinaryEdit->mobj = mapped_object::createMappedObject(desc, newBinaryEdit);
    if (!newBinaryEdit->mobj) {
        startup_printf("%s[%d]: failed to create mapped object for %s\n",
                       FILE__, __LINE__, file.c_str());
        return NULL;
    }

    /* PatchAPI stuffs */
    if (!mgr) {
       newBinaryEdit->initPatchAPI();
    } else {
       newBinaryEdit->setMgr(mgr);
       assert(patch);
       newBinaryEdit->setPatcher(patch);
    }
    newBinaryEdit->addMappedObject(newBinaryEdit->mobj);
    /* End of PatchAPI stuffs */

    // We now need to access the start of the new section we're creating.

    // I'm going through the mapped_object interface for now - 
    // I assume we'll pass it to DynSymtab, then add our base
    // address to it at the mapped_ level. 
    Symtab* linkedFile = newBinaryEdit->getAOut()->parse_img()->getObject();
    Region *newSec = NULL;
    linkedFile->findRegion(newSec, ".dyninstInst");
    if (newSec) {
         // We're re-instrumenting - will fail for now
         fprintf(stderr, "ERROR:  unable to open/reinstrument previously instrumented binary %s!\n", file.c_str());
         return NULL;
    }
    Address base = linkedFile->getFreeOffset(50*1024*1024);
    base += (1024*1024);
    base -= (base & (1024*1024-1));

    newBinaryEdit->highWaterMark_ = base;
    newBinaryEdit->lowWaterMark_ = newBinaryEdit->highWaterMark_;

    // Testing

    newBinaryEdit->makeInitAndFiniIfNeeded();

    newBinaryEdit->createMemoryBackingStore(newBinaryEdit->getAOut());
    newBinaryEdit->initialize();

    //Don't count initialization in determining dirty
    newBinaryEdit->isDirty_ = false; //!(foundInit && foundFini);
    return newBinaryEdit;
}

#if !defined(os_linux) && !defined(os_freebsd)
void BinaryEdit::makeInitAndFiniIfNeeded()
{
}

bool BinaryEdit::archSpecificMultithreadCapable() {
    return false;
}
#endif

bool BinaryEdit::getStatFileDescriptor(const std::string &name, fileDescriptor &desc) {
   desc = fileDescriptor(name.c_str(),
                         0, // code base address
                         0); // data base address
   return true;
}

#if !defined(os_linux) && !defined(os_freebsd)
mapped_object *BinaryEdit::openResolvedLibraryName(std::string filename, std::map<std::string, BinaryEdit *> &allOpened) {
    /*
     * Note: this does not actually do any library name resolution, as that is OS-dependent
     * If resolution is required, it should be implemented in an OS-dependent file
     * (see linux.C for an example)
     *
     * However, this version allows the RT library to be opened with this function regardless
     * if library name resolution has been implemented on a platform.
     */
    std::map<std::string, BinaryEdit *> retMap;
    assert(mgr());
    BinaryEdit *temp = BinaryEdit::openFile(filename, mgr(), patcher());

    if( temp && temp->getAddressWidth() == getAddressWidth() ) {
       allOpened.insert(std::make_pair(filename, temp));
       return temp->getMappedObject();
    }
    
    return NULL;
}

bool BinaryEdit::getResolvedLibraryPath(const std::string &, std::vector<std::string> &) {
    assert(!"Not implemented");
    return false;
}
#endif

#if !(defined(cap_binary_rewriter) && (defined(arch_x86) || defined(arch_x86_64)\
		|| defined(arch_power)   \
		|| defined(arch_aarch64) \
		)) 
bool BinaryEdit::doStaticBinarySpecialCases() {
    return true;
}
#endif

bool BinaryEdit::isMultiThreadCapable()
{
   Symtab *symtab = mobj->parse_img()->getObject();
   std::vector<std::string> depends = symtab->getDependencies();
   for (std::vector<std::string>::iterator curDep = depends.begin();
        curDep != depends.end(); curDep++) {
     if(    (curDep->find("libpthread") != std::string::npos) 
         || (curDep->find("libthread") != std::string::npos)
         || (curDep->find("libthr") != std::string::npos) )
     {
        return true;
     }
   }

   return archSpecificMultithreadCapable();
}

bool BinaryEdit::getAllDependencies(std::map<std::string, BinaryEdit*>& deps)
{
   Symtab *symtab = mobj->parse_img()->getObject();
   std::deque<std::string> depends;
   depends.insert(depends.end(), symtab->getDependencies().begin(), symtab->getDependencies().end());
   while(!depends.empty())
   {
     std::string lib = depends.front();
     if(deps.find(lib) == deps.end()) {
        std::map<std::string, BinaryEdit*> res;
        if(!openResolvedLibraryName(lib, res)) return false;
         std::map<std::string, BinaryEdit*>::iterator bedit_it;
         for(bedit_it = res.begin(); bedit_it != res.end(); ++bedit_it) {
           if (bedit_it->second) {
             deps.insert(*bedit_it);
             if(!bedit_it->second->getAllDependencies(deps))
             {
               return false;
             }
           } else {
             return false;
           }
         }
     }
     depends.pop_front();
   }
   return true;
}

bool BinaryEdit::writeFile(const std::string &newFileName) 
{
   // Step 1: changes. 


      inst_printf(" writing %s ... \n", newFileName.c_str());

      Symtab *symObj = mobj->parse_img()->getObject();

      // link to the runtime library if tramp guards are currently enabled
      if ( !symObj->isStaticBinary() && !BPatch::bpatch->isTrampRecursive() ) {
          assert(!runtime_lib.empty());
          symObj->addLibraryPrereq((*runtime_lib.begin())->fileName());
      }

      if( symObj->isStaticBinary() && isDirty() ) {
         if( !doStaticBinarySpecialCases() ) {
	   cerr << "Failed to write file " << newFileName << ": static binary handler failed" << endl;
	   return false;
         }
      }

   delayRelocation_ = false;
      relocate();
      
      vector<Region*> oldSegs;
      symObj->getAllRegions(oldSegs);

      //Write any traps to the mutatee
      if (canUseTraps()) {
         trapMapping.shouldBlockFlushes(false);
         trapMapping.flush();
      }

      // Now, we need to copy in the memory of the new segments
      for (unsigned i = 0; i < oldSegs.size(); i++) {
         codeRange *segRange = NULL;
         if (!memoryTracker_.find(oldSegs[i]->getMemOffset(), segRange)) {
               continue;
         }
	 memoryTracker* mt = dynamic_cast<memoryTracker*>(segRange);
	 assert(mt);
	 if(mt->dirty) {
            oldSegs[i]->setPtrToRawData(segRange->get_local_ptr(), oldSegs[i]->getMemSize());
	 }
      }

      // Okay, that does it for the old stuff.

      // Now we need to get the new stuff. That's all the allocated memory. First, big
      // buffer to hold it.  Use calloc so gaps from inferiorFree/Realloc are just zero.

      void *newSectionPtr = calloc(highWaterMark_ - lowWaterMark_, 1);

      std::vector<codeRange *> writes;
      memoryTracker_.elements(writes);

      for (unsigned i = 0; i < writes.size(); i++) {
         assert(newSectionPtr);
         memoryTracker *tracker = dynamic_cast<memoryTracker *>(writes[i]);
         assert(tracker);
         if (!tracker->alloced) continue;

         // Copy whatever is in there into the big buffer, at the appropriate address
         assert(tracker->get_address() >= lowWaterMark_);
         Address offset = tracker->get_address() - lowWaterMark_;
         assert((offset + tracker->get_size()) < highWaterMark_);
         void *ptr = (void *)(offset + (Address)newSectionPtr);
         memcpy(ptr, tracker->get_local_ptr(), tracker->get_size());
      }
            
      // Righto. Now, that includes the old binary - by design - 
      // so skip it and see what we're talking about size-wise. Which should
      // be less than the highWaterMark, so we can double-check.

      // Next, make a new section. We have the following parameters:
      // Offset vaddr: we get this from Symtab - "first free address with sufficient space"
      // std::string name: without reflection, ".dyninstInst"
      // unsigned long flags: these are a SymtabAPI abstraction. We're going with text|data because
      //    we might have both.
      // bool loadable: heck yeah...
        
      Region *newSec = NULL;
      symObj->findRegion(newSec, ".dyninstInst");
      if (newSec) {
         // We're re-instrumenting - will fail for now
         fprintf(stderr, "ERROR:  unable to open/reinstrument previously instrumented binary %s!\n", newFileName.c_str());
         return false;
      }

      symObj->addRegion(lowWaterMark_,
                        newSectionPtr,
                        highWaterMark_ - lowWaterMark_,
                        ".dyninstInst",
                        Region::RT_TEXTDATA,
                        true);
      
      symObj->findRegion(newSec, ".dyninstInst");
      assert(newSec);

      
      if (mobj == getAOut()) {
         // Add dynamic symbol relocations
         for (unsigned i=0; i < dependentRelocations.size(); i++) {
            Address to = dependentRelocations[i]->getAddress();
            Symbol *referring = dependentRelocations[i]->getReferring();
               
            // Create the relocationEntry
            relocationEntry localRel(to, referring->getMangledName(), referring,
                                     relocationEntry::getGlobalRelType(getAddressWidth(), referring));
	    
            symObj->addExternalSymbolReference(referring, newSec, localRel);
         }
      }
      
      std::vector<Symbol *> newSyms;
      auto *mod = symObj->getContainingModule(lowWaterMark_);
      if(!mod) {
	  mod = new Module(lang_Unknown, lowWaterMark_, "dyninstInst", symObj);
	  symObj->getObject()->addModule(mod);
      }
      buildDyninstSymbols(newSyms, newSec, mod);
      for (unsigned i = 0; i < newSyms.size(); i++) {
         symObj->addSymbol(newSyms[i]);
      }
      
      // Okay, now...
      // Hand textSection and newSection to DynSymtab.
        
      // First, textSection.
        
      // From the SymtabAPI documentation: we have the following methods we want to use.
      // Symtab::addSection(Offset vaddr, void *data, unsigned int dataSize, std::string name, 
      //                    unsigned long flags, bool loadable)
      // Symtab::updateCode(void *buffer, unsigned size)
      // Symtab::emit(std::string filename)
        
      // First, text
      
      
      // And now we generate the new binary
      if (!symObj->emit(newFileName.c_str())) {
         SymtabError lastError = Symtab::getLastSymtabError();
         showErrorCallback(109, Symtab::printError(lastError));
         return false;
      }
   return true;
}

Address BinaryEdit::maxAllocedAddr() {
   inferiorFreeCompact();
   Address hi = lowWaterMark_;

   for (auto iter = heap_.heapActive.begin();
        iter != heap_.heapActive.end(); ++iter) {
      Address localHi = iter->second->addr + iter->second->length;
      if (localHi > hi) hi = localHi;
   }
   return hi;
}


bool BinaryEdit::inferiorMallocStatic(unsigned size) {
    // Should be set by now
    assert(highWaterMark_ != 0);
    
    Address newStart = highWaterMark_;

    // If there is a free heap that _ends_ at the highWaterMark,
    // just extend it. This is a special case of inferiorFreeCompact;
    // when we make that function faster, we can just call it. 
    bool found = false;
    for (unsigned i = 0; i < heap_.heapFree.size(); i++) {
        heapItem *h = heap_.heapFree[i];
        assert(h);
        Address end = h->addr + h->length;
        if (end == newStart) {
            found = true;
            h->length += size;
            break;
        }
    }
    if (!found) {
        // Build tracking objects for it
        heapItem *h = new heapItem(highWaterMark_, 
                                   size,
                                   anyHeap,
                                   true,
                                   HEAPfree);
        addHeap(h);
    }

    highWaterMark_ += size;

    return true;
}


bool BinaryEdit::createMemoryBackingStore(mapped_object *obj) {
    // We want to create a buffer for every section in the
    // binary so that we can store updates.

    Symtab *symObj = obj->parse_img()->getObject();
    vector<Region*> regs;
    symObj->getAllRegions(regs);

   for (unsigned i = 0; i < regs.size(); i++) {
      if (regs[i]->getRegionType() == Region::RT_BSS || (regs[i]->getMemSize() == 0))
      {
         continue;
      }
      auto *newTracker = new memoryTracker(regs[i]->getMemOffset(),
                                           regs[i]->getMemSize(),
                                           regs[i]->getPtrToRawData());
      newTracker->alloced = false;
      memoryTracker_.insert(newTracker);
   }
   return true;
}


bool BinaryEdit::initialize() {
   //Load the RT library
   
   // Create the tramp guard
   
   // Initialization. For now we're skipping threads, since we can't
   // get the functions we need. However, we kinda need the recursion
   // guard. This is an integer (one per thread, for now - 1) that 
   // begins initialized to 1.

    return true;
}

void BinaryEdit::addDependentRelocation(Address to, Symbol *referring) {
  // prevent duplicate relocations
  std::vector<depRelocation *>::iterator it;
  for (it = dependentRelocations.begin(); it != dependentRelocations.end(); it++)
    if ((*it)->getAddress() == to && (*it)->getReferring() == referring)
      return;
  // create a new relocation and add it to the collection
  depRelocation *reloc = new depRelocation(to, referring);
  dependentRelocations.push_back(reloc);
}

Address BinaryEdit::getDependentRelocationAddr(Symbol *referring) {
	Address retAddr = 0x0;
	for (unsigned i=0; i < dependentRelocations.size(); i++) {
		if (dependentRelocations[i]->getReferring() == referring) {
			retAddr = dependentRelocations[i]->getAddress();
			break;
		}
	}
	return retAddr;
}

void BinaryEdit::addLibraryPrereq(std::string libname) {
   Symtab *symObj = mobj->parse_img()->getObject();
   symObj->addLibraryPrereq(libname);
}


// Build a list of symbols describing instrumentation and relocated functions. 
// To keep this list (somewhat) short, we're doing one symbol per extent of 
// instrumentation + relocation for a particular function. 
// New: do this for one mapped object. 
void BinaryEdit::buildDyninstSymbols(std::vector<Symbol *> &newSyms, 
                                     Region *newSec,
                                     Module *newMod) {
   for (std::vector<SymtabAPI::Symbol *>::iterator iter = newDyninstSyms_.begin();
        iter != newDyninstSyms_.end(); ++iter) {
      (*iter)->setModule(newMod);
      (*iter)->setRegion(newSec);
      newSyms.push_back(*iter);
   }
                                                                              

   for (CodeTrackers::iterator i = relocatedCode_.begin();
        i != relocatedCode_.end(); ++i) {
      Relocation::CodeTracker *CT = *i;
      func_instance *currFunc = NULL;
      Address start = 0;
      unsigned size = 0;
      
      for (Relocation::CodeTracker::TrackerList::const_iterator iter = CT->trackers().begin();
           iter != CT->trackers().end(); ++iter) {
         const Relocation::TrackerElement *tracker = *iter;
         
         func_instance *tfunc = tracker->func();
         
         if (currFunc != tfunc) {
            // Starting a new function
            if (currFunc) {
               // Record the old one
               // currfunc set
               // start set
               size = tracker->reloc() - start;
               
               std::string name = currFunc->prettyName();
               name.append("_dyninst");
               
               Symbol *newSym = new Symbol(name.c_str(),
                                           Symbol::ST_FUNCTION,
                                           Symbol::SL_GLOBAL,
                                           Symbol::SV_DEFAULT,
                                           start,
                                           newMod,
                                           newSec,
                                           size);                                        
               newSyms.push_back(newSym);
            }
            currFunc = tfunc;
            start = tracker->reloc();
            size = 0;
         }
         else {
            // Accumulate size
            size = tracker->reloc() - start;
         }
      }
   }
}
    
void BinaryEdit::markDirty()
{
   isDirty_ = true;
}

bool BinaryEdit::isDirty()
{
   return isDirty_;
}

mapped_object *BinaryEdit::getMappedObject()
{
   return mobj;
}

void BinaryEdit::setupRTLibrary(std::vector<BinaryEdit *> &r)
{
   rtlib = r;

   // Update addressSpace collection for RT library
   runtime_lib.clear();
   std::vector<BinaryEdit *>::iterator rtlib_it;
   for(rtlib_it = r.begin(); rtlib_it != r.end(); ++rtlib_it) {
       runtime_lib.insert((*rtlib_it)->getMappedObject());
   }
}

vector<BinaryEdit *> &BinaryEdit::rtLibrary()
{
   return rtlib;
}

func_instance *BinaryEdit::findOnlyOneFunction(const std::string &name,
                                              const std::string &libname,
                                              bool search_rt_lib)
{
   func_instance *f = AddressSpace::findOnlyOneFunction(name, libname, search_rt_lib);
   if (!f && search_rt_lib) {
      std::vector<BinaryEdit *>::iterator rtlib_it;
      for(rtlib_it = rtlib.begin(); rtlib_it != rtlib.end(); ++rtlib_it) {
          f = (*rtlib_it)->findOnlyOneFunction(name, libname, false);
          if( f ) break;
      }
   }
   return f;
}

void BinaryEdit::setMultiThreadCapable(bool b)
{
   multithread_capable_ = b;
}

void BinaryEdit::addSibling(BinaryEdit *be)
{
   if (this != be) {
      siblings.push_back(be);
   }
}

std::vector<BinaryEdit *> &BinaryEdit::getSiblings()
{
   return siblings;
}



// Here's the story. We may need to install a trap handler for instrumentation
// to work in the rewritten binary. This doesn't play nicely with trap handlers
// that the binary itself registers. So we're going to replace every call to
// sigaction in the binary with a call to our wrapper. This wrapper:
//   1) Ignores attempts to register a SIGTRAP
//   2) Passes everything else through to sigaction
// It's called "dyn_sigaction".

bool BinaryEdit::usedATrap() {
    return (!trapMapping.empty());
}

// Find all calls to sigaction equivalents and replace with
// calls to dyn_<sigaction_equivalent_name>. 
bool BinaryEdit::replaceTrapHandler() {

    vector<string> sigaction_names;
    OS::get_sigaction_names(sigaction_names);

    std::vector<func_instance *> allFuncs;
    getAllFunctions(allFuncs);
    
    bool replaced = false;
    for (unsigned nidx = 0; nidx < sigaction_names.size(); nidx++) 
    {
        // find replacement function
        std::stringstream repname;
        repname << "dyn_" << sigaction_names[nidx];
        func_instance *repfunc = findOnlyOneFunction(repname.str().c_str());
        assert(repfunc);
    
        // replace all callsites to current sigaction function
        for (unsigned i = 0; i < allFuncs.size(); i++) {
            func_instance *func = allFuncs[i];        
            assert(func);

            for (PatchFunction::Blockset::const_iterator iter = func->blocks().begin();
                 iter != func->blocks().end(); ++iter) {
                block_instance* iblk = SCAST_BI(*iter);
                if (iblk->containsCall()) {
                    
                    // the function name could have up to two underscores
                    // prepended (e.g., sigaction, _sigaction, __sigaction),
                    // try all three possibilities for each name
                    std::string calleeName = iblk->calleeName();
                    std::string sigactName = sigaction_names[nidx];
                    for (int num_s=0; 
                         num_s <= 2; 
                         num_s++, sigactName.append(string("_"),0,1)) {
                        if (calleeName == sigactName) {
                            modifyCall(iblk, repfunc, func);
                            replaced = true;
                            break;
                        }
                    }
                }
            } // for each func in the rewritten binary
        } // for each sigaction-equivalent function to replace
    }

    if (!replaced) return true;

    /* PatchAPI stuffs */
    return AddressSpace::patch(this);
    /* End of PatchAPI stuffs */
    // return relocate();
}

bool BinaryEdit::needsPIC()
{
   Symtab *symtab = getMappedObject()->parse_img()->getObject();
   assert(symtab);
   if(getMappedObject()->fileName().find("lib") == 0)
   {
       if(getMappedObject()->fileName().find(".so") != std::string::npos)
       {
           return true;
       }
   }
   //If there is a fixed load address, then we can calculate 
   // absolute addresses.
   return (symtab->getLoadAddress() == 0);  
}

void BinaryEdit::addTrap(Address from, Address to, codeGen &gen) {
   gen.invalidate();
   gen.allocate(4);
   gen.setAddrSpace(this);
   gen.setAddr(from);
   if (sigILLTrampoline_) {
      insnCodeGen::generateIllegal(gen);
   } else {
      insnCodeGen::generateTrap(gen);
   }
   trapMapping.addTrapMapping(from, to, true);
   springboard_cerr << "Generated springboard trap " << hex << from << "->" << to << dec << endl;
}

