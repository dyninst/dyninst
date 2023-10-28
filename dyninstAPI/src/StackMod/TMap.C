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

#include "debug.h"
#include "registers/MachRegister.h"

#include "StackLocation.h"
#include "TMap.h"

using namespace Dyninst;

void TMap::update(StackLocation* loc, int delta)
{
    for (auto iter = _map.begin(); iter != _map.end(); ++iter) {
        StackLocation* curSrc = (*iter).first;
        if (curSrc->off() == loc->off()) {
            if (curSrc->isRegisterHeight() == loc->isRegisterHeight()) {
                if (curSrc->reg() == loc->reg()) {
                    StackLocation* curDest = (*iter).second;
                    curDest->setOff(curDest->off() + delta);
                    stackmods_printf("\t\t\t Updating tMap: %s -> %s\n", loc->format().c_str(), curDest->format().c_str());
                    return;
                }
            }
        }
    }

   StackLocation* tmp;
   if (loc->isRegisterHeight()) {
       tmp = new StackLocation(loc->off() + delta, loc->type(), loc->reg());
   } else {
       tmp = new StackLocation(loc->off() + delta, loc->size(), loc->type(), loc->isRegisterHeight(), loc->valid());

   }
    assert(tmp);
    _map.insert(std::make_pair(loc, tmp));
    stackmods_printf("\t\t\t Adding to tMap: %s -> %s\n", loc->format().c_str(), tmp->format().c_str());
}

void TMap::update(StackLocation* loc, MachRegister reg, int size)
{
    auto found = _map.find(loc);
    if (found == _map.end()) {
        StackLocation* tmp = new StackLocation(reg, size);
        assert(tmp);
        _map.insert(std::make_pair(loc, tmp));
    } else {
        if (_map.at(loc)->isStackMemory() || _map.at(loc)->reg() != reg) {
            assert(0 && "Modifications conflict");
        }
        _map.at(loc)->setSize(size);
    }
}


StackLocation* TMap::findInMap(StackLocation* src)
{
    StackLocation* dest = NULL;
    for (auto iter = _map.begin(); iter != _map.end(); ++iter) {
        if ((*iter).first == src) {
            dest = (*iter).second;
            break;
        }
        if (!((*iter).first)->isNull() && (*iter).first->off() > src->off()) {
            break;
        }
    }
    return dest;
}

void TMap::print() const
{
    for(auto iter = _map.rbegin(); iter != _map.rend(); ++iter) {
        StackLocation* src = (*iter).first;
        StackLocation* dest = (*iter).second;
        stackmods_printf("\t %s -> %s\n", src->format().c_str(), dest->format().c_str());
    }
}

