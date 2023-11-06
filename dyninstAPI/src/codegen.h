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
#if !defined(_codegen_h_)
#define _codegen_h_

#include <utility>
#include <vector>
#include <string>
#include <map>
#include "dyntypes.h"
#include "dyn_register.h"
#include "common/src/arch.h"
#include "dyninstAPI/src/patch.h"

#if defined(arch_power)
#include "codegen-power.h"
using namespace NS_power;
#elif defined(i386_unknown_nt4_0) \
   || defined(arch_x86)           \
   || defined(arch_x86_64)
#include "codegen-x86.h"
using namespace NS_x86;
#elif defined(arch_aarch64)
#include "codegen-aarch64.h"
using namespace NS_aarch64;
#else
#error "unknown architecture"
#endif

#include "bitArray.h"
#include "pcrel.h"

#include "arch-forward-decl.h" // instruction

//hateful windows.h
#if defined(_MSC_VER)
#undef max
#endif

class AddressSpace;
class instPoint;
class registerSpace;
class regTracker_t;
class AstNode;
class Emitter;
class pcRelRegion;
class func_instance;
class PCThread;
class baseTramp;
class block_instance;

// Code generation
// This class wraps the actual code generation mechanism: we keep a buffer
// and a pointer to where we are in the buffer. This may vary by platform,
// hence the typedef wrappers.
class codeGen {
    // Instruction modifies these.
    //friend class instruction;
    friend class AstNode;
 public:
    // Default constructor -- makes an empty generation area
    codeGen();
    // Make a generation buffer with the given size
    codeGen(unsigned size);
    // Use a preallocated buffer
    codeGen(codeBuf_t *buf, int size);
    ~codeGen();

    bool valid() { return buffer_ != NULL; }

    bool verify();

    // Copy constructor. Deep-copy -- allocates
    // a new buffer
    codeGen(const codeGen &);

    // We consider our pointer to either be the start
    // of the buffer, or NULL if the buffer is empty
    bool operator==(void *ptr) const;
    bool operator!=(void *ptr) const;

    // Assignment....
    codeGen &operator=(const codeGen &param);

    // Initialize the current using the argument as a "template"
    void applyTemplate(const codeGen &codeTemplate);
    static codeGen baseTemplate;

    // Allocate a certain amount of space
    void allocate(unsigned);
    // And invalidate
    void invalidate();

    // Finally, tighten down the memory usage. This frees the 
    // buffer if it's bigger than necessary and copies everything
    // to a new fixed buffer.
    void finalize();
    
    // Copy a buffer into here and move the offset
    void copy(const void *buf, const unsigned size);
    void copy(const void *buf, const unsigned size, const codeBufIndex_t index);
    void copy(const std::vector<unsigned char> &buf);
    // Insert buffer into index, moving previous content
    void insert(const void *buf, const unsigned size, const codeBufIndex_t index);
    // Workaround for copying strings on word-aligned platforms
    void copyAligned(const void *buf, const unsigned size);

    // Similar, but slurp from the start of the parameter
    void copy(codeGen &gen);

    // How much space are we using?
    unsigned used() const;

    unsigned size() const { return size_; }
    unsigned max() const { return max_; }

    // Blind pointer to the start of the code area
    void *start_ptr() const;
    // With ptr() and used() you can copy into the mutatee.

    // Pointer to the current location...
    void *cur_ptr() const;

    // And pointer to a given offset
    void *get_ptr(unsigned offset) const;

    // For things that make a copy of the current pointer and
    // play around with it. This recalculates the current offset
    // based on a new pointer
    void update(codeBuf_t *ptr);

    // Set the offset at a particular location.
    void setIndex(codeBufIndex_t offset);

    codeBufIndex_t getIndex() const;

    // Move up or down a certain amount
    void moveIndex(int disp);

    // To calculate a jump between the "from" and where we are
    static  long getDisplacement(codeBufIndex_t from, codeBufIndex_t to);

    // For code generation -- given the current state of 
    // generation and a base address in the mutatee, 
    // produce a "current" address.
    Dyninst::Address currAddr() const;
    Dyninst::Address currAddr(Dyninst::Address base) const;
    
    enum { cgNOP, cgTrap, cgIllegal };

    void fill(unsigned fillSize, int fillType);
    // Since we have a known size
    void fillRemaining(int fillType);

    std::string format() const;

