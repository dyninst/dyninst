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

#ifndef __DYNINTERVALMAP_H__
#define __DYNINTERVALMAP_H__

#include <map>
#include <vector>
#include <iostream>


namespace Dyninst  {

// IntervalValue
//
// Helper class used by IntervalMap, containing the range and elements in this range

template <typename K, typename V, typename ExtractRange>
class IntervalValue
{
public:
    IntervalValue(K l, K h, const V &e) : low(l), high(h), elements({e}) {}
    IntervalValue(K l, K h, std::vector<V> v) : low(l), high(h), elements(std::move(v)) {}
    K low;                      // inclusive low of this interval
    K high;                     // exclusive high of this interval
    std::vector<V> elements;    // elements whose interval contains this
                                // interval

    const std::vector<const V> GetELements()    { return elements; }
    K GetLow()                                  { return low; }
    K GetHigh()                                 { return high; }

    friend std::ostream &operator<<(std::ostream &os, const IntervalValue &iv)
    {
        os << "[low=" << iv.low << " high=" << iv.high << " :";
        for (auto const &e: iv.elements)  {
            os << " " << e;
        }
        os << " ]";
        return os;
    }
};


// IntervalMap class
//
// maps a set of intervals to an element { [K low_i, K high_i) -> V v_i }
// allowing lookup of K k returning a set of V containing
// All {v_i | low_i <= k < high_i}
//
// The data is stored in a std::map as a sequence of intervals IntervalValue's
// with the low value of the interval mapped to an IntervalValue type containing
// the range of the interval and a vector of all v_i's contained by this
// interval.  New IntervalValues are added/split along with adding elements to
// existing IntervalValues when new Intervals are added to the IntervalMap.
//
// By the nature of the map the intervals are ordered by the low value of the
// interval.  Each IntervalValue is a non-empty interval (low < high) and no
// interval overlaps another (currentInterval.high <= nextInterval.low).
// If currentInterval.high = nextInterval.low, then there is a gap between
// the intervals that contains no elements.

template <typename K, typename V, typename ExtractRange>
class IntervalMap
{
public:
    using IntervalValueType = IntervalValue<K, V, ExtractRange>;
    using IntervalMapType = std::map<K, IntervalValueType>;
    using key_type = typename IntervalMapType::key_type;                // K
    using mapped_type = typename IntervalMapType::mapped_type;          // IntervalValueType
    using value_type = typename IntervalMapType::value_type;
    using size_type = typename IntervalMapType::size_type;
    using difference_type = typename IntervalMapType::difference_type;
    using key_compare = typename IntervalMapType::key_compare;
    using reference = typename IntervalMapType::reference;
    using const_reference = typename IntervalMapType::const_reference;
    using iterator = typename IntervalMapType::iterator;
    using const_iterator = typename IntervalMapType::const_iterator;

    // Add element v to the range [low, high).  If the range is empty
    // then nothing is added
    void AddElement(K low, K high, V v);

    // Add element v extracting range from v
    void AddElement(V v)
    {
        auto r = ExtractRange{}(v);
        AddElement(r.first, r.second, v);
    }

    // Lookup elements for key k returning the vector of elements, returns
    // an empty vector if there if and interval containing k was never inserted
    std::vector<V> Lookup(K k) const
    {
        auto i = m.upper_bound(k);
        if (i != m.begin())  {
            --i;
            if (!(k < i->second.low) && (k < i->second.high))  {
                // contained by previous intervals bounds, return elements
                return i->second.elements;
            }
        }
        return {};  // not found
    }

    const_iterator cbegin() const noexcept      { return m.cbegin(); }
    const_iterator cend() const noexcept        { return m.cend(); }
    const_iterator begin() const noexcept       { return m.cbegin(); }
    const_iterator end() const noexcept         { return m.cend(); }

    friend std::ostream &operator<<(std::ostream &os, const IntervalMap &im)
    {
        for (auto const &i: im.m)  {
            auto const &iv = i.second;
            os << i.first << " " << iv << "\n";
        }
        return os;
    }
private:
    K lowest;                   // lowest range low added
    K highest;                  // highest range high added
    IntervalMapType m;

