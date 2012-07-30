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
#include <vector>
#include <map>

#include "dyntypes.h"

#include "CodeSource.h"
#include "debug_parse.h"
#include "util.h"

using namespace std;
using namespace Dyninst;
using namespace Dyninst::ParseAPI;

/** CodeSource **/
void
CodeSource::addRegion(CodeRegion * cr)
{
    _regions.push_back(cr);

    // check for overlapping regions
    if(!_regions_overlap) {
        set<CodeRegion *> exist;
        _region_tree.find(cr,exist);
        if(!exist.empty())
            _regions_overlap = true;
    }

    _region_tree.insert(cr);
}

int
CodeSource::findRegions(Address addr, set<CodeRegion *> & ret) const
{
    return _region_tree.find(addr,ret);
}



