/*
 * Copyright (c) 1998 Barton P. Miller
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

#include <iostream.h>
#include <stdio.h>
#include <assert.h>
#include "dyninstAPI/src/arch-mips.h"
#include "dyninstAPI/src/inst-mips.h"
#include "dyninstAPI/src/symtab.h"    // pd_Function, image
#include "dyninstAPI/src/instPoint.h" // instPoint
#include "dyninstAPI/src/process.h"   // process
#include "dyninstAPI/src/instP.h"     // instWaitingList
#include "dyninstAPI/src/stats.h"     // accounting
#include "dyninstAPI/src/dyninstP.h"  // isApplicationPaused()
#include "util/h/debugOstream.h"
#include <disassembler.h>

#ifndef BPATCH_LIBRARY
//#include "rtinst/h/trace.h"
//#include "paradynd/src/metric.h"
//#include "paradynd/src/main.h"
#include "paradynd/src/perfStream.h" // firstRecordTime
#include "paradynd/src/context.h"    // elapsedPauseTime, startPause
//#include "dyninstAPI/src/showerror.h"
#endif


typedef signed short       SignedImm;
typedef unsigned short     UnsignedImm;

// external prototypes
extern bool isPowerOf2(int value, int &result);

// global variables
trampTemplate baseTemplate;
registerSpace *regSpace;
/* excluded registers: 
   0(zero)
   1($at)
   2-3(return)
   4-11(args)
   25($t9)
   31($ra)
*/
Register Dead[] = 
{
  /* 0,*/ /* 1,*/ /* 2,*/ /* 3,*/ /* 4,*/ /* 5,*/ /* 6,*/ /* 7,*/ 
  /* 8,*/ /* 9,*/ /*10,*/ /*11,*/   12,     13,     14,     15,  
    16,     17,     18,     19,     20,     21,     22,     23,  
    24,   /*25,*/   26,     27,     28,     29,     30,   /*31,*/
};
const unsigned int nDead = sizeof(Dead) / sizeof(Dead[0]);
#define MAX_IMM16 ((SignedImm)0x7fff)
#define MIN_IMM16 ((SignedImm)0x8000)

// local variables
static dictionary_hash<string, unsigned> funcFrequencyTable(string::hash);

FILE *Stderr = stderr; // hack for debugging
void print_inst_pts(const vector<instPoint*> &pts, pd_Function *fn) 
{
  for (unsigned i = 0; i < pts.size(); i++) {
    fprintf(stderr, "  0x%p: ", (void *)(pts[i]->offset() + fn->getAddress(0)));
    switch(pts[i]->type()) {
    case IPT_ENTRY:
      fprintf(stderr, "entry\n"); break;
    case IPT_EXIT:
      fprintf(stderr, "exit\n"); break;
    case IPT_CALL:
      fprintf(stderr, "call\n"); break;
    case IPT_NONE:      
    default:
      fprintf(stderr, "??? (%i)\n", pts[i]->type()); break;
    }
  }
}

void print_function(pd_Function *f)
{
  fprintf(stderr, "0x%p: %s(%u insns):\n", 
	  (void *)f->getAddress(0), f->prettyName().string_of(), f->size() / INSN_SIZE);
  vector<instPoint*> t;
  t += const_cast<instPoint*>(f->funcEntry(0));
  print_inst_pts(t, f);
  print_inst_pts(f->funcCalls(0), f);
  print_inst_pts(f->funcExits(0), f);
}


Address lookup_fn(process *p, const string &f)
{
  //fprintf(stderr, ">>> lookup_fn(%s)\n", f.string_of());
  Address ret = 0;

  // findInternalSymbol()
  /*
  if (ret == 0 ) {
    internalSym sym;
    if (p->findInternalSymbol(f, false, sym)) {
      ret = sym.getAddr();
      //fprintf(stderr, "  findInternalSymbol: 0x%08x\n", ret);
    }
  }
  */

  // findInternalAddress()
  if (ret == 0) {
    bool err;
    ret = p->findInternalAddress(f, false, err);
    //if (ret) fprintf(stderr, "  findInternalAddress: 0x%08x\n", ret);
  }
  
  // findOneFunction()
  if (ret == 0) {
    pd_Function *pdf = (pd_Function *)p->findOneFunction(f);
    if (pdf) {
      Address obj_base;
      p->getBaseAddress(pdf->file()->exec(), obj_base);
      ret = obj_base + pdf->getAddress(p);
      //fprintf(stderr, "  findOneFunction: 0x%08x\n", ret);
    }
  }

  return ret;
}

static bool mips_dis_init = true;
void disDataSpace(process *p, void *addr_, int ninsns, 
		  const char *pre, FILE *stream)
{
  /*
  if (mips_dis_init) {
    mips_dis_init = false;
    dis_init32(0, 0, reg_names, 1);
  }
  
  instruction *addr = (instruction *)addr_;
  assert(isAligned((Address)addr));
  instruction insn;
  char buf[64];
  Elf32_Addr regmask, value, lsreg;
  for (int i = 0; i < ninsns; i++) {
    void *inTraced = addr + i;
    p->readDataSpace(inTraced, INSN_SIZE, &insn, true);
    disasm32(buf, (Elf32_Addr)inTraced, *(Elf32_Addr *)&insn, &regmask, &value, &lsreg);
    fprintf(stream, "%s%s\n", (pre) ? (pre) : (""), buf);
  }
  */
}

void dis(void *actual_, void *addr_, int ninsns, 
	 const char *pre, FILE *stream)
{
  /*
  instruction *actual = (instruction *)actual_;
  instruction *addr = (instruction *)addr_;
  if (addr == NULL) addr = actual;

  if (mips_dis_init) {
    mips_dis_init = false;
    dis_init32(0, 0, 0, 0);
  }

  char buf[64];
  Elf32_Addr regmask, value, lsreg;
  for (int i = 0; i < ninsns; i++) {
    disasm32(buf, (Elf32_Addr)(addr + i), *(Elf32_Addr *)(actual + i), 
	     &regmask, &value, &lsreg);
    fprintf(stream, "%s%s\n", (pre) ? (pre) : (""), buf);
  }
  */
}


/* initialization: check representation sizes - csserra */
void mips_sgi_irix6_4_init(void)
{
  static int Init = 1;
  if (Init) {
    Init = 0;
    assert(sizeof(instruction) == INSN_SIZE);
    assert(sizeof(Address) == sizeof(void *));
  }
}

/*
 * findInstPoints(): EXPORTED
 *
 * pd_Function members populated:
 *   funcEntry_
 *   funcReturns
 *   calls
 *   relocatable_
 *   noStackFrame
 *   isTrap
 *
 */
#define UNINSTR(str) fprintf(stderr, "uninstrumentable: %s (%0#10x: %i insns) - %s\n", \
			     prettyName().string_of(), \
			     file_->exec()->getObject().get_base_addr() + getAddress(0), \
			     size() / INSN_SIZE, \
			     str)

bool pd_Function::findInstPoints(const image *owner) {
  //fprintf(stderr, "\n>>> pd_Function::findInstPoints()\n");
  //fprintf(stderr, "%0#10x: %s(%u insns):\n", 
  //getAddress(0), prettyName().string_of(), size() / INSN_SIZE);
  if (size() == 0) {
    //cerr << "Function " << prettyName().string_of() << ", size = 0" << endl;
#ifdef CSS_DEBUG_INST
    UNINSTR("zero length");
#endif
    return false;
  }

  // check representation sizes
  mips_sgi_irix6_4_init();

  // default values
  isTrap = false;
  relocatable_ = false;
  noStackFrame = true;

  // parse instPoints
  Address start = getAddress(0);
  Address end = start + size();
  Offset off;
  instruction insn;

  //for (off = 0; off < end - start; off += INSN_SIZE) {
  //insn.raw = owner->get_instruction(start + off);
  // find first "addiu sp,sp,-XX" insn
  //if (isSaveInsn(insn)) {
  //noStackFrame = false;
  //break;
  //}
  //}
  // if no stack frame, use start of fn
  //funcEntry_ = new entryPoint(this, (noStackFrame) ? (off) : (0));

  /* stack frame info */
  Address entry = findStackFrame(owner);
  
  /* ENTRY point */
  // use address from findStackFrame() (1st save insn or fn start)
  /* check if function has a stack frame:
   * yes: entry point is "save" instruction (addiu sp,sp,-XX)
   * no:  entry point is start of function
   */
  funcEntry_ = new entryPoint(this, entry); 
  assert(funcEntry_);


  /* CALL and EXIT points */
  for (off = 0; off < end - start; off += INSN_SIZE) {
    insn.raw = owner->get_instruction(start + off);
      
    /* CALL points */
    if (isCallInsn(insn)) {
      /* simple call */
      //cerr << "  found call pt" << endl;
      calls += new callPoint(this, off);
    } else if (isInsnType(insn, BGEZALLmask, BGEZALLmatch)) {
      /* optimized recursive call: branch to start of function */
      //cerr << "  found optimized recursive call pt" << endl;
      signed branchOff = insn.regimm.simm16 << 2;
      Offset targetOff = off + INSN_SIZE + branchOff;
      if (targetOff == 0) {
	calls += new callPoint(this, off, IP_RecursiveBranch);
      }
    } else if (isInsnType(insn, SYSCALLmask, SYSCALLmatch)) {
      isTrap = true;
      // debug
      /*
      instruction prev;
      prev.raw = owner->get_instruction(start + off - INSN_SIZE);
      if (isInsnType(prev, ADDIUmask, ADDIUmatch) &&
	  prev.itype.rs == REG_ZERO &&
	  prev.itype.rt == REG_V0) {
	int syscall_num = prev.itype.simm16;
	fprintf(stderr, "<%s,0x%08x>: syscall %i\n", 
		prettyName().string_of(),
		owner->getObject().get_base_addr() + 
		getAddress(0) + off,
		syscall_num);
      }
      */
    }

      
    /* EXIT points */
    // TODO - distinguish between return and switch? (maybe "jr ra" vs "jr v1")
    if (isReturnInsn(insn)) {
      //cerr << "  found return pt" << endl;
      funcReturns += new exitPoint(this, off);
    }
  }

  setIDs(); // set CALL and EXIT vectorIds

  return checkInstPoints();
}
// below fns (commented out) are "defined but not used"
/*
static bool contains(vector<int> &V, int val)
{
  for (unsigned i = 0; i < V.size(); i++) {
    if (V[i] == val) return true;
  }
  return false;
}
*/
/*
static void addIfNew(vector<int> &V, int val)
{
  if (contains(V, val)) return;
  V += val;
}
*/
/*
static void print_saved_registers(pd_Function *fn, const vector<vector<int> > &slots)
{

  //vector<vector<int> > slots2(slots.size());
  //vector<int> locals;
  //for (unsigned i = 0; i < slots.size(); i++) {
  //for (int j = 0; j < slots[i].size(); j++) {
  //int slot = slots[i][j];
  //bool dup = false;
  //if (contains(locals, slot)) dup = true;
  //for (int k = 0; k < slots.size() && !dup; k++) {
  //if (k == i) continue;
  //if (contains(slots[k], slot)) dup = true;
  //}
  //if (!dup) addIfNew(slots2[i], slot);
  //  else addIfNew(locals, slot);
  //}
  //}


  bool mult = false;
  for (unsigned i = 0; i < slots.size(); i++) {
    if (slots[i].size() > 1) {
      mult = true;
      break;
    }
  }


  if (mult) {
    fprintf(stderr, "*** %s (0x%p: %i insns): stack frame\n",
	    fn->prettyName().string_of(), (void *)fn->getAddress(0), 
	    fn->size() / INSN_SIZE);
    vector<int> locals;
    for (unsigned i = 0; i < slots.size(); i++) {
      if (slots[i].size() > 0) {
	fprintf(stderr, "  $%-4s:", reg_names[i]);
	for (unsigned j = 0; j < slots[i].size(); j++) {
	  int slot = slots[i][j];
	  bool dup = false;
	  for (unsigned k = 0; k < slots.size() && !dup; k++) {
	    if (k == i) continue;
	    for (unsigned l = 0; l < slots[k].size() && !dup; l++) {
	      if (slots[k][l] == slot) {
		addIfNew(locals, slot);
		dup = true;
	      }
	    }
	  }
	  fprintf(stderr, " %3i", slot);
	  if (dup) fprintf(stderr, "*");
	}      	
	fprintf(stderr, "\n");
      }
    }
    fprintf(stderr, "  vars :");
    for (unsigned i = 0; i < locals.size(); i++) {
      fprintf(stderr, " %3i", locals[i]);
    }
    fprintf(stderr, "\n");
  }
}
*/

