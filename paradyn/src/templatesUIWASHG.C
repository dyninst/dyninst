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

// $Id: templatesUIWASHG.C,v 1.11 2004/03/23 01:12:25 eli Exp $

// templatesUIWASHG.C
// templates in the UIthread related to the where axis and search history graph

// seems to be needed for sunos; why I don't know
#ifndef NULL
#define NULL 0
#endif

/* *************************************
 * UIthread WhereAxis stuff
 */

#include "common/h/Vector.h"
#include "common/src/Dictionary.C"

#include "paradyn/src/UIthread/where4tree.C"
#include "paradyn/src/UIthread/rootNode.h"
template class where4tree<whereAxisRootNode>;
template class pdvector<const whereAxisRootNode *>;
template class pdvector<where4tree<whereAxisRootNode>::childstruct>;

template class dictionary_hash< resourceHandle, where4tree<whereAxisRootNode> * >;
template class pdvector<dictionary_hash< resourceHandle, where4tree<whereAxisRootNode> * >::entry>;
template class pdvector<where4tree<whereAxisRootNode> *>;

#include "paradyn/src/UIthread/graphicalPath.C"
template class whereNodeGraphicalPath<whereAxisRootNode>;

#include "paradyn/src/UIthread/simpSeq.C"
template class simpSeq<unsigned>;

#include "paradyn/src/UIthread/abstractions.h"
template class pdvector<abstractions::whereAxisStruct>;

/* *************************************
 * UIthread Search History Graph stuff
 */

#include "paradyn/src/UIthread/shgRootNode.h"
template class where4tree<shgRootNode>;
template class pdvector<where4tree<shgRootNode> *>;
template class pdvector<where4tree<shgRootNode>::childstruct>;
template class whereNodeGraphicalPath<shgRootNode>;
template class pdvector<const shgRootNode *>;

#include "paradyn/src/UIthread/shgPhases.h"
template class pdvector<shgPhases::shgStruct>;

template class dictionary_hash<unsigned, where4tree<shgRootNode> *>;
template class pdvector<dictionary_hash<unsigned, where4tree<shgRootNode> *>::entry>;

template class dictionary_hash<where4tree<shgRootNode> *, where4tree<shgRootNode> *>;
template class pdvector<dictionary_hash<where4tree<shgRootNode> *, where4tree<shgRootNode> *>::entry>;

template class dictionary_hash< unsigned, pdvector<where4tree<shgRootNode>*> >;
template class pdvector<dictionary_hash< unsigned, pdvector<where4tree<shgRootNode>*> >::entry>;
template class pdvector< pdvector<where4tree<shgRootNode>*> >;

template class pdvector<Tk_3DBorder>; // shg consts
 
/* *************************************
 * UIthread Call Graph stuff
 */
#include "paradyn/src/UIthread/callGraphRootNode.h"
template class where4tree<callGraphRootNode>;
template class pdvector<where4tree<callGraphRootNode>::childstruct>;
template class whereNodeGraphicalPath<callGraphRootNode>;
template class pdvector<const callGraphRootNode *>;

#include "paradyn/src/UIthread/callGraphs.h"
template class pdvector<callGraphs::callGraphStruct>;

template class dictionary_hash<unsigned, where4tree<callGraphRootNode> *>;
template class pdvector<dictionary_hash<unsigned, where4tree<callGraphRootNode> *>::entry>;
template class pdvector<where4tree<callGraphRootNode> *>;

template class dictionary_hash<where4tree<callGraphRootNode> *, where4tree<callGraphRootNode> *>;
template class pdvector<dictionary_hash<where4tree<callGraphRootNode> *, where4tree<callGraphRootNode> *>::entry>;

template class dictionary_hash< unsigned, pdvector<where4tree<callGraphRootNode>*> >;
template class pdvector<dictionary_hash< unsigned, pdvector<where4tree<callGraphRootNode>*> >::entry>;
template class pdvector< pdvector<where4tree<callGraphRootNode>*> >;
