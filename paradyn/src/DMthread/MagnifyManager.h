/*
 * Copyright (c) 1996-2004 Barton P. Miller
 * 
 * We provide the Paradyn Parallel Performance Tools (below
 * described as "Paradyn") on an AS IS basis, and do not warrant its
 * validity or performance.  We reserve the right to update, modify,
 * or discontinue this software at any time.  We shall have no
 * obligation to supply such updates or modifications or any other
 * form of support to you.
 * 
 * This license is for research uses.  For such uses, there is no
 * charge. We define "research use" to mean you may freely use it
 * inside your organization for whatever purposes you see fit. But you
 * may not re-distribute Paradyn or parts of Paradyn, in any form
 * source or binary (including derivatives), electronic or otherwise,
 * to any other organization or entity without our permission.
 * 
 * (for other uses, please contact us at paradyn@cs.wisc.edu)
 * 
 * All warranties, including without limitation, any warranty of
 * merchantability or fitness for a particular purpose, are hereby
 * excluded.
 * 
 * By your use of Paradyn, you understand and agree that we (or any
 * other person or entity with proprietary rights in Paradyn) are
 * under no obligation to provide either maintenance services,
 * update services, notices of latent defects, or correction of
 * defects for Paradyn.
 * 
 * Even if advised of the possibility of such damages, under no
 * circumstances shall we (or any other person or entity with
 * proprietary rights in the software licensed hereunder) be liable
 * to you or any third party for direct, indirect, or consequential
 * damages of any character regardless of type of action, including,
 * without limitation, loss of profits, loss of use, loss of good
 * will, or computer failure or malfunction.  You agree to indemnify
 * us (and any other person or entity with proprietary rights in the
 * software licensed hereunder) for any and all liability it may
 * incur to third parties resulting from your use of Paradyn.
 */

// $Id: MagnifyManager.h,v 1.4 2004/03/23 01:12:26 eli Exp $

#ifndef __magnify_manager_h__
#define __magnify_manager_h__


#include "DMinclude.h"
#include "DMresource.h"

// Class for taking care of applying different types/flavors
//  of magnification to resourceList::magnify().  These flavors
//  are used to change how things which rely on magnify work (e.g.
//  PC searches) without adding new resource hierarchies.
// What might seem like a good C++ approach - subclassing, doesn't
//  fit well here because the resourceLists need to be set up before 
//  any info about the type of search is known, and because
//  adding multiple resourceLists (of different types) to serve
//  different search flavors would apparently screw up assumptions about
//  the structure of the resource hierarchy coded into paradyn in
//  many places.
//  - Thus, another hack in paradyn is born....
class MagnifyManager {

 public:
    // Magnify the specified resource using the specified flavor of 
    //  magnification.  
    // If the specified flavor doesn't apply to all resource hierarchies,
    //  and rh represents a resource in a hierarchy to which type does
    //  not apply, default to using the OriginalSearch type, which applies to
    //  all resource hierarchies....
    static pdvector <resourceHandle>* getChildren(resource *rh,magnifyType type);
};


// __magnify_manager_h__
#endif
