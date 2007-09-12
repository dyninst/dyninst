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

// $Id: binaryEdit.C,v 1.1 2007/09/12 20:57:14 bernat Exp $

#include "binaryEdit.h"
#include "common/h/headers.h"
#include "mapped_object.h"
#include "multiTramp.h"
#include "debug.h"

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
    void *use = (void *) mapApparentToReal((Address)inOther, amount);
    memcpy((void *)inSelf, use, amount);
    return true;
}

bool BinaryEdit::writeTextSpace(void *inOther,
                            u_int amount,
                            const void *inSelf) {
    void *use = (void *) mapApparentToReal((Address)inOther, amount);
    memcpy(use, inSelf, amount);
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

    assert(0 && "Need to find address mapping of mapped object");

    assert(newBinaryEdit->highWaterMark_);

    addrMapping *aMap = new addrMapping(0, 0, 0);
    aMap->alloc = false;
    newBinaryEdit->apparentToReal_.insert(aMap);
    
    return newBinaryEdit;
}

bool BinaryEdit::getStatFileDescriptor(const pdstring &name, fileDescriptor &desc) {
    // Wow, I think this was easy
    desc = fileDescriptor(name,
                          0, // code base address
                          0, // data base address
                          true); // a.out
    return true;
}

bool BinaryEdit::writeFile(const pdstring &newFileName) {
    assert(0 && "Make Giri do work");
    return false;
}

bool BinaryEdit::inferiorMallocStatic(unsigned size) {
    // Should be set by now
    assert(highWaterMark_ != 0);

    void *buf = malloc(size);
    if (!buf) return false;
    
    // Build tracking objects for it
    heapItem *h = new heapItem(highWaterMark_, 
                               size,
                               anyHeap,
                               true,
                               HEAPfree);
    h->setBuffer(buf);
    addHeap(h);
    
    addrMapping *aMap = new addrMapping(highWaterMark_,
                                        (Address) buf,
                                        size);
    aMap->alloc = true;
    apparentToReal_.insert(aMap);

    highWaterMark_ += size;

    return true;
}

Address BinaryEdit::mapApparentToReal(const Address apparent,
                                      unsigned size) {
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
    
    // Otherwise, we're in the original binary space. Now, there's
    // two ways we could be in the original binary space. We could
    // just be pokin' around, in which case we're done. Or we
    // could be workin' on something that we wrote already, 
    // like a jump, in which case we need to point out to our
    // little buffers.

    codeRange *tmp2_ = NULL;
    if (!overwrittenToReal_.find(apparent, tmp2_)) {
        // Okay, original binary. 
        // Which we _don't_ want to do. So we patch-allocate
        // a chunk for it and return a pointer to 
        // that. 
        void *buffer = (void *)malloc(size);
        
        addrMapping *aMap = new addrMapping(apparent,
                                            (Address) buffer,
                                            size);
        aMap->alloc = true;
        overwrittenToReal_.insert(aMap);
        return (Address) buffer;
    }

    addrMapping *tmp2 = dynamic_cast<addrMapping *>(tmp2_);
    assert(tmp2);

    unsigned offset = apparent - tmp2->in;
    assert(offset <= tmp2->size);
    // Ugh pointer math!
    Address ret = (tmp2->out + offset);
    return ret;
    
}    

