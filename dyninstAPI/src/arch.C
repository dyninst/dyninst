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

// $Id: arch.C,v 1.8 2006/11/22 04:03:06 bernat Exp $
// Code generation

//////////////////////////
// Move to arch.C
//////////////////////////

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "common/h/Types.h"
#if defined (os_osf)
#include <malloc.h>
#endif
#include "arch.h"
#include "util.h"

#if defined(arch_x86) || defined(arch_x86_64)
#define CODE_GEN_OFFSET_SIZE 1
#elif defined(arch_ia64)
#define CODE_GEN_OFFSET_SIZE (sizeof(codeBuf_t))
#else
#define CODE_GEN_OFFSET_SIZE (instruction::size())
#endif

const int codeGenPadding = 256;

codeGen::codeGen() :
    buffer_(NULL),
    offset_(0),
    size_(0),
    allocated_(false),
    proc_(NULL),
    thr_(NULL),
    lwp_(NULL),
    rs_(NULL),
	t_(NULL),
    addr_((Address)-1),
    ip_(NULL)
{}

// size is in bytes
codeGen::codeGen(unsigned size) :
    buffer_(NULL),
    offset_(0),
    size_(size),
    allocated_(true),
    proc_(NULL),
    thr_(NULL),
    lwp_(NULL),
    rs_(NULL),
	t_(NULL),
    addr_((Address)-1),
    ip_(NULL)

{
    buffer_ = (codeBuf_t *)malloc(size+codeGenPadding);
    memset(buffer_, 0, size+codeGenPadding);
    if (!buffer_)
        fprintf(stderr, "Malloc failed for buffer of size %d\n", size_);
    assert(buffer_);
}


codeGen::~codeGen() {
    if (allocated_ && buffer_) free(buffer_);
}

// Deep copy
codeGen::codeGen(const codeGen &g) :
    offset_(g.offset_),
    size_(g.size_),
    allocated_(g.allocated_),
    proc_(g.proc_),
    thr_(g.thr_),
    lwp_(g.lwp_),
    rs_(g.rs_),
	t_(g.t_),
    addr_(g.addr_),
    ip_(g.ip_)
{
    if (size_ != 0) {
        assert(allocated_); 
        buffer_ = (codeBuf_t *) malloc(size_+codeGenPadding);
        memcpy(buffer_, g.buffer_, size_+codeGenPadding);
    }
    else
        buffer_ = NULL;
}

bool codeGen::operator==(void *p) const {
    return (p == (void *)buffer_);
}

bool codeGen::operator!=(void *p) const {
    return (p != (void *)buffer_);
}

codeGen &codeGen::operator=(const codeGen &g) {
    // Same as copy constructor, really
    invalidate();
    offset_ = g.offset_;
    size_ = g.size_;
    allocated_ = g.allocated_;
    thr_ = g.thr_;
    lwp_ = g.lwp_;

    if (size_ != 0) {
        assert(allocated_); 
        buffer_ = (codeBuf_t *) malloc(size_+codeGenPadding);
        memcpy(buffer_, g.buffer_, size_+codeGenPadding);
    }
    else
        buffer_ = NULL;
    return *this;
}

void codeGen::allocate(unsigned size) 
{
    // No implicit reallocation
    assert(buffer_ == NULL);
    size_ = size;
    offset_ = 0;
    buffer_ = (codeBuf_t *)malloc(size+codeGenPadding);
    allocated_ = true;
    if (!buffer_) {
      fprintf(stderr, "%s[%d]:  malloc (%d) failed: %s\n", FILE__, __LINE__, size, strerror(errno));
#if defined (os_osf)
    //struct mallinfo my_mallinfo  = mallinfo();
    //extern struct mallinfo = mallinfo();
    fprintf(stderr, "malloc info:\n");
    fprintf(stderr, "\t arena = %d\n", mallinfo().arena);
    fprintf(stderr, "\t ordblocks = %d\n", mallinfo().ordblks);
    fprintf(stderr, "\t free ordblocks = %d\n", mallinfo().fordblks);
    fprintf(stderr, "\t smblocks = %d\n", mallinfo().smblks);
    fprintf(stderr, "\t free smblocks = %d\n", mallinfo().fsmblks);
#endif
    }
    assert(buffer_);
}

// Very similar to destructor
void codeGen::invalidate() {
    if (allocated_ && buffer_)
        free(buffer_);
    buffer_ = NULL;
    size_ = 0;
    offset_ = 0;
    allocated_ = false;
}

void codeGen::finalize() {
    assert(buffer_);
    assert(size_);
    if (size_ == offset_) return;
    if (offset_ == 0) {
        invalidate();
        return;
    }
    codeBuf_t *newbuf = (codeBuf_t *)malloc(used());
    memcpy((void *)newbuf, (void *)buffer_, used());
    size_ = used(); // Don't use offset :D

    free(buffer_);
    buffer_ = newbuf;
}

void codeGen::copy(const void *b, const unsigned size) {
    assert(buffer_);
    memcpy(cur_ptr(), b, size);
    // "Upgrade" to next index side
    int disp = size;
    if (disp % CODE_GEN_OFFSET_SIZE) {
        disp += (CODE_GEN_OFFSET_SIZE - (disp % CODE_GEN_OFFSET_SIZE));
    }
    moveIndex(disp);
}

