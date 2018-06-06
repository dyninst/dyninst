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


#ifndef __FINGERPRINT_H__
#define __FINGERPRINT_H__

#include <map>
#include <stack>
#include <vector>

#include "database.h"

using namespace std;
using namespace Dyninst;

class Fingerprint {
    private:

        bool retrieveValue(ParseAPI::Function * f,
                trapLoc & tloc,
                Absloc & reg,
                long int & val,
                stack<ParseAPI::Function *> & callstack);

        bool retrieveValues(ParseAPI::Function * f,
                trapLoc & tloc,
                vector<AbsRegion> & regsToSliceFor,
                stack<ParseAPI::Function *> & callstack,
                vector<ParamType> & params,
                SemanticDescriptorElem & curTrap);

        void processFunc(ParseAPI::Function * f,
            SymtabAPI::Function * symFunc);

        void identify(ParseAPI::Function * f,
                SemanticDescriptor & sd,
                SymtabAPI::Function * symFunc);

        void learn(ParseAPI::Function * f, 
                SemanticDescriptor & sd);

        bool parse(ParseAPI::Function * f,
                stack<ParseAPI::Function *> & callstack,
                SemanticDescriptor & retSD);

        void buildMappings();

        Database db;
        Mode mode;
        string relPath; 
        map<ParseAPI::Function *, vector<trapLoc> > trapAddresses;
        set<pair<ParseAPI::Function *, trapLoc> > trapInfo;
        bool oneSymbol;
        bool verbose;

    public:
        Fingerprint(Database _db, Mode m, string _relPath, bool _oneSymbol, bool _verbose) : 
            db(_db), 
            mode(m), 
            relPath(_relPath), 
            oneSymbol(_oneSymbol),
            verbose (_verbose)  {}

        void addTrapInfo(ParseAPI::Function * f, trapLoc & t) {
            trapInfo.insert(make_pair(f,t));
        }
        
        void findMain(SymtabAPI::Symtab * symtab, 
                ParseAPI::SymtabCodeSource * sts,
                SymtabAPI::Module * defmod);

        void run(SymtabAPI::Symtab * symtab);
};

#endif
