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

#define BPATCH_FILE

#include "BPatch_loopTreeNode.h"
#include "BPatch_basicBlockLoop.h"
#include "util.h"
#include "symtab.h"

class BPatch_basicBlockLoop;

BPatch_loopTreeNode::BPatch_loopTreeNode(BPatch_basicBlockLoop *l, 
					 const char *n) {
    loop = l;
    hierarchicalName = NULL;
    if (n != NULL) {
	hierarchicalName = new char[strlen(n)+1]; 
	strcpy(hierarchicalName, n);
    }
}
 

const char * 
BPatch_loopTreeNode::getCalleeName(unsigned int i) 
{
    assert(i < callees.size());
    assert(callees[i] != NULL);
    return callees[i]->prettyName().c_str();
}

const char * 
BPatch_loopTreeNode::name()
{
    assert(loop != NULL);
    return (const char *)hierarchicalName; 
}

unsigned int
BPatch_loopTreeNode::numCallees() { 
    return callees.size(); 
}


BPatch_loopTreeNode::~BPatch_loopTreeNode() {
    delete loop;

    for (unsigned i = 0; i < children.size(); i++)
	delete children[i];

    delete[] hierarchicalName;
    // don't delete callees!
}


BPatch_basicBlockLoop *
BPatch_loopTreeNode::findLoop(const char *name) 
{ 
    if (loop) {
        if (0==strcmp(name,hierarchicalName)) 
            return loop;
    }
    for (unsigned i = 0; i < children.size(); i++) {
        BPatch_basicBlockLoop *lp = children[i]->findLoop(name);
        if (lp) return lp;
    }
    return NULL;
}
