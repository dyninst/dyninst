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


#ifndef __SEMANTIC_DESCRIPTOR_H__
#define __SEMANTIC_DESCRIPTOR_H__

#include <map>
#include <string>
#include <vector>

class Database;
class Matches;

using namespace std;
using namespace Dyninst;

class SemanticDescriptorElem {
    private:
        vector<void *> elem;

    public:
        SemanticDescriptorElem() {}
        
        void push_back(void * v) { elem.push_back(v); }
        
        int size() { return elem.size(); }

        void clear() { elem.clear(); }

        bool equals(SemanticDescriptorElem & e2, Database & db);
        
        bool operator<(const SemanticDescriptorElem & e2) const {
            if (elem.size() == 0) return true;
            if (e2.elem.size() == 0) return false;

            char * val1 = (char*)elem[0];
            char * val2 = (char*)(e2.elem[0]);

            int ret = strcmp(val1, val2);
            if (ret <= 0) return true;
            else return false;
        }

        void * operator[](const int i) const {
            return elem[i];
        }
};

class SemanticDescriptor {
    private:
        vector<SemanticDescriptorElem> _sd;

    public:
        SemanticDescriptor() {}

        typedef vector<SemanticDescriptorElem>::iterator iterator;
        
        void insert(SemanticDescriptorElem vec) { _sd.push_back(vec); }
        void sort() { stable_sort(_sd.begin(), _sd.end()); }
        string format(Database & _db);
        int size() { return _sd.size(); }
            
        iterator begin() { return _sd.begin(); }
        iterator end() { return _sd.end(); }


        bool operator<(const SemanticDescriptor & sd2) const {
            return _sd < sd2._sd;
        }

        int count(SemanticDescriptorElem & elem, Database & db);
        double coverage(SemanticDescriptor & sd2, Database & db);
        bool equals(SemanticDescriptor & sd2, Database & db);

        Matches find(Database & db);
        bool findExactMatches(Database & db, Matches & matches);
        void findClosestMatch(Database & db, Matches & matches);
        int closerMatch(SemanticDescriptor & a, SemanticDescriptor & b, Database & db);

};

#endif
