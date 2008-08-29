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

// $Id: binaryEdit.C,v 1.23 2008/08/29 20:24:17 legendre Exp $

#include "binaryEdit.h"
#include "common/h/headers.h"
#include "mapped_object.h"
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
    if (!memoryTracking_.find(addr, range))
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

    bool debug = false;
    if (inOther == (void *)0x81c6d75) {
        debug = true;
    }

    while (to_do) {
       // Look up this address in the code range tree of memory
       codeRange *range = NULL;
       if (!memoryTracking_.find(addr, range)) {
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

    if (ret == 0x854106c) {
        fprintf(stderr, "RETURNING B0RKEN ADDRESS\n");
    }
    
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

BinaryEdit::BinaryEdit() : highWaterMark_(0) {
}

BinaryEdit::~BinaryEdit() {
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

    mapped_object *bin = mapped_object::createMappedObject(desc, newBinaryEdit);
    if (!bin) {
        startup_printf("%s[%d]: failed to create mapped object for %s\n",
                       FILE__, __LINE__, file.c_str());
        return NULL;
    }

    newBinaryEdit->mapped_objects.push_back(bin);
    newBinaryEdit->addOrigRange(bin);

    // We now need to access the start of the new section we're creating.

    // I'm going through the mapped_object interface for now - 
    // I assume we'll pass it to DynSymtab, then add our base
    // address to it at the mapped_ level. 
    
    newBinaryEdit->highWaterMark_ = newBinaryEdit->getAOut()->parse_img()->getObject()->getFreeOffset(50*1024*1024);
    newBinaryEdit->lowWaterMark_ = newBinaryEdit->highWaterMark_;


    newBinaryEdit->createMemoryBackingStore();

    newBinaryEdit->initialize();
    
    newBinaryEdit->openAllDependencies();

    return newBinaryEdit;
}

bool BinaryEdit::getStatFileDescriptor(const std::string &name, fileDescriptor &desc) {
    // Wow, I think this was easy
    desc = fileDescriptor(name.c_str(),
                          0, // code base address
                          0, // data base address
                          false); // a.out
    return true;
}

#if defined(os_windows)
std::string* BinaryEdit::resolveLibraryName(const std::string &)
{
	return NULL;
}
#else
std::string* BinaryEdit::resolveLibraryName(const std::string &filename)
{
    char *libPathStr, *libPath;
    std::vector<std::string> libPaths;
    struct stat dummy;
    std::string* fullPath = NULL;
    bool found = false;
    FILE *ldconfig;
    char buffer[512];
    char *pos, *key, *val;
    
    // prefer qualified file paths
    if (stat(filename.c_str(), &dummy) == 0) {
        fullPath = new std::string(filename);
        return fullPath;
    }

    // search paths from environment variables
    libPathStr = getenv("LD_LIBRARY_PATH");
    libPath = strtok(libPathStr, ":");
    while (libPath != NULL) {
        libPaths.push_back(std::string(libPath));
        libPath = strtok(NULL, ":");
    }
    //libPaths.push_back(std::string(getenv("PWD")));
    for (unsigned int i = 0; !found && i < libPaths.size(); i++) {
        fullPath = new std::string(libPaths.at(i) + "/" + filename);
        if (stat(fullPath->c_str(), &dummy) == 0) {
            found = true;
        } else {
            delete fullPath;
            fullPath = NULL;
        }
    }
    if (fullPath != NULL)
        return fullPath;

    // search ld.so.cache
    ldconfig = popen("/sbin/ldconfig -p", "r");
    if (ldconfig != NULL) {
        fgets(buffer, 512, ldconfig); // ignore first line
        while (fullPath == NULL && fgets(buffer, 512, ldconfig) != NULL) {
            pos = buffer;
            while (*pos == ' ' || *pos == '\t') pos++;
            key = pos;
            while (*pos != ' ') pos++;
            *pos = '\0';
            while (*pos != '=' && *(pos+1) != '>') pos++;
            pos += 2;
            while (*pos == ' ' || *pos == '\t') pos++;
            val = pos;
            while (*pos != '\n' && *pos != '\0') pos++;
            *pos = '\0';
            if (strcmp(key, filename.c_str()) == 0)
                fullPath = new std::string(val);
        }
        fclose(ldconfig);
    }
    if (fullPath != NULL)
        return fullPath;
 
    // search hard-coded system paths
    libPaths.clear();
    libPaths.push_back("/usr/local/lib");
    libPaths.push_back("/usr/share/lib");
    libPaths.push_back("/usr/lib");
    libPaths.push_back("/lib");
    for (unsigned int i = 0; !found && i < libPaths.size(); i++) {
        fullPath = new std::string(libPaths.at(i) + "/" + filename);
        if (stat(fullPath->c_str(), &dummy) == 0) {
            found = true;
        } else {
            delete fullPath;
            fullPath = NULL;
        }
    }

    return fullPath;
}
#endif

bool BinaryEdit::openAllDependencies()
{
    return openAllDependencies(getAOut()->parse_img()->getObject());
}

bool BinaryEdit::openAllDependencies(const std::string &file)
{
    Symtab *st;
    bool result = false;
    std::string* fullPath = resolveLibraryName(file);
    if (fullPath != NULL) {
        Symtab::openFile(st, fullPath->c_str());
        result = openAllDependencies(st);
    }
    return result;
}

bool BinaryEdit::openAllDependencies(Symtab *st)
{
    // extract list of dependencies
    std::vector<std::string> depends = st->getDependencies();

    // for each dependency
    unsigned int count = depends.size();
    for (unsigned int i = 0; i < count; i++) {

        // if we can find the file...
        std::string* fullPath = resolveLibraryName(depends.at(i));
        if (fullPath != NULL) {

            // get list of subdependencies
            Symtab *subst;
            Symtab::openFile(subst, fullPath->c_str());
            std::vector<std::string> subdepends = subst->getDependencies();

            // add any new dependencies to the main list
            for (unsigned int k = 0; k < subdepends.size(); k++) {
                bool found = false;
                for (unsigned int l = 0; !found && l < depends.size(); l++) {
                    if (depends.at(l).compare(subdepends.at(k))==0) {
                        found = true;
                    }
                }
                if (!found) {
                    depends.push_back(subdepends.at(k));
                    count++;
                }
            }

            mapped_object *bin = addSharedObject(fullPath);
            if (bin == NULL) {
                return false;
            }

            delete fullPath;
        }
    }
    return true;
}

bool BinaryEdit::openSharedLibrary(const std::string &file, bool openDependencies)
{
    std::string* fullPath = resolveLibraryName(file);

    if (!fullPath) {
        fprintf(stderr, "ERROR:  unable to open shared library %s\n", file.c_str());
        return false;
    }

    mapped_object *bin = addSharedObject(fullPath);

    delete fullPath;
    if (bin == NULL) {
        return false;
    }

    if (openDependencies) {
        return openAllDependencies(bin->parse_img()->getObject());
    }
    return true;
}

mapped_object * BinaryEdit::addSharedObject(const std::string *fullPath)
{
#ifdef BINEDIT_DEBUG
    fprintf(stdout, "DEBUG - opening new dependency: %s\n", fullPath->c_str());
#endif

    // make sure the executable exists
    if (!OS::executableExists(*fullPath)) {
        startup_printf("%s[%d]:  failed to read file %s\n", 
                FILE__, __LINE__, fullPath->c_str());
        std::string msg = std::string("Can't read executable file ") + 
            (*fullPath) + (": ") + strerror(errno);
        showErrorCallback(68, msg.c_str());
        return NULL;
    }

    // check to make sure the module hasn't already been loaded
    mapped_object *bin = NULL;
    bool found = false;
    for (unsigned int i = 0; i < mapped_objects.size(); i++) {
        if (*fullPath == mapped_objects[i]->fullName()) {
            bin = mapped_objects[i];
            found = true;
            break;
        }
    }

    if (!found) {

        fileDescriptor desc(fullPath->c_str(), 
           0, 0,    // code and data base addresses
           true);   // not a.out

        bin = mapped_object::createMappedObject(desc, this);
        if (!bin) {
            startup_printf("%s[%d]: failed to create mapped object for %s\n",
                          FILE__, __LINE__, fullPath->c_str());
            return NULL;
        }

        mapped_objects.push_back(bin);

#ifdef BINEDIT_DEBUG
        fprintf(stdout, " - added!\n");
    } else {
        fprintf(stdout, " - duplicate\n");
#endif
    }

    return bin;
}

#if 1

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

    Symtab *symObj = getAOut()->parse_img()->getObject();

    vector<Segment> oldSegs;
    symObj->getSegments(oldSegs);

    vector<Segment> newSegs = oldSegs;

    // Now, we need to copy in the memory of the new segments
    for (unsigned i = 0; i < newSegs.size(); i++) {
        codeRange *segRange = NULL;
        if (!memoryTracking_.find(newSegs[i].loadaddr, segRange)) {
            // Looks like BSS
            if (newSegs[i].name == ".bss")
                continue;
            assert(0);
        }
        newSegs[i].data = segRange->get_local_ptr();
    }

    // Okay, that does it for the old stuff.

    // Now we need to get the new stuff. That's all the allocated memory. First, big
    // buffer to hold it.

    void *newSectionPtr = malloc(highWaterMark_ - lowWaterMark_);

    pdvector<codeRange *> writes;
    memoryTracking_.elements(writes);

    for (unsigned i = 0; i < writes.size(); i++) {
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

	// Add dynamic symbol relocations
	Symbol *referring, *newSymbol;
	for (unsigned i=0; i < dependentRelocations.size(); i++) {
		Address to = dependentRelocations[i]->getAddress();
		referring = dependentRelocations[i]->getReferring();
		newSymbol = new Symbol(
				referring->getName(), "DEFAULT_MODULE",
				Symbol::ST_FUNCTION, Symbol::SL_UNKNOWN,
				to, newSec, 8);
		symObj->addSymbol(newSymbol, referring);
        if (!symObj->hasRel() && !symObj->hasRela()) {
            // TODO: probably should add new relocation section and
            // corresponding .dynamic table entries
            fprintf(stderr, "ERROR:  binary has no pre-existing relocation sections!\n");
            return false;
        } else if (!symObj->hasRel() && symObj->hasRela()) {
            newSymbol->getSec()->addRelocationEntry(to, newSymbol, relocationEntry::dynrel, Region::RT_RELA);
        } else {
            newSymbol->getSec()->addRelocationEntry(to, newSymbol, relocationEntry::dynrel);
        }
	}

    // Put in symbol table entries for known code
    pdvector<codeRange *> newCode;
    textRanges_.elements(newCode);
    for (unsigned i = 0; i < newCode.size(); i++) {
        char buffer[1025];        
        
        if (newCode[i]->is_multitramp()) {
            snprintf(buffer, 1024, "%s_inst_0x%lx_0x%lx",
                     newCode[i]->is_multitramp()->func()->symTabName().c_str(),
                     newCode[i]->is_multitramp()->instAddr(),
                     newCode[i]->is_multitramp()->instAddr() +
                     newCode[i]->is_multitramp()->instSize());
        }
        else if (newCode[i]->is_basicBlockInstance()) {
            // Relocated function
            snprintf(buffer, 1024, "%s_reloc",
                     newCode[i]->is_basicBlockInstance()->func()->symTabName().c_str());
        }
        else {
            continue;
        }
        
        Symbol *newSym = new Symbol(buffer,
                                    "DyninstInst",
                                    Symbol::ST_FUNCTION,
                                    Symbol::SL_GLOBAL,
                                    newCode[i]->get_address(),
                                    newSec,
                                    newCode[i]->get_size(),
                                    (void *)newCode[i]);
            
        symObj->addSymbol(newSym, false);
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
            fprintf(stderr, "Data not equivalent, %d, %s\n",
                    i, oldSegs[i].name.c_str());
            if (oldSegs[i].name == ".text") {
                symObj->updateCode(newSegs[i].data,
                                   newSegs[i].size);
            }
        }
    }
    

    // And now we generate the new binary
    if (!symObj->emit(newFileName.c_str())) {
        return false;
    }

    return true;
}

#else // Old version, keeping for reference

bool BinaryEdit::writeFile(const std::string &newFileName) {
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

    Symtab *symObj = getAOut()->parse_img()->getObject();

    vector<Segment> oldSegs;
    symObj->getSegments(oldSegs);

    vector<Segment> newSegs = oldSegs;

    // We may also modify over new code - keep those here
    // when we find them and apply later.
    pdvector<codeRange *> danglingMods;
    
    // And now apply changes
    // Note: I want an iterator. Stat. 
    pdvector<codeRange *> modified;
    modifiedRanges_.elements(modified);
    for (unsigned i = 0; i < modified.size(); i++) {
        assert(modified[i]->get_address() >= getAOut()->get_address());
        bool found = false;

        // Find the segment this modification occurs in
        for (unsigned j = 0; j < oldSegs.size(); j++) {
            if ((modified[i]->get_address() >= oldSegs[j].loadaddr) &&
                (modified[i]->get_address() < (oldSegs[j].loadaddr + oldSegs[j].size))) {
                if (oldSegs[j].data == newSegs[j].data) {
                    newSegs[j].data = malloc(oldSegs[j].size);
                    memcpy(newSegs[j].data, oldSegs[j].data, oldSegs[j].size);
                }
                Address offset = modified[i]->get_address() - oldSegs[j].loadaddr;

                assert(offset < oldSegs[j].size);
                void *local_addr = (void *)((Address) newSegs[j].data + offset);
                memcpy(local_addr, modified[i]->get_local_ptr(), modified[i]->get_size());
                found = true;

                break;
            }
        }
        if (!found) {
	  if ((modified[i]->get_address() >= lowWaterMark_) &&
	      ((modified[i]->get_address() + modified[i]->get_size()) <= highWaterMark_)) {
            danglingMods.push_back(modified[i]);
	  }
	  else {
	    // Where does this one belong?
	    fprintf(stderr, "Modification to unknown location 0x%lx\n",
		    modified[i]->get_address());
	    for (unsigned j = 0; j < oldSegs.size(); j++) {
	      fprintf(stderr, "Segment %d: 0x%lx to 0x%lx\n",
		      j, oldSegs[j].loadaddr, oldSegs[j].loadaddr + oldSegs[j].size);
	      
	    }
	  }
	  
	}
	
    }
    
	    

    // Okay, that's our text section. 
    // Now for the new stuff.
    
    pdvector<codeRange *> newStuff;
    textRanges_.elements(newStuff);

    // Righto. Now, that includes the old binary - by design - 
    // so skip it and see what we're talking about size-wise. Which should
    // be less than the highWaterMark, so we can double-check.
    assert(newStuff.size());

    void *newSection = NULL;
    unsigned newSectionSize = 0;

    // Wouldn't it be funny if they didn't add any new code at all?
    if (newStuff.size() > 1) {
        Address low = lowWaterMark_;
        Address high = highWaterMark_;
        newSectionSize = high - low;
        assert(newSectionSize <= highWaterMark_); // That would be... odd.

        newSection = malloc(newSectionSize);


        // Next, make a new section. We have the following parameters:
        // Offset vaddr: we get this from Symtab - "first free address with sufficient space"
        // void *data: newSection
        // unsigned int dataSize: newSectionSize
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
                           newSection,
                           newSectionSize,
                           ".dyninstInst",
                           Region::RT_TEXTDATA,
                           true);

        symObj->findRegion(newSec, ".dyninstInst");
        assert(newSec);
        
        for (unsigned i = 0; i < newStuff.size(); i++) {
            if (newStuff[i] == getAOut()) continue;
            
            Address offset = newStuff[i]->get_address() - low;
            assert((offset + newStuff[i]->get_size()) <= newSectionSize);
            void *local_addr = (void *)(offset + (Address)newSection);
            memcpy(local_addr, newStuff[i]->get_local_ptr(), newStuff[i]->get_size());

            char buffer[1025];


            if (newStuff[i]->is_multitramp()) {
                snprintf(buffer, 1024, "%s_inst_0x%lx_0x%lx",
                         newStuff[i]->is_multitramp()->func()->symTabName().c_str(),
                         newStuff[i]->is_multitramp()->instAddr(),
                         newStuff[i]->is_multitramp()->instAddr() +
                         newStuff[i]->is_multitramp()->instSize());
            }
            else if (newStuff[i]->is_basicBlockInstance()) {
                // Relocated function
                snprintf(buffer, 1024, "%s_reloc",
                         newStuff[i]->is_basicBlockInstance()->func()->symTabName().c_str());
            }
            else
                snprintf(buffer, 1024, "unknown");

            Symbol *newSym = new Symbol(buffer,
                                        "DyninstInst",
                                        Symbol::ST_FUNCTION,
                                        Symbol::SL_GLOBAL,
                                        newStuff[i]->get_address(),
                                        newSec,
                                        newStuff[i]->get_size(),
                                        (void *)newStuff[i]);
            
            symObj->addSymbol(newSym, false);
        }
        fprintf(stderr, "Applying %d dangling modifications...\n", danglingMods.size());
        for (unsigned i = 0; i < danglingMods.size(); i++) {
            // Overwrite whatever is there...
            Address offset = danglingMods[i]->get_address() - low;
            assert((offset + danglingMods[i]->get_size()) <= newSectionSize);
            void *local_addr = (void *)(offset + (Address)newSection);
            memcpy(local_addr, danglingMods[i]->get_local_ptr(), danglingMods[i]->get_size());

        }
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
            if (oldSegs[i].name == ".text") {
                symObj->updateCode(newSegs[i].data,
                                   newSegs[i].size);
            }
        }
    }


    // And now we generate the new binary
    if (!symObj->emit(newFileName.c_str())) {
        return false;
    }

    return true;
}