    //Add a new PCRelative region that should be generated after 
    // addresses are fixed
    void addPCRelRegion(pcRelRegion *reg);

    //Have each region generate code with this codeGen object being
    // placed at addr
    void applyPCRels(Dyninst::Address addr);

    //Return true if there are any active regions.
    bool hasPCRels() const;

    //Add a new patch point
    void addPatch(const relocPatch &p);

    //Create a patch into the codeRange
    void addPatch(codeBufIndex_t index, patchTarget *source, 
                  unsigned size = sizeof(Dyninst::Address),
                  relocPatch::patch_type_t ptype = relocPatch::patch_type_t::abs,
                  Dyninst::Offset off = 0);

    std::vector<relocPatch> &allPatches();

    //Apply all patches that have been added
    void applyPatches();

    void setAddrSpace(AddressSpace *a);
    void setThread(PCThread *t) { thr_ = t; }
    void setRegisterSpace(registerSpace *r) { rs_ = r; }
    void setAddr(Dyninst::Address a) { addr_ = a; }
    void setPoint(instPoint *i) { ip_ = i; }
    void setRegTracker(regTracker_t *t) { t_ = t; }
    void setCodeEmitter(Emitter *emitter) { emitter_ = emitter; }
    void setFunction(func_instance *f) { f_ = f; }
    void setBT(baseTramp *i) { bt_ = i; }
    void setInInstrumentation(bool i) { inInstrumentation_ = i; }

    unsigned width() const;
    AddressSpace *addrSpace() const;
    PCThread *thread();
    Dyninst::Address startAddr() const { return addr_; }
    instPoint *point() const;
    baseTramp *bt() const { return bt_; }
    func_instance *func() const;
    registerSpace *rs() const;
    regTracker_t *tracker() const;
    Emitter *codeEmitter() const;
    Emitter *emitter() const { return codeEmitter(); } // A little shorter
    bool inInstrumentation() const { return inInstrumentation_; }

    bool insertNaked() const { return insertNaked_; }
    void setInsertNaked(bool i) { insertNaked_ = i; }
    
    bool modifiedStackFrame() const { return modifiedStackFrame_; }
    void setModifiedStackFrame(bool i) { modifiedStackFrame_ = i; }

    Dyninst::Architecture getArch() const;

    void beginTrackRegDefs();
    void endTrackRegDefs();
    const bitArray &getRegsDefined();
    void markRegDefined(Dyninst::Register r);
    bool isRegDefined(Dyninst::Register r);

    void setPCRelUseCount(int c) { pc_rel_use_count = c; }
    int getPCRelUseCount() const { return pc_rel_use_count; }

    // SD-DYNINST
    // 
    typedef std::pair<Dyninst::Address, unsigned> Extent;
    void registerDefensivePad(block_instance *, Dyninst::Address, unsigned);
    std::map<block_instance *, Extent> &getDefensivePads() { return defensivePads_; }
    
    // Immediate uninstrumentation
    void registerInstrumentation(baseTramp *bt, Dyninst::Address loc) { instrumentation_[bt] = loc; }
    std::map<baseTramp *, Dyninst::Address> &getInstrumentation() { return instrumentation_; }
    
    void registerRemovedInstrumentation(baseTramp *bt, Dyninst::Address loc) { removedInstrumentation_[bt] = loc; }
    std::map<baseTramp *, Dyninst::Address> &getRemovedInstrumentation() { return removedInstrumentation_; }

 private:
    void realloc(unsigned newSize); 

    codeBuf_t *buffer_;
    codeBufIndex_t offset_;
    unsigned size_;
    unsigned max_;
    int pc_rel_use_count;

    Emitter *emitter_;
    bool allocated_;

    AddressSpace *aSpace_;
    PCThread *thr_;
    registerSpace *rs_;
    regTracker_t *t_;
    Dyninst::Address addr_;
    instPoint *ip_;
    func_instance *f_;
    baseTramp *bt_;
    bool isPadded_;

    bitArray regsDefined_;
    bool trackRegDefs_;

    bool inInstrumentation_;

    bool insertNaked_;
    bool modifiedStackFrame_;

    std::vector<relocPatch> patches_;
    std::vector<pcRelRegion *> pcrels_;

    std::map<block_instance *, Extent> defensivePads_;
    std::map<baseTramp *, Dyninst::Address> instrumentation_;
    std::map<baseTramp *, Dyninst::Address> removedInstrumentation_;
};

#endif
