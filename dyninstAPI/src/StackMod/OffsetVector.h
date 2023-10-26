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

#ifndef _OFFSETVECTOR_H_
#define _OFFSETVECTOR_H_

#include <map>
#include <set>
#include <utility>
#include "registers/MachRegister.h"
#include "common/src/IntervalTree.h"
#include "stackanalysis.h"

#include "StackLocation.h"

using namespace Dyninst;

class OffsetVector
{
    public:
        typedef StackAnalysis::Height K;
        typedef StackLocation* V;

        OffsetVector() {}

        IntervalTree<K,V> stack() { return _stack; }
        std::map<MachRegister, IntervalTree<K,V> > definedRegs() { return _definedRegs; }

        void insert(K lb, K ub, V v, bool isRegHeight);

        void update(K lb, K newUB);
        void update(K lb, V newVal);

        bool erase(K key, bool stack=true);
        bool erase(MachRegister r, V val);

        bool find(K key, V& val) const;
        bool find(K key, MachRegister reg, V& val) const;
        bool find(K key, K& lb, K& ub, V& val) const;

        void addSkip(MachRegister, StackAnalysis::Height, Address, Address);
        bool isSkip(MachRegister, StackAnalysis::Height, Address, Address) const;
        void printSkips() const;

        void print() const;

    private:
        IntervalTree<K,V> _stack;
        std::map<MachRegister, IntervalTree<K,V> > _definedRegs;

        std::map<MachRegister, std::map<StackAnalysis::Height, std::set<std::pair<Address,Address> >* >* > _skipRegPCs;

};

#endif
