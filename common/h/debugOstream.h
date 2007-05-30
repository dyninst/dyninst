/*
 * Copyright (c) 1996-2007 Barton P. Miller
 * 
 * We provide the Paradyn Parallel Performance Tools (below
 * described as "Paradyn") on an AS IS basis, and do not warrant its
 * validity or performance.  We reserve the right to update, modify,
 * or discontinue this software at any time.  We shall have no
 * obligation to supply such updates or modifications or any other
 * form of support to you.
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */


// debugOstream.h

#ifndef _DEBUG_OSTREAM_H_
#define _DEBUG_OSTREAM_H_

#if !defined(DLLEXPORT)
#if defined (_MSC_VER)
/* If we're on Windows, we need to explicetely export these functions: */
	#if defined(DLL_BUILD)
		#define DLLEXPORT __declspec(dllexport)
	#else
		#define DLLEXPORT __declspec(dllimport)	
	#endif
#else
	#define DLLEXPORT 
#endif
#endif

#include "common/h/std_namesp.h"
#include "common/h/Types.h"

#ifdef NEW_TIME_TYPES
class timeUnit;
class timeBase;
class timeLength;
class timeStamp;
class relTimeStamp;
#endif

class DLLEXPORT debug_ostream {
 private:
   ostream &actual_ostream; // probably cerr or cout
   bool on;

 public:
   debug_ostream(ostream &iActual, bool iOn) : actual_ostream(iActual) {on=iOn;}
  ~debug_ostream() {}
   bool isOn() { return on; }
   void turnOn()  { on = true;  }
   void turnOff() { on = false; }
   debug_ostream &operator<<(char c);
   debug_ostream &operator<<(unsigned char c);
   debug_ostream &operator<<(short s);
   debug_ostream &operator<<(unsigned short s);
   debug_ostream &operator<<(int i);
   debug_ostream &operator<<(unsigned i);
#if !defined(TYPE64BIT)
   // int64_t is a long which causes multiply defined functions if this
   // is included
   debug_ostream &operator<<(long l);
   debug_ostream &operator<<(unsigned long l);
#endif
   debug_ostream &operator<<(int64_t l);
   debug_ostream &operator<<(uint64_t l);

   debug_ostream &operator<<(const char *str);
   debug_ostream &operator<<(const unsigned char *str);

   debug_ostream &operator<<(const void *ptr);

   debug_ostream &operator<<(float f);
   debug_ostream &operator<<(double d);

   debug_ostream& operator<<( ostream& (*f)(ostream&) );
#ifdef NEW_TIME_TYPES
   debug_ostream& operator<<(timeUnit   &tu);
   debug_ostream& operator<<(timeBase   tb);
   debug_ostream& operator<<(timeLength tl);
   debug_ostream& operator<<(timeStamp  ts);
   debug_ostream& operator<<(relTimeStamp  ts);
#endif
};

#endif