Address pd_Function::findStackFrame(const image *owner)
{
  /*
  fprintf(stderr, "*** %s (%0#10x: %i insns): stack frame\n",
	  prettyName().string_of(), getAddress(0), size() / INSN_SIZE);
  */

  frameSize = 0;
  for (int i = 0; i < NUM_REGS; i++) {
    frameOff[i] = (int)-1;
    saveInsn[i] = (Address)-1;
  }

  // multiple register saves
  //vector<vector<int> >slots(32);

  // parse instPoints
  Address start = getAddress(0);
  Address end = start + size();
  Offset off;
  instruction insn;

  for (off = 0; off < end - start; off += INSN_SIZE) {
    insn.raw = owner->get_instruction(start + off);
    struct fmt_itype &itype = insn.itype;

    // stack frame save: "addiu sp,sp,-XX" insn
    if (itype.op == ADDIUop && 
	itype.rs == REG_SP &&
	itype.rt == REG_SP &&
	itype.simm16 < 0)
      {
	noStackFrame = false;
	frameSize = -itype.simm16;
	saveInsn[REG_ZERO] = off;
      }

    // register save
    // not seen yet: "sw  RR,-XX(sp)"
    assert(!(itype.op == SWop &&
	     itype.rs == REG_SP &&
	     itype.simm16 < 0));
    // not seen yet: "sd  RR,-XX(sp)"
    assert(!(itype.op == SDop &&
	     itype.rs == REG_SP &&
	     itype.simm16 < 0));
    if ((itype.op == SDop || itype.op == SWop) &&
	itype.rs == REG_SP &&
	itype.rt != REG_ZERO &&
	itype.simm16 >= 0) // TODO: bogus constraint?
      {
	int r = itype.rt;
	// earliest save insn
	if (saveInsn[r] == (Address)-1) {
	  frameOff[r] = itype.simm16;
	  saveInsn[r] = off;
	}
	// multiple register saves
	//addIfNew(slots[r], itype.simm16);
      }
  }

  // multiple register saves
  //print_saved_registers(this, slots);

  /*
  if (!noStackFrame) {
    Address saveOff = saveInsn[REG_ZERO];
    if (saveOff > 4*INSN_SIZE) 
      fprintf(stderr, "!!! late save insn (%s, %0#10x, %i insns)\n", 
	      prettyName().string_of(), 
	      getAddress(0) + saveOff,
	      saveOff / INSN_SIZE);
  }
  */

  // default return value (entry point = first insn of fn)
  return (noStackFrame) ? (0) : (saveInsn[REG_ZERO]);
}

void pd_Function::setIDs()
{
  //fprintf(stderr, ">>> pd_Function::setIDS()\n");
  for (unsigned i = 0; i < calls.size(); i++) 
    calls[i]->vectorId = i;
  for (unsigned i = 0; i < funcReturns.size(); i++) 
    funcReturns[i]->vectorId = i;
}

/* compare instPoints by Address: used in checkInstPoints() */
static int cmpByAddr(const void *A, const void *B)
{
  instPoint *ptA = *(instPoint **)const_cast<void*>(A);
  instPoint *ptB = *(instPoint **)const_cast<void*>(B);
  Offset offA = ptA->offset();
  Offset offB = ptB->offset();
  if (offA < offB) return -1;
  if (offA > offB) return 1;
  return 0;
}

/* checkInstPoints(): check for special instPoint conditions...
 * - overlapping instPoints
 * - first instPoint is NOT an entry point
 */
bool pd_Function::checkInstPoints()
{
  //fprintf(stderr, ">>> pd_Function::checkInstPoints()\n");
  vector<instPoint*> pts;

  /* function has no entry point */
  if (!funcEntry_) {
    //fprintf(stderr, "!!/ no entry pt\n");
#ifdef CSS_DEBUG_INST
    UNINSTR("no entry point");
#endif
    return false;
  } 
  /* function has no exit points */
  if (funcReturns.size() == 0) {
    //fprintf(stderr, "!! no exit pts\n");
#ifdef CSS_DEBUG_INST
    UNINSTR("no exit points");
    print_inst_pts(pts, this);
#endif
    return false; 
  }

  /* create complete list of function's instPoints, sorted by address */
  pts += funcEntry_;
  for (unsigned i = 0; i < funcReturns.size(); i++) 
    pts += funcReturns[i];
  for (unsigned i = 0; i < calls.size(); i++) 
    pts += calls[i];
  pts.sort(cmpByAddr);

  // debug: print sorted list of inst pts
  /*
  for (unsigned i = 0; i < pts.size(); i++) {
    fprintf(stderr, "%0#10x: ", pts[i]->offset());
    switch(pts[i]->type()) {
    case IPT_ENTRY:
      fprintf(stderr, "entry\n"); break;
    case IPT_EXIT:
      fprintf(stderr, "exit\n"); break;
    case IPT_CALL:
      fprintf(stderr, "call\n"); break;
    case IPT_NONE:      
    default:
      fprintf(stderr, "??? (%i)\n", pts[i]->type()); break;
    }
  }
  */

  /* first instPoint is not an entry point */
  if (pts[0]->type() != IPT_ENTRY) {
    //fprintf(stderr, "!! first inst pt not entry\n");
#ifdef CSS_DEBUG_INST
    UNINSTR("1st inst pt not entry");
    print_inst_pts(pts, this);
#endif
    return false;
  }

  for (unsigned i = 0; i < pts.size() - 1; i++) {
    instPoint *p = pts[i];
    instPoint *p2 = pts[i+1];

    /* check for overlapping instPoints */
    if (p2->offset() < p->offset() + p->size()) {
      relocatable_ = true;
      isTrap = true; // alias for relocatable_ (TODO)
      p2->flags |= IP_Overlap;
    }
  }

#ifdef CSS_DEBUG_INST
  if (relocatable_) {
    fprintf(stderr, "relocate: %s (%0#10x: %i insns)\n",
	    prettyName().string_of(), getAddress(0),
	    size() / INSN_SIZE);
    print_inst_pts(pts, this);
  }
#endif


  // TODO: function relocation not yet implemented
  if (relocatable_) {
    return false;
  }
  return true;
}

/* checkCallPoints():
 * Determine if callee is a "library" or "user" function.  
 * This cannot be done until all functions have been seen.  
 */
void pd_Function::checkCallPoints() 
{
  //fprintf(stderr, ">>> pd_Function::checkCallPoints()\n");
#ifdef CSS_DEBUG_INST
  fprintf(stderr, ">>> %s(%0#10x: %u insns)\n", 
	  prettyName().string_of(), getAddress(0), size() / INSN_SIZE);
#endif
  //fprintf(stderr, "%0#10x: %s(%u insns)\n", 
  //file()->exec()->getObject().get_base_addr() + getAddress(0), 
  //prettyName().string_of(), 
  //size() / INSN_SIZE);
  
  Address fnStart = getAddress(0);
  vector<instPoint*> calls2;
  for (unsigned i = 0; i < calls.size(); i++) {
    instPoint *ip = calls[i];
    assert(ip);
    
    Address tgt_addr = findTarget(ip);
    if (tgt_addr) {
      pd_Function *tgt_fn = file_->exec()->findFunction(tgt_addr);
      ip->setCallee(tgt_fn); // possibly NULL

      /*
      if (!tgt_fn) 
	fprintf(stderr, ">>> difficult <%s:%0#10x>: bogus target calculation\n", 
		prettyName().string_of(), 
		ip->iPgetOwner()->getObject().get_base_addr() + ip->address(0));
      */      

      // NOTE: (target == fnStart) => optimized recursive call
      if (!tgt_fn && tgt_addr > fnStart && tgt_addr < fnStart + size()) {
	// target is inside same function (i.e. branch)
	//fprintf(stderr, "!!! internal call target (%s,%0#10x): 0x%08x\n",
	//prettyName().string_of(), ip->address(), target);	  
	delete ip;
	continue;
      }

#ifdef CSS_DEBUG_INST
      Address objBase = ip->iPgetOwner()->getObject().get_base_addr();
      fprintf(stderr, "0x%08x: call ", objBase + ip->iPgetAddress());
      if (tgt_fn) fprintf(stderr, "\"%s\"\n", tgt_fn->prettyName().string_of());
      else fprintf(stderr, "<unknown>\n");
    } else {
      fprintf(stderr, "0x%08x: call ", ip->iPgetAddress());
      if (ip->hint_) fprintf(stderr, "(GOT:0x%08x)\n", 
			    file_->exec()->getObject().get_base_addr() + ip->hint_);
      else fprintf(stderr, "<unknown>\n");
#endif      
    }
     
    /*
    if (!tgt_addr && !ip->hint_) 
      fprintf(stderr, ">>> difficult <%s:%0#10x>: target unknown\n", 
	      prettyName().string_of(), 
	      ip->iPgetOwner()->getObject().get_base_addr() + ip->address(0));
    */      

    /* fallthrough cases (call sites are kept):
     * - target is unknown (trap)
     * - target is external, but callee function cannot be found
     */
    calls2 += ip;
  }
  calls = calls2;
  setIDs();
}

/* findTarget(): calculate target of call point
 * return of 0 means unknown 
 */
