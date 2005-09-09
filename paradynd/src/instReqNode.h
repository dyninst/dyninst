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

#ifndef INST_REQ_NODE
#define INST_REQ_NODE


#include "common/h/Vector.h"
#include "dyninstAPI/h/BPatch_snippet.h"
#include "dyninstAPI/h/BPatch_process.h"
#include "dyninstAPI/src/instP.h" // for mtHandle
#include "dyninstAPI/src/miniTramp.h"

class instrDataNode;
class instReqNode;
class pd_process;

typedef enum { unknown_res,  // Default
	       failure_res,  // Failed (memory allocation failure)
	       unlinked_res, // Basetramp failed somehow
	       success_res }  loadMiniTramp_result;


class catchupReq {
 public:
   catchupReq() : frame(), handledFunctionEntry(false), handledLoopEntry(false) {};
   catchupReq(Frame frame2) :
      frame(frame2), handledFunctionEntry(false), handledLoopEntry(false) {};
   catchupReq(const catchupReq &src) {
       frame = src.frame;
       for (unsigned i = 0; i < src.reqNodes.size(); i++)
           reqNodes.push_back(src.reqNodes[i]);
       handledFunctionEntry = src.handledFunctionEntry;
       handledLoopEntry = src.handledLoopEntry;
   }
   
   pdvector<instReqNode*> reqNodes;
   Frame        frame;
   bool handledFunctionEntry;
   bool handledLoopEntry;
};


class instReqNode {
 private:
   // disallow this
   instReqNode &operator=(const instReqNode &) { return *this; }
   
 public:
   instReqNode(BPatch_point *iPoint, BPatch_snippet *iSnip, 
               BPatch_callWhen iWhen, BPatch_snippetOrder o) :
       point(iPoint), snip(iSnip),
       when(iWhen), order(o), 
       instrAdded_(false),
       instrGenerated_(false),
       instrLinked_(false),
       rpcCount(0), loadInstAttempts(0) 
   {
   }
   
   ~instReqNode();
   
   // normal copy constructor, used eg. in vector<instReqNode> expansion
   instReqNode(const instReqNode &par) : 
       point(par.point), snip(par.snip), when(par.when), 
       order(par.order), mtHandle(par.mtHandle), 
       instrAdded_(par.instrAdded_),
       instrGenerated_(par.instrGenerated_),
       instrLinked_(par.instrLinked_),                       
       rpcCount(par.rpcCount), 
       loadInstAttempts(par.loadInstAttempts)
   { }
   
   // special copy constructor used for fork handling
   instReqNode(const instReqNode &par, pd_process *childProc);

   bool instrAdded()    const { return instrAdded_; }
   bool instrGenerated()    const { return instrGenerated_; }
   bool instrLinked()       const { return instrLinked_; }

   bool addInstr(pd_process *theProc);
   bool generateInstr();
   // This is handed down... if we optimized the stack walking
   // code to realize that we could return the same one
   // if the stack is walked multiple times, then we'd be cool
   // with letting the low-level code check for its own stuff.
   // As it is, this is safer.
   bool checkInstr(pdvector<pdvector<Frame> > &stackWalks);
   bool linkInstr();

   void disable();
   timeLength cost(pd_process *theProc) const;
   //returnInstance *getRInstance() const { return rinstance; }
   void setAffectedDataNodes(miniTrampFreeCallback cb, void *v); 
   
   void catchupRPCCallback(void *returnValue);
   
   bool triggeredInStackFrame(Frame &frame,
                              pd_process *p);
   
   BPatch_point *Point() {return point;}
   BPatch_snippet* Snippet()  {return snip;}
   BPatch_callWhen When() {return when;}
   BPatch_snippetOrder Order() { return order; }
   // This is what needs to change...
   miniTramp *MiniTramp() { return mtHandle; }

 private:
   BPatch_point *point;
   BPatch_snippet   *snip;
   BPatch_callWhen  when;
   BPatch_snippetOrder order;
   
   miniTramp *mtHandle;

   bool instrAdded_;
   bool instrGenerated_;
   bool instrLinked_;
   
   // Counts the number of rpcs which have successfully completed for this
   // node.  This is needed because we may need to manually trigger multiple
   // times for recursive functions.
   int rpcCount;
   
   int loadInstAttempts;  // count of deferred load instrumentation attempts
};



#endif
