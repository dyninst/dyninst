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

#include "ExceptionBlock.h"


namespace Dyninst  {
namespace SymtabAPI  {


SYMTAB_EXPORT ExceptionBlock::ExceptionBlock(Offset tStart,
      unsigned tSize,
      Offset cStart)
: tryStart_(tStart), trySize_(tSize), catchStart_(cStart), hasTry_(true),
  tryStart_ptr(0), tryEnd_ptr(0), catchStart_ptr(0), fdeStart_ptr(0), fdeEnd_ptr(0)
{
}

   SYMTAB_EXPORT ExceptionBlock::ExceptionBlock(Offset cStart)
: tryStart_(0), trySize_(0), catchStart_(cStart), hasTry_(false),
  tryStart_ptr(0), tryEnd_ptr(0), catchStart_ptr(0), fdeStart_ptr(0), fdeEnd_ptr(0)
{
}

SYMTAB_EXPORT bool ExceptionBlock::hasTry() const
{
   return hasTry_;
}

SYMTAB_EXPORT Offset ExceptionBlock::tryStart() const
{
   return tryStart_;
}

SYMTAB_EXPORT Offset ExceptionBlock::tryEnd() const
{
   return tryStart_ + trySize_;
}

SYMTAB_EXPORT Offset ExceptionBlock::trySize() const
{
   return trySize_;
}

SYMTAB_EXPORT bool ExceptionBlock::contains(Offset a) const
{
   return (a >= tryStart_ && a < tryStart_ + trySize_);
}


}
}
