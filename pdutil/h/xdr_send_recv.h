/*
 * Copyright (c) 1996-1999 Barton P. Miller
 * 
 * We provide the Paradyn Parallel Performance Tools (below
 * described as Paradyn") on an AS IS basis, and do not warrant its
 * validity or performance.  We reserve the right to update, modify,
 * or discontinue this software at any time.  We shall have no
 * obligation to supply such updates or modifications or any other
 * form of support to you.
 * 
 * This license is for research uses.  For such uses, there is no
 * charge. We define "research use" to mean you may freely use it
 * inside your organization for whatever purposes you see fit. But you
 * may not re-distribute Paradyn or parts of Paradyn, in any form
 * source or binary (including derivatives), electronic or otherwise,
 * to any other organization or entity without our permission.
 * 
 * (for other uses, please contact us at paradyn@cs.wisc.edu)
 * 
 * All warranties, including without limitation, any warranty of
 * merchantability or fitness for a particular purpose, are hereby
 * excluded.
 * 
 * By your use of Paradyn, you understand and agree that we (or any
 * other person or entity with proprietary rights in Paradyn) are
 * under no obligation to provide either maintenance services,
 * update services, notices of latent defects, or correction of
 * defects for Paradyn.
 * 
 * Even if advised of the possibility of such damages, under no
 * circumstances shall we (or any other person or entity with
 * proprietary rights in the software licensed hereunder) be liable
 * to you or any third party for direct, indirect, or consequential
 * damages of any character regardless of type of action, including,
 * without limitation, loss of profits, loss of use, loss of good
 * will, or computer failure or malfunction.  You agree to indemnify
 * us (and any other person or entity with proprietary rights in the
 * software licensed hereunder) for any and all liability it may
 * incur to third parties resulting from your use of Paradyn.
 */

#ifndef _XDR_SEND_RECV_H_
#define _XDR_SEND_RECV_H_

//#include <rpc/xdr.h> // XDR
#include <sys/types.h> // uint32_t
#include "common/h/triple.h"
#include "common/h/String.h"
#include "common/h/Vector.h"
#include "common/h/vectorSet.h"
#include "ByteArray.h"
#include "common/h/Pair.h"

class xdr_send_fail {};
class xdr_recv_fail {};

// New style XDR send/recv routines for basic types
bool P_xdr_send(XDR *xdr, const bool &);
bool P_xdr_send(XDR *xdr, const short &);
bool P_xdr_send(XDR *xdr, const unsigned short &);

#if defined(i386_unknown_nt4_0)
bool P_xdr_send(XDR *xdr, const int &); 
bool P_xdr_send(XDR *xdr, const unsigned &); 
bool P_xdr_send(XDR *xdr, const long &);
bool P_xdr_send(XDR *xdr, const unsigned long &);
//bool P_xdr_send(XDR *xdr, const long long &);
//bool P_xdr_send(XDR *xdr, const unsigned long long &);
#endif

bool P_xdr_send(XDR *xdr, const uint32_t &);
bool P_xdr_send(XDR *xdr, const int32_t &);
bool P_xdr_send(XDR *xdr, const uint64_t &);
bool P_xdr_send(XDR *xdr, const int64_t &);
bool P_xdr_send(XDR *xdr, const float &);
bool P_xdr_send(XDR *xdr, const double &);
bool P_xdr_send(XDR *xdr, const byteArray &);
bool P_xdr_send(XDR *xdr, const string &);

bool P_xdr_recv(XDR *xdr, bool &);
bool P_xdr_recv(XDR *xdr, short &);
bool P_xdr_recv(XDR *xdr, unsigned short &);

#if defined(i386_unknown_nt4_0)
bool P_xdr_recv(XDR *xdr, int &i);
bool P_xdr_recv(XDR *xdr, unsigned &u);
bool P_xdr_recv(XDR *xdr, long &l);
bool P_xdr_recv(XDR *xdr, unsigned long &ul);
//  bool P_xdr_recv(XDR *xdr, long long &ll);
//  bool P_xdr_recv(XDR *xdr, unsigned long long &ull);
#endif

