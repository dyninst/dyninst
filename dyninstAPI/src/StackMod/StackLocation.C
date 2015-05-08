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

#include <sstream>

#include "StackAccess.h"
#include "StackLocation.h"

std::string printRanges(ValidPCRange* valid)
{
    std::stringstream ret;
    ret << "(";
    if (valid) {
        for (auto iter = valid->begin(); iter != valid->end(); ++iter) {
            ret << "[" << std::hex << (*iter).first << ", " << (*iter).second.first << "]";
        }
    } else {
        ret << "WHOLE FUNCTION";
    }
    ret << ")";
    return ret.str();
}

std::string StackLocation::format() const
{
    std::stringstream ret;

    ret << "[";
    if (isNull()) {
        ret << "NULL";
    } else {
        if (isStackMemory()) {
            if (isRegisterHeight()) {
                ret << _off << ", " << _reg.name();
            }
            else {
                ret << _off << ", size " << _size;
            }
        } else {
            ret << _reg.name() << ", size " << _size;
        }
        ret << ", is " << StackAccess::printStackAccessType(_type);
        ret << ", valid = " << printRanges(_valid);
    }
    ret << "]";

    return ret.str();
}

