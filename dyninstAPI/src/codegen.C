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

// Code generation

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "addressSpace.h"
#include "dynThread.h"
#include "dynProcess.h"
#include "compiler_annotations.h"
#include "codegen.h"
#include "util.h"
#include "function.h"
#include "instPoint.h"
#include "registerSpace.h"
#include "pcrel.h"
#include "bitArray.h"

#include "instructionAPI/h/InstructionDecoder.h"

#if defined(arch_x86) || defined(arch_x86_64)
#define CODE_GEN_OFFSET_SIZE 1U
#else
#define CODE_GEN_OFFSET_SIZE (instruction::size())
#endif

const unsigned int codeGenPadding = (128);
const unsigned int codeGenMinAlloc = (4 * 1024);

codeGen::codeGen() :
    buffer_(NULL),
    offset_(0),
    size_(0),
    max_(0),
    pc_rel_use_count(0),
    emitter_(NULL),
    allocated_(false),
    aSpace_(NULL),
    thr_(NULL),
    rs_(NULL),
    t_(NULL),
    addr_((Dyninst::Address)-1),
    ip_(NULL),
    f_(NULL),
    bt_(NULL),
    isPadded_(true),
    trackRegDefs_(false),
    inInstrumentation_(false), // save default
    insertNaked_(false),
    modifiedStackFrame_(false)
{}

// size is in bytes
codeGen::codeGen(unsigned size) :
    buffer_(NULL),
    offset_(0),
    size_(size),
    max_(size+codeGenPadding),
    pc_rel_use_count(0),
    emitter_(NULL),
    allocated_(true),
    aSpace_(NULL),
    thr_(NULL),
    rs_(NULL),
	t_(NULL),
    addr_((Dyninst::Address)-1),
    ip_(NULL),
    f_(NULL),
    bt_(NULL),
    isPadded_(true),
    trackRegDefs_(false),
    inInstrumentation_(false),
    insertNaked_(false),
    modifiedStackFrame_(false)
{
    buffer_ = (codeBuf_t *)malloc(size+codeGenPadding);
    if (!buffer_) {
       fprintf(stderr, "%s[%d]: malloc failed: size is %u + codeGenPadding = %u\n", FILE__, __LINE__, size, codeGenPadding);
	}
    assert(buffer_);
    memset(buffer_, 0, size+codeGenPadding);
}

// size is in bytes
codeGen::codeGen(codeBuf_t *buffer, int size) :
    buffer_(buffer),
    offset_(0),
    size_(size-codeGenPadding),
    max_(size+codeGenPadding),
    pc_rel_use_count(0),
    emitter_(NULL),
    allocated_(false),
    aSpace_(NULL),
    thr_(NULL),
    rs_(NULL),
    t_(NULL),
    addr_((Dyninst::Address)-1),
    ip_(NULL),
    f_(NULL),
    bt_(NULL),
    isPadded_(true),
    trackRegDefs_(false),
    inInstrumentation_(false),
    insertNaked_(false),
    modifiedStackFrame_(false)
{
    assert(buffer_);
    memset(buffer_, 0, size+codeGenPadding);
}


codeGen::~codeGen() {
    if (allocated_ && buffer_) {
        free(buffer_);
    }
}

