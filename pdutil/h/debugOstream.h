/*
 * Copyright (c) 1996 Barton P. Miller
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

// debugOstream.h

#ifndef _DEBUG_OSTREAM_H_
#define _DEBUG_OSTREAM_H_

#include <iostream.h>

class debug_ostream {
 private:
   ostream &actual_ostream; // probably cerr or cout
   bool on;

 public:
   debug_ostream(ostream &iActual, bool iOn) : actual_ostream(iActual) {on=iOn;}
  ~debug_ostream() {}

   debug_ostream &operator<<(char c);
   debug_ostream &operator<<(unsigned char c);
   debug_ostream &operator<<(short s);
   debug_ostream &operator<<(unsigned short s);
   debug_ostream &operator<<(int i);
   debug_ostream &operator<<(unsigned i);
   debug_ostream &operator<<(long l);
   debug_ostream &operator<<(unsigned long l);
#if !defined(i386_unknown_nt4_0)
   // long long is not supported on all platforms
   debug_ostream &operator<<(long long l);
   debug_ostream &operator<<(unsigned long long l);
#endif

   debug_ostream &operator<<(const char *str);
   debug_ostream &operator<<(const unsigned char *str);

   debug_ostream &operator<<(const void *ptr);

   debug_ostream &operator<<(float f);
   debug_ostream &operator<<(double d);

#if !defined(i386_unknown_nt4_0) && \
    !(defined(mips_sgi_irix6_4) && defined(USES_NATIVE_CC))
   // TODO: fix for native compiler
   // This code may be g++ specific!
   debug_ostream &operator<<(__omanip);
      // for things like endl, which are actually functions (!) (see iostream.h
      // in g++-include)
#endif

   debug_ostream &flush();

};

#endif
