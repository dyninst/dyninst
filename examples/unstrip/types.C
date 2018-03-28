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


#include <iostream>
#include <string>
#include <sstream>

#include "util.h"
#include "types.h"

using namespace std;

/* Convert string param type into ParamType int */
ParamType getParamType(char * type)
{
    if (type != NULL) {
        char val = type[0];

        if (val == 'i') return _i;
        else if (val == 's') return _s;
        else if (val == 'p') return _p;
        else if (val == 'o') return _o;
    }

    return _u;
}

string Matches::format()
{
    stringstream ret;

    iterator mIter;
    iterator test;
    for (mIter = m.begin(); mIter != m.end(); ++mIter) {
        test = mIter;
        ++test;
        if (test == m.end())
            ret << *mIter;
        else
            ret << *mIter << "/";
    }
    return ret.str(); 

}

void Matches::clear()
{
   m.clear(); 
}

string Matches::operator[](const int pos) const
{
    return *(m.begin());
}
