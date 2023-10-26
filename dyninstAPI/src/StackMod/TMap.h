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

#ifndef _TMAP_H_
#define _TMAP_H_

#include <map>
#include <utility>

#include "StackLocation.h"

using namespace Dyninst;

class TMap
{
    typedef std::map<StackLocation*, StackLocation*, less_StackLocation> mapping;
    public:
        TMap() {}

        std::pair<mapping::iterator,bool> insert(std::pair<StackLocation*,StackLocation*> p) {
                return _map.insert(p);
            }

        mapping::iterator find(StackLocation* l) { return _map.find(l); }
        StackLocation* at(StackLocation* l) { return _map.at(l); }

        mapping::iterator begin() { return _map.begin(); }
        mapping::iterator end() { return _map.end(); }
        mapping::reverse_iterator rbegin() { return _map.rbegin(); }
        mapping::reverse_iterator rend() { return _map.rend(); }

        void update(StackLocation* loc, int delta);
        void update(StackLocation* loc, MachRegister reg, int size);

        StackLocation* findInMap(StackLocation* src);

        void print() const;

    private:
        mapping _map;
};

#endif
