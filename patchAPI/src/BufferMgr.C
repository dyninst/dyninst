/* Utility */

#include "BufferMgr.h"
#include "Tracker.h"
//#include "Widget.h" //  Currently Patch is defined here; we may want to move it.

using Dyninst::InstructionAPI::InstructionDecoder;
using Dyninst::InstructionAPI::Instruction;
using Dyninst::PatchAPI::BufferMgr;

const int BufferMgr::Label::INVALID = -1;

BufferMgr::BufferElement::BufferElement()
  : addr_(0), size_(0), patch_(NULL), labelID_(Label::INVALID) {};

BufferMgr::BufferElement::~BufferElement() {
  if (patch_) delete patch_;
}

void BufferMgr::BufferElement::addPIC(const unsigned char *input,
                                      unsigned size,
                                      Element *tracker) {
  addTracker(tracker);
  std::copy(input, input + size, std::back_inserter(buffer_));
}

void BufferMgr::BufferElement::addPIC(const RawBuffer &buf,
                                       Element *tracker) {
  addTracker(tracker);
  std::copy(buf.begin(), buf.end(), std::back_inserter(buffer_));
}

void BufferMgr::BufferElement::setPatch(Patch *patch,
                                        Element *tracker) {
  addTracker(tracker);
  assert(patch_ == NULL);
  patch_ = patch;
}

void BufferMgr::BufferElement::setLabelID(int id) {
  assert(labelID_ == Label::INVALID);
  labelID_ = id;
}

void BufferMgr::BufferElement::addTracker(Element *tracker) {
  trackers_[buffer_.size()] = tracker;
}

bool BufferMgr::BufferElement::empty() {
   // We're empty if:
   // No patch;
   // No label;
   // No buffer
   if (patch_) return false;
   if (labelID_ != Label::INVALID) return false;
   if (!buffer_.empty()) return false;
   return true;
}

bool BufferMgr::BufferElement::generate(BufferMgr *mgr,
                                        Buffer* buf,
                                        int &shift,
                                        bool &regenerate) {
  codeBufIndex_t start = buf->getIndex();
  addr_ = buf->currAddr();

  // By definition, labels can only apply to the start of a
  // BufferElement. Update it now with our current address.
  mgr->updateLabel(labelID_, addr_ - buf->startAddr(), regenerate);

  // Get the easy bits out of the way
  buf->copy(buffer_);

  if (patch_) {
    // Now things get interesting
    if (!patch_->apply(buf)) {
      return false;
    }
  }
  unsigned newSize = buf->getDisplacement(start, buf->getIndex());
  if (newSize > size_) {
    shift += newSize - size_;
    size_ = newSize;
    regenerate = true;
  }
  else {
    // We don't want sizes to decrease or we can get stuck in generation
    buf->fillNOP(size_ - newSize);
  }

  return true;
}

bool BufferMgr::BufferElement::extractTrackers(Tracker &t) {
  // Update tracker information (address, size) and add it to the
  // Tracker we were handed in.
  for (Trackers::iterator iter = trackers_.begin();
       iter != trackers_.end(); ++iter) {
    Element *e = iter->second; assert(e);
    unsigned size = 0;
    Trackers::iterator next = iter; ++next;
    if (next != trackers_.end()) {
      size = next->first - iter->first;
    }
    else {
      size = size_ - iter->first;
    }
    if (!size) continue;
    Address relocAddr = iter->first + addr_;
    e->setReloc(relocAddr);
    e->setSize(size);
    t.addTracker(e);
  }
  return true;
}

BufferMgr::BufferMgr()
   : size_(0), curIteration_(0), curLabelID_(1), shift_(0), generated_(false) {

}

BufferMgr::~BufferMgr() {};

void BufferMgr::initialize(Buffer* buf) {
  buf_ = buf;
  buf_->setMgr(this);
}

int BufferMgr::getLabel() {
  int id = curLabelID_++;
  // Labels must begin BufferElements, so if the current BufferElement
  // has anything in it, create a new one
  if (buffers_.empty() ||
      (!buffers_.back().empty())) {
    buffers_.push_back(BufferElement());
  }
  buffers_.back().setLabelID(id);

  // Fill in our data structures as well
  labels_[id] = Label(Label::Relative, id, size_);

  return id;
}