// Deep copy
codeGen::codeGen(const codeGen &g) :
    buffer_(NULL),
    offset_(g.offset_),
    size_(g.size_),
    max_(g.max_),
    pc_rel_use_count(g.pc_rel_use_count),
    emitter_(NULL),
    allocated_(g.allocated_),
    aSpace_(g.aSpace_),
    thr_(g.thr_),
    rs_(g.rs_),
	t_(g.t_),
    addr_(g.addr_),
    ip_(g.ip_),
    f_(g.f_),
    bt_(g.bt_),
    isPadded_(g.isPadded_),
    trackRegDefs_(g.trackRegDefs_),
    inInstrumentation_(g.inInstrumentation_),
    insertNaked_(g.insertNaked_),
    modifiedStackFrame_(g.modifiedStackFrame_)
{
    if (size_ != 0) {
        assert(allocated_); 
        int bufferSize = size_ + (isPadded_ ? codeGenPadding : 0);
        buffer_ = (codeBuf_t *) malloc(bufferSize);
        memcpy(buffer_, g.buffer_, bufferSize);
    }
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
    max_ = g.max_;
    pc_rel_use_count = g.pc_rel_use_count;
    allocated_ = g.allocated_;
    thr_ = g.thr_;
    isPadded_ = g.isPadded_;
    int bufferSize = size_ + (isPadded_ ? codeGenPadding : 0);
    inInstrumentation_ = g.inInstrumentation_;
    insertNaked_ = g.insertNaked_;
    modifiedStackFrame_ = g.modifiedStackFrame_;

    if (size_ != 0) {
       assert(allocated_); 

       buffer_ = (codeBuf_t *) malloc(bufferSize);
       //allocate(g.size_);
	
       memcpy(buffer_, g.buffer_, bufferSize);
    }
    else
        buffer_ = NULL;
    return *this;
}

void codeGen::allocate(unsigned size) 
{
   if (buffer_ && size > size_) {
      free(buffer_);
      buffer_ = NULL;
   }

   size_ = size;
   max_ = size_ + codeGenPadding;

   if (buffer_ == NULL)
   {
      buffer_ = (codeBuf_t *)malloc(max_);
      isPadded_ = true;
   }
   
   offset_ = 0;
   allocated_ = true;
   if (!buffer_) {
      fprintf(stderr, "%s[%d]:  malloc (%u) failed: %s\n", FILE__, __LINE__, size, strerror(errno));
   }
   assert(buffer_);
}

// Very similar to destructor
void codeGen::invalidate() {
    if (allocated_ && buffer_) {
        free(buffer_);
    }
    buffer_ = NULL;
    size_ = 0;
    max_ = 0;
    offset_ = 0;
    allocated_ = false;
    isPadded_ = false;
}

bool codeGen::verify() {
    return true;
}

void codeGen::finalize() {
    assert(buffer_);
    assert(size_);
    cerr << "FINALIZE!" << endl;
    applyPatches();
    if (size_ == offset_) return;
    if (offset_ == 0) {
        fprintf(stderr, "Warning: offset is 0 in codeGen::finalize!\n");
        invalidate();
        return;
    }
    buffer_ = (codeBuf_t *)::realloc(buffer_, used());
    max_ = used();
    size_ = used();
    isPadded_ = false;
}

void codeGen::copy(const void *b, const unsigned size, const codeBufIndex_t index) {
  if (size == 0) return;

  codeBufIndex_t current = getIndex();
  setIndex(index);
  copy(b, size);
  setIndex(current);

}

void codeGen::copy(const void *b, const unsigned size) {
  if (size == 0) return;

  assert(buffer_);
  
  realloc(used() + size);

  memcpy(cur_ptr(), b, size);

  moveIndex(size);
}

void codeGen::copy(const std::vector<unsigned char> &buf) {
  if (buf.empty()) return;

   assert(buffer_);
   realloc(used() + buf.size());
   
   unsigned char * ptr = (unsigned char *)cur_ptr();
	assert(ptr);

   std::copy(buf.begin(), buf.end(), ptr);

   moveIndex(buf.size());
}

void codeGen::copy(codeGen &gen) {
  if ((used() + gen.used()) >= size_) {
    realloc(used() + gen.used()); 
  }

  memcpy((void *)cur_ptr(), (void *)gen.start_ptr(), gen.used());
  offset_ += gen.offset_;
  assert(used() <= size_);
}

void codeGen::insert(const void *b, const unsigned size, const codeBufIndex_t index) {
    if (size == 0) return;
    assert(buffer_);

    realloc(used() + size);

    char * temp = (char*)get_ptr(index);
    memmove(temp + size, temp, used()-index);
    memcpy(temp, b, size);

    moveIndex(size);
}

