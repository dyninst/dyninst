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

// $Id: binaryEdit.C,v 1.6 2007/10/04 22:04:49 giri Exp $

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

bool BinaryEdit::readTextSpace(const void *,
                               u_int,
                               const void *) {
    return true;
}

bool BinaryEdit::writeTextSpace(void *,
                            u_int,
                            const void *) {
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
    // We should never see this getting called...
    fprintf(stderr, "WARNING: deleting generatedCodeObject %p - unexpected operation!\n", del);

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
}

BinaryEdit *BinaryEdit::openFile(const pdstring &file) {
    if (!OS::executableExists(file)) {
        startup_printf("%s[%d]:  failed to read file %s\n", FILE__, __LINE__, file.c_str());
        pdstring msg = pdstring("Can't read executable file ") + file + (": ") + strerror(errno);
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
    
    newBinaryEdit->highWaterMark_ = newBinaryEdit->getAOut()->parse_img()->getObject()->getFreeOffset(10*1024*1024);
    newBinaryEdit->lowWaterMark_ = newBinaryEdit->highWaterMark_;
    
    return newBinaryEdit;
}

bool BinaryEdit::getStatFileDescriptor(const pdstring &name, fileDescriptor &desc) {
    // Wow, I think this was easy
    desc = fileDescriptor(name.c_str(),
                          0, // code base address
                          0, // data base address
                          true); // a.out
    return true;
}

bool BinaryEdit::writeFile(const pdstring &newFileName) {
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
    
    // And now apply changes
    // Note: I want an iterator. Stat. 
    pdvector<codeRange *> modified;
    modifiedRanges_.elements(modified);
    for (unsigned i = 0; i < modified.size(); i++) {
        assert(modified[i]->get_address_cr() >= getAOut()->get_address_cr());

        // Find the segment this modification occurs in
        for (unsigned j = 0; j < oldSegs.size(); j++) {
            if ((modified[i]->get_address_cr() >= oldSegs[j].loadaddr) &&
                (modified[i]->get_address_cr() < (oldSegs[j].loadaddr + oldSegs[j].size))) {
                if (oldSegs[j].data == newSegs[j].data) {
                    newSegs[j].data = malloc(oldSegs[j].size);
                    memcpy(newSegs[j].data, oldSegs[j].data, oldSegs[j].size);
                }
                Address offset = modified[i]->get_address_cr() - oldSegs[j].loadaddr;

                assert(offset < oldSegs[j].size);
                void *local_addr = (void *)((Address) newSegs[j].data + offset);
                memcpy(local_addr, modified[i]->get_local_ptr(), modified[i]->get_size_cr());
                break;
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
        for (unsigned i = 0; i < newStuff.size(); i++) {
            if (newStuff[i] == getAOut()) continue;

            Address offset = newStuff[i]->get_address_cr() - low;
            assert((offset + newStuff[i]->get_size_cr()) <= newSectionSize);
            void *local_addr = (void *)(offset + (Address)newSection);
            memcpy(local_addr, newStuff[i]->get_local_ptr(), newStuff[i]->get_size_cr());

            char buffer[1025];
            
            if (newStuff[i]->is_multitramp()) {
                snprintf(buffer, 1024, "%s-inst-0x%lx-0x%lx",
                         newStuff[i]->is_multitramp()->func()->symTabName().c_str(),
                         newStuff[i]->is_multitramp()->instAddr(),
                         newStuff[i]->is_multitramp()->instAddr() +
                         newStuff[i]->is_multitramp()->instSize());
            }
            else if (newStuff[i]->is_basicBlockInstance()) {
                // Relocated function
                snprintf(buffer, 1024, "%s-reloc",
                         newStuff[i]->is_basicBlockInstance()->func()->symTabName().c_str());
            }
            else
                snprintf(buffer, 1024, "unknown");
            

            Symbol *newSym = new Symbol(buffer,
                                        "DyninstInst",
                                        Symbol::ST_FUNCTION,
                                        Symbol::SL_LOCAL,
                                        newStuff[i]->get_address_cr(),
                                        NULL, // Should be our new section
                                        newStuff[i]->get_size_cr(),
                                        (void *)newStuff[i]);
            
            symObj->addSymbol(newSym);
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

    // Next, make a new section. We have the following parameters:
    // Offset vaddr: we get this from Symtab - "first free address with sufficient space"
    // void *data: newSection
    // unsigned int dataSize: newSectionSize
    // std::string name: without reflection, ".dyninstInst"
    // unsigned long flags: these are a SymtabAPI abstraction. We're going with text|data because
    //    we might have both.
    // bool loadable: heck yeah...
    
    if (newSectionSize) {
        symObj->addSection(lowWaterMark_,
                           newSection,
                           newSectionSize,
                           ".dyninstInst",
                           Section::textSection | Section::dataSection,
                           true);
    }

    // And now we generate the new binary
    if (!symObj->emit(newFileName.c_str(), true)) {
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
    
    // Build tracking objects for it
    heapItem *h = new heapItem(highWaterMark_, 
                               size,
                               anyHeap,
                               true,
                               HEAPfree);
    addHeap(h);


    highWaterMark_ += size;

    return true;
}

