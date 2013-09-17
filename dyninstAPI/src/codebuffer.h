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
#if !defined(_codebuf_h_)
#define _codebuf_h_

#include <vector>

#include "common/src/arch.h"
#include "dyninstAPI/src/patch.h"

// This class serves as a flexible buffer target for code generation.
// We support two methods of writing: either copying data into the
// buffer via codebuf methods or accessing a pointer and data copying externally.
// The second is ugly but works well as a legacy method.

// However, neither handles the problem of generating code that has variable size.
// We want to be efficient in handling PIC code (which can just be moved), and an awful
// lot of what we do is PIC. However, we need to handle patching mechanisms, both when
// the size of the patch is known (e.g., easy) and unknown (e.g., hard). We do this by
// creating a list of alternating PIC code buffers (possibly annotated with fixed-size
// patches) and non-fixed code generating patches. 
// 
// Thus an index is actually a pair that identifies which internal code buffer to use
// and where in that code buffer we want. 
//
// Finally, we have a huge pile of interface methods that completely hide this from our
// users. We do have one big restriction: you cannot put in any references (e.g., branch
// targets) across a variable-size patch. 

class AstNode;

class codeBuf {
  friend class AstNode;

 public:
  typedef struct {
    unsigned index;
    unsigned bufferID; } index_t;

  struct intBuffer {
    char *buffer;
    unsigned size;
    Patch *patch; 
  intBuffer() :
    buffer(NULL), size(0), patch(NULL) {};
    ~intBuffer() {
      if (buffer) delete buffer;
      if (patch) delete patch;
    };
  };
  

  typedef unsigned char *ptr_t;

  // Default constructor -- makes an empty generation area
  codeBuf();
  // Make a generation buffer with the given size
  codeBuf(unsigned size);
  // Use a preallocated buffer
  codeBuf(ptr_t buf, int size);
  ~codeBuf();
  
  // Copy constructor. Deep-copy -- allocates
  // a new buffer
  codeBuf(const codeBuf &);
  
  // We consider our pointer to either be the start
  // of the buffer, or NULL if the buffer is empty
  bool operator==(void *ptr) const;
  bool operator!=(void *ptr) const;
  
  // Assignment....
  codeBuf &operator=(const codeBuf &param);
  
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
  void copy(const void *buf, const unsigned size, const index_t index);
  
  // Similar, but slurp from the start of the parameter
  void copy(codeBuf &gen);

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
  void update(ptr_t *ptr);
  
  // Set the offset at a particular location.
  void setIndex(index_t offset);
  
  index_t getIndex() const;
  
  // Move up or down a certain amount
  void moveIndex(int disp);
  
  // To calculate a jump between the "from" and where we are
  static int getDisplacement(index_t from, index_t to);
  
  // For code generation -- given the current state of 
  // generation and a base address in the mutatee, 
  // produce a "current" address.
  Address currAddr() const;
  Address currAddr(Address base) const;

  // We have like three different patch mechanisms. Unfortunately, we 
  // also need to support them all. 

  // Complex patch mechanism - takes a GenState object and produces
  // some code. 
  void addPatch(Patch *);

  // Simplified patch mechanism that has a known, fixed length. 
  // This is a patch localized to a single point with a known size
  //Add a new patch point
  void addPatch(const relocPatch &p);
  
  //Create a patch into the codeRange
  void addPatch(codeBufIndex_t index, patchTarget *source, 
		unsigned size = sizeof(Address),
		relocPatch::patch_type_t ptype = relocPatch::abs,
		Dyninst::Offset off = 0);
  
  std::vector<relocPatch> &allPatches();
  
  //Apply all patches that have been added
  void applyPatches();
  
  

 private:
  void realloc(unsigned newSize); 

  void inc();

  std::vector<relocPatch> patches_;
  vector<intBuffer> buffers_;
};

#endif
