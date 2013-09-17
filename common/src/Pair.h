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

/************************************************************************
 * Pair.h: definition of pairs for dictionaries and sets.
************************************************************************/

#if defined(external_templates)
#pragma interface
#endif

#if !defined(_Pair_h_)
#define _Pair_h_

//#ifdef STL...
//#include <stl.h>
//#else

/************************************************************************
 * template<class T1, class T2> struct pdpair
************************************************************************/

// Note that pdpaired classes must provide operator== and operator<

template<class T1, class T2>
struct pdpair {
 public: //needed so nt build doesn't think members are private
  T1 first;
  T2 second;

  bool operator==(const pdpair<T1, T2>& p) {
    return (first == p.first) && (second == p.second); 
  }
  bool operator!=(const pdpair<T1, T2>& p) {
    return !((first == p.first) && (second == p.second));
  }
  /*
  bool operator<(const pdpair<T1, T2>& p) { 
    return (first < p.first) || (!(p.first < first) && second < p.second); 
  }
  bool operator>(const pdpair<T1, T2>& p) {
    return (p.first < first) || (!(first < p.first) && p.second < second);
  }
  */
  pdpair () : first(), second()                                    {}
  pdpair (const T1& k) : first(k), second(0)                       {}
  pdpair (const T1& k, const T2& v) : first(k), second(v)          {}
  pdpair(const pdpair<T1, T2>& p) : first(p.first), second(p.second) {}
};

// Return a T1 pair containing the min and max elements of a  vector of
// type T2<T1>. If the vector contains no elements a 0,0 pair is returned.
template <class T1, class T2>
pdpair<T1, T1> min_max_pdpair (const T2 & vect)
{
    if (vect.size() == 0) {
	T1 def = 0;
	return pdpair<T1,T1>(def, def);
    }

    if (vect.size() == 1) {
	return pdpair<T1,T1>(vect[0], vect[0]);
    }
    
    T1 min = vect[0];
    T1 max = vect[0];
  
    for (unsigned int i = 0; i < vect.size (); i++) {
	if (vect[i] < min)
	    min = vect[i];
	
	if (vect[i] > max)
	    max = vect[i];
    }
    
    return pdpair<T1,T1>(min, max);
}

//#endif

#endif /* !defined(_PDPair_h_) */
