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

#include "OffsetVector.h"
#include "StackLocation.h"

using namespace Dyninst;

void OffsetVector::insert(OffsetVector::K lb,
        OffsetVector::K ub,
        OffsetVector::V v,
        bool isRegHeight)
{
    if(isRegHeight) {
        auto found = _definedRegs.find(v->reg());
        if (found != _definedRegs.end()) {
            found->second.insert(lb, ub, v);
        } else {
            IntervalTree<K,V> newset;
            newset.insert(lb, ub, v);
            _definedRegs.insert(std::make_pair(v->reg(), newset));
        }
    } else {
        _stack.insert(lb, ub, v);
    }
}

void OffsetVector::update(OffsetVector::K lb, OffsetVector::K newUB) {
    _stack.update(lb, newUB);
}

void OffsetVector::update(OffsetVector::K lb, OffsetVector::V newVal) {
    _stack.updateValue(lb, newVal);
}

bool OffsetVector::erase(OffsetVector::K key, bool isStack)
{
    if (isStack) {
        _stack.erase(key);
    } else {
        assert(0);
    }
    return true;
}

bool OffsetVector::erase(MachRegister r, OffsetVector::V val)
{
    auto found = _definedRegs.find(r);
    if (found != _definedRegs.end()) {
        found->second.erase(val->off());
    } else {
        return false;
    }
    return true;
}


bool OffsetVector::find(OffsetVector::K key, OffsetVector::V& val) const
{
    return _stack.find(key, val);
}

bool OffsetVector::find(OffsetVector::K key,
        MachRegister reg,
        OffsetVector::V& val) const
{
    auto found = _definedRegs.find(reg);
    if (found != _definedRegs.end()) {
        return found->second.find(key, val);
    } else {
        return false;
    }
}

bool OffsetVector::find(OffsetVector::K key,
        OffsetVector::K& lb,
        OffsetVector::K& ub,
        OffsetVector::V& val) const
{
    return _stack.find(key, lb, ub, val);
}

void OffsetVector::addSkip(MachRegister reg, StackAnalysis::Height height, Address blockAddr, Address addr)
{
    auto found = _skipRegPCs.find(reg);
    if (found == _skipRegPCs.end()) {
        std::map<StackAnalysis::Height, std::set<std::pair<Address,Address> >* >* newmap = new std::map<StackAnalysis::Height, std::set<std::pair<Address,Address> >* >();
        std::set<std::pair<Address,Address> >* s = new std::set<std::pair<Address,Address> >();
        s->insert(std::make_pair(blockAddr, addr));
        newmap->insert(std::make_pair(height, s));
        _skipRegPCs.insert(std::make_pair(reg, newmap));
    } else {
        std::map<StackAnalysis::Height, std::set<std::pair<Address, Address> >* >* m = found->second;
        auto found2 = m->find(height);
        if (found2 == m->end()) {
            std::set<std::pair<Address,Address> >* s = new std::set<std::pair<Address,Address> >();
            s->insert(std::make_pair(blockAddr, addr));
            m->insert(std::make_pair(height, s));
        } else {
            found2->second->insert(std::make_pair(blockAddr,addr));
        }
    }
}

bool OffsetVector::isSkip(MachRegister reg, StackAnalysis::Height height, Address blockAddr, Address addr) const
{
    auto found = _skipRegPCs.find(reg);
    if (found != _skipRegPCs.end()) {
        auto found2 = found->second->find(height);
        if (found2 != found->second->end()) {
            for (auto iter = found2->second->begin(); iter != found2->second->end(); ++iter) {
                Address b = iter->first;
                Address a = iter->second;
                if (blockAddr == b && addr >= a) {
                    return true;
                }
            }
        }
    }
    return false;
}

void OffsetVector::printSkips() const
{
    for (auto rIter = _skipRegPCs.begin(); rIter != _skipRegPCs.end(); ++rIter) {
        MachRegister reg = (*rIter).first;
        std::map<StackAnalysis::Height, std::set<std::pair<Address,Address> >* >* m = (*rIter).second;
        if (!m) continue;
        for (auto mIter = m->begin(); mIter != m->end(); ++mIter) {
            StackAnalysis::Height h = (*mIter).first;
            std::set<std::pair<Address,Address> >* s = (*mIter).second;
            if (!s) continue;
            for (auto sIter = s->begin(); sIter != s->end(); ++sIter) {
                stackmods_printf("SKIP: %s @ %ld @ block 0x%lx, addr 0x%lx\n", reg.name().c_str(), h.height(), (*sIter).first, (*sIter).second);
            }
        }
    }
}

void OffsetVector::print() const
{
    if (!_definedRegs.empty()) {
        stackmods_printf("\t Register heights:\n");
        for (auto iter = _definedRegs.begin(); iter != _definedRegs.end(); ++iter) {
            for (auto iter2 = (*iter).second.begin(); iter2 != (*iter).second.end(); ++iter2) {
                StackLocation* loc = (*iter2).second.second;
                stackmods_printf("\t\t %ld: %s\n", loc->off().height(), loc->format().c_str());
            }

        }
    }

    printSkips();

    if (!_stack.empty()) {
        stackmods_printf("\t Stack memory:\n");
        for (auto iter = _stack.begin(); iter != _stack.end(); ++iter) {
            StackLocation* loc = (*iter).second.second;
            stackmods_printf("\t\t [%ld, %ld): %s\n", (*iter).first.height(), (*iter).second.first.height(), loc->format().c_str());
        }
    }
}