Address pd_Function::findTarget(instPoint *p)
{
  //fprintf(stderr, ">>> pd_Function::findTarget()\n");
  assert(p->type() == IPT_CALL);
  Address ret = 0;
  instruction i;
  i.raw = p->code();

  if (isBranchInsn(i)) {
    ret = findBranchTarget(p, i);
  } else if (isJumpInsn(i)) {
    ret = findJumpTarget(p, i);
  } else if (isTrapInsn(i)) {
    fprintf(stderr, "!!! pd_Function::findTarget(): trap insn\n");
    assert(0); // not seen yet
  } else {
    fprintf(stderr, "!!! pd_Function::findTarget(): unknown call insn (0x%08x)\n", i.raw);
    assert(0); // hopefully not reached
  }

  return ret;
}

Address pd_Function::findBranchTarget(instPoint *p, instruction i)
{
  // PC-relative branch
  Address base = p->address() + INSN_SIZE;
  signed off = i.itype.simm16 << 2;
  Address ret = base + off;
  //fprintf(stderr, ">>> pd_Function::findBranchTarget() => 0x%08x\n", ret);
  return ret;
}

Address pd_Function::findJumpTarget(instPoint *p, instruction i)
{
  Address ret = 0;
  //fprintf(stderr, ">>> pd_Function::findJumpTarget():");
  unsigned opcode = i.decode.op;
  switch (opcode) {
  case Jop:
  case JALop:
    { // PC-region branch
      Address hi = (p->address() + INSN_SIZE) & (REGION_NUM_MASK);
      Address lo = (i.jtype.imm26 << 2) & REGION_OFF_MASK;
      ret = hi | lo;
    } break;
  case SPECIALop: 
    // indirect (register) jump
    ret = findIndirectJumpTarget(p, i);
    break;
  default:
    fprintf(stderr, "pd_Function::findJumpTarget(): bogus instruction %0#10x\n", i.raw);
    assert(0);
  }
  return ret;
}

void print_sequence(int targetReg, vector<int> &baseAdjusts,
		    vector<int> &adjusts, int n, char * /*pre*/ = NULL)
{
  if ((unsigned)n == baseAdjusts.size()) {
    fprintf(stderr, "%s", reg_names[targetReg]);
    return;
  }

  fprintf(stderr, "[");
  print_sequence(targetReg, baseAdjusts, adjusts, n+1);
  if (baseAdjusts[n] != 0) fprintf(stderr, "%+i", baseAdjusts[n]);
  fprintf(stderr, "]");
  if (adjusts[n] != 0) fprintf(stderr, "%+i", adjusts[n]);
}

unsigned get_word(const Object &elf, Address a)
{
  unsigned ret = 0;
  if (a >= elf.code_off() && a < elf.code_off() + (elf.code_len() << 2)) {
    // TODO: const_cast problem?
    char *base = (char *)const_cast<Word*>(elf.code_ptr());
    char *ptr = base + (a - elf.code_off());
    ret = *(unsigned *)ptr;
    //fprintf(stderr, ">>> get_word(%0#10x): code    => %0#10x\n", a, ret);
  } else if (a >= elf.data_off() && a < elf.data_off() + (elf.data_len() << 2)) {
    // TODO: const_cast problem?
    char *base = (char *)const_cast<Word*>(elf.data_ptr());
    char *ptr = base + (a - elf.data_off());
    ret = *(unsigned *)ptr;
    //fprintf(stderr, ">>> get_word(%0#10x): data    => %0#10x\n", a, ret);
  } else {
    //fprintf(stderr, "!!! get_word(%0#10x): unknown => %0#10x\n", a, ret);
  }
  return ret;
}

Address pd_Function::findIndirectJumpTarget(instPoint *ip, instruction i)
{
  //fprintf(stderr, ">>> pd_Function::findIndirectJumpTarget(%s,%0#10x)",
  //prettyName().string_of(), ip->address());

  assert(i.rtype.op == SPECIALop);
  assert(i.rtype.ops == JALRops || i.rtype.ops == JRops);
  
  // look for the following instruction sequence(s):
  // lw        R1,XX(gp)
  // [daddiu]  R1,R1,XX
  // j[al]r    R2,R1

  // TODO: need control flow info
  // TODO: internal indirect jump (switch)

  // indirect jump sequence
  Register targetReg = i.rtype.rs;
  vector<int> baseRegs;
  vector<int> baseAdjusts;
  vector<int> adjusts;
  vector<unsigned int> insns;
  vector<Address> insnAddrs;

  // parse code
  Address start = getAddress(0);
  image *owner = file_->exec();
  int adjust = 0;
  instruction i2;
  // indirect jump insn (debug)
  insns += i.raw;
  insnAddrs += ip->offset() + start;
  int off; // must be signed
  // start at "ip+4" to parse delay slot insn
  for (off = ip->offset() + INSN_SIZE; off >= 0; off -= INSN_SIZE) {
    // TODO: is this valid?
    if (targetReg == REG_GP) break;
    if (targetReg == REG_SP) break;

    i2.raw = owner->get_instruction(start+off);

    // indirect jump sequence:
    //   lw      R1,X(gp)
    // * addiu   R1,R1,X
    // * daddiu  R1,R1,X
    // * lw      R2,X(R1)
    // * move    R3,R2
    //   jr      R3
    //   (* - zero or more)
 
    // move R2,R1
    if (isInsnType(i2, ORmask, ORmatch) &&
	i2.rtype.rt == REG_ZERO &&
	i2.rtype.rd == targetReg) 
      {
	targetReg = i2.rtype.rs;
	// debug
	insns += i2.raw;
	insnAddrs += start+off;
      }
    // daddiu R1,R1,X
    if (isInsnType(i2, ADDIUmask, ADDIUmatch) &&
	     i2.itype.rs == targetReg &&
	     i2.itype.rt == targetReg) 
      {
	adjust += i2.itype.simm16;
	// debug
	insns += i2.raw;
	insnAddrs += start+off;
      }
    // daddiu R1,R1,X
    if (isInsnType(i2, DADDIUmask, DADDIUmatch) &&
	     i2.itype.rs == targetReg &&
	     i2.itype.rt == targetReg) 
      {
	adjust += i2.itype.simm16;
	// debug
	insns += i2.raw;
	insnAddrs += start+off;
      }
    // lw R2,X(R1)
    if (isInsnType(i2, LWmask, LWmatch) &&
	     i2.itype.rt == targetReg) 
      {
	baseRegs += (int)i2.itype.rs;
	baseAdjusts += (int)i2.itype.simm16;
	adjusts += adjust;
	adjust = 0;
	targetReg = i2.itype.rs;
	// debug
	insns += i2.raw;
	insnAddrs += start+off;
      }
  }

  // sanity check
  unsigned int n_loads = baseRegs.size();
  if (n_loads < 1) {
    //fprintf(stderr, "!!! no loads in indirect jump (%s,%0#10x)\n",
    //prettyName().string_of(), ip->address());
    //fprintf(stderr, ": XXX no loads\n");
    return 0;
  }
  //assert(n_loads > 0); // jump to $a1
  assert(baseAdjusts.size() == n_loads);
  assert(adjusts.size() == n_loads);
  //if (adjust != 0) fprintf(stderr, "assert(adjust == 0) failed\n");
  //assert(adjust == 0);

  // debug: target arithmetic
  //fprintf(stderr, "  arithmetic");
  //fprintf(stderr, ": ");
  //print_sequence(targetReg, baseAdjusts, adjusts, 0, NULL);
  //fprintf(stderr, "\n");

  // debug: insn sequence
  instruction i3;
  for (int i = insns.size()-1; i >= 0; i--) {
    i3.raw = insns[i];
    //dis(&i3, (void *)insnAddrs[i], 1, "  ");
  }

  // check base register
  Address target;
  const Object &elf = owner->getObject();
  switch(targetReg) {
  case REG_GP:
    target = elf.get_gp_value();
    break;
  case REG_T9:
  default:
    // base register is dynamic
    //fprintf(stderr, "!!! bogus base register $%s (%s,%0#10x)\n",
    //reg_names[targetReg], prettyName().string_of(), ip->address());
    return 0;
  }

  Address obj_base = elf.get_base_addr();

  // special case: external symbol call via GOT entry
  // TODO: size of address in GOT (64- vs 32-bit)
  if (baseRegs.size() == 1 && 
      targetReg == REG_GP &&
      adjusts[0] == 0) 
    {
      Address got_entry_off = target + baseAdjusts[0] - obj_base;
      Address got_entry = (Address)get_word(elf, got_entry_off);
      Address fn_off = got_entry - obj_base;
      pd_Function *pdf = owner->findFunction(fn_off);
      if (pdf) return fn_off;
      // external symbol
      ip->hint_ = got_entry_off;
      return 0;
    }
    
  // calculate jump target
  // TODO: size of address in GOT (64- vs 32-bit)
  for (int i = baseRegs.size()-1; i >= 0; i--) {
    Address vaddr = target + baseAdjusts[i];
    unsigned word = get_word(elf, vaddr - obj_base);
    target = word + adjusts[i];
  }
  target -= obj_base; // relative addressing
  
  return target;
}

bool doNotOverflow(int value)
{
  //fprintf(stderr, ">>> doNotOverflow()\n");
  if (value >= MIN_IMM16 && value <= MAX_IMM16) return true;
  return false;
}

void generateNoOp(process *proc, Address addr) {
  //fprintf(stderr, ">>> generateNoOp()\n");
  proc->writeTextWord((caddr_t)addr, NOP_INSN);
}

bool branchWithinRange(Address branch, Address target)
{
  //fprintf(stderr, ">>> branchWithinRange(0x%08x,0x%08x)\n", branch, target);
  Address slot = branch + INSN_SIZE; // delay slot insn

  // PC-region jump
  if (region_num(slot) == region_num(target)) return true;  

  // PC-relative branch
  RegValue offset = target - slot;
  if (offset >= BRANCH_MIN && offset <= BRANCH_MAX) return true;

  return false;
}

/* generateBranch():
   called by inst.C for instrumentation jumps
   - instPoint to basetramp
   - basetramp to minitramp
   - minitramp to minitramp
   - minitramp to basetramp
   - basetramp to instPoint
*/
void generateBranch(process *proc, Address branch, Address target)
{
  //fprintf(stderr, "!!! generateBranch(): %0#10x to %0#10x", branch, target);
  assert(isAligned(branch));
  assert(isAligned(target));

  instruction i;

  Address slot = branch + INSN_SIZE; // address of delay slot insn
  if (region_num(slot) == region_num(target)) {
    // same 256MB region: direct jump
    //fprintf(stderr, " (PC-region branch)\n");
    i.jtype.op = Jop;
    i.jtype.imm26 = (target & REGION_OFF_MASK) >> 2;
  } else {
    RegValue offset = target - slot;
    if (offset >= BRANCH_MIN && offset <= BRANCH_MAX) {
      // within 18-bit offset: branch
      //fprintf(stderr, " (PC-relative branch)\n");
      genItype(&i, BEQop, REG_ZERO, REG_ZERO, offset >> 2);
    } else {
      // out of range: indirect jump (yuck)
      fprintf(stderr, " (indirect branch: out of range)\n");
      assert(0); // TODO: implement
    }
  }
 
  // TODO: delay slot insn?
  proc->writeTextWord((caddr_t)branch, i.raw);
}


