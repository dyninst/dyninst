// triple.h

#ifndef _TRIPLE_H_
#define _TRIPLE_H_

template <class T1, class T2, class T3>
struct triple {
   T1 first;
   T2 second;
   T3 third;
   
   triple(const T1 &ifirst, const T2 &isecond, const T3 &ithird) :
      first(ifirst), second(isecond), third(ithird) {
   }
   triple() : first(T1()), second(T2()), third(T3()) {
   }
};

template <class T1, class T2, class T3>
triple<T1, T2, T3>
make_triple(const T1 &ifirst, const T2 &isecond, const T3 &ithird) {
   return triple<T1, T2, T3>(ifirst, isecond, ithird);
}

#endif