int BufferMgr::defineLabel(Address addr) {
  // A label for something that will not move
  int id = curLabelID_++;

  // Since it doesn't move it isn't part of the BufferElement sequence.
  // Instead, we update the Labels structure directly
  labels_[id] = Label(Label::Absolute, id, addr);
  return id;
}

void BufferMgr::addPIC(const unsigned char *input, unsigned size, Element *tracker) {
  current().addPIC(input, size, tracker);
  size_ += size;
}

void BufferMgr::addPIC(const void *input, unsigned size, Element *tracker) {
  addPIC((const unsigned char *)input, size, tracker);
  size_ += size;
}

void BufferMgr::addPIC(RawBuffer buf, Element *tracker) {
  current().addPIC(buf, tracker);
  size_ += buf.size();
}

void BufferMgr::addPatch(Patch *patch, Element *tracker) {
  current().setPatch(patch, tracker);
  size_ += patch->estimate(buf_);
}

BufferMgr::BufferElement &BufferMgr::current() {
  if (buffers_.empty() ||
      buffers_.back().full()) {
    buffers_.push_back(BufferElement());
  }
  return buffers_.back();
}

bool BufferMgr::extractTrackers(Tracker &t) {
  for (Buffers::iterator iter = buffers_.begin();
       iter != buffers_.end(); ++iter) {
    if (!iter->extractTrackers(t)) return false;
  }
  return true;
};

bool BufferMgr::generate(Address baseAddr) {
  generated_ = false;
  buf_->setAddr(baseAddr);
  bool doOver = false;

  do {
    doOver = false;
    curIteration_++;
    shift_ = 0;
    buf_->invalidate();
    buf_->allocate(size_);

    for (Buffers::iterator iter = buffers_.begin();
         iter != buffers_.end(); ++iter) {
      bool regenerate = false;
      if (!iter->generate(this, buf_, shift_, regenerate)) {
        return false;
      }
      doOver |= regenerate;
    }

  } while (doOver);

  shift_ = 0;
  size_ = buf_->used();

  generated_ = true;
  return true;
}

void BufferMgr::disassemble() const {
  // InstructionAPI to the rescue!!!
  InstructionDecoder decoder(buf_->start_ptr(),
                             buf_->used(),
                             buf_->getArch());
  Address addr = buf_->startAddr();
  Instruction::Ptr cur = decoder.decode();
  while (cur && cur->isValid()) {
    cerr << "\t" << std::hex << addr << std::dec << ": " << cur->format() << endl;
    addr += cur->size();
    cur = decoder.decode();
  }
}

void BufferMgr::updateLabel(int id, Address offset, bool &regenerate) {
  Labels::iterator iter = labels_.find(id);
  if (iter == labels_.end()) return;
  if (!iter->second.valid()) return;

  if (id == -1) {
    cerr << "Illegal ID that wasn't caught by validity check, spinning" << endl;
    cerr << "Label info @ " << id << ":" << endl;
    cerr << "\t" << labels_[id].addr << endl;
    cerr << "\t" << labels_[id].iteration << endl;
    cerr << "\t" << labels_[id].type << endl;
    while(1);
  }

  if (iter->second.addr != offset) {
    regenerate = true;
  }
  iter->second.addr = offset;
  iter->second.iteration++;
  iter->second.type = Label::Estimate;
}

Address BufferMgr::getLabelAddr(int id) {
  assert(generated_);
  shift_ = 0;
  return predictedAddr(id);
}

Address BufferMgr::predictedAddr(int id) {
  Labels::iterator iter = labels_.find(id);
  assert(iter != labels_.end());

  Label &label = iter->second;
  switch(label.type) {
  case Label::Absolute:
    return label.addr;
  case Label::Relative:
    assert(buf_->startAddr());
    assert(buf_->startAddr() != (Address) -1);
    return label.addr + buf_->startAddr();
  case Label::Estimate: {
    // In this case we want to adjust the address by
    // our current shift value, only if the iteration
    // it was updated in is less than our current
    // iteration
    assert(buf_->startAddr());
    assert(buf_->startAddr() != (Address) -1);
    Address ret = label.addr + buf_->startAddr();
    if (label.iteration < curIteration_)
      ret += shift_;
    return ret;
  }
  default:
    assert(0);
  }
  assert(0);
  return 0;
}

unsigned BufferMgr::size() const {
  return size_;
}

void *BufferMgr::ptr() const {
  return buf_->start_ptr();
}