void genRtype(instruction *insn, int ops, reg rs, reg rt, 
	      reg rd, int sa) 
{
  struct fmt_rtype *i = &insn->rtype;
  i->op = SPECIALop;
  i->rs = rs;
  i->rt = rt;
  i->rd = rd;
  i->sa = sa;
  i->ops = ops;
}
void genItype(instruction *insn, int op, reg rs, reg rt, signed short imm)
{
  struct fmt_itype *i = &insn->itype;
  i->op = op;
  i->rs = rs;
  i->rt = rt;
  i->simm16 = imm;
}
void genJtype(instruction *insn, int op, unsigned imm)
{
  struct fmt_jtype *i = &insn->jtype;
  i->op = op;
  i->imm26 = imm;
}
void genBranch(instruction *insn, Address branch, Address target) {
  Address slot = branch + INSN_SIZE;
  RegValue disp_ = (target - slot) >> 2;
  assert(disp_ <= MAX_IMM16 && disp_ >= MIN_IMM16);
  signed short disp = (signed short)disp_;
  genItype(insn, BEQop, REG_ZERO, REG_ZERO, disp);
}
bool genJump(instruction *insn, Address branch, Address target) {
  Address slot = branch + INSN_SIZE;
  // try PC-region branch
  if (region_num(slot) == region_num(target)) {
    genJtype(insn, Jop, (target & REGION_OFF_MASK) >> 2);
    return true;
  }
  // try PC-relative branch
  RegValue disp_ = (target - slot) >> 2;
  if (disp_ <= MAX_IMM16 && disp_ >= MIN_IMM16) {
    genBranch(insn, branch, target);
    return true;
  }
  // one-insn branch failed
  assert(false); // TODO
  return false;
}
void genNop(instruction *insn) {
  insn->raw = NOP_INSN;
}
void genTrap(instruction *insn) {
  insn->raw = TRAP_INSN;
}
void genIll(instruction *insn) {
  insn->raw = ILLEGAL_INSN;
}
// "mov rd,rs" emitted as "or rd,rs,r0"
void genMove(instruction *insn, reg rs, reg rd) {
  genRtype(insn, ORops, rs, REG_ZERO, rd);
}
/* set "rd" on "(rs <op> 0)" */
void genZeroCC(instruction *i, opCode op, reg rs, reg rd, int &ninsns)
{
  switch (op) {

  case greaterOp:
    genItype(i, BGTZLop, rs, 0, 0x2);
    break;

  case leOp:
    genItype(i, BLEZLop, rs, 0, 0x2);
    break;

  case geOp:
    genItype(i, REGIMMop, rs, BGEZLopr, 0x2);
    break;

  case eqOp:
    genItype(i, BEQLop, rs, REG_ZERO, 0x2);
    break;

  case neOp:
    genItype(i, BNELop, rs, REG_ZERO, 0x2);
    break;

  case lessOp: // should never be called
  default:
    assert(0);
  }

  genItype(i+1, ORIop, REG_ZERO, rd, 0x1);
  genItype(i+2, ORIop, REG_ZERO, rd, 0x0);

  ninsns = 1 + 1 + 1; // branch + set + set
}

#define IMM_NBITS ((Address)0x10)
#define IMM_MASK  ((Address)0xffff)
// next two must be same type as getImmField() return
#define IMM_ZERO  ((UnsignedImm)0)
#define IMM_ONES  ((UnsignedImm)~(UnsignedImm)0)
#define bit(n,v) ((v >> n) & 0x1)
UnsignedImm getImmField(Address val, int n)
{
  Address offset = n * IMM_NBITS;
  Address mask = IMM_MASK << offset;
  UnsignedImm ret = (val & mask) >> offset;
  return ret;
}

void genLoadNegConst(reg dst, Address imm, char *code, Address &base, bool /*noCost*/)
{
  //fprintf(stderr, ">>> genLoadNegConst(%0#18llx)\n", imm);
  Address base_orig = base; // debug

  int i;
  int nbits = sizeof(Address) * 8; // N-bit registers
  int nimm = (nbits/IMM_NBITS); // # of immediate (16-bit) fields

  int zerobit; // highest significance zero bit (field #)
  for (zerobit = nimm - 1; zerobit > 0; zerobit--) {
    if (getImmField(imm, zerobit) != IMM_ONES) break;
  }
  int signbit = zerobit; // lowest significance sign bit (field #)
  // check if zero bit occurred in highest bit of a 16-bit field
  // if so, the previous field is the one with the last sign bit
  UnsignedImm zerofield = getImmField(imm, zerobit);
  if (!bit(IMM_NBITS-1, zerofield)) signbit++;
  assert(signbit < nimm);
  
  instruction *insn = (instruction *)(code + base);
  switch (signbit) {
  case 0:
    genItype(insn, DADDIop, REG_ZERO, dst, imm);
    base += INSN_SIZE;
    break;
  default:
    genItype(insn, LUIop, 0, dst, getImmField(imm, signbit));
    base += INSN_SIZE;
    if (getImmField(imm, signbit-1) != 0) {
      genItype(insn+1, ORIop, dst, dst, getImmField(imm, signbit-1));
      base += INSN_SIZE;
    }
    for (i = signbit-2; i >= 0; i--) {
      insn = (instruction *)(code + base);
      genRtype(insn, DSLLops, 0, dst, dst, IMM_NBITS);
      base += INSN_SIZE;
      if (getImmField(imm, i) != 0) {
	genItype(insn+1, ORIop, dst, dst, getImmField(imm, i));
	base += INSN_SIZE;
      }
    }
  }

  // debug
  for (Address i = base_orig; i < base; i += INSN_SIZE) {
    //dis((instruction *)(code + i), NULL, 1, "  ");
  }
}

void genLoadConst(reg dst, RegValue imm, char *code, Address &base, bool noCost)
{
  // if negative, use genLoadNegConst()
  if (imm < 0) return genLoadNegConst(dst, (Address)imm, code, base, noCost);

  //fprintf(stderr, ">>> genLoadConst(0x%lx)\n", imm);
  Address base_orig = base; // debug

  int nbits = sizeof(RegValue) * 8; // N-bit addresses
  int nimm = (nbits/IMM_NBITS); // # of immediate (16-bit) fields
  int nonzero = 0; // highest significance nonzero field
  for (nonzero = nimm - 1; nonzero > 0; nonzero--) {
    if (getImmField(imm, nonzero) != IMM_ZERO) break;
  }

  instruction *insn = (instruction *)(code + base);
  switch (nonzero) {
  case 0:
    if (imm & 0x8000) genItype(insn, ORIop, REG_ZERO, dst, imm);
    else genItype(insn, ADDIUop, REG_ZERO, dst, imm);
    base += INSN_SIZE;
    break;
  default:
    genItype(insn, LUIop, 0, dst, getImmField(imm, nonzero));
    base += INSN_SIZE;
    if (getImmField(imm, nonzero-1) != IMM_ZERO) {
      genItype(insn+1, ORIop, dst, dst, getImmField(imm, nonzero-1));
      base += INSN_SIZE;
    }
    for (int i = nonzero-2; i >= 0; i--) {
      insn = (instruction *)(code + base);
      genRtype(insn, DSLLops, 0, dst, dst, IMM_NBITS);
      base += INSN_SIZE;
      if (getImmField(imm, i) != IMM_ZERO) {
	genItype(insn+1, ORIop, dst, dst, getImmField(imm, i));
	base += INSN_SIZE;
      }
    }
  }

  // debug
  for (Address i = base_orig; i < base; i += INSN_SIZE) {
    //dis((instruction *)(code + i), NULL, 1, "  ");
  }
}



// HERE BE DRAGONS



// return Address
// [ifOp, branchOp, trampPreamble, trampTrailer]
// TODO: "dst" is a Register, should be RegValue (holds branch offset)
Address emitA(opCode op, Register src1, Register /*src2*/, Register dst, 
	      char *code, Address &base, bool /*noCost*/)
{
  Address ret = 0;
  instruction *insn = (instruction *)(code + base);
  RegValue word_off_;
  SignedImm word_off;

  switch (op) {

  case ifOp:
    // "src1"     : condition register
    // "dst"      : branch target offset (bytes)
    // return val : branch insn offset (bytes)
    // TODO: nonzero conditon is true?
    //fprintf(stderr, ">>> emit(ifOp)\n"); 
    // BEQ offset is relative to delay slot and has word units
    // zero offsets are often used for dummy calls (padding)
    word_off_ = (dst) ? ((dst - INSN_SIZE) >> 2) : (0);
    assert(word_off_ <= MAX_IMM16);
    word_off = (SignedImm)word_off_;
    ret = base;
    genItype(insn, BEQop, src1, REG_ZERO, word_off);
    genNop(insn+1);
    base += 2*INSN_SIZE;
    break;

  case branchOp:
    // "dst"      : branch target offset (bytes)
    // return val : branch insn offset (bytes)
    //fprintf(stderr, ">>> emit(branchOp)\n");
    // BEQ offset is relative to delay slot and has word units
    // zero offsets are often used for dummy calls (padding)
    word_off_ = (dst) ? ((dst - INSN_SIZE) >> 2) : (0);
    assert(word_off_ <= MAX_IMM16);
    word_off = (SignedImm)word_off_;
    ret = base;
    genItype(insn, BEQop, REG_ZERO, REG_ZERO, word_off);
    genNop(insn+1);
    base += 2*INSN_SIZE;
    break;

  case trampPreamble:
    //fprintf(stderr, ">>> emit(trampPreamble)\n"); 
    // no trampoline preamble for this platform
    break;

  case trampTrailer:
    // return = offset (in bytes) of branch insn
    // TODO: instInstance::returnAddr stores return value
    //       offset of branch insn could change (nops padded in front)
    //fprintf(stderr, ">>> emit(trampTrailer)\n");
    // allocate enough space for indirect jump (just in case)
    ret = base;
    for (int i = 0; i < 8; i++) genNop(insn+i);
    base += 8*INSN_SIZE;
    break;

  default:
    assert(0);
  }

  return ret;
}

