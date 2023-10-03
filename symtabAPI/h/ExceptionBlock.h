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

#ifndef __EXCEPTIONBLOCK_H__
#define __EXCEPTIONBLOCK_H__

#include <iosfwd>

#include "util.h"
#include "dyntypes.h"
#include "Annotatable.h"


namespace Dyninst {

namespace SymtabAPI {


/**
 * Used to represent something like a C++ try/catch block.
 * Currently only used on Linux
 **/

class SYMTAB_EXPORT ExceptionBlock : public AnnotatableSparse {
  // Accessors provide consistent access to the *original* offsets.
  // We allow this to be updated (e.g. to account for relocated code
   public:
      ExceptionBlock(Offset tStart, unsigned tSize, Offset cStart);
      ExceptionBlock(Offset cStart);
      SYMTAB_EXPORT ExceptionBlock(const ExceptionBlock &eb) = default;
      SYMTAB_EXPORT ~ExceptionBlock() = default;
      SYMTAB_EXPORT ExceptionBlock() = default;
      SYMTAB_EXPORT ExceptionBlock& operator=(const ExceptionBlock &eb) = default;

      bool hasTry() const;
      Offset tryStart() const;
      Offset tryEnd() const;
      Offset trySize() const;
      Offset catchStart() const;
      bool contains(Offset a) const;
      void setTryStart(Offset ts)
      {
	tryStart_ptr = ts;
      }
      void setTryEnd(Offset te)
      {
	tryEnd_ptr = te;
      }

      void setCatchStart(Offset cs)
      {
	catchStart_ptr = cs;
      }

      void setFdeStart(Offset fs)
      {
	fdeStart_ptr = fs;
      }

      void setFdeEnd(Offset fe)
      {
	fdeEnd_ptr = fe;
      }


      friend SYMTAB_EXPORT std::ostream &operator<<(std::ostream &os, const ExceptionBlock &q);
   private:
      Offset tryStart_{};
      unsigned trySize_{};
      Offset catchStart_{};
      bool hasTry_{};
      Offset tryStart_ptr{};
      Offset tryEnd_ptr{};
      Offset catchStart_ptr{};
      Offset fdeStart_ptr{};
      Offset fdeEnd_ptr{};
};

SYMTAB_EXPORT  std::ostream &operator<<(std::ostream &os, const ExceptionBlock &q);

}//namespace SymtabAPI

}//namespace Dyninst
#endif