void codeGen::copyAligned(const void *b, const unsigned size) {
  if (size == 0) return;

  assert(buffer_);
  
  realloc(used() + size);

  memcpy(cur_ptr(), b, size);

  unsigned alignedSize = size;
  alignedSize += (CODE_GEN_OFFSET_SIZE - (alignedSize % CODE_GEN_OFFSET_SIZE));

  moveIndex(alignedSize);
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
        fprintf(stderr, "ERROR: sizeof codeBuf %zu, OFFSET %u\n",
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
    if (used() >= size_) {
        if ((used() - size_) >= codeGenPadding) {
	  cerr << "Used too much extra: " << used() - size_ << " bytes" << endl;
	  assert(0 && "Overflow in codeGen");
        }
	realloc(2*used());
    }

    assert(used() <= size_);
}

void codeGen::setIndex(codeBufIndex_t index) {
    offset_ = index;
    
    // Keep the pad
    if (used() >= size_) {
      //fprintf(stderr, "WARNING: overflow of codeGen structure (%d requested, %d actual), trying to enlarge\n", used(), size_);

        if ((used() - size_) > codeGenPadding) {
            assert(0 && "Overflow in codeGen");
        }
	realloc(used());
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
        fprintf(stderr, "Error in codeGen: current index %u/%d, moving by %d, mod %u\n",
                getIndex(), cur, disp, cur % CODE_GEN_OFFSET_SIZE);
    }
    assert((cur % CODE_GEN_OFFSET_SIZE) == 0);
    setIndex(cur / CODE_GEN_OFFSET_SIZE);
}

long codeGen::getDisplacement(codeBufIndex_t from, codeBufIndex_t to) {
   long from_l = (long) from;
   long to_l = (long) to;
   return ((to_l - from_l) * CODE_GEN_OFFSET_SIZE);
}

Dyninst::Address codeGen::currAddr() const {
  if(addr_ == (Dyninst::Address) -1) return (Dyninst::Address) -1;
  assert(addr_ != (Dyninst::Address) -1);
  return currAddr(addr_);
}

Dyninst::Address codeGen::currAddr(Dyninst::Address base) const {
    return (offset_ * CODE_GEN_OFFSET_SIZE) + base;
}

void codeGen::fill(unsigned fillSize, int fillType) {
    switch(fillType) {
    case cgNOP:
        insnCodeGen::generateNOOP(*this, fillSize);
        break;
    case cgTrap: {
        unsigned curUsed = used();
        while ((used() - curUsed) < (unsigned) fillSize)
            insnCodeGen::generateTrap(*this);
        assert((used() - curUsed) == (unsigned) fillSize);
        break;
    }
    case cgIllegal: {
        unsigned curUsed = used();
        while ((used() - curUsed) < (unsigned) fillSize)
            insnCodeGen::generateIllegal(*this);
	if ((used() - curUsed) != fillSize) {
	  cerr << "ABORTING: " << used() << " - " << curUsed << " != " << fillSize << endl;
	}
        assert((used() - curUsed) == (unsigned) fillSize);
        break;
    }
    default:
        assert(0 && "unimplemented");
    }
}

void codeGen::fillRemaining(int fillType) {
    if (fillType == cgNOP) {
        insnCodeGen::generateNOOP(*this,
                                  size_ - used());
    }
    else {
        assert(0 && "unimplemented");
    }
}

void codeGen::applyTemplate(const codeGen &c) {
    // Copy off necessary bits...

  emitter_ = c.emitter_;
  aSpace_ = c.aSpace_;
  thr_ = c.thr_;
  rs_ = c.rs_;
  t_ = c.t_;
  ip_ = c.ip_;
  f_ = c.f_;
  bt_ = c.bt_;
  inInstrumentation_ = c.inInstrumentation_;
  insertNaked_ = c.insertNaked_;
  modifiedStackFrame_ = c.modifiedStackFrame_;
}

void codeGen::setAddrSpace(AddressSpace *a)
{ 
   aSpace_ = a; 
   setCodeEmitter(a->getEmitter());
}

void codeGen::realloc(unsigned newSize) {  
   if (newSize <= size_) return;

   unsigned increment = newSize - size_;
   if (increment < codeGenMinAlloc) increment = codeGenMinAlloc;

   size_ += increment;
   max_ += increment;
   buffer_ = (codeBuf_t *)::realloc(buffer_, max_);
   
   assert(buffer_);
}

void codeGen::addPCRelRegion(pcRelRegion *reg) {
#if !defined(cap_noaddr_gen)
   assert(0);
#endif
   reg->gen = this;
   reg->cur_offset = used();

   if (startAddr() != (Dyninst::Address) -1 && reg->canPreApply()) {
     //If we already have addressess for everything (usually when relocating a function)
     // then don't bother creating the region, just generate the code.
     reg->apply(startAddr() + reg->cur_offset);
     delete reg;
   }
   else {
     reg->cur_size = reg->maxSize();
     fill(reg->cur_size, cgNOP);
     pcrels_.push_back(reg);
   }
}

void codeGen::applyPCRels(Dyninst::Address base)
{
   vector<pcRelRegion *>::iterator i;

   codeBufIndex_t orig_position = used();
   for (i = pcrels_.begin(); i != pcrels_.end(); i++) {
      pcRelRegion *cur = *i;
      bool is_last_entry = ((cur->cur_offset + cur->cur_size) >= orig_position);

      //Apply the patch
      setIndex(cur->cur_offset / CODE_GEN_OFFSET_SIZE);
      unsigned patch_size = cur->apply(base + cur->cur_offset);
      assert(patch_size <= cur->cur_size);
      unsigned size_change = cur->cur_size - patch_size;

      if (size_change) {
         if (is_last_entry) {
            //If we resized the last object in the codeGen, then change the
            // codeGen's end address
            orig_position = cur->cur_offset + patch_size;
         }
         //Fill in any size changes with nops
         fill(size_change, cgNOP);
      }
      delete cur;
   }
   setIndex(orig_position / CODE_GEN_OFFSET_SIZE);
   pcrels_.clear();
}

bool codeGen::hasPCRels() const {
   return (pcrels_.size() != 0);
}


pcRelRegion::pcRelRegion(const instruction &i) :
   gen(NULL),
   orig_instruc(i),
   cur_offset(0),
   cur_size(0)
{
}

pcRelRegion::~pcRelRegion()
{
}

std::vector<relocPatch>& codeGen::allPatches() {
   return patches_;
}

void codeGen::addPatch(codeBufIndex_t index, patchTarget *source, 
                       unsigned size,
                       relocPatch::patch_type_t ptype,
                       Dyninst::Offset off)
{
   relocPatch p(index, source, ptype, this, off, size);
   patches_.push_back(p);
}

void codeGen::addPatch(const relocPatch &p)
{
   patches_.push_back(p);
}

void codeGen::applyPatches()
{
   std::vector<relocPatch>::iterator i;
   for (i = patches_.begin(); i != patches_.end(); i++)
      (*i).applyPatch();
}


relocPatch::relocPatch(codeBufIndex_t index, patchTarget *s, patch_type_t ptype,
                       codeGen *gen, Dyninst::Offset off, unsigned size) :
  dest_(index),
  source_(s),
  size_(size),
  ptype_(ptype),
  gen_(gen),
  offset_(off),
  applied_(false)
{

}

void relocPatch::applyPatch()
{
   if (applied_)
      return;

   Dyninst::Address addr = source_->get_address();


   switch (ptype_) {
      case patch_type_t::pcrel:
	addr = addr - (gen_->startAddr() + offset_);
	DYNINST_FALLTHROUGH;
      case patch_type_t::abs:
	gen_->copy(&addr, size_, dest_);
	break;
      default:
         assert(0);
   }
   applied_ = true;
}

bool relocPatch::isApplied()
{
   return applied_;
}

bool pcRelRegion::canPreApply()
{
  return false;
}

std::string patchTarget::get_name() const {
   return std::string("UNNAMED");
}

toAddressPatch::~toAddressPatch() {
}

Dyninst::Address toAddressPatch::get_address() const
{ 
  return addr; 
}

unsigned toAddressPatch::get_size() const { 
  return 0; 
}

void toAddressPatch::set_address(Dyninst::Address a) {
   addr = a;
}

codeGen codeGen::baseTemplate;

PCThread *codeGen::thread() {
    return thr_;
}

unsigned codeGen::width() const {
  return addrSpace()->getAddressWidth();
}

AddressSpace *codeGen::addrSpace() const {
    if (aSpace_) { return aSpace_; }
    if (f_) { return f_->proc(); }
    if (ip_) { return ip_->proc(); }
    if (thr_) { return thr_->getProc(); }
    return NULL;
}

instPoint *codeGen::point() const {
    return ip_;
}

func_instance *codeGen::func() const {
    if (f_) return f_;
    if (ip_) return ip_->func();
    return NULL;
}

registerSpace *codeGen::rs()  const{
    return rs_;
}

regTracker_t *codeGen::tracker() const {
    return t_; 
}

Emitter *codeGen::codeEmitter() const {
    return emitter_;
}

void codeGen::beginTrackRegDefs()
{
   trackRegDefs_ = true;
#if defined(arch_x86) || defined(arch_x86_64)
    regsDefined_ = bitArray(REGNUM_IGNORED+1);
#elif defined(arch_power)
    regsDefined_ = bitArray(registerSpace::lastReg);
#elif defined(arch_aarch64)
    regsDefined_ = bitArray(registerSpace::fpsr);
#else
    regsDefined_ = bitArray();
#endif

}

void codeGen::endTrackRegDefs()
{
   trackRegDefs_ = false;
}
 
const bitArray &codeGen::getRegsDefined()
{
   return regsDefined_;
}

void codeGen::markRegDefined(Dyninst::Register r) {
   if (!trackRegDefs_)
      return;
   regsDefined_[r] = true;
}

bool codeGen::isRegDefined(Dyninst::Register r) {
   assert(trackRegDefs_);
   return regsDefined_[r];
}

Dyninst::Architecture codeGen::getArch() const {
  // Try whatever's defined
  if (func()) {
    return func()->ifunc()->isrc()->getArch();
  }
  if (addrSpace()) {
     return addrSpace()->getArch();
  }

  assert(0);
  return Arch_none;
}

void codeGen::registerDefensivePad(block_instance *callBlock, Dyninst::Address padStart, unsigned padSize) {
  // Dyninst::Register a match between a call instruction
  // and a padding area post-reloc-call for
  // control flow interception purposes.
  // This is kind of hacky, btw.
    //cerr << "Registering pad [" << hex << padStart << "," << padStart + padSize << "], for block @ " << callBlock->start() << dec << endl;
  defensivePads_[callBlock] = Extent(padStart, padSize);
}


#include "InstructionDecoder.h"
using namespace InstructionAPI;

std::string codeGen::format() const {
   if (!aSpace_) return "<codeGen>";

   stringstream ret;

   Dyninst::Address base = (addr_ != (Dyninst::Address)-1) ? addr_ : 0;
   InstructionDecoder deco
      (buffer_,used(),aSpace_->getArch());
   Instruction insn = deco.decode();
   ret << hex;
   while(insn.isValid()) {
       ret << "\t" << base << ": " << insn.format(base) << " / " << *((const unsigned *)insn.ptr()) << endl;
       base += insn.size();
       insn = deco.decode();
   }
   ret << dec;
   return ret.str();
}
   
