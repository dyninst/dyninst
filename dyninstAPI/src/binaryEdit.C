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

// $Id: binaryEdit.C,v 1.26 2008/10/28 18:42:44 bernat Exp $

#include "binaryEdit.h"
#include "common/h/headers.h"
#include "mapped_object.h"
#include "mapped_module.h"
#include "multiTramp.h"
#include "debug.h"
#include "os.h"

// #define USE_ADDRESS_MAPS

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
                               const void *inSelf) {
    Address addr = (Address) inOther;
    
    // Look up this address in the code range tree of memory
    codeRange *range = NULL;
    if (!memoryTracker_->find(addr, range))
        return false;
    assert(addr >= range->get_address());

    Address offset = addr - range->get_address();
    assert(offset < range->get_size());

    void *local_ptr = ((void *) (offset + (Address)range->get_local_ptr()));
    memcpy(const_cast<void *>(inSelf), local_ptr, size);

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
       if (!memoryTracker_->find(addr, range)) {
          assert(0);
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
       //inst_printf("Copying to 0x%lx [base=0x%lx] from 0x%lx (%d bytes)  target=0x%lx  offset=0x%lx\n", 
              //local_ptr, range->get_local_ptr(), local, chunk_size, addr, offset);
       //range->print_range();
       memcpy(local_ptr, (void *)local, chunk_size);
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

const Address ADDRESS_LO = (Address)0;
const Address ADDRESS_HI = (Address)(~(Address)0);
const unsigned HEAP_STAT_BUF_SIZE = (0x100000);

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
        if (ret) break;
    }

#ifdef BINEDIT_DEBUG
    if (ret == 0x854106c) {
        fprintf(stderr, "RETURNING B0RKEN ADDRESS\n");
    }
#endif
    
    return ret;
}

unsigned BinaryEdit::getAddressWidth() const {
    assert(mapped_objects.size());
    
    return mapped_objects[0]->parse_img()->getObject()->getAddressWidth();
}

bool BinaryEdit::multithread_capable(bool) {
    // TODO
    return false;
}

bool BinaryEdit::multithread_ready(bool) {
    return multithread_capable();
}

void BinaryEdit::deleteGeneratedCode(generatedCodeObject *del) {
  // This can happen - say that someone writes a file (which generates), 
  // then goes around uninstrumenting... yeah, it can happen.

  // Or a failed atomic insert.

    // No reason to delay deletion.
    delete del;
}

BinaryEdit::BinaryEdit() : 
   highWaterMark_(0),
   lowWaterMark_(0),
   isDirty_(false),
   memoryTracker_(NULL) 
{
}

BinaryEdit::~BinaryEdit() 
{
}

void BinaryEdit::deleteBinaryEdit() {
    deleteAddressSpace();
    highWaterMark_ = 0;
    lowWaterMark_ = 0;

    // TODO: is this cleanup necessary?
    depRelocation *rel;
    while (dependentRelocations.size() > 0) {
        rel = dependentRelocations[0];
        dependentRelocations.erase(dependentRelocations.begin());
        delete rel;
    }
}

BinaryEdit *BinaryEdit::openFile(const std::string &file) {
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

    newBinaryEdit->mapped_objects.push_back(newBinaryEdit->mobj);
    newBinaryEdit->addOrigRange(newBinaryEdit->mobj);

    // We now need to access the start of the new section we're creating.

    // I'm going through the mapped_object interface for now - 
    // I assume we'll pass it to DynSymtab, then add our base
    // address to it at the mapped_ level. 
    
    newBinaryEdit->highWaterMark_ = newBinaryEdit->getAOut()->parse_img()->getObject()->getFreeOffset(50*1024*1024);
    newBinaryEdit->lowWaterMark_ = newBinaryEdit->highWaterMark_;


    newBinaryEdit->createMemoryBackingStore(newBinaryEdit->getAOut());
    newBinaryEdit->initialize();

    newBinaryEdit->isDirty_ = false; //Don't count initialization in 
                                     // determining dirty
    return newBinaryEdit;
}

bool BinaryEdit::getStatFileDescriptor(const std::string &name, fileDescriptor &desc) {
   desc = fileDescriptor(name.c_str(),
                         0, // code base address
                         0); // data base address
   return true;
}

#if !defined(cap_binary_rewriter)
std::string BinaryEdit::resolveLibraryName(std::string)
{
	return NULL;
}
#endif

bool BinaryEdit::getAllDependencies(std::queue<std::string> &deps)
{
   Symtab *symtab = mobj->parse_img()->getObject();
   std::vector<std::string> depends = symtab->getDependencies();
   for (unsigned i=0; i<depends.size(); i++) {
      std::string full_name = resolveLibraryName(depends[i]);
      if (full_name.length())
         deps.push(full_name);
   }
   return true;
}