bool P_xdr_recv(XDR *xdr, uint32_t &);
bool P_xdr_recv(XDR *xdr, int32_t &);
bool P_xdr_recv(XDR *xdr, uint64_t &);
bool P_xdr_recv(XDR *xdr, int64_t &);
bool P_xdr_recv(XDR *xdr, float &);
bool P_xdr_recv(XDR *xdr, double &);
bool P_xdr_recv(XDR *xdr, byteArray &);
bool P_xdr_recv(XDR *xdr, string &);

bool read1_bool(XDR *xdr);
#ifndef i386_unknown_nt4_0
uint16_t read1_uint16(XDR *xdr);
#endif
uint32_t read1_uint32(XDR *xdr);
unsigned read1_unsigned(XDR *xdr);
string read1_string(XDR *xdr);

//--------------------

// New style XDR send/recv for pointers to objects

template <class T>
bool P_xdr_send(XDR *xdr, T *data) {
   assert(xdr->x_op == XDR_ENCODE);
   bool is_null = (data == NULL);
   if (!P_xdr_send(xdr, is_null)) return false;
   if (!data) return true;
   return P_xdr_send(xdr, *data);
}

template <class T>
bool P_xdr_recv(XDR *xdr, T *&data) {
   assert(xdr->x_op == XDR_DECODE);
   bool is_null;
   if (!P_xdr_recv(xdr, is_null)) return false;
   if (is_null) { data = NULL; return true; }
   data = (T *)malloc(sizeof(T));
   return P_xdr_recv(xdr, *data);
}


//--------------------
// New style XDR send/recv for pairs and triples

template <class P1, class P2>
bool P_xdr_send(XDR *xdr, const pair<P1, P2> &p) {
   return P_xdr_send(xdr, p.first) &&
          P_xdr_send(xdr, p.second);
}

template <class P1, class P2>
bool P_xdr_recv(XDR *xdr, pair<P1, P2> &p) {
   return P_xdr_recv(xdr, p.first) &&
          P_xdr_recv(xdr, p.second);
}

template <class P1, class P2, class P3>
bool P_xdr_send(XDR *xdr, const triple<P1, P2, P3> &p) {
   return P_xdr_send(xdr, p.first) &&
          P_xdr_send(xdr, p.second) &&
          P_xdr_send(xdr, p.third);
}

template <class P1, class P2, class P3>
bool P_xdr_recv(XDR *xdr, triple<P1, P2, P3> &p) {
   return P_xdr_recv(xdr, p.first) &&
          P_xdr_recv(xdr, p.second) &&
          P_xdr_recv(xdr, p.third);
}

// --------------------
// New style XDR send routines for vectors

template <class T, class A>
bool P_xdr_send_common(XDR *xdr, const vector<T, A> &vec,
                       bool (*writerfn)(XDR *, const T)) {
   assert(xdr->x_op == XDR_ENCODE);
   
   const uint32_t nelems = vec.size(); // uint32_t helps portability
   if (!P_xdr_send(xdr, nelems))
      return false;

   if (nelems == 0)
      return true;
   
   vector<T, A>::const_iterator finish = vec.end();
   for (vector<T, A>::const_iterator iter = vec.begin(); iter != finish; ++iter) {
      const T item = *iter;

      if (!writerfn(xdr, item))
         return false;
   }

   return true;
}

template <class T>
bool writerfn_method(XDR *xdr, const T item) {
   return item.send(xdr);
}
template <class T>
bool writerfn_noMethod(XDR *xdr, const T item) {
   return P_xdr_send(xdr, item);
}

template <class T, class A>
bool P_xdr_send_method(XDR *xdr, const vector<T, A> &vec) {
   return P_xdr_send_common(xdr, vec, writerfn_method<T>);
}
template <class T, class A>
bool P_xdr_send(XDR *xdr, const vector<T, A> &vec) {
   return P_xdr_send_common(xdr, vec, writerfn_noMethod<T>);
}

template <class T, class A>
bool P_xdr_send_pointers(XDR *xdr, const vector<T*, A> &vec) {
   assert(xdr->x_op == XDR_ENCODE);
   
   const uint32_t nelems = vec.size(); // uint32_t helps portability
   if (!P_xdr_send(xdr, nelems))
      return false;

   if (nelems == 0)
      return true;
   
   vector<T*, A>::const_iterator finish = vec.end();
   for (vector<T*, A>::const_iterator iter = vec.begin(); iter != finish; ++iter) {
      const T *item = *iter;
      if (!P_xdr_send(xdr, *item))
         return false;
   }

   return true;
}

