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

#if !defined(_CODE_BUFFER_H_)
#define _CODE_BUFFER_H_



// Infrastructure for the code generating Widgets; try to automate
// as much of the handling as we can.
//
// Background
//  An Widget can generate two types of code: PIC and non-PIC. PIC code can be treated
//  as a sequence of bytes that can be freely copied around, whereas non-PIC code must
//  be tweaked whenever its address changes as part of code generation. Note, it is safe
//  treat PIC code as non-PIC (though less than optimal); the reverse is not true. For
//  efficiency we want to bundle PIC code together and avoid re-generating it in every pass,
//  while keeping callbacks for non-PIC code. 
//
//  Another requirement of this system is to build a tracking data structure that maps from
//  address in the generated code to who generated that code, so we can maintain a mapping 
//  between original addresses and moved (relocated) code. We also wish to do this automatically.
//
//  Thus, we provide an interface that automates these two functions as much as possible. We provide
//  two ways of specifying code: copy and patch. Copy handles PIC code, and pulls in a buffer of bytes
//  (e.g., memcpy). Patch registers a callback for later that will provide some code. 
//
//  With these two methods the user can specify a tracking data structure that specifies what kind
//  of code they have provided.

#include "common/h/dyntypes.h"
#include <assert.h>
#include <map>
#include <vector>
#include <list>
#include "dyninstAPI/src/codegen.h"

class codeGen;

namespace Dyninst {
namespace Relocation {
        
// The input grammar to the CodeBuffer is [PIC|nonPIC]*. However, it is safe to accumulate
// between adjacent PICs, so we can simplify this to ((PIC*)nonPIC)*. This drives our internal
// storage, which is a list of (buffer, patch) pairs; the buffer contains (PIC*) and the
// patch represents a nonPIC entry. For each pair, we also associate a list of code trackers, 
// since there is a 1:1 relationship between each element (PIC, nonPIC) and a tracker. 
        
class TrackerElement;
struct Patch;
class CodeTracker;
class CodeMover;

class CodeBuffer {
  public: 
   typedef std::vector<unsigned char> Buffer;
  private:
   
   struct Label {
      typedef enum {
         Invalid,
         Absolute,
         Relative,
         Estimate } Type;
      typedef unsigned Id;

      Type type;
      Id id;
      int iteration;
      // This is a bit of a complication. We want to estimate
      // where a label will go pre-generation to try and
      // reduce how many iterations we need to go through. Thus,
      // addr may either be an absolute address,
      // an offset, or an estimated address.
      Address addr;

      static const unsigned INVALID;

      Label() noexcept
      : type(Invalid), id(0), iteration(0), addr(0) {}
      Label(Type a, Id b, Address c)
      : type(a), id(b), iteration(0), addr(c) { assert(id != INVALID); }
      bool valid() { return type != Invalid; }
   };

   class BufferElement {
      friend class CodeBuffer;
     public:
      BufferElement();
      BufferElement(const BufferElement&) = delete;
      BufferElement(BufferElement&&);
      ~BufferElement();
      void setLabelID(unsigned id);
      void addPIC(const unsigned char *input, unsigned size, TrackerElement *tracker);
      void addPIC(const Buffer &buffer, TrackerElement *tracker);
      void setPatch(Patch *patch, TrackerElement *tracker);

      bool full() { return patch_ != NULL; }
      bool empty();
      bool generate(CodeBuffer *buf,
                    codeGen &gen,
                    int &shift,
                    bool &regenerate);
      bool extractTrackers(CodeTracker *t);

     private:
      BufferElement& operator=(BufferElement&) = default;
      void addTracker(TrackerElement *tracker);

      Address addr_{};
      unsigned size_{};
      Buffer buffer_;
      Patch *patch_{};
      unsigned labelID_{Label::INVALID};
      // Here the Offset is an offset within the buffer, starting at 0.
      typedef std::map<Offset, TrackerElement *> Trackers;
      Trackers trackers_;
   };
   
  public:
   CodeBuffer();
   ~CodeBuffer();
   
   void initialize(const codeGen &templ, unsigned numBlocks);
   
   unsigned getLabel();
   unsigned defineLabel(Address addr);
   void addPIC(const unsigned char *input, unsigned size, TrackerElement *tracker);
   void addPIC(const void *input, unsigned size, TrackerElement *tracker);
   void addPIC(const codeGen &gen, TrackerElement *tracker);
   void addPIC(const Buffer buf, TrackerElement *tracker);
   void addPatch(Patch *patch, TrackerElement *tracker);
   bool extractTrackers(CodeTracker *t);

   unsigned size() const;
   void *ptr() const;

   bool generate(Address baseAddr);
   void updateLabel(unsigned id, Address addr, bool &regenerate);
   
   Address predictedAddr(unsigned labelID);
   Address getLabelAddr(unsigned labelID);

   void disassemble() const;


   codeGen &gen() { return gen_; }
  private:

   BufferElement &current();

   typedef std::list<BufferElement> Buffers;
   Buffers buffers_;

   unsigned size_;

   codeGen gen_;

   int curIteration_;

   //typedef std::map<int, Label> Labels;
   typedef std::vector<Label> Labels;

   Labels labels_;
   int curLabelID_;

   int shift_;

   bool generated_;
};
}
}


#endif
