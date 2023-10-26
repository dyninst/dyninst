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

// $Id: InstructionSource.h,v 1.2 2007/09/12 20:57:10 bernat Exp $

#ifndef INSTRUCTION_SOURCE_H
#define INSTRUCTION_SOURCE_H

#include "Architecture.h"
#include "dyntypes.h"

namespace Dyninst {

class PARSER_EXPORT InstructionSource
{
public:
    InstructionSource() {}
    virtual ~InstructionSource() {}
    virtual bool isValidAddress(const Address) const { return true; }
    virtual void* getPtrToInstruction(const Address) const = 0;
    virtual void* getPtrToData(const Address) const = 0;
    virtual unsigned int getAddressWidth() const = 0;
    virtual bool isCode(const Address) const = 0;
    virtual bool isData(const Address) const = 0;
    virtual bool isReadOnly(const Address) const = 0;
    virtual Address offset() const = 0;
    virtual Address length() const = 0;
    virtual Architecture getArch() const = 0;

    // The following routines have default, architecture-specific behavior
    virtual bool isAligned(const Address) const;
};

}


#endif // INSTRUCTION_SOURCE_H