#endif // Old version

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
    memoryTracking_.insert(newTracker);

    highWaterMark_ += size;


    return true;
}


bool BinaryEdit::createMemoryBackingStore() {
    // We want to create a buffer for every section in the
    // binary so that we can store updates.

    Symtab *symObj = getAOut()->parse_img()->getObject();
    
    vector<Segment> segs;
    symObj->getSegments(segs);

    for (unsigned i = 0; i < segs.size(); i++) {
        memoryTracker *newTracker = NULL;
        if (segs[i].name == ".bss") {
            continue;
        }
        else {
            newTracker = new memoryTracker(segs[i].loadaddr,
                                           segs[i].size,
                                           segs[i].data);
        }
        newTracker->alloced = false;
        memoryTracking_.insert(newTracker);
    }

    
    return true;
}

bool BinaryEdit::initialize() {
    // Create the tramp guard

    // Initialization. For now we're skipping threads, since we can't
    // get the functions we need. However, we kinda need the recursion
    // guard. This is an integer (one per thread, for now - 1) that 
    // begins initialized to 1.

    
    trampGuardBase_ = inferiorMalloc(sizeof(int));
    // And initialize
    int trampInit = 1;
    // And make a range for it.
    writeDataSpace((void *)trampGuardBase_, sizeof(unsigned), &trampInit);


    fprintf(stderr, "Tramp guard created at address 0x%lx\n", trampGuardBase_);

    return true;
}

void BinaryEdit::addDependentRelocation(Address to, Symbol *referring) {
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