// return Register
// [getParamOp, getSysParamOp, getRetValOp, getSysRetValOp]
Register emitR(opCode op, Register src1, Register /*src2*/, Register dst, 
	       char *code, Address &base, bool /*noCost*/)
{
  Register ret = REG_ZERO;
  int frame_off = 0;

  switch (op) {

  case getParamOp:
    // "src1"     : argument number [0,7]
    // "dst"      : allocated return register
    // return val : argument register
    // TODO: extract >8 parameters from stack (need to know stack frame size)
    //fprintf(stderr, ">>> emit(getParamOp): %i\n", src1);
    assert(src1 < 8);
    ret = dst;
    frame_off = 216 - (BYTES_PER_ARG * src1); // see tramp-mips.S
    genItype((instruction *)(code + base), LDop, REG_SP, ret, frame_off);
    base += INSN_SIZE;
    break;

  case getSysParamOp:
    // "src1"     : argument number [0,7]
    // "dst"      : allocated return register
    // return val : argument register
    // TODO: extract >8 parameters from stack (need to know stack frame size)
    //fprintf(stderr, ">>> emit(getSysParamOp)\n");
    assert(src1 < 8);
    ret = dst;
    frame_off = 216 - (BYTES_PER_ARG * src1); // see tramp-mips.S
    genItype((instruction *)(code + base), LDop, REG_SP, ret, frame_off);
    base += INSN_SIZE;
    break;

  case getRetValOp:
    // "dst"      : allocated return register
    // return val : return value register
    //fprintf(stderr, ">>> emit(getRetValOp)\n");
    ret = dst;
    frame_off = 232; // see tramp-mips.S
    genItype((instruction *)(code + base), LDop, REG_SP, ret, frame_off);
    base += INSN_SIZE;
    break;

  case getSysRetValOp:
    // "dst"      : allocated return register
    // return val : return value register
    //fprintf(stderr, ">>> emit(getSysRetValOp)\n");
    ret = dst;
    frame_off = 232; // see tramp-mips.S
    genItype((instruction *)(code + base), LDop, REG_SP, ret, frame_off);
    base += INSN_SIZE;
    break;

  default:
    assert(0);
  }

  return ret;
}

// return void
// [loadIndirOp, storeIndirOp, noOp, saveRegOp,
//  plusOp, minusOp, timesOp, divOp,
//  orOp, andOp, eqOp, neOp, lessOp, leOp, greaterOp, geOp]
void emitV(opCode op, Register src1, Register src2, Register dst, 
	   char *code, Address &base, bool /*noCost*/)
{
  instruction *insn = (instruction *)(code + base);

  switch (op) {

  case noOp:
    //fprintf(stderr, ">>> emit(noOp)\n");
    genNop(insn);
    base += INSN_SIZE;
    break;

  case saveRegOp:
    // not used on this platform
    fprintf(stderr, "!!! emit(saveRegOp): should never be called\n");
    assert(0);

    /* memory operators */

  case loadIndirOp:
    // TODO: 32/64-bit value
    // "src1"     : address register (from)
    // "dst"      : value register (to)
    //fprintf(stderr, ">>> emit(loadIndirOp)\n"); 
    genItype(insn, LWop, src1, dst, 0);
    base += INSN_SIZE;
    break;
  case storeIndirOp:
    // TODO: 32/64-bit value
    // "src1"     : value register (from)
    // "dst"      : address register (to)
    //fprintf(stderr, ">>> emit(storeIndirOp)\n"); 
    genItype(insn, SWop, dst, src1, 0);
    base += INSN_SIZE;
    break;

    /* arithmetic operators */

  case plusOp:
    //fprintf(stderr, ">>> emit(plusOp)\n");
    genRtype(insn, ADDUops, src1, src2, dst);
    base += INSN_SIZE;
    break;
  case minusOp:
    //fprintf(stderr, ">>> emit(minusOp)\n");
    genRtype(insn, SUBUops, src1, src2, dst);
    base += INSN_SIZE;
    break;
  case timesOp:
    /* multiply ("mul rd,rs,imm") = four instructions
       mult  rs,rd   # (HI,LO) <- rs * rd
       mflo  rd      # rd <- LO
       nop           # padding for "mflo"
       nop           # padding for "mflo"
    */
    //fprintf(stderr, ">>> emit(timesOp)\n");
    genRtype(insn, MULTops, src1, src2, 0);
    genRtype(insn+1, MFLOops, 0, 0, dst);
    genNop(insn+2);
    genNop(insn+3);
    base += 4 * INSN_SIZE;
    break;
  case divOp:
    /* divide ("div rd,rs,imm") = four instructions
       div   rs,rd   # (HI,LO) <- rs / rd
       mflo  rd      # rd <- LO
       nop           # padding for "mflo"
       nop           # padding for "mflo"
    */
    //fprintf(stderr, ">>> emit(divOp)\n");
    genRtype(insn, DIVops, src1, src2, 0);
    genRtype(insn+1, MFLOops, 0, 0, dst);
    genNop(insn+2);
    genNop(insn+3);
    base += 4 * INSN_SIZE;
    break;

    /* relational operators */

  case lessOp:
    //fprintf(stderr, ">>> emit(lessOp)\n");
    genRtype(insn, SLTUops, src1, src2, dst);
    base += INSN_SIZE;
    break;
  case greaterOp:
    //fprintf(stderr, ">>> emit(greaterOp)\n");
    genRtype(insn, SLTUops, src2, src1, dst);
    base += INSN_SIZE;
    break;    
  case leOp:
    //fprintf(stderr, ">>> emit(leOp)\n");
    genItype(insn, BEQLop, src1, src2, 0x2);
    genItype(insn+1, ORIop, REG_ZERO, dst, 0x1);
    genRtype(insn+2, SLTUops, src1, src2, dst);
    base += 3 * INSN_SIZE;
    break;
  case geOp:
    //fprintf(stderr, ">>> emit(geOp)\n");
    genItype(insn, BEQLop, src1, src2, 0x2);
    genItype(insn+1, ORIop, REG_ZERO, dst, 0x1);
    genRtype(insn+2, SLTUops, src2, src1, dst);
    base += 3 * INSN_SIZE;
    break;
  case eqOp:
    //fprintf(stderr, ">>> emit(eqOp)\n");
    genItype(insn, BEQLop, src1, src2, 0x2);
    genItype(insn+1, ORIop, REG_ZERO, dst, 0x1);
    genItype(insn+2, ORIop, REG_ZERO, dst, 0x0);
    base += 3 * INSN_SIZE;
    break;
  case neOp:
    //fprintf(stderr, ">>> emit(neOp)\n");
    genItype(insn, BNELop, src1, src2, 0x2);
    genItype(insn+1, ORIop, REG_ZERO, dst, 0x1);
    genItype(insn+2, ORIop, REG_ZERO, dst, 0x0);
    base += 3 * INSN_SIZE;
    break;

    /* boolean operators */

  case orOp:
    //fprintf(stderr, ">>> emit(orOp)\n");
    genRtype(insn, ORops, src1, src2, dst);
    base += INSN_SIZE;
    break;
  case andOp:
    //fprintf(stderr, ">>> emit(andOp)\n");
    genRtype(insn, ANDops, src1, src2, dst);
    base += INSN_SIZE;
    break;


  default:
    fprintf(stderr, "!!! illegal operator %i emitted\n", op);
    assert(0);
  }
}

// [loadConstOp, loadOp]
void emitVload(opCode op, Address src1, Register /*src2*/, Register dst, 
	       char *code, Address &base, bool noCost)
{
  switch (op) {

  case loadConstOp:
    // "src1" : constant value to load
    //fprintf(stderr, ">>> emit(loadConstOp)\n");
    genLoadConst(dst, src1, code, base, noCost);
    break;

  case loadOp:
    // TODO: 32/64-bit value
    // "src1" : address value (from)
    // "dst"  : value register (to)
    //fprintf(stderr, ">>> emit(loadOp)\n");
    genLoadConst(dst, src1, code, base, noCost);
    emitV(loadIndirOp, dst, 0, dst, code, base, noCost);
    break;

  default: 
    assert(0);
  }
}

// [storeOp]
void emitVstore(opCode op, Register src1, Register src2, Address dst, 
                char *code, Address &base, bool noCost)
{
  assert(op == storeOp);
  // "src1" : value register (from)
  // "src2" : scratch address register (to)
  // "dst"  : address value (to)
  //fprintf(stderr, ">>> emit(storeOp)\n");
  genLoadConst(src2, dst, code, base, noCost);
  emitV(storeIndirOp, src1, 0, src2, code, base, noCost);
}

// [updateCostOp]
void emitVupdate(opCode op, RegValue src1, Register /*src2*/, Address dst, 
		 char *code, Address &base, bool noCost)
{
  //fprintf(stderr, ">>> emit(updateCostOp)\n");
  //Address base_orig = base; // debug
  assert(op == updateCostOp);

  if (!noCost) {
    Address cost_addr = dst;
    Address cost_update = src1;
    /* (15 insns max)
       set     r1,cost_addr (6 insns max)
       lw      r3,r1
       set     r2,cost_update (6 insns max)
       daddu   r2,r2,r3
       sw      r2,r1
       (stomps three registers)
    */
    Register r1 = regSpace->allocateRegister(code, base, noCost); // cost address
    Register r3 = regSpace->allocateRegister(code, base, noCost); // cost
    genLoadConst(r1, cost_addr, code, base, noCost);
    // NOTE: 32-bit value, see obsCostLow and processCost()
    genItype((instruction *)(code + base), LWop, r1, r3, 0);
    base += INSN_SIZE;
    if (doNotOverflow(cost_update)) {
      emitImm(plusOp, r3, cost_update, r3, code, base, noCost);
    } else {
      Register r2 = regSpace->allocateRegister(code, base, noCost); // cost update
      genLoadConst(r2, cost_update, code, base, noCost);
      emitV(plusOp, r2, r3, r3, code, base, noCost);
      regSpace->freeRegister(r2);
    }
    genItype((instruction *)(code + base), SWop, r1, r3, 0);
    base += INSN_SIZE;
    regSpace->freeRegister(r3);
    regSpace->freeRegister(r1);
  }

  // debug
  //int ninsns = (base - base_orig) / INSN_SIZE;
  //fprintf(stderr, "  updateCostOp code (%i insns):\n", ninsns);
  //dis(code + base_orig, NULL, ninsns, "  ");
}


