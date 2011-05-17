/* Utility */

#ifndef PATCHAPI_H_BUFFERMGR_H_
#define PATCHAPI_H_BUFFERMGR_H_

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
//  between original addresses and moved (relocated) code. We also wish to do
//  this automatically.
//
//  Thus, we provide an interface that automates these two functions as much as possible. We provide
//  two ways of specifying code: copy and patch. Copy handles PIC code, and pulls in a buffer of bytes
//  (e.g., memcpy). Patch registers a callback for later that will provide some code.
//
//  With these two methods the user can specify a tracking data structure that specifies what kind
//  of code they have provided.

#include "common.h"

typedef unsigned codeBufIndex_t;
namespace Dyninst {
namespace PatchAPI {


class Element;
struct Patch;
class Tracker;
class Buffer;

/* A generic code patching mechanism */
struct Patch {
  virtual bool apply(Buffer *buf) = 0;
  virtual unsigned estimate(Buffer* buf) = 0;
  virtual ~Patch() {};
};

/* The input grammar to the BufferMgr is [PIC|nonPIC]*. However, it is safe to accumulate
   between adjacent PICs, so we can simplify this to ((PIC*)nonPIC)*. This drives our internal
   storage, which is a list of (buffer, patch) pairs; the buffer contains (PIC*) and the
   patch represents a nonPIC entry. For each pair, we also associate a list of code trackers,
   since there is a 1:1 relationship between each element (PIC, nonPIC) and a tracker. */
class BufferMgr {
  public:
    typedef std::vector<unsigned char> RawBuffer;

  private:
    struct Label {
      typedef enum {
        Invalid,
        Absolute,
        Relative,
        Estimate } Type;
      typedef int Id;

      Type type;
      Id id;
      int iteration;

      // This is a bit of a complication. We want to estimate
      // where a label will go pre-generation to try and
      // reduce how many iterations we need to go through. Thus,
      // addr may either be an absolute address,
      // an offset, or an estimated address.
      Address addr;

      static const int INVALID;

      Label()
      : type(Invalid), id(0), iteration(0), addr(0) {};
      Label(Type a, Id b, Address c)
      : type(a), id(b), iteration(0), addr(c) { assert(id != -1); };
      ~Label() {};
      bool valid() { return type != INVALID; };
   };

   class BufferElement {
     public:
       BufferElement();
       ~BufferElement();
       void setLabelID(int id);
       void addPIC(const unsigned char *input, unsigned size, Element *tracker);
       void addPIC(const RawBuffer &buffer, Element *tracker);
       void setPatch(Patch *patch, Element *tracker);

       bool full() { return patch_ != NULL; };
       bool empty();
       bool generate(BufferMgr *mgr,
                     Buffer* buf,
                     int &shift,
                     bool &regenerate);
      bool extractTrackers(Tracker &t);

     private:
       Address addr_;
       unsigned size_;
       RawBuffer buffer_;
       Patch *patch_;
       int labelID_;

       // Here the Offset is an offset within the buffer, starting at 0.
       typedef std::map<Offset, Element *> Trackers;
       Trackers trackers_;

       void addTracker(Element *tracker);
   };

  public:
    BufferMgr();
    ~BufferMgr();

    void initialize(Buffer* buf);
    int getLabel();
    int defineLabel(Address addr);
    void addPIC(const unsigned char *input, unsigned size, Element *tracker);
    void addPIC(const void *input, unsigned size, Element *tracker);
    void addPIC(const RawBuffer buf, Element *tracker);
    void addPatch(Patch *patch, Element *tracker);
    bool extractTrackers(Tracker &t);
    unsigned size() const;
    void *ptr() const;
    bool generate(Address baseAddr);
    void updateLabel(int id, Address addr, bool &regenerate);
    Address predictedAddr(int labelID);
    Address getLabelAddr(int labelID);
    void disassemble() const;
    Buffer* buffer() { return buf_; }

  private:

    BufferElement &current();
    typedef std::list<BufferElement> Buffers;
    Buffers buffers_;
    unsigned size_;
    int curIteration_;
    typedef std::map<int, Label> Labels;
    Labels labels_;
    int curLabelID_;
    int shift_;
    bool generated_;

    Buffer* buf_;
};

/* Split dyninst/src/codeGen into two parts.
   1. Buffer, representing a buffer to hold generated binary code
   2. Context, the information needed to generate code
   -- this is fulfilled by Trace abstraction */
class Buffer {
  public:
    virtual void invalidate() = 0;
    virtual void allocate(Address addr) = 0;

    // How much space are we using?
    virtual Address used() = 0;

    // Blind pointer to the start of the code area
    virtual void *start_ptr() const = 0;

    // Target architecture
    virtual Dyninst::Architecture getArch() const = 0;

    // Get the offset at a particular location.
    virtual codeBufIndex_t getIndex() = 0;

    // For code generation -- given the current state of
    // generation and a base address in the mutatee,
    // produce a "current" address.
    virtual Address currAddr() const = 0;

    // return the base address in the mutatee
    virtual Address startAddr() = 0;

    // set the base address in the mutatee
    virtual void setAddr(Address addr) = 0;

    // Copy a buffer into here and move the offset
    virtual void copy(std::vector<unsigned char>& buf) = 0;

    // To calculate a jump between the "from" and "to"
    virtual int getDisplacement(codeBufIndex_t from, codeBufIndex_t to) = 0;

    // fill in a series of NOPs
    virtual void fillNOP(unsigned fillSize) = 0;

    virtual void setMgr(BufferMgr* mgr) { mgr_ = mgr; }
    virtual BufferMgr* mgr() { return mgr_; }

    virtual ~Buffer() { }

  private:
    BufferMgr *mgr_;
};

};
};

#endif /* PATCHAPI_H_BUFFERMGR_H_ */
