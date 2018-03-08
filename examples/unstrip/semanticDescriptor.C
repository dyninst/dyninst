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


#include <stdio.h>
#include <map>
#include <set>
#include <string>
#include <vector>

#include "util.h"
#include "types.h"
#include "semanticDescriptor.h"
#include "database.h"

using namespace std;

/*
 * Determine if two semantic descriptor elements are equal 
 */
bool SemanticDescriptorElem::equals(SemanticDescriptorElem & e2, Database & db) {
    char * curEAX = (char*)elem[0];
    string curEAXstring(curEAX);

    map<string, vector<ParamType> >::iterator iter = db.spDB.find(curEAXstring);
    vector<ParamType> params;
    if (iter != db.spDB.end())
        params = iter->second;
    else
        return true;

    for (unsigned i = 0; i < elem.size(); i++) {
        if (_i == params[i] || _p == params[i]) {
            if ((long int)elem[i] != (long int)e2[i])
                return false;
        } else if (_s == params[i]) {
            if (strcmp((char*)elem[i], (char*)e2[i]))
                return false;        
        } else if (_o == params[i]) {
            // Skip
        } else {
            if (elem[i] != e2[i]) return false;
        }
    }

    return true;     
}

/* 
 * Produce formatted string representation for semantic descriptor */
string SemanticDescriptor::format(Database & db)
{
    SemanticDescriptorElem curTrap;
    stringstream idstring;

    char * curEAX;

    map<string, vector<ParamType> >::iterator pIter;
    vector<ParamType> params;
    SemanticDescriptor::iterator tIter;
    for (tIter = begin(); tIter != end(); ++tIter) {
        curTrap = *tIter;
        curEAX = (char*)curTrap[0];
        string curEAXstring(curEAX);
        pIter = db.spDB.find(curEAXstring);
        params.clear();
        if (pIter != db.spDB.end()) {
            params = pIter->second;
        }
        else {
            params.push_back(_s);
        }

        for (int i = 0; i < curTrap.size(); i++) {
            if (i > 0) idstring << ",";

            if (params[i] == _s) {
                idstring << (char*)curTrap[i];
            } else if (params[i] == _i) {
                idstring << (long int)curTrap[i];
            } else if (params[i] == _p) {
                idstring << (long int)curTrap[i];
            } else if (params[i] == _o) {
                idstring << (long int)curTrap[i];
            } else {
                cerr << "Encountered unknown param type (curEAX=" << curEAX << ")"<< endl;
            }
        }
        idstring << ";";
    }

    return idstring.str();
}

/* 
 * Return the number of times elem appears in the semantic descriptor
 */
int SemanticDescriptor::count(SemanticDescriptorElem & elem, Database & db) 
{
    int ret = 0;

    iterator iter;
    for (iter = _sd.begin(); iter != _sd.end(); ++iter) {
        if (!strcmp((char*)(*iter)[0],
                    (char*)(elem[0]))) {
            if (elem.equals(*iter, db)) ret++;
        }
    }

    return ret;
}

/* 
 * Compute coverage of SemanticDescriptor sd2
 */
double SemanticDescriptor::coverage(SemanticDescriptor & sd2, Database & db) 
{
    double covered = 0;
    double size = (double)sd2.size();

    iterator iter;
    for (iter = sd2.begin(); iter != sd2.end(); ++iter) {
        if (count(*iter, db) > 0) covered++;
    }

    return covered/size;
}

/* 
 * Returns 1 if a is a better match than b, -1 if not, or 0 if they are equivalent matches.
 */
int SemanticDescriptor::closerMatch(SemanticDescriptor & a, 
        SemanticDescriptor & b,
        Database & db) 
{
    double coverageA = a.coverage(*this, db);
    double coverageB = b.coverage(*this, db);

    if (coverageA > coverageB) return 1;
    else if (coverageB > coverageA) return -1;
    else {
        if (a.size() < b.size()) return 1;
        else if (a.size() == b.size()) return 0;
        else return -1;
    }
}


/* 
 * Find the closest match in db, return as element of matches
 */
void SemanticDescriptor::findClosestMatch(Database & db, Matches & matches) 
{
    SemanticDescriptor bestMatch = (*(db.dDB.begin())).first;

    //matches.insert((*(db.dDB.begin())).second);

    SemanticDescriptor compareID;
    int matchStrength;

    map<SemanticDescriptor, string>::iterator iter;
    for (iter = db.dDB.begin(); iter != db.dDB.end(); ++iter) {
        compareID = iter->first;
        matchStrength = closerMatch(compareID, bestMatch, db);
        /* If compareID is a better match, reset matches */
        if (matchStrength > 0)
            if (compareID.coverage(*this, db) >= .5) {
                matches.clear();
                matches.insert(iter->second);
                bestMatch = compareID;
            }
        /* If compareID is an equal match, then add it to the set of matches */
        if (matchStrength == 0)
            if (compareID.coverage(*this, db) >= .5)
                matches.insert(iter->second);
    }
}

/* 
 * Determine if two semantic descriptors are equal
 */
bool SemanticDescriptor::equals(SemanticDescriptor & sd2, Database & db)
{
    iterator iter = begin();
    iterator iter2 = sd2.begin();

    if (size() == sd2.size()) {
        for ( ; iter != end(); ++iter) {
            if (!strcmp((char*)((*iter)[0]), (char*)((*iter2)[0]))) {
                if (!((*iter).equals(*iter2, db))) return false;
            } else return false;

            ++iter2;
        }
    } else return false;

    return true;

}

/*
 * Find exact matches for the semantic descriptor in the database
 */
bool SemanticDescriptor::findExactMatches(Database & db, Matches & matches)
{
    SemanticDescriptor compareID;

    map<SemanticDescriptor, string>::iterator iter;

    for (iter = db.dDB.begin(); iter != db.dDB.end(); ++iter) {
        compareID = iter->first;

        if (equals(compareID, db)) {
            matches.insert(iter->second);
        }
    }

    return matches.size();
}


/* 
 * Find matches for the semantic descriptor in the database
 */
Matches SemanticDescriptor::find(Database & db)
{
    Matches matches;
    if (!findExactMatches(db, matches))
        findClosestMatch(db, matches);

    return matches;
}

