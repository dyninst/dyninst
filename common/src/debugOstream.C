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

// debugOstream.C

#if 0
#include "common/src/debugOstream.h"
#include "common/src/Time.h"
#include "common/src/int64iostream.h"

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

#if !defined(TYPE64BIT)
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
#endif
