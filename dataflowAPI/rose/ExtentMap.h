//
// Created by ssunny on 6/17/16.
//

#ifndef DYNINST_EXTENTMAP_H
#define DYNINST_EXTENTMAP_H

#include <iosfwd>
#include <string>
#include "rangemap.h"
#include "typedefs.h"
#include "util/Interval.h"
#include "util/IntervalSet.h"

typedef Range<rose_addr_t> Extent;
typedef Sawyer::Container::Interval<rose_addr_t> AddressInterval;
typedef Sawyer::Container::IntervalSet<AddressInterval> AddressIntervalSet;

class ExtentMap: public RangeMap<Extent> {
public:
    ExtentMap(): RangeMap<Extent>() {}
    template<class Other> ExtentMap(const Other &other): RangeMap<Extent>(other) {}
    static char category(const Extent &a, const Extent &b);
    ExtentMap subtract_from(const Extent &e) const {
        return invert_within<ExtentMap>(e);
    }
    void allocate_at(const Extent &request);
    Extent allocate_best_fit(const rose_addr_t size);
    Extent allocate_first_fit(const rose_addr_t size);
    void dump_extents(std::ostream&, const std::string &prefix="", const std::string &label="") const;
    void dump_extents(FILE *f, const char *prefix, const char *label, bool pad=true) const;
};

#endif //DYNINST_EXTENTMAP_H