/* emitImm(): This function is complicated because of the MIPS RISC
   architecture.  Specifically, the only immediate instruction
   primitives are "add" and "set less than". 
*/
// TODO - signed operations
// TODO - immediate insns use 32- or 64-bit operands?
void emitImm(opCode op, Register src, RegValue imm, Register dst, 
	     char *code, Address &base, bool noCost)
{
  //fprintf(stderr, ">>> emitImm(): op %s,%s,%i\n", reg_names[dst], reg_names[src], imm);
  instruction *insn = (instruction *)(code + base);
  int n;

  // immediate value should fit in immediate field
  switch (op) {
    
    /* arithmetic operands */
    
  case plusOp:
    if (!doNotOverflow(imm)) break;
    genItype(insn, ADDIUop, src, dst, imm);
    base += INSN_SIZE;
    return;
  case minusOp:
    if (!doNotOverflow(-imm)) break;
    genItype(insn, ADDIUop, src, dst, -imm);
    base += INSN_SIZE;
    return;
  case timesOp:
    if (!isPowerOf2(imm, n) || (n >= 64)) break;
    // use left shift for powers of 2
    if (n < 32) genRtype(insn, DSLLops, 0, src, dst, n);
    else genRtype(insn, DSLL32ops, 0, src, dst, n-32);
    base += INSN_SIZE;
    return;
  case divOp:
    if (!isPowerOf2(imm, n) || n >= 64) break;
    // use right shift for powers of 2
    if (n < 32) genRtype(insn, DSRAops, 0, src, dst, n);
    else genRtype(insn, DSRA32ops, 0, src, dst, n-32);
    base += INSN_SIZE;
    return;
    
    /* relational operands */
    
  case lessOp:
    if (!doNotOverflow(imm)) break;
    genItype(insn, SLTIUop, src, dst, imm);
    base += INSN_SIZE;
    return; 
  case eqOp:
    if (!doNotOverflow(-imm)) break;
    // saves one register over emitV()
    genItype(insn, ADDIUop, src, dst, -imm);
    genItype(insn+1, BEQLop, dst, REG_ZERO, 0x2);
    genItype(insn+2, ORIop, REG_ZERO, dst, 0x1);
    genItype(insn+3, ORIop, REG_ZERO, dst, 0x0);
    base += 4 * INSN_SIZE;
    return;
  case neOp:
    if (!doNotOverflow(-imm)) break;
    // saves one register over emitV()
    genItype(insn, ADDIUop, src, dst, -imm);
    genItype(insn+1, BNELop, dst, REG_ZERO, 0x2);
    genItype(insn+2, ORIop, REG_ZERO, dst, 0x1);
    genItype(insn+3, ORIop, REG_ZERO, dst, 0x0);
    base += 4 * INSN_SIZE;
    return;
  case greaterOp:
  case leOp:
  case geOp:
    // unsafe as immediate insns
    break;
    
    /* boolean operands */
    
  case orOp:
    if (!doNotOverflow(imm)) break;
    genItype(insn, ORIop, src, dst, imm);
    base += INSN_SIZE;
    return;
  case andOp:
    if (!doNotOverflow(imm)) break;
    genItype(insn, ANDIop, src, dst, imm);
    base += INSN_SIZE;
    return;
      
  default:
    assert(0);
  }

  // default: use the general-purpose code generator "emitV()"
  // load the "immediate" value into a register and call emitV()
  Register src2 = regSpace->allocateRegister(code, base, noCost);
  genLoadConst(src2, imm, code, base, noCost);
  emitV(op, src, src2, dst, code, base, noCost);
  regSpace->freeRegister(src2);
  return;
}

bool process::emitInferiorRPCheader(void *code_, Address &base)
{
  //fprintf(stderr, ">>> process::emitInferiorRPCheader()\n");
  char *code = (char *)code_;
  instruction *insn = (instruction *)(code + base);

  // TODO: why 512 bytes? not using basetramp code, right?
  genItype(insn, ADDIUop, REG_SP, REG_SP, -512); // addiu sp,sp,-512
  base += INSN_SIZE;

  return true;
}

// TODO: should offset parameters be Address's?
bool process::emitInferiorRPCtrailer(void *code_, Address &base,
				     unsigned &breakOffset,
				     bool stopForResult,
				     unsigned &stopForResultOffset,
				     unsigned &justAfter_stopForResultOffset)
{
  //fprintf(stderr, ">>> process::emitInferiorRPCtrailer()\n");
  char *code = (char *)code_;

  // optional code for grabbing RPC result
  if (stopForResult) {
    instruction *insn = (instruction *)(code + base);
    genTrap(insn); // trap to grab result
    stopForResultOffset = base;
    base += INSN_SIZE;
    justAfter_stopForResultOffset = base;
  }

  // mandatory RPC trailer: restore, trap, illegal
  instruction *insn = (instruction *)(code + base);
  // addiu sp,sp,512
  genItype(insn, ADDIUop, REG_SP, REG_SP, 512);
  base += INSN_SIZE;
  // trap insn
  genTrap(insn+1);
  breakOffset = base;
  base += INSN_SIZE;
  // illegal insn
  genIll(insn+2);
  base += INSN_SIZE;

  return true;
}

int getInsnCost(opCode op)
{
  //fprintf(stderr, ">>> getInsnCost()\n");
  switch(op) {

  case plusOp:
  case minusOp: 
  case timesOp:
  case divOp:
  case orOp:
  case andOp:
    return 1;

  case lessOp:
  case greaterOp:
    return 1;
  case leOp:
  case geOp:
  case eqOp:
  case neOp:
    return 3;

  case noOp:
    return 1;

  case loadConstOp:
    return 3; // average = 3.0625

  case loadOp:
  case storeOp:
    return 4; // average = 4.0625

  case ifOp:
  case branchOp:
    return 2;

  case callOp:
    // assume 2 parameters
    // mov, mov, lui, ori, jalr, nop
    return 6;

  case trampPreamble:
    return 0;

  case trampTrailer:
    // padded in case indirect jump needed
    return 8;

  case getRetValOp:
  case getSysRetValOp: 
  case getParamOp:
  case getSysParamOp:      
    return 1;

  case loadIndirOp:
  case storeIndirOp:
    return 1;

  case updateCostOp:
    // padded for two constant loads (cost value and address)
    return 15;

  case saveRegOp:
    // not used on this platform
    assert(0);
    
  default:
    assert(0);
  }
}

// baseTramp assembly code symbols
extern "C" void baseTramp();
extern "C" void baseTramp_skipPreInsn();
extern "C" void baseTramp_globalPreBranch();
extern "C" void baseTramp_localPreBranch();
extern "C" void baseTramp_localPreReturn();
extern "C" void baseTramp_updateCostInsn();
extern "C" void baseTramp_emulateInsn();
extern "C" void baseTramp_skipPostInsn();
extern "C" void baseTramp_globalPostBranch();
extern "C" void baseTramp_localPostBranch();
extern "C" void baseTramp_localPostReturn();
extern "C" void baseTramp_returnInsn();
extern "C" void baseTramp_endTramp();
void initTramps()
{
  static bool inited = false;
  if (inited) return;
  inited = true;

  // register space
  regSpace = new registerSpace(nDead, Dead, 0, NULL);
  assert(regSpace);

  // base trampoline template
  Address i, base = (Address)baseTramp;
  for (i = base; i < (Address)baseTramp_endTramp; i += INSN_SIZE) {
    Address off = i - base;
    // note: these should not be made into if..else blocks
    // (some of the label values are the same)
    if (i == (Address)baseTramp_skipPreInsn)
      baseTemplate.skipPreInsOffset = off;
    if (i == (Address)baseTramp_globalPreBranch)
      baseTemplate.globalPreOffset = off;
    if (i == (Address)baseTramp_localPreBranch)
      baseTemplate.localPreOffset = off;
    if (i == (Address)baseTramp_localPreReturn)
      baseTemplate.localPreReturnOffset = off;
    if (i == (Address)baseTramp_updateCostInsn)
      baseTemplate.updateCostOffset = off;
    if (i == (Address)baseTramp_emulateInsn)
      baseTemplate.emulateInsOffset = off;
    if (i == (Address)baseTramp_skipPostInsn)
      baseTemplate.skipPostInsOffset = off;
    if (i == (Address)baseTramp_globalPostBranch)
      baseTemplate.globalPostOffset = off;
    if (i == (Address)baseTramp_localPostBranch)
      baseTemplate.localPostOffset = off;
    if (i == (Address)baseTramp_localPostReturn)
      baseTemplate.localPostReturnOffset = off;
    if (i == (Address)baseTramp_returnInsn)
      baseTemplate.returnInsOffset = off;
  }  
  baseTemplate.trampTemp = (void *)baseTramp;
  // TODO: include endTramp insns? (2 nops)
  baseTemplate.size = (Address)baseTramp_endTramp - (Address)baseTramp;
  baseTemplate.cost = 8;           // cost if both pre- and post- skipped
  baseTemplate.prevBaseCost = 135; // cost of [global_pre_branch, update_cost)
  baseTemplate.postBaseCost = 134; // cost of [global_post_branch, return_insn)
  baseTemplate.prevInstru = false;
  baseTemplate.postInstru = false;
}

Register emitFuncCall(opCode op, registerSpace *rs, char *code, Address &base, 
		      const vector<AstNode *> &params, const string &calleeName,
		      process *p, bool noCost, const function_base *callee)
{
  //fprintf(stderr, ">>> emitFuncCall(%s)\n", calleeName.string_of());
  assert(op == callOp);  
  instruction *insn;
  Address calleeAddr = (callee) 
    ? (callee->getEffectiveAddress(p))
    : (lookup_fn(p, calleeName));
  //Address base_orig = base; // debug
  //fprintf(stderr, "  <0x%08x:%s>\n", calleeAddr, calleeName.string_of());
  
  // generate argument values
  vector<reg> args;
  for (unsigned i = 0; i < params.size(); i++) {
    args += params[i]->generateCode(p, rs, code, base, noCost, false);
  }
  
  unsigned nargs = args.size();
  bool stackArgs = (nargs > NUM_ARG_REGS) ? (true) : (false);
  int stackBytes = 0;

  // put parameters 0-7 in argument registers
  for (unsigned i = 0; i < nargs && i < NUM_ARG_REGS; i++) {
    insn = (instruction *)(code + base);
    genMove(insn, args[i], REG_A0 + i);
    base += INSN_SIZE;
    rs->freeRegister(args[i]);
  }

  // put parameters 8+ on the stack
  if (stackArgs) {
    // grow stack frame
    stackBytes = (nargs - NUM_ARG_REGS) * BYTES_PER_ARG; // 8 bytes per parameter
    insn = (instruction *)(code + base);
    genItype (insn, ADDIUop, REG_SP, REG_SP, -stackBytes);
    base += INSN_SIZE;
    /* NOTE: the size of the stack frame has been temporarily
       increased; its size is restored by the "addiu sp,sp,stackBytes"
       generated below */

    // store parameters in stack frame
    for (unsigned i = NUM_ARG_REGS; i < nargs; i++) {
      int stackOff = (i - NUM_ARG_REGS) * BYTES_PER_ARG;
      insn = (instruction *)(code + base);
      genItype(insn, SDop, REG_SP, args[i], stackOff);
      base += INSN_SIZE;
      rs->freeRegister(args[i]);
    }
  }

  // call function
  genLoadConst(REG_T9, calleeAddr, code, base, noCost);
  insn = (instruction *)(code + base);
  genRtype(insn, JALRops, REG_T9, 0, REG_RA);
  genNop(insn+1);
  base += 2*INSN_SIZE;

  // restore stack frame (if necessary)
  if (stackArgs) {
    insn = (instruction *)(code + base);
    genItype(insn, ADDIUop, REG_SP, REG_SP, stackBytes);
    base += INSN_SIZE;
  }

  // debug
  //fprintf(stderr, "  emitFuncCall code:\n");
  //dis(code+base_orig, NULL, (base-base_orig)/INSN_SIZE);

  return REG_V0;
}

