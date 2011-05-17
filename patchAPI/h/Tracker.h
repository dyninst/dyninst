/* Utility */

#ifndef PATCHAPI_H_DYNINST_ELEMENT_H_
#define PATCHAPI_H_DYNINST_ELEMENT_H_

#include "common.h"

namespace Dyninst {
namespace PatchAPI {

/* Keeps a list of relocated Widgets (compessed where possible) for mapping
   addresses and types between original and relocated code. */
class Element {
  public:
    typedef enum {
      original,
      emulated,
      instrumentation
    } type_t;

    Element(Address o, PatchBlock *b)
      : orig_(o), reloc_(0), size_(0),
      block_(b) {assert(o); assert(b);};
    virtual ~Element() {};

    virtual Address relocToOrig(Address reloc) const = 0;
    virtual Address origToReloc(Address orig) const = 0;
    virtual type_t type() const = 0;
    Address orig() const { return orig_; };
    Address reloc() const { return reloc_; };
    unsigned size() const { return size_; };
    PatchBlock *block() const { return block_; };

    void setReloc(Address reloc) { reloc_ = reloc; };
    void setSize(unsigned size) { size_ = size; }

  protected:
    Element() {};
    Address orig_;
    Address reloc_;
    unsigned size_;
    PatchBlock *block_;
};

class Tracker {
  public:
    struct RelocatedElements {
      Address instruction;
      Address instrumentation;
      RelocatedElements() : instruction(0), instrumentation(0) {};
    };

    typedef Address UniqueFunctionID;
    typedef std::list<Element *> TrackerList;
    typedef std::map<Address, RelocatedElements> ForwardsMap;
    typedef ForwardsMap::const_iterator FM_citer;
    typedef std::map<UniqueFunctionID, ForwardsMap> BlockForwardsMap;
    typedef BlockForwardsMap::const_iterator BFM_citer;
    typedef class IntervalTree<Address, Element *> ReverseMap;

    Tracker() {};
    ~Tracker() {};

    bool origToReloc(Address origAddr, PatchFunction *func, RelocatedElements &relocs) const;
    Element *findByReloc(Address relocAddr) const;

    Address lowOrigAddr() const;
    Address highOrigAddr() const;
    Address lowRelocAddr() const;
    Address highRelocAddr() const;

    void addTracker(Element *);

    void createIndices();

    void debug();

 private:

   // We make this int_function specific to handle shared code
   BlockForwardsMap origToReloc_;
   ReverseMap relocToOrig_;
   TrackerList trackers_;
};

};
};

std::ostream &
operator<<(std::ostream &os, const Dyninst::PatchAPI::Element &e);

#endif /* PATCHAPI_H_DYNINST_ELEMENT_H_ */
