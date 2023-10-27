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

#include "CodeBuffer.h"
#include "CodeTracker.h"
#include "Widgets/Widget.h" //  Currently Patch is defined here; we may want to move it.

#include "dyninstAPI/src/debug.h"
#include "dyninstAPI/src/codegen.h"
#include <iostream>

#include "InstructionDecoder.h"
#include "Instruction.h"
#include "dyninstAPI/src/addressSpace.h"

using namespace Dyninst;
using namespace Relocation;
using namespace InstructionAPI;

const unsigned CodeBuffer::Label::INVALID = (unsigned) -1;


CodeBuffer::BufferElement::BufferElement() = default;

CodeBuffer::BufferElement::BufferElement(CodeBuffer::BufferElement&& other)  {
   *this = other;
   other.patch_ = nullptr;
}


CodeBuffer::BufferElement::~BufferElement() {
   if (patch_) delete patch_;
}

void CodeBuffer::BufferElement::addPIC(const unsigned char *input,
                                       unsigned size,
                                       TrackerElement *tracker) {
   addTracker(tracker);

   std::copy(input, input + size, std::back_inserter(buffer_));
}

void CodeBuffer::BufferElement::addPIC(const Buffer &buf,
                                       TrackerElement *tracker) {
   addTracker(tracker);
   
   std::copy(buf.begin(), buf.end(), std::back_inserter(buffer_));
}

void CodeBuffer::BufferElement::setPatch(Patch *patch,
                                         TrackerElement *tracker) {
   addTracker(tracker);
   assert(patch_ == NULL);

   patch_ = patch;
}

void CodeBuffer::BufferElement::setLabelID(unsigned id) {
   assert(labelID_ == Label::INVALID);
   labelID_ = id;
}

void CodeBuffer::BufferElement::addTracker(TrackerElement *tracker) {
   trackers_[buffer_.size()] = tracker;
}

bool CodeBuffer::BufferElement::empty() {
   // We're empty if:
   // No patch;
   // No label;
   // No buffer
   if (patch_) return false;
   if (labelID_ != Label::INVALID) return false;
   if (!buffer_.empty()) return false;
   return true;
}

unsigned totalPadding = 0;

bool CodeBuffer::BufferElement::generate(CodeBuffer *buf,
                                         codeGen &gen,
                                         int &shift,
                                         bool &regenerate) {
   codeBufIndex_t start = gen.getIndex();
   addr_ = gen.currAddr();

   // By definition, labels can only apply to the start of a
   // BufferElement. Update it now with our current address.
   buf->updateLabel(labelID_, addr_ - gen.startAddr(), regenerate);

   // Get the easy bits out of the way
   gen.copy(buffer_);

   if (patch_) {
      // Now things get interesting
      if (!patch_->apply(gen, buf)) {
	relocation_cerr << "Patch failed application, ret false" << std::endl;
         return false;
      }
   }
   unsigned newSize = gen.getDisplacement(start, gen.getIndex());
   if (newSize > size_) {
      shift += newSize - size_;
      size_ = newSize;
      regenerate = true;
   }
   else {
      gen.fill(size_ - newSize, codeGen::cgNOP);
   }
#if 0
   else if (newSize < size_) {
      shift -= size_ - newSize;
      size_ = newSize;
      regenerate = true;
   }
#endif
   //relocation_cerr << "BufferElement::generate, new size " << size_ << endl;

   return true;
}

bool CodeBuffer::BufferElement::extractTrackers(CodeTracker *t) {
   // Update tracker information (address, size) and add it to the
   // CodeTracker we were handed in.

   //relocation_cerr << "*** Begin tracker extraction from BufferElement" << endl;

   for (Trackers::iterator iter = trackers_.begin();
        iter != trackers_.end(); ++iter) {
      TrackerElement *e = iter->second;
      if (!e) continue; // 0-length "mark me" Widgets may not have trackers

      //relocation_cerr << "\t Tracker element: " << *e << endl;
      unsigned size = 0;
      Trackers::iterator next = iter; ++next;
      if (next != trackers_.end()) {
         //relocation_cerr << "\t\t\t Size calc: " << next->first << " - " << iter->first << endl;
         size = next->first - iter->first;
      }
      else {
         //relocation_cerr << "\t\t\t Size calc: " << size_ << " - " << iter->first << endl;
         size = size_ - iter->first;
      }
      //relocation_cerr << "\t\t Calculated size: " << size << endl;
      if (!size) continue;
      
      Address relocAddr = iter->first + addr_;
      e->setReloc(relocAddr);
      e->setSize(size);
      t->addTracker(e);
   }

   //relocation_cerr << "*** End tracker extraction from BufferElement" << endl;
   return true;
}

CodeBuffer::CodeBuffer()
   : size_(0), curIteration_(0), curLabelID_(1), shift_(0), generated_(false) {}

CodeBuffer::~CodeBuffer() {}

void CodeBuffer::initialize(const codeGen &templ, unsigned numBlocks) {
   gen_.applyTemplate(templ);
   // We don't start labels at 0.
   labels_.resize(numBlocks+2);
}

unsigned CodeBuffer::getLabel() {
   unsigned id = curLabelID_++;
   // Labels must begin BufferElements, so if the current BufferElement
   // has anything in it, create a new one
   if (buffers_.empty() ||
       (!buffers_.back().empty())) {
      buffers_.push_back(BufferElement());
   }
   buffers_.back().setLabelID(id);

   if (id >= labels_.size()) labels_.resize(id+1);

   // Fill in our data structures as well
   labels_[id] = Label(Label::Relative, id, size_);
   
   return id;
}

