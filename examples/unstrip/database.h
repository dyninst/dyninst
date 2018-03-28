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

#ifndef __DATABASE_H__
#define __DATABASE_H__

#include <map>
#include <string>
#include <vector>

#include "util.h"
#include "types.h"
#include "semanticDescriptor.h"

using namespace std;
using namespace Dyninst;

class Database;
class Fingerprint;

class SyscallParamDatabase {
    friend class Database;
    
    public:
        SyscallParamDatabase() {}
        
        typedef map<string, vector<ParamType> >::iterator iterator;

        bool build(); 

        iterator find(string val) { return db.find(val); }
        iterator begin() { return db.begin(); }
        iterator end() { return db.end(); }
    
    private:
        map<string, vector<ParamType> > db;

};

class SyscallNumbersDatabase {
    friend class Database;
    
    public:
        SyscallNumbersDatabase() {}
        
        typedef map<int, string>::iterator iterator;

        bool build();

        iterator find(int val) { return db.find(val); }
        iterator begin() { return db.begin(); }
        iterator end() { return db.end(); }
    
    private: 
        map<int, string> db;
};

class DescriptorDatabase {
    friend class Database;
   
    public:
        DescriptorDatabase() {}
        
        DescriptorDatabase(SyscallParamDatabase * sp) : spDB(sp) {}
        
        typedef map<SemanticDescriptor, string>::iterator iterator;
        
        void insert(pair<SemanticDescriptor, string> val) { db.insert(val); }

        bool build();

        iterator begin() { return db.begin(); }
        iterator end() { return db.end(); }

    private:
        SemanticDescriptorElem process(char * _trapVector);
        
        map<SemanticDescriptor, string> db;
        SyscallParamDatabase * spDB;
};


class Database {
    friend class Fingerprint;
    
    public:
        Database() {};

        bool setup(SymtabAPI::Symtab * symtab, Mode mode, string relPath);

        DescriptorDatabase dDB; // descriptor database
        SyscallParamDatabase spDB; // syscall param database
        SyscallNumbersDatabase snDB; // syscall numbers database
        Address syscallTrampStore;
};

#endif
