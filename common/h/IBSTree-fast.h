
#include "IBSTree.h"
#include <boost/multi_index_container.hpp>
#include <boost/multi_index/ordered_index.hpp>
#include <boost/multi_index/mem_fun.hpp>
#include <boost/multi_index/identity.hpp>

namespace Dyninst 
{
  template <typename ITYPE = interval<> >
  class IBSTree_fast 
  {
    typedef typename ITYPE::type interval_type;
    
    
    IBSTree<ITYPE> overlapping_intervals;
    typedef boost::multi_index_container<ITYPE*, 
    indexed_by<
    ordered_unique<const_mem_fun<ITYPE, interval_type, &ITYPE::high> >
    > > interval_set;
      
    //typedef std::set<ITYPE*, order_by_lower<ITYPE> > interval_set;
    interval_set unique_intervals;

  public:
    IBSTree_fast() 
    {
    }
    ~IBSTree_fast()
    {
    }
    int size() const 
    {
      return overlapping_intervals.size() + unique_intervals.size();
    }
    bool empty() const
    {
      return unique_intervals.empty() && overlapping_intervals.empty();
    }
    void insert(ITYPE*);
    void remove(ITYPE*);
    int find(interval_type, std::set<ITYPE*> &) const;
    int find(ITYPE* I, std::set<ITYPE*>&) const;
    void successor(interval_type X, std::set<ITYPE*>& ) const;
    ITYPE* successor(interval_type X) const;
    void clear();
      
  };
    
  template <class ITYPE>
  void IBSTree_fast<ITYPE>::insert(ITYPE* entry)
  {
    // find in overlapping first
    std::set<ITYPE*> dummy;
    if(overlapping_intervals.find(entry, dummy)) 
    {
      overlapping_intervals.insert(entry);
      return;
    }
    typename interval_set::iterator lower =
    unique_intervals.upper_bound(entry->low());
    // lower.high first >= entry.low
    if(lower != unique_intervals.end() && *lower == entry) return;
    auto upper = lower;
    while(upper != unique_intervals.end() &&
	  (*upper)->low() <= entry->high())
    {
      overlapping_intervals.insert(*upper);
      ++upper;      
    }
    if(upper != lower)
    {
      unique_intervals.erase(lower, upper);
      overlapping_intervals.insert(entry);
    }
    else
    {
      unique_intervals.insert(entry);
    }
  }
  template <class ITYPE>
  void IBSTree_fast<ITYPE>::remove(ITYPE* entry)
  {
    overlapping_intervals.remove(entry);
    auto found = unique_intervals.find(entry->high());
    if(found != unique_intervals.end() && *found == entry) unique_intervals.erase(found);
  }
  template<class ITYPE>
  int IBSTree_fast<ITYPE>::find(interval_type X, std::set<ITYPE*> &results) const
  {
    int num_old_results = results.size();
      
    int num_overlapping = overlapping_intervals.find(X, results);
    if(num_overlapping > 0) return num_overlapping;
      
    typename interval_set::const_iterator found_unique = unique_intervals.upper_bound(X);
    if(found_unique != unique_intervals.end())
    {
      if((*found_unique)->low() > X) return 0;
      results.insert(*found_unique);
    }
    return results.size() - num_old_results;
  }
  template <typename ITYPE>
  int IBSTree_fast<ITYPE>::find(ITYPE* I, std::set<ITYPE*>&results) const
  {
    int num_old_results = results.size();
    int num_overlapping = overlapping_intervals.find(I, results);
    if(num_overlapping) return num_overlapping;
    typename interval_set::const_iterator lb = unique_intervals.upper_bound(I->low());
    auto ub = lb;
    while(ub != unique_intervals.end() && (*ub)->low() < I->high())
    {
      results.insert(*ub);
    }
    return results.size() - num_old_results;
  }
  template<typename ITYPE>
  void IBSTree_fast<ITYPE>::successor(interval_type X, std::set<ITYPE*>& results) const
  {
    ITYPE* overlapping_ub = overlapping_intervals.successor(X);
      
    typename interval_set::const_iterator unique_ub = unique_intervals.upper_bound(X);
    if(overlapping_ub && (unique_ub != unique_intervals.end()))
    {
      results.insert(overlapping_ub->low() < (*unique_ub)->low() ? overlapping_ub : *unique_ub);
    }
    else if(overlapping_ub)
    {
      results.insert(overlapping_ub);
    }
    else if(unique_ub != unique_intervals.end())
    {
      results.insert(*unique_ub);
    }
  }
  template <typename ITYPE>
  ITYPE* IBSTree_fast<ITYPE>::successor(interval_type X) const
  {
    std::set<ITYPE*> tmp;
    successor(X, tmp);
    assert(tmp.size() <= 1);
    if(tmp.empty()) return NULL;
    return *tmp.begin();
      
  }
  template <typename ITYPE>
  void IBSTree_fast<ITYPE>::clear()
  {
    overlapping_intervals.clear();
    unique_intervals.clear();
  }
    
    
}
