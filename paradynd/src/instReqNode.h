/*
 * Copyright (c) 1996 Barton P. Miller
 * 
 * We provide the Paradyn Parallel Performance Tools (below
 * described as Paradyn") on an AS IS basis, and do not warrant its
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
#include "dyninstAPI/src/inst.h"
#include "dyninstAPI/src/frame.h"

class instPoint;
class AstNode;
class instInstance;

class instReqNode;

class catchupReq {
 public:
  catchupReq() : frame() {};
  catchupReq(Frame frame2) :
    frame(frame2) {};
  catchupReq(const catchupReq &src) {
    frame = src.frame;
    for (unsigned i = 0; i < src.reqNodes.size(); i++)
      reqNodes.push_back(src.reqNodes[i]);
  }


  vector<instReqNode *> reqNodes;
  Frame        frame;
};


class instReqNode {
 public:
  instReqNode(instPoint*, AstNode *, callWhen, callOrder order);
  ~instReqNode();
  
  instReqNode() {
    // needed by Vector class
    ast = NULL; 
    point=NULL; 
    instance = NULL; 
    rinstance = NULL; 
  }
  
  instReqNode(const instReqNode &src) {
    point = src.point;
    when = src.when;
    order = src.order;
    instance = src.instance;
    rinstance = src.rinstance;
    ast = assignAst(src.ast);
  }
  instReqNode &operator=(const instReqNode &src) {
    if (this == &src)
      return *this;
    
    point = src.point;
    ast = assignAst(src.ast);
    when = src.when;
    order = src.order;
    instance = src.instance;
    rinstance = src.rinstance;
    
    return *this;
  }

  instInstance* loadInstrIntoApp(process *theProc, 
				 returnInstance *&retInstance, bool *deferred);
  
  void disable(const vector<Address> &pointsToCheck);
  timeLength cost(process *theProc) const;
  
  static instReqNode forkProcess(const instReqNode &parent,
                         const dictionary_hash<instInstance*,instInstance*> &);
  // should become a copy-ctor...or at least, a non-static member fn.
  
  bool unFork(dictionary_hash<instInstance*, instInstance*> &map) const;
  // The fork syscall duplicates all trampolines from the parent into the
  // child. For those mi's which we don't want to propagate to the child,
  // this creates a problem.  We need to remove instrumentation code from the
  // child.  This routine does that.  "map" maps instInstances of the parent
  // to those in the child.

  instInstance *getInstance() const { return instance; }
  returnInstance *getRInstance() const { return rinstance; }
  
  bool postCatchupRPC(process *theProc, Frame &triggeredFrame, int mid);
  static void catchupRPCCallbackDispatch(process * /*theProc*/,
					 void *userData, void *returnValue)
    { ((instReqNode*)userData)->catchupRPCCallback( returnValue ); }
  void catchupRPCCallback(void *returnValue);
  
  bool triggeredInStackFrame(Frame &frame, process *p);
  
  instPoint *Point() {return point;}
  AstNode* Ast()  {return ast;}
  callWhen When() {return when;}
  callOrder Order() { return order; }
  
private:
  instPoint	*point;
  AstNode	*ast;
  callWhen	when;
  callOrder	order;
  instInstance	*instance; // undefined until loadInstrIntoApp() calls addInstFunc
  returnInstance *rinstance;
  
  // Counts the number of rpcs which have successfully completed for this
  // node.  This is needed because we may need to manually trigger multiple
  // times for recursive functions.
  int rpcCount;
};



#endif
