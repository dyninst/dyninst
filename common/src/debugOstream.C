/*
 * Copyright (c) 1996-2004 Barton P. Miller
 * 
 * We provide the Paradyn Parallel Performance Tools (below
 * described as "Paradyn") on an AS IS basis, and do not warrant its
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

// debugOstream.C

#include "common/h/debugOstream.h"
#include "common/h/Time.h"
#include "common/h/int64iostream.h"

debug_ostream &debug_ostream::operator<<(char c) {
   if (on)
      actual_ostream << c;
   return *this;
}

debug_ostream &debug_ostream::operator<<(unsigned char c) {
   if (on)
      actual_ostream << c;
   return *this;
}

debug_ostream &debug_ostream::operator<<(short s) {
   if (on)
      actual_ostream << s;
   return *this;
}

debug_ostream &debug_ostream::operator<<(unsigned short s) {
   if (on)
      actual_ostream << s;
   return *this;
}

debug_ostream &debug_ostream::operator<<(int i) {
   if (on)
      actual_ostream << i;
   return *this;
}

debug_ostream &debug_ostream::operator<<(unsigned i) {
   if (on)
      actual_ostream << i;
   return *this;
}

#if !defined(mips_sgi_irix6_4) \
 && !defined(alpha_dec_osf4_0) \
 && !defined(x86_64_unknown_linux2_4) \
 && !defined(ia64_unknown_linux2_4)
debug_ostream &debug_ostream::operator<<(long l) {
   if (on)
      actual_ostream << l;
   return *this;
}

debug_ostream &debug_ostream::operator<<(unsigned long l) {
   if (on)
      actual_ostream << l;
   return *this;
}
#endif

debug_ostream &debug_ostream::operator<<(int64_t ll) {
   if (on)
      actual_ostream << ll;
   return *this;
}

debug_ostream &debug_ostream::operator<<(uint64_t ull) {
   if (on)
      actual_ostream << ull;
   return *this;
}

debug_ostream &debug_ostream::operator<<(const char *str) {
   if (on)
      actual_ostream << str;
   return *this;
}

debug_ostream &debug_ostream::operator<<(const unsigned char *str) {
   if (on)
      actual_ostream << str;
   return *this;
}

debug_ostream &debug_ostream::operator<<(const void *ptr) {
   if (on)
      actual_ostream << ptr;
   return *this;
}

debug_ostream &debug_ostream::operator<<(float f) {
   if (on)
      actual_ostream << f;
   return *this;
}

debug_ostream &debug_ostream::operator<<(double d) {
   if (on)
      actual_ostream << d;
   return *this;
}

debug_ostream& debug_ostream::operator<<( ostream& (*f)(ostream&) )
{
    if( on )
    {
        actual_ostream << f;
    }
    return *this;
}

#ifdef NEW_TIME_TYPES
debug_ostream& debug_ostream::operator<<(timeUnit &tu) {
  if(on)  actual_ostream << tu;
  return *this;
}
debug_ostream& debug_ostream::operator<<(timeBase tb) {
  if(on)  actual_ostream << tb;
  return *this;
}
debug_ostream& debug_ostream::operator<<(timeLength tl) {
  if(on)  actual_ostream << tl;
  return *this;
}
debug_ostream& debug_ostream::operator<<(timeStamp ts) {
  if(on)  actual_ostream << ts;
  return *this;
}
debug_ostream& debug_ostream::operator<<(relTimeStamp rts) {
  if(on)  actual_ostream << rts;
  return *this;
}
#endif
