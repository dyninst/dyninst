/*
 * Copyright (c) 1996-2011 Barton P. Miller
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */
#if !defined SYM_READER_H_
#define SYM_READER_H_

#include "dyntypes.h"
#include <string>

namespace Dyninst
{

class SymbolReaderFactory;

/**
 * Symbol_t is an anonymous struct that any SymReader can use for a symbol 
 * handle.  Some symbol readers may not want to store the objects behind a 
 * 'void*' on the heap, so we're making Symbol_t big enough that it could 
 * act as a full symbol handle.  Or a SymReader could just choose fill in one
 * of the void pointers as a handle to a heap object, if it's comfortable
 * doing so.
 **/
typedef struct {
   void *v1;
   void *v2;
   int i1;
   int i2;
} Symbol_t;

typedef struct {
   void *v1;
   void *v2;
   int i1;
   int i2;
} Section_t;

typedef struct {
   Dyninst::Offset file_offset;
   Dyninst::Address mem_addr;
   size_t file_size;
   size_t mem_size;
   int type;
   int perms;
} SymRegion;

/**
 * This may seem like a clunky interface in places, but it was designed such 
 * that the underlying implementation could be made re-enterant safe (so it 
 * could be called from a signal handler).
 **/
class SymReader
{
 protected:
   SymReader() {};
   virtual ~SymReader() {};
 public:
   virtual Symbol_t getSymbolByName(std::string symname) = 0;
   virtual Symbol_t getContainingSymbol(Dyninst::Offset offset) = 0;
   virtual std::string getInterpreterName() = 0;
   virtual unsigned getAddressWidth() = 0;
   
   virtual unsigned numRegions() = 0;
   virtual bool getRegion(unsigned num, SymRegion &reg) = 0; 

   virtual Dyninst::Offset getSymbolOffset(const Symbol_t &sym) = 0;
   virtual std::string getSymbolName(const Symbol_t &sym) = 0;
   virtual std::string getDemangledName(const Symbol_t &sym) = 0;
   virtual unsigned long getSymbolSize(const Symbol_t &sym) = 0;
   virtual bool isValidSymbol(const Symbol_t &sym) = 0;

   virtual Section_t getSectionByName(std::string name) = 0;
   virtual Section_t getSectionByAddress(Dyninst::Address addr) = 0;
   virtual Dyninst::Address getSectionAddress(Section_t sec) = 0;
   virtual std::string getSectionName(Section_t sec) = 0;
   virtual bool isValidSection(Section_t sec) = 0;

   virtual Dyninst::Offset imageOffset() = 0;
   virtual Dyninst::Offset dataOffset() = 0;
};

class SymbolReaderFactory
{
 public:
   SymbolReaderFactory() {};
   virtual ~SymbolReaderFactory() {};
   virtual SymReader *openSymbolReader(std::string pathname) = 0;
   virtual SymReader *openSymbolReader(const char *buffer, unsigned long size) = 0;
   virtual bool closeSymbolReader(SymReader *sr) = 0;
};

}

extern "C" {
   Dyninst::SymbolReaderFactory *getSymReaderFactory();
}

#endif