unsigned CodeBuffer::defineLabel(Address addr) {
   // A label for something that will not move
   unsigned id = curLabelID_++;

   // Since it doesn't move it isn't part of the BufferElement sequence.
   
   // Instead, we update the Labels structure directly
   if (id >= labels_.size()) labels_.resize(id+1);
   labels_[id] = Label(Label::Absolute, id, addr);
   return id;
}

void CodeBuffer::addPIC(const unsigned char *input, unsigned size, TrackerElement *tracker) {
   current().addPIC(input, size, tracker);
   size_ += size;
}

void CodeBuffer::addPIC(const void *input, unsigned size, TrackerElement *tracker) {
   addPIC((const unsigned char *)input, size, tracker);
}

void CodeBuffer::addPIC(const codeGen &input, TrackerElement *tracker) {
   addPIC(input.start_ptr(), input.used(), tracker);
}

void CodeBuffer::addPIC(Buffer buf, TrackerElement *tracker) {
   current().addPIC(buf, tracker);
   size_ += buf.size();
}

void CodeBuffer::addPatch(Patch *patch, TrackerElement *tracker) {
   current().setPatch(patch, tracker);
   size_ += patch->estimate(gen_);
}

CodeBuffer::BufferElement &CodeBuffer::current() {
   if (buffers_.empty() ||
       buffers_.back().full()) {
      buffers_.push_back(BufferElement());
   }
   return buffers_.back();
}

bool CodeBuffer::extractTrackers(CodeTracker *t) {
   for (Buffers::iterator iter = buffers_.begin();
        iter != buffers_.end(); ++iter) {
      if (!iter->extractTrackers(t)) return false;
   }
   return true;
}

bool CodeBuffer::generate(Address baseAddr) {
   generated_ = false;
   gen_.setAddr(baseAddr);
   bool doOver = false;

   do {
      doOver = false;
      curIteration_++;
      shift_ = 0;
      gen_.invalidate();
      gen_.allocate(size_);
      totalPadding = 0;

      for (Buffers::iterator iter = buffers_.begin();
           iter != buffers_.end(); ++iter) {
	bool regenerate = false;
         if (!iter->generate(this, gen_, shift_, regenerate)) {
            return false;
         }
         doOver |= regenerate;
      }
      
   } while (doOver);

   shift_ = 0;
   size_ = gen_.used();


   generated_ = true;
   return true;
}

void CodeBuffer::disassemble() const {
   // InstructionAPI to the rescue!!!
   InstructionAPI::InstructionDecoder decoder(gen_.start_ptr(),
                                              gen_.used(),
                                              gen_.getArch());
   Address addr = gen_.startAddr();

   Instruction cur = decoder.decode();
   while (cur.isValid()) {
      cerr << "\t" << std::hex << addr << std::dec << ": " << cur.format() << std::endl;
      addr += cur.size();
      cur = decoder.decode();
   }
}

void CodeBuffer::updateLabel(unsigned id, Address offset, bool &regenerate) {
  if (id == (unsigned) -1) return;


   if (id >= labels_.size()) {
      cerr << "ERROR: id of " << id << " but only " << labels_.size() << " labels!" << std::endl;
   }
   assert(id < labels_.size());
   assert(id > 0);
   Label &l = labels_[id];
   if (!l.valid()) return;

   //relocation_cerr << "\t Updating label " << id 
//                   << " -> " << std::hex << offset << std::dec << endl;
   if (l.addr != offset) {
      //relocation_cerr << "\t\t Old value " << std::hex << labels_[id].addr
//                      << ", regenerating!" << std::dec << endl;
      regenerate = true;
   }
   l.addr = offset;
   l.iteration++;
   l.type = Label::Estimate;
}

Address CodeBuffer::getLabelAddr(unsigned id) {
   assert(generated_);
   shift_ = 0;
   return predictedAddr(id);
}

Address CodeBuffer::predictedAddr(unsigned id) {
   if (id >= labels_.size()) {
      cerr << "ERROR: id of " << id << " but only " << labels_.size() << " labels!" << std::endl;
   }
   assert(id < labels_.size());
   assert(id > 0);
   Label &label = labels_[id];
   switch(label.type) {
      case Label::Absolute:
         //relocation_cerr << "\t\t Requested predicted addr for " << id
//                         << ", label is absolute, ret " << std::hex << label.addr << std::dec << endl;
         return label.addr;
      case Label::Relative:
         assert(gen_.startAddr());
         assert(gen_.startAddr() != (Address) -1);
         //relocation_cerr << "\t\t Requested predicted addr for " << id
//                         << ", label is relative, ret " << std::hex << label.addr + gen_.startAddr()
//                         << " = " << label.addr << " + " << gen_.startAddr()
            //             << std::dec << endl;
         return label.addr + gen_.startAddr();
      case Label::Estimate: {
         // In this case we want to adjust the address by 
         // our current shift value, only if the iteration
         // it was updated in is less than our current
         // iteration
         assert(gen_.startAddr());
         assert(gen_.startAddr() != (Address) -1);
         Address ret = label.addr + gen_.startAddr();
         if (label.iteration < curIteration_)
            ret += shift_;
         //relocation_cerr << "\t\t Requested predicted addr for " << id
//                         << ", label is relative, ret " << std::hex << ret
    //                     << " = " << label.addr << " + " << gen_.startAddr()
   //                      << " + (" << label.iteration << " < " 
   //                      << curIteration_ << ") ? " << shift_ 
   //                      << " : 0" << std::dec << endl;
         return ret;
      }
      default:
         assert(0);
   }
   assert(0);
   return 0;
}

unsigned CodeBuffer::size() const {
   return size_;
}

void *CodeBuffer::ptr() const {
   return gen_.start_ptr();
}