void codeGen::copy(codeGen &gen) {
    memcpy((void *)cur_ptr(), (void *)gen.start_ptr(), gen.used());
    offset_ += gen.offset_;
    assert(used() <= size_);
}

// codeBufIndex_t stores in platform-specific units.
unsigned codeGen::used() const {
    return offset_ * CODE_GEN_OFFSET_SIZE;
}

void *codeGen::start_ptr() const {
    return (void *)buffer_;
}

void *codeGen::cur_ptr() const {
    assert(buffer_);
    if (sizeof(codeBuf_t) != CODE_GEN_OFFSET_SIZE)
        fprintf(stderr, "ERROR: sizeof codeBuf %d, OFFSET %d\n",
                sizeof(codeBuf_t), CODE_GEN_OFFSET_SIZE);
    assert(sizeof(codeBuf_t) == CODE_GEN_OFFSET_SIZE);
    codeBuf_t *ret = buffer_;
    ret += offset_;
    return (void *)ret;
}

void *codeGen::get_ptr(unsigned offset) const {
    assert(buffer_);
    assert(offset < size_);
    assert(sizeof(codeBuf_t) == CODE_GEN_OFFSET_SIZE);
    assert((offset % CODE_GEN_OFFSET_SIZE) == 0);
    unsigned index = offset / CODE_GEN_OFFSET_SIZE;
    codeBuf_t *ret = buffer_;
    ret += index;
    return (void *)ret;
}

void codeGen::update(codeBuf_t *ptr) {
    assert(buffer_);
    assert(sizeof(unsigned char) == 1);
    unsigned diff = ((unsigned char *)ptr) - ((unsigned char *)buffer_);
    
    // Align...
    if (diff % CODE_GEN_OFFSET_SIZE) {
        diff += CODE_GEN_OFFSET_SIZE - (diff % CODE_GEN_OFFSET_SIZE);
        assert ((diff % CODE_GEN_OFFSET_SIZE) == 0);
    }
    // and integer division rules
    offset_ = diff / CODE_GEN_OFFSET_SIZE;

    // Keep the pad
    if (used() > size_) {
        fprintf(stderr, "WARNING: overflow of codeGen structure, trying to enlarge\n");
        if ((used() - size_) > codeGenPadding) {
            assert(0 && "Overflow in codeGen");
        }
        // Add an extra codeGenPadding to the end
        size_ += codeGenPadding;
        buffer_ = (codeBuf_t *)realloc(buffer_, size_ + codeGenPadding);
        assert(buffer_);
    }

    assert(used() <= size_);
}

void codeGen::setIndex(codeBufIndex_t index) {
    offset_ = index;
    
    // Keep the pad
    if (used() > size_) {
        fprintf(stderr, "WARNING: overflow of codeGen structure (%d requested, %d actual), trying to enlarge\n", used(), size_);

        if ((used() - size_) > codeGenPadding) {
            assert(0 && "Overflow in codeGen");
        }
        // Add an extra codeGenPadding to the end
        size_ += codeGenPadding;
        buffer_ = (codeBuf_t *)realloc(buffer_, size_ + codeGenPadding);
        assert(buffer_);
    }
    assert(used() <= size_);
}

codeBufIndex_t codeGen::getIndex() const {
    return offset_;
}

void codeGen::moveIndex(int disp) {
    int cur = getIndex() * CODE_GEN_OFFSET_SIZE;
    cur += disp;
    if (cur % CODE_GEN_OFFSET_SIZE) {
        fprintf(stderr, "Error in codeGen: current index %d/%d, moving by %d, mod %d\n",
                getIndex(), cur, disp, cur % CODE_GEN_OFFSET_SIZE);
    }
    assert((cur % CODE_GEN_OFFSET_SIZE) == 0);
    setIndex(cur / CODE_GEN_OFFSET_SIZE);
}

int codeGen::getDisplacement(codeBufIndex_t from, codeBufIndex_t to) {
    return ((to - from) * CODE_GEN_OFFSET_SIZE);
}

Address codeGen::currAddr() const {
    assert(addr_ != (Address) -1);
    return currAddr(addr_);
}

Address codeGen::currAddr(Address base) const { 
    return (offset_ * CODE_GEN_OFFSET_SIZE) + base;
}

void codeGen::fill(unsigned fillSize, int fillType) {
    switch(fillType) {
    case cgNOP:
        instruction::generateNOOP(*this, fillSize);
        break;
    case cgTrap: {
        unsigned curUsed = used();
        while ((used() - curUsed > (unsigned) fillSize))
            instruction::generateTrap(*this);
        assert((used() - curUsed) == (unsigned) fillSize);
        break;
    }
    case cgIllegal: {
        unsigned curUsed = used();
        while ((used() - curUsed > (unsigned) fillSize))
            instruction::generateIllegal(*this);
        assert((used() - curUsed) == (unsigned) fillSize);
        break;
    }
    default:
        assert(0 && "unimplemented");
    }
}

void codeGen::fillRemaining(int fillType) {
    if (fillType == cgNOP) {
        instruction::generateNOOP(*this,
                                  size_ - used());
    }
    else {
        assert(0 && "unimplemented");
    }
}

void codeGen::applyTemplate(codeGen &c) {
    // Copy off necessary bits...

    proc_ = c.proc_;
    thr_ = c.thr_;
    lwp_ = c.lwp_;
    rs_ = c.rs_;
    addr_ = c.addr_;
    ip_ = c.ip_;
}

codeGen codeGen::baseTemplate;