bool BinaryEdit::writeFile(const std::string &newFileName) 
{
   // We've made a bunch of changes and additions to the
   // mapped object.
   //   Changes: modifiedRanges_
   //   Additions: textRanges_, excepting the file itself. 
   // 
   // Although, since we're creating a new file name we want
   // textRanges. Basically, we want to serialize the contents
   // of textRanges_, dataRanges_, and modifiedRanges_.

   // A _lot_ of this method will depend on how the new file
   // generation ends up working. Do we provide buffers? I'm guessing
   // so.

   // Step 1: changes. 

      inst_printf(" writing %s ... \n", newFileName.c_str());

      Symtab *symObj = mobj->parse_img()->getObject();

      vector<Segment> oldSegs;
      symObj->getSegments(oldSegs);

      vector<Segment> newSegs = oldSegs;

      // Now, we need to copy in the memory of the new segments
      for (unsigned i = 0; i < newSegs.size(); i++) {
         codeRange *segRange = NULL;
         if (!memoryTracker_->find(newSegs[i].loadaddr, segRange)) {
            // Looks like BSS
            if (newSegs[i].name == ".bss")
               continue;
            //inst_printf (" segment name: %s\n", newSegs[i].name.c_str());
            assert(0);
         }
         //inst_printf(" ==> memtracker: Copying to 0x%lx from 0x%lx\n", 
         //newSegs[i].loadaddr, segRange->get_local_ptr());
         newSegs[i].data = segRange->get_local_ptr();
      }

      // Okay, that does it for the old stuff.

      // Now we need to get the new stuff. That's all the allocated memory. First, big
      // buffer to hold it.

      void *newSectionPtr = malloc(highWaterMark_ - lowWaterMark_);

      pdvector<codeRange *> writes;
      memoryTracker_->elements(writes);

      for (unsigned i = 0; i < writes.size(); i++) {
         memoryTracker *tracker = dynamic_cast<memoryTracker *>(writes[i]);
         assert(tracker);
         //inst_printf("memory tracker: 0x%lx  load=0x%lx  size=%d  %s\n", 
         //tracker->get_local_ptr(), tracker->get_address(), tracker->get_size(),
         //tracker->alloced ? "[A]" : "");
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
         fprintf(stderr, "ERROR:  unable to reinstrument previously instrumented binary!\n");
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
         Symbol *referring, *newSymbol;
         for (unsigned i=0; i < dependentRelocations.size(); i++) {
            Address to = dependentRelocations[i]->getAddress();
            referring = dependentRelocations[i]->getReferring();
            newSymbol = new Symbol(
                                   referring->getName(), "DEFAULT_MODULE",
                                   Symbol::ST_FUNCTION, Symbol::SL_GLOBAL,
                                   Symbol::SV_DEFAULT, (Address)0, NULL, 8, true, false);
            symObj->addSymbol(newSymbol, referring);
            if (!symObj->hasRel() && !symObj->hasRela()) {
               // TODO: probably should add new relocation section and
               // corresponding .dynamic table entries
               fprintf(stderr, "ERROR:  binary has no pre-existing relocation sections!\n");
               return false;
            } else if (!symObj->hasRel() && symObj->hasRela()) {
               newSec->addRelocationEntry(to, newSymbol, relocationEntry::dynrel, Region::RT_RELA);
            } else {
               if (mobj->isSharedLib()) {
                  //inst_printf("  ::: shared lib jump slot - 0x%lx [base=0x%lx]\n", 
                  //to - obj->imageOffset(), obj->imageOffset());
                  newSec->addRelocationEntry(
                                             to - mobj->imageOffset(), newSymbol, relocationEntry::dynrel);
               }
               else {
                  //inst_printf("  ::: regular jump slot - 0x%lx\n", to);
                  newSec->addRelocationEntry(to, newSymbol, relocationEntry::dynrel);
               }
            }
         }
      }

      pdvector<Symbol *> newSyms;
      buildDyninstSymbols(newSyms, newSec);
      for (unsigned i = 0; i < newSyms.size(); i++) {
         symObj->addSymbol(newSyms[i], false);
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
      assert(symObj);
        
      for (unsigned i = 0; i < oldSegs.size(); i++) {
         if (oldSegs[i].data != newSegs[i].data) {
            //inst_printf("Data not equivalent, %d, %s  load=0x%lx\n", 
            //i, oldSegs[i].name.c_str(), oldSegs[i].loadaddr);
            if (oldSegs[i].name == ".text") {
               //inst_printf("  TEXT SEGMENT: old-base=0x%lx  old-size=%d  new-base=0x%lx  new-size=%d\n",
               //oldSegs[i].data, oldSegs[i].size, newSegs[i].data, newSegs[i].size);
               symObj->updateCode(newSegs[i].data,
                                  newSegs[i].size);
            }
         }
      }
        

      // And now we generate the new binary
      //if (!symObj->emit(newFileName.c_str())) {
      if (!symObj->emit(newFileName.c_str())) {
         return false;
      }
   return true;
}

bool BinaryEdit::inferiorMallocStatic(unsigned size) {
    // Should be set by now
    assert(highWaterMark_ != 0);

#if defined(USE_ADDRESS_MAPS)
    void *buf = malloc(size);
    if (!buf) return false;
#endif
    
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

    memoryTracker *newTracker = new memoryTracker(highWaterMark_, size);
    newTracker->alloced = true;
    if (!memoryTracker_)
       memoryTracker_ = new codeRangeTree();
    memoryTracker_->insert(newTracker);

    //printf("[%s:%u] - New memory tracker for %p at 0x%lx:  load=0x%lx  size=%d\n",
    //__FILE__, __LINE__, this, 
    //newTracker->get_local_ptr(), highWaterMark_, size);

    highWaterMark_ += size;


    return true;
}


bool BinaryEdit::createMemoryBackingStore(mapped_object *obj) {
    // We want to create a buffer for every section in the
    // binary so that we can store updates.

    Symtab *symObj = obj->parse_img()->getObject();
    /*
    vector<Segment> segs;
    symObj->getSegments(segs);*/
	vector<Region*> regs;
	symObj->getAllRegions(regs);

    for (unsigned i = 0; i < regs.size(); i++) {
        memoryTracker *newTracker = NULL;
        if (regs[i]->getRegionName() == ".bss") {
            continue;
        }
        else {
			/*
            newTracker = new memoryTracker(segs[i].loadaddr,
                                           segs[i].size,
                                           segs[i].data);*/
			newTracker = new memoryTracker(regs[i]->getRegionAddr(),
										   regs[i]->getDiskSize(),
										   regs[i]->getPtrToRawData());
		}
        newTracker->alloced = false;
        if (!memoryTracker_)
           memoryTracker_ = new codeRangeTree();
        memoryTracker_->insert(newTracker);

        //printf("[%s:%u] - New memory tracker for %p at 0x%lx:  load=0x%lx  size=%d  [%s]\n",
        //__FILE__, __LINE__, this,
        //newTracker->get_local_ptr(), segs[i].loadaddr, segs[i].size, segs[i].name.c_str());
    }

    
    return true;
}


bool BinaryEdit::initialize() {
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


// Build a list of symbols describing instrumentation and relocated functions. 
// To keep this list (somewhat) short, we're doing one symbol per extent of 
// instrumentation + relocation for a particular function. 
// New: do this for one mapped object. 


void BinaryEdit::buildDyninstSymbols(pdvector<Symbol *> &newSyms, 
                                     Region *newSec) {
    pdvector<codeRange *> ranges;
    textRanges_.elements(ranges);

    int_function *currFunc = NULL;
    codeRange *startRange = NULL;

    Address startAddr = 0;
    unsigned size = 0;

    for (unsigned i = 0; i < ranges.size(); i++) {
        multiTramp *multi = ranges[i]->is_multitramp();
        bblInstance *bbl = ranges[i]->is_basicBlockInstance();

        bool finishCurrentRegion = false;
        bool startNewRegion = false;
        bool extendCurrentRegion = false;

        if (multi) {
            if (multi->func() != currFunc) {
                finishCurrentRegion = true;
                startNewRegion = true;
            }
            else {
                extendCurrentRegion = true;
            }
        }
        if (bbl) {
            if (bbl->func() != currFunc) {
                finishCurrentRegion = true;
                startNewRegion = true;
            }
            else {
                extendCurrentRegion = true;
            }
        }
        else
            continue;

        
        if (finishCurrentRegion && (currFunc != NULL)) {
            std::string name = currFunc->prettyName();
            name.append("_dyninst");

            Symbol *newSym = new Symbol(name.c_str(),
                                        "DyninstInst",
                                        Symbol::ST_FUNCTION,
                                        Symbol::SL_GLOBAL,
                                        Symbol::SV_DEFAULT,
                                        startAddr,
                                        newSec,
                                        size,
                                        (void *)startRange);
            newSyms.push_back(newSym);

            currFunc = NULL;
            startAddr = 0;
            size = 0;
            startRange = NULL;
        }
        if (startNewRegion) {
            assert(currFunc == NULL);
            
            currFunc = (multi != NULL) ? (multi->func()) : (bbl->func());
            assert(currFunc != NULL);
            startRange = ranges[i];
            startAddr = ranges[i]->get_address();
            size = ranges[i]->get_size();
        }
        if (extendCurrentRegion) {
            size += ranges[i]->get_size() + (ranges[i]->get_address() - (startAddr + size));
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
