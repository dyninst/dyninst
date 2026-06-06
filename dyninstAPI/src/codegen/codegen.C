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
#include "arch-regs-x86.h"
#include "dynproc/dynThread.h"
#include "dynproc/dynProcess.h"
#include "compiler_annotations.h"
#include "codegen/codegen.h"
#include "patching/function.h"
#include "patching/instPoint.h"
#include "registerSpace/registerSpace.h"
#include "bitArray.h"
#include "InstructionDecoder.h"

#if defined(DYNINST_CODEGEN_ARCH_I386) || defined(DYNINST_CODEGEN_ARCH_X86_64)
#define CODE_GEN_OFFSET_SIZE 1U
#else
#define CODE_GEN_OFFSET_SIZE (instruction::size())
#endif

const unsigned int codeGenPadding = (128);
const unsigned int codeGenMinAlloc = (4 * 1024);

codeGen::~codeGen() {
    if (allocated_ && buffer_) {
        free(buffer_);
    }
}

codeGen::codeGen(codeGen &&rhs) noexcept {
  offset_ = rhs.offset_;
  size_ = rhs.size_;
  max_ = rhs.max_;
  pc_rel_use_count = rhs.pc_rel_use_count;
  emitter_ = rhs.emitter_;
  allocated_ = rhs.allocated_;
  aSpace_ = rhs.aSpace_;
  thr_ = rhs.thr_;
  rs_ = rhs.rs_;
  t_ = rhs.t_;
  addr_ = rhs.addr_;
  ip_ = rhs.ip_;
  f_ = rhs.f_;
  bt_ = rhs.bt_;
  regsDefined_ = rhs.regsDefined_;
  trackRegDefs_ = rhs.trackRegDefs_;
  inInstrumentation_ = rhs.inInstrumentation_;
  insertNaked_ = rhs.insertNaked_;
  modifiedStackFrame_ = rhs.modifiedStackFrame_;
  patches_ = rhs.patches_;

  buffer_ = rhs.buffer_;
  rhs.buffer_ = nullptr;
  rhs.allocated_ = false;
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
      memset(buffer_, 0, max_);
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

void codeGen::insert(const void *b, const unsigned size, const codeBufIndex_t index) {
    if (size == 0) return;
    assert(buffer_);

    realloc(used() + size);

    char * temp = (char*)get_ptr(index);
    memmove(temp + size, temp, used()-index);
    memcpy(temp, b, size);

    moveIndex(size);
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
  return (offset_ * CODE_GEN_OFFSET_SIZE) + addr_;
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
#if defined(DYNINST_CODEGEN_ARCH_I386) || defined(DYNINST_CODEGEN_ARCH_X86_64)
    regsDefined_ = bitArray(REGNUM_IGNORED+1);
#elif defined(DYNINST_CODEGEN_ARCH_POWER)
    regsDefined_ = bitArray(registerSpace::lastReg);
#elif defined(DYNINST_CODEGEN_ARCH_AARCH64)
    regsDefined_ = bitArray(registerSpace::fpsr);
#elif defined(DYNINST_CODEGEN_ARCH_AMDGPU_GFX908)
    regsDefined_ = bitArray(700); // TODO: use the actual last register instead of a random constant
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

std::string codeGen::format() const {
   if (!aSpace_) return "<codeGen>";

   using namespace InstructionAPI;

   stringstream ret;

   Dyninst::Address base = (addr_ != Dyninst::ADDRESS_INVALID) ? addr_ : 0;
   InstructionDecoder deco
      (buffer_,used(),aSpace_->getArch());
   Instruction insn = deco.decode();
   ret << hex;
   while(insn.isValid()) {
       ret << "\t" << base << ": " << insn.format(base) << " / 0x";
       for(auto i=0UL; i<insn.size(); i++) {
         auto *ptr = static_cast<unsigned char const*>(insn.ptr());
         ret << std::hex << static_cast<unsigned int>(ptr[i]);
       }
       ret << '\n';
       base += insn.size();
       insn = deco.decode();
   }
   ret << dec;
   return ret.str();
}
   