void returnInstance::installReturnInstance(process *p)
{
  //fprintf(stderr, ">>> returnInstance::installReturnInstance(%0#10x, %i bytes)\n", addr_, instSeqSize);
  p->writeTextSpace((void *)addr_, instSeqSize, instructionSeq);
  //disDataSpace(p, (void *)addr_, 4, "!!! jump to basetramp: ");
}




/***********************/
/*** HERE BE DRAGONS ***/
/***********************/



void relocateInstruction(instruction * /*insn*/, Address /*origAddr*/, 
			 Address /*relocAddr*/, process * /*p*/)
{
  //fprintf(stderr, "!!! relocateInstruction(0x%08x): ", relocAddr);
  //dis(insn, origAddr);
}

trampTemplate *installBaseTramp(process *p, instPoint *&ip)
{
  //fprintf(stderr, ">>> installBaseTramp()\n");

  int btSize = baseTemplate.size; // basetramp size  
  int ipSize = ip->size_; // instPoint footprint size
  Address objAddr;
  p->getBaseAddress(ip->iPgetOwner(), objAddr);
  Address ipAddr = objAddr + ip->address();
  
  // allocate basetramp buffer (inferior)
  Address btAddr = inferiorMalloc(p, btSize, anyHeap, ipAddr);
  assert(btAddr);
  // TODO: if inferiorMalloc fails, try again w/o address constraints

  // allocate basetramp buffer (local)
  char *code = new char[btSize];
  memcpy(code, (char *)baseTemplate.trampTemp, btSize);

  // trampTemplate return value: copy template
  trampTemplate *ret = new trampTemplate(baseTemplate);
  ret->baseAddr = btAddr;
  ret->costAddr = btAddr + ret->updateCostOffset;

  
  /*** populate basetramp slots ***/
  Address toAddr, fromAddr;

  /* populate emulateInsn slot */
  //fprintf(stderr, "  instPoint footprint: %i insns\n", ipSize/INSN_SIZE);
  for (int insnOff = 0; insnOff < ipSize; insnOff += INSN_SIZE) {
    int btOff = ret->emulateInsOffset + insnOff;
    instruction *insn = (instruction *)(code + btOff);
    toAddr = btAddr + btOff;
    fromAddr = ipAddr + insnOff;
    // copy original insn and perform relocation transformation
    insn->raw = ip->owner_->get_instruction(fromAddr - objAddr);
    relocateInstruction(insn, fromAddr, toAddr, p);
  }
  
  /* populate returnInsn slot */
  instruction *returnInsn = (instruction *)(code + ret->returnInsOffset);
  fromAddr = btAddr + ret->returnInsOffset;
  toAddr = ipAddr + ipSize;
  // TODO: if can't do single-insn jump, build multi-insn jump
  genJump(returnInsn, fromAddr, toAddr);
  genNop(returnInsn+1); // delay slot

  /* populate skipPreInsn slot */
  instruction *skipPreInsn = (instruction *)(code + ret->skipPreInsOffset);
  fromAddr = btAddr + ret->skipPreInsOffset;
  toAddr = btAddr + ret->updateCostOffset;
  genBranch(skipPreInsn, fromAddr, toAddr);
  genNop(skipPreInsn+1); // delay slot

  /* populate skipPostInsn slot */
  instruction *skipPostInsn = (instruction *)(code + ret->skipPostInsOffset);
  fromAddr = btAddr + ret->skipPostInsOffset;
  toAddr = btAddr + ret->returnInsOffset;
  genBranch(skipPostInsn, fromAddr, toAddr);
  genNop(skipPostInsn+1); // delay slot

  /* implicitly populated fields (NOPs):
     - updateCostInsn
     - globalPreBranch
     - localPreBranch
     - globalPostBranch
     - localPostBranch
  */

  // debug
  //fprintf(stderr, "  base trampoline code (%i insns):\n", btSize/INSN_SIZE);
  //dis(code, btAddr, btSize/INSN_SIZE, "  ");

  // copy basetramp to application
  p->writeDataSpace((void *)btAddr, btSize, code);
  delete [] code;

  return ret;
}

trampTemplate *findAndInstallBaseTramp(process *p, 
				       instPoint *&ip,
				       returnInstance *&retInst,
				       bool /*noCost*/)
{
  //fprintf(stderr, ">>> findAndInstallBaseTramp(): "); ip->print();
  retInst = NULL;
  trampTemplate *ret = NULL;

  // check if base tramp already exists
  if (p->baseMap.find((const instPoint *)ip, ret))
    return ret;

  pd_Function *fn = (pd_Function *)ip->func_;
  if (fn->isTrapFunc()) { 
    // TODO: relocate function to instrument?
  }

  // runtime address of instPoint
  Address objAddr = 0;
  p->getBaseAddress(ip->iPgetOwner(), objAddr);
  Address ipAddr = objAddr + ip->address(p);

  ret = installBaseTramp(p, ip);

  // debug --csserra
  //char buf[1024];
  //sprintf(buf, "!!! installed basetramp (%i insns): ", (int)ret->size/INSN_SIZE);
  //ip->print(stderr, buf);
  //disDataSpace(p, (void *)ret->baseAddr, ret->size/INSN_SIZE, "  ");

  // generate jump from instPoint to basetramp
  instruction *insn = new instruction[2];
  genJump(insn, ipAddr, ret->baseAddr);
  genNop(insn+1);
  retInst = new returnInstance(insn, 2*INSN_SIZE, ipAddr, 2*INSN_SIZE);
  
  if (ret) p->baseMap[ip] = ret;
  return ret;
}

void installTramp(instInstance *inst, char *code, int codeSize)
{
  //fprintf(stderr, ">>> installTramp(%i insns)\n", codeSize/INSN_SIZE);
  // accounting
  totalMiniTramps++;
  insnGenerated += codeSize/INSN_SIZE;

  // write tramp to application space
  inst->proc->writeDataSpace((void *)inst->trampBase, codeSize, code);

  // overwrite branches for skipping instrumentation
  trampTemplate *base = inst->baseInstance;
  if (inst->when == callPreInsn && base->prevInstru == false) {
    base->cost += base->prevBaseCost;
    base->prevInstru = true;
    generateNoOp(inst->proc, base->baseAddr + base->skipPreInsOffset);
  } else if (inst->when == callPostInsn && base->postInstru == false) {
    base->cost += base->postBaseCost;
    base->postInstru = true;
    generateNoOp(inst->proc, base->baseAddr + base->skipPostInsOffset);
  }

  // debug - csserra
  //char buf[1024];
  //sprintf(buf, "!!! installed minitramp (%i insns): ", codeSize/INSN_SIZE);
  //inst->location->print(stderr, buf);
  //disDataSpace(inst->proc, (void *)inst->trampBase, codeSize/INSN_SIZE, "  ");
}

void instWaitingList::cleanUp(process *p, Address pc)
{
  fprintf(stderr, ">>> instWaitingList::cleanUp()\n");
  p->writeTextSpace((void *)pc, INSN_SIZE, &relocatedInstruction);
  p->writeTextSpace((void *)addr_, instSeqSize, instructionSeq);
}

// TODO: more flexible implementation?
// PROBLEMS: delay slot, define $t9, out of range, GOT lookup
// replaceFunctionCall(): this function has many requirements
// (1) "ip" must be a call site
// (2) "ip" must not be instrumented yet
// (3) old callee must be known
// (4) delay slot of instPoint must be a NOP insn
// (5) new callee must be within range of a JAL insn
// (6) callees must be within 16 bits (imm16) of each other
bool process::replaceFunctionCall(const instPoint *ip, 
				  const function_base *newFunc)
{
  //fprintf(stderr, ">>> process::replaceFunctionCall(%s to %s)\n",
  //(ip->callee_) ? (ip->callee_->prettyName().string_of()) : ("<unknown>"),
  //(newFunc) ? (newFunc->prettyName().string_of()) : ("<nothing>"));
  // constraints: (1) call site, (2) no basetramp
  if (ip->type() != IPT_CALL) return false;
  if (baseMap.defines(ip)) return false;

  // runtime address of instPoint
  Address pt_base  = 0;
  getBaseAddress(ip->iPgetOwner(), pt_base);
  Address pt_addr = pt_base + ip->address(this);

  // if "newFunc" is null, stomp existing call with NOP
  if (newFunc == NULL) {
    generateNoOp(this, pt_addr);
    return true;
  }

  // runtime address of new callee
  pd_Function *dst2_pdf = (pd_Function *)const_cast<function_base*>(newFunc);
  Address dst2_base = 0;
  getBaseAddress(dst2_pdf->file()->exec(), dst2_base);
  Address dst2_addr = dst2_base + dst2_pdf->getAddress(this);

  // requirements
  if (!ip->callee_) {
    fprintf(stderr, "!!! replaceFunctionCall: previous callee unknown\n");
    return false;
  }
  if (ip->delayInsn_.raw != NOP_INSN) {
      fprintf(stderr, "!!! replaceFunctionCall: delay slot already in use\n");
      return false;
  }
  if (region_num(pt_addr + INSN_SIZE) != region_num(dst2_addr)) {
    fprintf(stderr, "!!! replaceFunctionCall: new callee too far\n");
    return false;
  }

  // debug
  /*
  pd_Function *from_pdf = (pd_Function *)const_cast<function_base*>(ip->iPgetFunction());
  Address from_base = 0;
  getBaseAddress(from_pdf->file()->exec(), from_base);
  Address from_addr = from_base + from_pdf->getAddress(this);
  int from_n = from_pdf->size() / INSN_SIZE;
  fprintf(stderr, "  caller (0x%08x, %s, %i insns):\n",
	  from_addr, from_pdf->prettyName().string_of(), from_n);
  disDataSpace(this, (void *)from_addr, from_n, "  ");
  */

  // runtime address of old callee
  pd_Function *dst1_pdf = (pd_Function *)ip->callee_;
  Address dst1_base = 0;
  getBaseAddress(dst1_pdf->file()->exec(), dst1_base);
  Address dst1_addr = dst1_base + dst1_pdf->getAddress(this);    
  signed long dst_diff = dst2_addr - dst1_addr;
  if (dst_diff < MIN_IMM16 || dst_diff > MAX_IMM16) {
    fprintf(stderr, "!!! replaceFunctionCall: callees too far apart\n");
    return false;
  }

  // function call code
  instruction insn[2];
  // jal    dst2_addr
  insn[0].jtype.op = JALop;
  insn[0].jtype.imm26 = (dst2_addr & REGION_OFF_MASK) >> 2;
  // NOTE: modifying $t9 in the delay slot of "jalr ra,t9" doesn't seem to work
  // jalr   ra,t9
  //genRtype(&insn[0], JALRops, REG_T9, 0, REG_RA);
  // addiu  t9,t9,dst_diff
  genItype(&insn[1], ADDIop, REG_T9, REG_T9, dst_diff);
  this->writeTextSpace((void *)pt_addr, 2*INSN_SIZE, insn);

  // debug
  //fprintf(stderr, "!!! function call replacement\n");
  //disDataSpace(this, (void *)pt_addr, 2, "  ");

  return true;
}

