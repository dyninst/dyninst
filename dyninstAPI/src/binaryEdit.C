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

// $Id: binaryEdit.C,v 1.4 2007/09/19 21:54:39 giri Exp $

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
                               u_int amount,
                               const void *inSelf) {
#if defined(USE_ADDRESS_MAPS)
    // Let's keep this around - but I think instead we can
    // use our codeRanges to figure out where things go...
    // which means we delay writes.

    void *use = (void *) mapApparentToReal((Address)inOther, amount, false);
    memcpy((void *)inSelf, use, amount);
#endif
    return true;
}

bool BinaryEdit::writeTextSpace(void *inOther,
                            u_int amount,
                            const void *inSelf) {
#if defined(USE_ADDRESS_MAPS)
    // See above
    void *use = (void *) mapApparentToReal((Address)inOther, amount, true);
    memcpy(use, inSelf, amount);
#endif
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
            inferiorMallocStatic(HEAP_STAT_BUF_SIZE);
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
    apparentToReal_.clear();
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

    /*
      // I'm going through the mapped_object interface for now - 
      // I assume we'll pass it to DynSymtab, then add our base
      // address to it at the mapped_ level. 
      mapped_section *newSection = bin->createSection();
      assert(newSection);
      newBinaryEdit->highWaterMark_ = newSection->baseAddr();
      assert(newBinaryEdit->highWaterMark_);
    */
    
    addrMapping *aMap = new addrMapping(0, 0, 0);
    aMap->alloc = false;
    newBinaryEdit->apparentToReal_.insert(aMap);
    
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
    
    // Allocate a new buffer for the text of the program
    void *textSection = malloc(getAOut()->get_size_cr());

    // Copy in from the binary
    //assert(getAOut());
    memcpy(textSection, getAOut()->get_local_ptr(), getAOut()->get_size_cr());
    
    // And now apply changes
    // Note: I want an iterator. Stat. 
    pdvector<codeRange *> modified;
    modifiedRanges_.elements(modified);
    for (unsigned i = 0; i < modified.size(); i++) {
        assert(modified[i]->get_address_cr() >= getAOut()->get_address_cr());
        Address offset = modified[i]->get_address_cr() - getAOut()->get_address_cr();
        assert(offset < getAOut()->get_size_cr());
        void *local_addr = (void *)((Address) textSection + offset);
        memcpy(local_addr, modified[i]->get_local_ptr(), modified[i]->get_size_cr());
    }

    // Okay, that's our text section. 
    // Now for the new stuff.
    
    pdvector<codeRange *> newStuff;
    textRanges_.elements(newStuff);

    // Righto. Now, that includes the old binary - by design - 
    // so skip it and see what we're talking about size-wise. Which should
    // be less than the highWaterMark, so we can double-check.
    assert(newStuff.size());
    assert(newStuff[0] == getAOut());

    void *newSection = NULL;

    // Wouldn't it be funny if they didn't add any new code at all?
    if (newStuff.size() > 1) {
        Address low = newStuff[1]->get_address_cr();
        Address high = newStuff.back()->get_address_cr() + newStuff.back()->get_size_cr();
        Address size = high - low;
        assert(size <= highWaterMark_); // That would be... odd.
        
        newSection = malloc(size);
        for (unsigned i = 1; i < newStuff.size(); i++) {

            Address offset = newStuff[i]->get_address_cr() - newStuff[1]->get_address_cr();
            assert(offset < size);
            void *local_addr = (void *)(offset);
            memcpy(local_addr, newStuff[i]->get_local_ptr(), newStuff[i]->get_size_cr());
        }
    }

    // Okay, now...
    // Hand textSection and newSection to DynSymtab.

    return false;
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
#if defined(USE_ADDRESS_MAPS)
    h->setBuffer(buf);
#endif

    addHeap(h);

#if defined(USE_ADDRESS_MAPS)
    addrMapping *aMap = new addrMapping(highWaterMark_,
                                        (Address) buf,
                                        size);
    aMap->alloc = true;
    apparentToReal_.insert(aMap);
#endif

    highWaterMark_ += size;

    return true;
}

// We can leave this around - it's harmless
Address BinaryEdit::mapApparentToReal(const Address apparent,
                                      unsigned size,
                                      bool writing) {
    // Our job: map an address in the "apparent" address
    // space (that of the binary we're editing) to a
    // "real" pointer (that is, into a buffer that we keep). 
    // Note that buffers are:
    //  1) Not contiguous by any means
    //  2) Have no relationship to the "apparent" address

    codeRange *tmp_ = NULL;
    if (!apparentToReal_.find(apparent, tmp_))
        return 0;
    
    addrMapping *tmp = dynamic_cast<addrMapping *>(tmp_);
    assert(tmp);

    if (tmp->alloc) {
        // Okay, we have a little bundle of happiness. Now
        // get a useful void *
        unsigned offset = apparent - tmp->in;
        assert(offset <= tmp->size);
        // Ugh pointer math!
        Address ret = (tmp->out + offset);
        return ret;
    }

    // Otherwise, we're in the original binary space, and we're
    // writing to it. Now, there's two ways we could be in the
    // original binary space. We could just be pokin' around, in which
    // case we're done. Or we could be workin' on something that we
    // wrote already, like a jump, in which case we need to point out
    // to our little buffers.

    codeRange *over_ = NULL;
    
    if (overwrittenToReal_.find(apparent, over_)) {
        // We found it.
        addrMapping *over = dynamic_cast<addrMapping *>(over);
        assert(over);
        
        unsigned offset = apparent - over->in;
        assert(offset <= over->size);
        // Ugh pointer math!
        Address ret = (over->out + offset);
        return ret;
    }
    else {
        // If we're writing, create a new patch area; otherwise,
        // return the binary.
        if (writing) {
            void *buffer = (void *)malloc(size);
            
            addrMapping *aMap = new addrMapping(apparent,
                                                (Address) buffer,
                                                size);
            aMap->alloc = true;
            overwrittenToReal_.insert(aMap);
            return (Address) buffer;
        }
        else {
            // Head back on up to the original lookup
            unsigned offset = apparent - tmp->in;
            assert(offset <= tmp->size);
            // Ugh pointer math!
            Address ret = (tmp->out + offset);
            return ret;
        }
    }
    return 0;
}    

