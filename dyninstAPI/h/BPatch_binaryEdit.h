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

#ifndef _BPatch_binaryEdit_h_
#define _BPatch_binaryEdit_h_


#include "BPatch_snippet.h"
#include "BPatch_dll.h"
#include "BPatch_Vector.h"
#include "BPatch_image.h"
#include "BPatch_eventLock.h"
#include "BPatch_point.h"
#include "BPatch_addressSpace.h"

#include "BPatch_callbacks.h"

#include <vector>

#include <stdio.h>
#include <signal.h>

class process;
class BinaryEdit;
class AddressSpace;
class dyn_thread;
class miniTrampHandle;
class miniTramp;
class BPatch;
class BPatch_thread;
class BPatch_process;
class BPatch_funcMap;
class BPatch_instpMap;
class int_function;
class rpcMgr;
struct batchInsertionRecord;

class BPATCH_DLL_EXPORT BPatch_binaryEdit : public BPatch_addressSpace {
    friend class BPatch;
    friend class BPatch_image;
    friend class BPatch_function;
    friend class BPatch_frame;
    friend class BPatch_module;
    friend class BPatch_basicBlock;
    friend class BPatch_flowGraph;
    friend class BPatch_loopTreeNode;
    friend class BPatch_point;
    friend class BPatch_funcCallExpr;
    friend class BPatch_instruction;
    friend class AstNode; // AST needs to translate instPoint to
		      // BPatch_point via instp_map
    friend class AstOperatorNode;
    friend class AstMemoryNode;

    private:
    BinaryEdit *llBinEdit;
    BPatch_binaryEdit(const char *path);
    bool creation_error;

    public:

    bool getType();
    AddressSpace * getAS();
    bool getTerminated() {return false;}
    bool getMutationsActive() {return true;}

    // BPatch_binaryEdit::writeFile
    API_EXPORT(Int, (outFile),
	       bool,writeFile,(const char * outFile));

  
    //  BPatch_binaryEdit::~BPatch_binaryEdit
    //
    //  Destructor
    API_EXPORT_DTOR(_dtor, (),
    ~,BPatch_binaryEdit,());

        
    
    //  BPatch_binaryEdit::insertSnippet
    //  
    //  Insert new code into the mutatee

    API_EXPORT(Int, (expr, point, order),
    BPatchSnippetHandle *,insertSnippet,(const BPatch_snippet &expr, BPatch_point &point,
                                         BPatch_snippetOrder order = BPatch_firstSnippet));

    //  BPatch_binaryEdit::insertSnippet
    //  
    //  Insert new code into the mutatee, specifying "when" (before/after point)

    API_EXPORT(When, (expr, point, when, order),
    BPatchSnippetHandle *,insertSnippet,(const BPatch_snippet &expr, BPatch_point &point,
                                         BPatch_callWhen when,
                                         BPatch_snippetOrder order = BPatch_firstSnippet));

    //  BPatch_binaryEdit::insertSnippet
    //  
    //  Insert new code into the mutatee at multiple points

    API_EXPORT(AtPoints, (expr, points, order),
    BPatchSnippetHandle *,insertSnippet,(const BPatch_snippet &expr,
                                         const BPatch_Vector<BPatch_point *> &points,
                                         BPatch_snippetOrder order = BPatch_firstSnippet));

    

    //  BPatch_binaryEdit::insertSnippet
    //  
    //  Insert new code into the mutatee at multiple points, specifying "when"

    API_EXPORT(AtPointsWhen, (expr, points, when, order),
    BPatchSnippetHandle *,insertSnippet,(const BPatch_snippet &expr,
                                         const BPatch_Vector<BPatch_point *> &points,
                                         BPatch_callWhen when,
                                         BPatch_snippetOrder order = BPatch_firstSnippet));

    
    //  BPatch_binaryEdit::beginInsertionSet()
    //
    //  Start the batch insertion of multiple points; all calls to insertSnippet*
    //  after this call will not actually instrument until finalizeInsertionSet is
    //  called

    API_EXPORT_V(Int, (),
                 void, beginInsertionSet, ());

    //  BPatch_binaryEdit::finalizeInsertionSet()
    //
    //  Finalizes all instrumentation logically added since a call to beginInsertionSet.
    //  Returns true if all instrumentation was successfully inserted; otherwise, none
    //  was. Individual instrumentation can be manipulated via the BPatchSnippetHandles
    //  returned from individual calls to insertSnippet.
    //
    //  atomic: if true, all instrumentation will be removed if any fails to go in.
    //  modified: if provided, and set to true by finalizeInsertionSet, additional
    //            steps were taken to make the installation work, such as modifying
    //            process state.  Note that such steps will be taken whether or not
    //            a variable is provided.

    API_EXPORT(Int, (atomic, modified),
               bool, finalizeInsertionSet, (bool atomic, bool *modified = NULL));
                                       
};    

#endif /* BPatch_binaryEdit_h_ */