// Emit code to jump to function CALLEE without linking.  (I.e., when
// CALLEE returns, it returns to the current caller.)
void emitFuncJump(opCode /*op*/, 
		  char * /*i*/, Address & /*base*/, 
		  const function_base * /*callee*/, 
		  process * /*proc*/)
{
     /* Unimplemented on this platform! */
     assert(0);
}

bool process::findCallee(instPoint &inst, function_base *&target)
{
  //fprintf(stderr, "!!! process::findCallee()\n");
  assert(inst.type() == IPT_CALL);

  // callee already known?
  function_base *callee = const_cast<function_base*>(inst.iPgetCallee());
  if (callee) {
    target = callee;
    return true;
  }
  
  /* GOT-based calls are partially resolved by checkCallPoints(); all
     we need to do here is correct the runtime address of the GOT
     entry and dereference it */
  if (inst.hint_) {
    // runtime address of GOT entry
    Address got_entry_base = 0;
    getBaseAddress(inst.iPgetOwner(), got_entry_base);
    // TODO: size of GOT entry (32/64)
    Address got_entry_off = inst.hint_;
    Address got_entry_addr = got_entry_base + got_entry_off;

    // function object
    void *fn_ptr;
    readDataSpace((void *)got_entry_addr, sizeof(void *), &fn_ptr, true);
    Address fn_addr = (Address)fn_ptr;
    // TODO: will this work on relocated functions?
    pd_Function *pdf = (pd_Function *)findFunctionIn(fn_addr);
    if (pdf) {
      Address fn_base;
      getBaseAddress(pdf->file()->exec(), fn_base);
      if (fn_base + pdf->getAddress(0) == fn_addr) {
	target = pdf;
	return true;
      }
    }
  }

  // TODO: if we get to this point, don't know what to do
  //fprintf(stderr, "process::findCallee(%s:0x%08x): unknown callee\n",
  //inst.iPgetFunction()->prettyName().string_of(), inst.iPgetAddress());

  target = NULL;
  return false;
}

/* initDefaultPointFrequencyTable() - define the expected call
   frequency of procedures.  Currently we just define several one
   shots with a frequency of one, and provide a hook to read a file
   with more accurate information. */
// TODO: implement
void initDefaultPointFrequencyTable()
{
    funcFrequencyTable["main"] = 1;
    funcFrequencyTable["DYNINSTsampleValues"] = 1;
    funcFrequencyTable[EXIT_NAME] = 1;

    // try to read file.
    FILE *fp = fopen("freq.input", "r");
    if (!fp) return;
    fprintf(stderr, ">>> initDefaultPointFrequencyTable(): "
	    "found \"freq.input\" file\n");

    float value;
    char name[512];
    while (!feof(fp)) {
        fscanf(fp, "%s %f\n", name, &value);
        funcFrequencyTable[name] = (int) value;
        fprintf(stderr, "  funcFrequencyTable: adding %s %f\n", name, value);
    }
    fclose(fp);
}

// measurements taken on 180MHz MIPS R10000
// costs are in units of cycles
void initPrimitiveCost()
{
  //fprintf(stderr, ">>> initPrimitiveCost()\n");

  // wall time
  //primitiveCosts["DYNINSTstartWallTimer"] =     264; // cycle ctr
  //primitiveCosts["DYNINSTstopWallTimer"] =      253; // cycle ctr
  primitiveCosts["DYNINSTstartWallTimer"] =    1170; // gettimeofday()
  primitiveCosts["DYNINSTstopWallTimer"] =     1190; // gettimeofday()

  // CPU time
  //primitiveCosts["DYNINSTstartProcessTimer"] =  3900; // PIOCGETEVCTRS
  //primitiveCosts["DYNINSTstopProcessTimer"] =   3800; // PIOCGETEVCTRS
  primitiveCosts["DYNINSTstartProcessTimer"] = 7760; // PIOCUSAGE
  primitiveCosts["DYNINSTstopProcessTimer"] =  7810; // PIOCUSAGE

  // TODO: tricky interactions with DYNINSTinit()
  primitiveCosts["DYNINSTinit"] = 1;
  primitiveCosts["DYNINSTprintCost"] = 1;     // calls TraceRecord
  primitiveCosts["DYNINSTreportNewTags"] = 1; // calls TraceRecord
  primitiveCosts["DYNINSTbreakPoint"] = 1;

  // below functions are not used on this platform
  // (obviated by SHM_SAMPLING)
  primitiveCosts["DYNINSTalarmExpire"] =      1;
  primitiveCosts["DYNINSTsampleValues"] =     1;
  primitiveCosts["DYNINSTreportTimer"] =      1;
  primitiveCosts["DYNINSTreportCounter"] =    1;
  primitiveCosts["DYNINSTincrementCounter"] = 1;
  primitiveCosts["DYNINSTdecrementCounter"] = 1;
}




//
// paradynd-only methods
//

string process::getProcessStatus() const 
{
  char ret[80];
  switch (status()) {
  case running:
    sprintf(ret, "%d running", pid);
    break;
  case neonatal:
    sprintf(ret, "%d neonatal", pid);
    break;
  case stopped:
    sprintf(ret, "%d stopped", pid);
    break;
  case exited:
    sprintf(ret, "%d exited", pid);
    break;
  default:
    sprintf(ret, "%d UNKNOWN State", pid);
    break;
  }
  return(ret);
}

bool returnInstance::checkReturnInstance(const vector<Address> &stack, u_int &index) 
{
  // if unsafe (ret=false), set "index" to first unsafe call stack index
  for (u_int i=0; i < stack.size(); i++) {
    index = i;
    if (stack[i] >= addr_ && stack[i] < addr_+size_) 
      return false;
  }  
  return true;
}

void returnInstance::addToReturnWaitingList(Address pc, process *proc) 
{
  // if there is already a trap set at this pc, don't generate another
  // b/c readDataSpace will return the wrong original instruction
  bool found = false;
  instruction insn;
  for (u_int i=0; i < instWList.size(); i++) {
    if (instWList[i]->pc_ == pc && instWList[i]->which_proc == proc) {
      found = true;
      insn = instWList[i]->relocatedInstruction;
      break;
    }
  }

  if(!found) {
    instruction insnTrap;
    genTrap(&insnTrap);
    proc->readDataSpace((void*)pc, INSN_SIZE, &insn, true);
    proc->writeTextSpace((void*)pc, INSN_SIZE, &insnTrap);
  }
  
  instWList += new instWaitingList(instructionSeq, instSeqSize,
				   addr_, pc, insn, pc, proc);
}

//
// return cost in cycles of executing at this point.  This is the cost
//   of the base tramp if it is the first at this point or 0 otherwise.
//
int getPointCost(process *proc, const instPoint *point)
{
  if (proc->baseMap.defines(point)) {
    return 0;
  } else {
    // worst case for base tramp is 299 cycles
    return(299);
  }
}

/*
 * Get an estimate of the frequency for the passed instPoint.  
 * The function whose frequency is reported is selected as follows:
 *  - if the instPoint is a call and the callee is known, use the callee
 *  - else use the function containing the instPoint
 *
 *  WARNING: This code contins arbitray values for function frequency (both 
 *  user and system).  This should be refined over time.
 *
 */
float getPointFrequency(instPoint *point)
{
  pd_Function *func;
  
  if (point->callee_) {
    func = (pd_Function *)point->callee_;
  } else {
    func = (pd_Function *)point->func_;
  }

  if (!funcFrequencyTable.defines(func->prettyName())) {
    // TODO: this value needs to be tuned
    return(50);       
  } else {
    return (funcFrequencyTable[func->prettyName()]);
  }
}

#ifndef BPATCH_LIBRARY
float computePauseTimeMetric(const metricDefinitionNode *) 
{
  // we don't need to use the metricDefinitionNode
  timeStamp now;
  timeStamp elapsed=0.0;
  
  now = getCurrentTime(false);
  if (firstRecordTime) {
    elapsed = elapsedPauseTime;
    if (isApplicationPaused()) elapsed += now - startPause;
    assert(elapsed >= 0.0); 
    return(elapsed);
  } else {
    return(0.0);
  }
}
#endif

/*
 * Define the various classes of library functions to inst. 
 *
 */
void initLibraryFunctions()
{
  /* should record waiting time in read/write, but have a conflict with
   *   use of these functions by our inst code.
   *   This happens when a CPUtimer that is stopped is stopped again by the
   *   write.  It is then started again at the end of the write and should
   *   not be running then.  We could let timers go negative, but this
   *   causes a problem when inst is inserted into already running code.
   *   Not sure what the best fix is - jkh 10/4/93
   *
   */
#ifdef notdef
  tagDict["write"] = TAG_LIB_FUNC | TAG_IO_OUT;
  tagDict["read"] = TAG_LIB_FUNC | TAG_IO_IN;
  
  tagDict["send"] = TAG_LIB_FUNC | TAG_CPU_STATE | TAG_MSG_SEND;
  tagDict["sendmsg"] = TAG_LIB_FUNC | TAG_CPU_STATE | TAG_MSG_SEND;
  tagDict["sendto"] = TAG_LIB_FUNC | TAG_CPU_STATE | TAG_MSG_SEND;
  
  tagDict["rev"] = TAG_LIB_FUNC | TAG_CPU_STATE | TAG_MSG_RECV;
  tagDict["recvmsg"] = TAG_LIB_FUNC | TAG_CPU_STATE | TAG_MSG_RECV;
  tagDict["recvfrom"] = TAG_LIB_FUNC | TAG_CPU_STATE | TAG_MSG_RECV;
  
  tagDict["DYNINSTalarmExpire"] = TAG_LIB_FUNC;
  tagDict["DYNINSTsampleValues"] = TAG_LIB_FUNC;
  tagDict[EXIT_NAME] = TAG_LIB_FUNC;
  tagDict["fork"] = TAG_LIB_FUNC;
  
  tagDict["cmmd_debug"] = TAG_LIB_FUNC;
  tagDict["CMRT_init"] = TAG_LIB_FUNC;
  tagDict["CMMD_send"] = TAG_LIB_FUNC;
  tagDict["CMMD_receive"] = TAG_LIB_FUNC;
  tagDict["CMMD_receive_block"] = TAG_LIB_FUNC;
  tagDict["CMMD_send_block"] = TAG_LIB_FUNC;
  tagDict["CMMD_send_async"] = TAG_LIB_FUNC;
  tagDict["CMMD_send_async"] = TAG_LIB_FUNC;
  
  tagDict["main"] = 0;
#endif
}