// --------------------

// New style XDR recv routines for vectors

template <class T, class A>
bool P_xdr_recv(XDR *xdr, vector<T, A> &vec) {
   assert(xdr->x_op == XDR_DECODE);

   uint32_t nelems;
   if (!P_xdr_recv(xdr, nelems))
      return false;

   // Call default ctor for "vec" (needed before reserve_for_inplace_construction()
   // can be invoked).  Fortunately, vector's default ctor is always very quick
   // (doesn't allocate anything).
   (void)new((void*)&vec)vector<T, A>();

   // assert that the default ctor did exactly what we expected it to do:
   assert(vec.size() == 0);
   assert(vec.capacity() == 0);
   assert(vec.begin() == NULL);

   if (nelems == 0)
      // don't call reserve_for_inplace_construction() with an argument of zero.
      return true;
   
   vector<T, A>::iterator iter = vec.reserve_for_inplace_construction(nelems);
   vector<T, A>::iterator finish = vec.end();

   // Reminder: Upon failure, we must still expect the dtor for vector<T, A> to
   // get called, so leave the object in a valid state no matter what.

   // OK for "do" instead of "while"; the check for nelems==0 above ensures that
   // nelems > 0 here.
   do {
      T *item = iter;
      if (!P_xdr_recv(xdr, *item))
         return false;
   } while (++iter != finish);

   return true;
}

template <class T, class A>
bool P_xdr_recv_ctor(XDR *xdr, vector<T, A> &vec) {
   assert(xdr->x_op == XDR_DECODE);

   uint32_t nelems;
   if (!P_xdr_recv(xdr, nelems))
      return false;

   // Call default ctor for "vec" (needed before reserve_for_inplace_construction()
   // can be invoked).  Fortunately, vector's default ctor is always very quick
   // (doesn't allocate anything).
   (void)new((void*)&vec)vector<T, A>();

   // assert that the default ctor did exactly what we expected it to do:
   assert(vec.size() == 0);
   assert(vec.capacity() == 0);
   assert(vec.begin() == NULL);

   if (nelems == 0)
      // don't call reserve_for_inplace_construction() with an argument of zero.
      return true;
   
   vector<T, A>::iterator iter = vec.reserve_for_inplace_construction(nelems);
   vector<T, A>::iterator finish = vec.end();

   // Reminder: Upon failure, we must still expect the dtor for vector<T, A> to
   // get called, so leave the object in a valid state no matter what.

   // OK for "do" instead of "while"; the check for nelems==0 above ensures that
   // nelems > 0 here.
   do {
      T *item = iter;
      (void)new((void*)item)T(xdr); // the only line that differs from above fn
   } while (++iter != finish);

   return true;
}

template <class T, class A>
bool P_xdr_recv_pointers(XDR *xdr, vector<T*, A> &vec) {
   assert(xdr->x_op == XDR_DECODE);

   uint32_t nelems;
   if (!P_xdr_recv(xdr, nelems))
      return false;

   // Call default ctor for "vec" (needed before reserve_for_inplace_construction()
   // can be invoked).  Fortunately, vector's default ctor is always very quick
   // (doesn't allocate anything).
   (void)new((void*)&vec)vector<T*, A>();

   // assert that the default ctor did exactly what we expected it to do:
   assert(vec.size() == 0);
   assert(vec.capacity() == 0);
   assert(vec.begin() == NULL);

   if (nelems == 0)
      // don't call reserve_for_inplace_construction() with an argument of zero.
      return true;

   // No need to be too clever with reserve_for_inplace_construction() here; since
   // the vector is of pointers, element copying will be cheap, etc.
   vector<T*, A>::iterator iter = vec.reserve_for_inplace_construction(nelems);
   vector<T*, A>::iterator finish = vec.end();

   // Reminder: Upon failure, we must still expect the dtor for vector<T, A> to
   // get called, so leave the object in a valid state no matter what.

   // OK for "do" instead of "while"; the check for nelems==0 above ensures that
   // nelems > 0 here.
   do {
      T *newitem = new T(xdr);
      assert(newitem);
      
      *iter = newitem;
   } while (++iter != finish);

   return true;
}

#endif