    // Add the intersection of [low, high) and the range [curInterval.begin, nextInterval.begin)
    K AddToIntervalAndGap(iterator curInterval, const_iterator nextInterval, K low, K high, const V &v);
};


template <typename K, typename V, typename ExtractRange>
void IntervalMap<K, V, ExtractRange>::AddElement(K low, K high, V v)
{
    if (!(low < high))  {
        // the interval to insert is empty, just return
        return;
    }
    auto isEmpty = m.empty();
    if (isEmpty)  {
        // no elements, just add new interval
        m.emplace(low, IntervalValueType{low, high, v});
        lowest = low;
        highest = high;
    }  else if (high < lowest)  {
        // before all intervals, just add new interval
        m.emplace_hint(m.begin(), low, IntervalValueType{low, high, v});
        lowest = low;
    }  else if (highest < low)  {
        // after all intervals, just add new interval
        m.emplace_hint(m.end(), low, IntervalValueType{low, high, v});
        highest = high;
    }  else  {
        // get the lowest interval whose low value is greater than low
        auto nextInterval = m.upper_bound(low);
        auto curInterval = nextInterval;
        if (curInterval != m.begin())  {
            // get the previous interval if there is one, (if there is not
            // one, curInterval, nextInterval, and m.begin() are equal
            // meaning insert in the gap before curInterval)
            --curInterval;
        }

        //  adjust lowest and highest (empty check is here so this will work
        //  without short circuit paths above
        if (isEmpty || low < lowest)  {
            lowest = low;
        }
        if (isEmpty || highest < high)  {
            highest = high;
        }

        // add/update curInterval and gap after, doing this for subsequent
        // intervals and gaps to insert v into [low, high)
        while (low < high)  {
            // update the interval map between the start of curInterval and
            // nextInterval; splitting curInterval and into the gap between
            // the end of curInterval and the beginning of nextInterval, low
            // is adjusted to the end of the highest interval that v was
            // added
            low = AddToIntervalAndGap(curInterval, nextInterval, low, high, v);

            // get the next set of intervals (
            curInterval = nextInterval;
            if (nextInterval != m.end())  {
                ++nextInterval;
            }
        }
    }
}


// AddToIntervalAndGap
//
// Add interval {[low, high) -> v} that overlaps the intervalValue
// curInterval, and the (possibly empty) gap between curInterval and the
// beginning of nextInterval.  If curInterval == nextInterval this
// indicates that it should be inserted in the (infinite) gap before
// curInterval (curInterval is the first/lowest interval).
//
// This function may split curInterval into up to two additional interval
// values and/or one new interval contained by the gap.
//
// This function returns the highest key that was added to the IntervalMap
// contained in the CurInterval and the subsequent gap.  If called with
// and empty interval or an interval that does not overlap, return the
// value of high and do not modify the IntervalMap.  This will not happen
// internally, but better safe.

template <typename K, typename V, typename ExtractRange>
K IntervalMap<K, V, ExtractRange>::AddToIntervalAndGap(iterator curInterval, const_iterator nextInterval, K low, K high, const V &v)
{
    auto curLow = low;      // these values place everything in the gap
    auto curHigh = low;     //
    auto gapHigh = high;    //
    if (curInterval != nextInterval)  {
        // there is an interval and gap to insert v into
        if (curInterval != m.end())  {
            // curInterval exists, get low and high
            // NOTE:; if curInterval == m.end(), then nextInterval would
            // have to also be m.end(), so this should always be true
            curLow = curInterval->first;
            curHigh = curInterval->second.high;
        }
        if (nextInterval != m.end())  {
            // nextInterval is not m.end(), so set the gap end to the start
            // of nextInterval
            gapHigh = nextInterval->first;
        }
    }  else if (curInterval != m.end())  {
        // curInterval == nextInterval && curInterval != m.end() indicates
        // that v is before begin, so set the gapHigh to curInterval's low
        //
        // curInterval == nextInterval && curInterval == m.end() can only
        // happen if the map is empty, then the initial values are correct
        gapHigh = curInterval->first;
        //assert(curInterval == m.begin());
    }

    if (!(low < high) || high < curLow || gapHigh < low)  {
        // the interval to insert is empty, or the interval to insert
        // is disjoint with the interval of currentInterval and subsequent
        // gap
        return high;
    }

    if (low < curHigh)  {
        // the new range covers (part of) curInterval
        if (curLow < low)  {
            // new range starts after the curInterval.low, so split
            // curInterval at low:
            // 1) adjust curInterval.high to split (low)
            // 2) add new interval [low, min(high, curHigh)) with copy of
            //    curInterval's elements
            // 3) assign this to curInterval so v is added to the new
            //    interval
            // 4) adjust low
            auto &iv = curInterval->second;
            iv.high = low;
            auto newHigh = std::min(high, curHigh);
            curInterval = m.emplace_hint(nextInterval, low,
                            IntervalValueType{low, newHigh, iv.elements});
            low = newHigh;
        }
        if (high < curHigh)  {
            // new range ends before curInterval.high, so split curInterval
            // at high:
            // 1) adjust curInterval.high to split (high)
            // 2) add new interval [high, curHigh) with copy of
            //    curInterval's elements
            // 3) adjust low
            auto &iv = curInterval->second;
            iv.high = high;
            m.emplace_hint(nextInterval, high,
                            IntervalValueType{high, curHigh, iv.elements});
            low = high;
        }  else  {
            // new range goes to end of original curInterval, adjust low
            low = curHigh;
        }

        // add v to curInterval
        curInterval->second.elements.push_back(v);
    }

    auto newHigh = std::min(high, gapHigh);
    if (low < newHigh)  {
        // remaining interval covers (part of) the  gap:
        // 1) insert new interval with elements {v}
        // 2) adjust low
        m.emplace_hint(nextInterval, low, IntervalValueType{low, newHigh, v});
        low = newHigh;
    }
    return low;
}

}

#endif
