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

#include <string>
#include <sstream>

#include "StackMod.h"

StackMod::MType StackMod::type() const
{
    return _type;
}

/* Insert */
Insert::Insert(int low, int high) :
    _low(low),
    _high(high)
{
    _order = StackMod::NEW;
    _type = StackMod::INSERT;
}

Insert::Insert(MOrder order, int low, int high) :
    _low(low),
    _high(high)
{
    _order = order;
    _type = StackMod::INSERT;
}

std::string Insert::format() const
{
    std::stringstream ret;
    ret << "INSERT [" << low() << ", " << high() << ")";
    return ret.str();
}

/* Remove */
Remove::Remove(int low, int high) :
    _low(low),
    _high(high)
{
    _order = StackMod::NEW;
    _type = StackMod::REMOVE;
}

Remove::Remove(MOrder order, int low, int high) :
    _low(low),
    _high(high)
{
    _order = order;
    _type = StackMod::REMOVE;
}

std::string Remove::format() const
{
    std::stringstream ret;
    ret << "REMOVE [" << low() << ", " << high() << ")";
    return ret.str();
}

/* Move */
Move::Move(int srcLow, int srcHigh, int destLow) :
    _srcLow(srcLow),
    _srcHigh(srcHigh),
    _destLow(destLow),
    _destHigh(destLow + (srcHigh - srcLow))
{
    _order = StackMod::NEW;
    _type = StackMod::MOVE;
}

std::string Move::format() const
{
    std::stringstream ret;
    ret << "MOVE [" << srcLow() << ", " << srcHigh() << ") to [" << destLow() <<  ", " << destHigh() << ")";
    return ret.str();
}

Canary::Canary()
{   
    _order = StackMod::NEW;
    _type = StackMod::CANARY;
    _failFunc = NULL;
}

Canary::Canary(BPatch_function* failFunc)
{   
    _order = StackMod::NEW;
    _type = StackMod::CANARY;
    _failFunc = failFunc;
}

std::string Canary::format() const 
{
    return "CANARY";
}

Randomize::Randomize(int seed)
{
    _isSeeded = true;
    _seed = seed;
    _order = StackMod::NEW;
    _type = StackMod::RANDOMIZE;
}

Randomize::Randomize()
{
    _isSeeded = false;
    _order = StackMod::NEW;
    _type = StackMod::RANDOMIZE;
}

std::string Randomize::format() const
{
    return "RANDOMIZE";
}
