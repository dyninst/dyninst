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
#include <iomanip.h>
#include <stdio.h>
#include <assert.h>
#include "dyninstAPI/src/arch-mips.h"
#include "dyninstAPI/src/inst-mips.h"
#include "dyninstAPI/src/symtab.h"    // pd_Function, image
#include "dyninstAPI/src/instPoint.h" // instPoint
#include "dyninstAPI/src/process.h"   // process
#include "dyninstAPI/src/instP.h"     // instWaitingList
#include "dyninstAPI/src/stats.h"     // accounting
#include "common/h/debugOstream.h"
#include <disassembler.h>

#ifdef BPATCH_LIBRARY
#include "BPatch_basicBlock.h"
#include "BPatch_flowGraph.h"
#else
//#include "rtinst/h/trace.h"
//#include "paradynd/src/metric.h"
//#include "paradynd/src/main.h"
#include "paradynd/src/perfStream.h" // firstRecordTime
//#include "dyninstAPI/src/showerror.h"
#include "dyninstAPI/src/dyninstP.h"  // isApplicationPaused()
#endif

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/

typedef signed short       SignedImm;
typedef unsigned short     UnsignedImm;

class NonRecursiveTrampTemplate : public trampTemplate
{

public:

  int guardOnPre_beginOffset;
  int guardOnPre_endOffset;

  int guardOffPre_beginOffset;
  int guardOffPre_endOffset;

  int guardOnPost_beginOffset;
  int guardOnPost_endOffset;

  int guardOffPost_beginOffset;
  int guardOffPost_endOffset;

};

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/

// external prototypes
extern bool isPowerOf2(int value, int &result);

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/

// global variables
trampTemplate baseTemplate;
registerSpace *regSpace;

trampTemplate conservativeTemplate;
registerSpace *conservativeRegSpace;

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

/* mihai Mon Feb 21 14:34:06 CST 2000
 * This template is an extension of the trampTemplate,
 * with several public member variables added. These
 * public member variables store the offsets of the
 * recursive guard code.
 */
NonRecursiveTrampTemplate nonRecursiveBaseTemplate;

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/

#define MAX_IMM16 ((SignedImm)0x7fff)
#define MIN_IMM16 ((SignedImm)0x8000)

#undef M_TRACE_DEBUG
#undef M_INFO_DEBUG

/*
 * mihai Thu Feb 17 17:51:16 CST 2000
 *
 * These are trace and debug macros:
 *
 * TRACE_B( string ) should be used at the beginning of a function
 * TRACE_E( string ) should be used at the end (exit points) of a function
 * TRACE_B and TRACE_E print the strings they are given indented, and
 * the indentation level is controlled by the number of TRACE_B and TRACE_E
 * calls. TRACE_B and TRACE_E will output the strings, only the strings
 * are found in the selected_for_trace array. The usual usage is to call
 * TRACE_? with the function name in which TRACE_? is found, and to
 * include that function name as an element in selected_for_trace.
 *
 * DEBUG takes a sequence of C++ stream parameters and writes them to
 * the DEBUG_STREAM. For example:
 * DEBUG( "Value of x = " << x << ", y = " << y );
 * generates:
 * { DEBUG_STREAM << "DEBUG: " << "Value of x = " << x << ", y = " << y << endl; };
 *
 * DEBUG_STREAM controls the output stream to which the macros write the
 * strings.
 *
 * TRACE_B and TRACE_E are enabled if M_TRACE_DEBUG is defined.
 * DEBUG is enabled if M_INFO_DEBUG is defined.
 *
 */

#define DEBUG_STREAM cout

#ifdef M_TRACE_DEBUG /* mihai's trace macros */

int level = 0;

char * selected_for_trace[] = { "",
				"addIfNew",
				"branchWithinRange",
				// "cmpByAddr",
				"computePauseTimeMetric",
				// "contains",
				"dis",
				"disDataSpace",
				"doNotOverflow",
				// "emitA",
				// "emitFuncCall",
				// "emitFuncJump",
				// "emitImm",
				// "emitLoadPreviousStackFrameRegister",
				// "emitR",
				// "emitV",
				// "emitVload",
				// "emitVstore",
				// "emitVupdate",
				"findAndInstallBaseTramp",
				"findFunctionLikeRld",
				"generate_base_tramp_recursive_guard_code",
				// "genBranch",
				// "genIll",
				// "genItype",
				// "genJtype",
				// "genJump",
				// "genLoadConst",
				// "genLoadNegConst",
				// "genMove",
				// "genNop",
				// "genRtype",
				// "genTrap",
				// "generateBranch",
				// "generateNoOp",
				// "getImmField",
				// "getInsnCost",
				// "getPointCost",
				// "getPointFrequency",
				// "get_dword",
				"got_ld_off",
				"initDefaultPointFrequencyTable",
				"initLibraryFunctions",
				"initPrimitiveCost",
				"initTramps",
				"instWaitingList::cleanUp",
				"installBaseTramp",
				"installTramp",
				// "lookup_fn",
				"mips_dis_init",
				// "pd_Function::checkCallPoints",
				// "pd_Function::checkInstPoints",
				// "pd_Function::findBranchTarget",
				// "pd_Function::findIndirectJumpTarget",
				// "pd_Function::findInstPoints",
				// "pd_Function::findJumpTarget",
				// "pd_Function::findStackFrame",
				// "pd_Function::findTarget",
				// "pd_Function::setVectorIds",
				"pdcmp_got_name",
				"print_function",
				"print_inst_pts",
				"print_saved_registers",
				"print_sequence",
				"process::MonitorCallSite",
				"process::emitInferiorRPCheader",
				"process::emitInferiorRPCtrailer",
				// "process::findCallee",
				"process::getProcessStatus",
				// "process::isDynamicCallSite",
				"process::replaceFunctionCall",
				"readAddressInMemory",
				"relocateInstruction",
				"returnInstance::addToReturnWaitingList",
				"returnInstance::checkReturnInstance",
				"returnInstance::installReturnInstance",
				""
                              };

bool is_in_string_set( char * needle )
{
  for( int i = 0; i < sizeof( selected_for_trace ) / sizeof( char * ); i++ )
    if( strcmp( needle, selected_for_trace[ i ] ) == 0 )
      return true;
  return false;
}

#define TRACE_B(msg) \
        { \
	  if( is_in_string_set( msg ) ) \
	    { \
	      level++; \
	      for( int i = 0; i < level ; i++ ) DEBUG_STREAM << "  "; \
	      DEBUG_STREAM << ">" << __FILE__ << "[" << __LINE__ << "]: " << ( msg ) << endl; \
	    } \
	}
#define TRACE_E(msg) \
        { \
	  if( is_in_string_set( msg ) ) \
	    { \
	      for( int i = 0; i < level; i++ ) DEBUG_STREAM << "  "; \
	      DEBUG_STREAM << "<" << __FILE__ << "[" << __LINE__ << "]: " << ( msg ) << endl; \
	      level--; \
	    } \
	}

#else

#define TRACE_B(msg)
#define TRACE_E(msg)

#endif

#ifdef M_INFO_DEBUG /* mihai's debug macros */

#define DEBUG(a) { DEBUG_STREAM << "DEBUG: " << a << endl; }

#else

#define DEBUG(a)

#endif

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/

// local variables
static dictionary_hash<string, unsigned> funcFrequencyTable(string::hash);

FILE *Stderr = stderr; // hack for debugging

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/

void print_inst_pts(const vector<instPoint*> &pts, pd_Function *fn) 
{
  TRACE_B( "print_inst_pts" );

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

  TRACE_E( "print_inst_pts" );
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/

void print_function(pd_Function *f)
{
  TRACE_B( "print_function" );

  fprintf(stderr, "0x%016lx: %s (%i insns):\n", 
	  f->getAddress(0), 
	  f->prettyName().string_of(), 
	  f->size() / (int)INSN_SIZE);

  vector<instPoint*> t;
  t += const_cast<instPoint*>(f->funcEntry(0));
  print_inst_pts(t, f);

  print_inst_pts(f->funcCalls(0), f);

  print_inst_pts(f->funcExits(0), f);

  TRACE_E( "print_function" );
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/

static void mips_dis_init()
{
  TRACE_B( "mips_dis_init" );

  static bool init = true;
  if (init) {
    //dis_init32("0x%016lx\t", 0, reg_names, 1);
    //dis_init64("0x%016lx\t", 0, reg_names, 1);
    init = false;
  }

  TRACE_E( "mips_dis_init" );
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/

void disDataSpace(process *p, void *addr_, int ninsns, 
		  const char *pre, FILE *stream)
{
  TRACE_B( "disDataSpace" );

  mips_dis_init();
  
  instruction *addr = (instruction *)addr_;
  assert(isAligned((Address)addr));
  instruction insn;
  char buf[64];
  static bool is_elf64 = p->getImage()->getObject().is_elf64();

  for (int i = 0; i < ninsns; i++) {
    void *inTraced = addr + i;
    p->readDataSpace(inTraced, INSN_SIZE, &insn, true);
    // Elf32_Addr regmask, lsreg; /* commented out by Mihai. Unused variables. */
    if (is_elf64) {

      // Elf64_Addr value; /* commented out by Mihai. Unused variables. */
      //disasm64(buf, (Elf64_Addr)inTraced, *(Elf32_Addr *)&insn, &regmask, &value, &lsreg);

    } else { // 32-bit app

      // Elf32_Addr value; /* commented out by Mihai. Unused variables. */
      //disasm32(buf, (Elf32_Addr)(Address)inTraced, *(Elf32_Addr *)&insn, &regmask, &value, &lsreg);

    }
    if (pre) fprintf(stream, "%s", pre);
    fprintf(stream, "%s\n", buf);
  }

  TRACE_E( "disDataSpace" );
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/

void dis(void *actual_, void *addr_, int ninsns, 
	 const char *pre, FILE *stream)
{
  TRACE_B( "dis" );

  mips_dis_init();

  instruction *actual = (instruction *)actual_;
  instruction *addr = (instruction *)addr_;
  if (addr == NULL) addr = actual;
  char buf[64];

#if 0 /* Unifdef and link with -lelfutil for debugging. */
  Elf32_Addr regmask, value, lsreg;
  for (int i = 0; i < ninsns; i++) {
    Elf32_Addr inSelf = (Elf32_Addr)(Address)(addr + i);
    Elf32_Addr insn = *(Elf32_Addr *)(actual + i);
    disasm32(buf, inSelf, insn, &regmask, &value, &lsreg);
    fprintf(stream, "%s%s\n", (pre) ? (pre) : (""), buf);
  }
#endif

  TRACE_E( "dis" );
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/

Address readAddressInMemory(process *p, Address ptr, bool is_elf64)
{
  TRACE_B( "readAddressInMemory" );

  void *ret = NULL;
  char *local_addr = (char *)&ret;
  unsigned nbytes = sizeof(void *);

  if (!is_elf64 && sizeof(void *) == sizeof(uint64_t)) {
    // 64-bit paradynd, 32-bit application
    local_addr += sizeof(uint32_t);
    nbytes -= sizeof(uint32_t);
  }

  // read pointer from memory
  bool ret2 = p->readDataSpace((void *)ptr, nbytes, local_addr, true);
  assert(ret2);

  TRACE_E( "readAddressInMemory" );

  return (Address)ret;
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/

Address lookup_fn(process *p, const string &f)
{
  TRACE_B( "lookup_fn" );

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

  TRACE_E( "lookup_fn" );

  return ret;
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/

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

#ifdef CSS_DEBUG_INST
#define UNINSTR(str) \
  fprintf(stderr, "uninstrumentable: %s (%0#10x: %i insns) - %s\n", \
	  prettyName().string_of(), \
	  file_->exec()->getObject().get_base_addr() + getAddress(0), \
	  size() / INSN_SIZE, \
	  str)
#else
#define UNINSTR(str)
#endif

bool pd_Function::findInstPoints(const image *owner) {
    TRACE_B( "pd_Function::findInstPoints" );

  //fprintf(stderr, "\n>>> pd_Function::findInstPoints()\n");
  //fprintf(stderr, "%0#10x: %s(%u insns):\n", 
  //getAddress(0), prettyName().string_of(), size() / INSN_SIZE);
  if (size() == 0) {
    UNINSTR("zero length");

    TRACE_E( "pd_Function::findInstPoints" );

    return false;
  }

  // default values
  isTrap = false;
  relocatable_ = false;
  noStackFrame = true;

  // parse instPoints
  Address start = getAddress(0);
  Address end = start + size();
  Offset off;
  instruction insn;

  /* stack frame info */
  /* check if function has a stack frame:
   *  - yes: entry point is "save" instruction ([d]addiu sp,sp,-XX)
   *  - no: entry point is start of function
   */
  Address entry = findStackFrame(owner);
  
  /* ENTRY point */
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
      // TODO: system call handling
    }

      
    /* EXIT points */
    // TODO - distinguish between return and switch? (maybe "jr ra" vs "jr v1")
    if (isReturnInsn(insn)) {
      //cerr << "  found return pt" << endl;
      funcReturns += new exitPoint(this, off);
    }
  }

  setVectorIds(); // set CALL and EXIT vectorIds

  TRACE_E( "pd_Function::findInstPoints" );

  return checkInstPoints();
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/

static bool contains(vector<int> &V, int val)
{
  TRACE_B( "contains" );

  for (unsigned i = 0; i < V.size(); i++) {
    if (V[i] == val)
      {
// 	TRACE_E( "contains" );

	return true;
      }
  }

  TRACE_E( "contains" );

  return false;
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/

static void addIfNew(vector<int> &V, int val)
{
  TRACE_B( "addIfNew" );

  if (contains(V, val))
    {
      TRACE_E( "addIfNew" );

      return;
    }

  V += val;

  TRACE_E( "addIfNew" );
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/

static void print_saved_registers(pd_Function *fn, const vector<vector<int> > &slots)
{
  TRACE_B( "print_saved_registers" );

  /*
  vector<vector<int> > slots2(slots.size());
  vector<int> locals;
  for (unsigned i = 0; i < slots.size(); i++) {
    for (int j = 0; j < slots[i].size(); j++) {
      int slot = slots[i][j];
      bool dup = false;
      if (contains(locals, slot)) dup = true;
      for (int k = 0; k < slots.size() && !dup; k++) {
	if (k == i) continue;
	if (contains(slots[k], slot)) dup = true;
      }
      if (!dup) addIfNew(slots2[i], slot);
      else addIfNew(locals, slot);
    }
  }
  */  
  
  bool mult = false;
  for (unsigned i = 0; i < slots.size(); i++) {
    if (slots[i].size() > 1) {
      mult = true;
      break;
    }
  }


  if (mult) {
    fprintf(stderr, "*** %s (0x%016lx: %i insns): stack frame\n",
	    fn->prettyName().string_of(), 
	    fn->getAddress(0), 
	    fn->size() / (int)INSN_SIZE);
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
	  if (contains(locals, slot)) continue;
	  fprintf(stderr, " %3i", slot);
	  if (dup) fprintf(stderr, "*");
	  //if (contains(locals, slot)) fprintf(stderr, "&");
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

  TRACE_E( "print_saved_registers" );
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/

Address pd_Function::findStackFrame(const image *owner)
{
  TRACE_B( "pd_Function::findStackFrame" );

  bool foundRest;
  int lastRestore;
  InactiveFrameRange ifr;
  
  /*
    fprintf(stderr, ">>> findStackFrame(): <0x%016lx:\"%s\"> %i insns\n",
    owner->getObject().get_base_addr() + getAddress(0), 
    prettyName().string_of(), size() / INSN_SIZE);
  */
 
  // initialize stack frame info
  for (int i = 0; i < NUM_REGS; i++) {
    reg_saves[i].slot = -1;
  }
  sp_mod = (Address)-1;
  frame_size = 0;
  fp_mod = (Address)-1;
  uses_fp = false;

  // register aliasing
  int aliases[NUM_REGS];
  for (int i = 0; i < NUM_REGS; i++) {
    aliases[i] = i;
  }

  // parse insns relevant to stack frame
  Address start = getAddress(0);
  Address end = start + size();
  Offset off;
  instruction insn;
  Address fn_addr = owner->getObject().get_base_addr() + getAddress(0);
  for (off = 0; off < end - start; off += INSN_SIZE) {
    insn.raw = owner->get_instruction(start + off);
    struct fmt_itype &itype = insn.itype;
    struct fmt_rtype &rtype = insn.rtype;
    //Address iaddr = fn_addr + off; // debug

    /* TODO: This stack frame parsing does not handle cases where the
       stack frame is adjusted multiple times in the same function.
       Dataflow analysis is required to handle this scenario
       correctly. */

    // stack frame (save): "[d]addiu sp,sp,-XX" insn
    if ((itype.op == ADDIUop || itype.op == DADDIUop) && 
        itype.rs == REG_SP &&
        itype.rt == REG_SP &&
        itype.simm16 < 0 &&
        noStackFrame == true)
    {
      noStackFrame = false;
      sp_mod = off;
      frame_size = -itype.simm16;
    }
    // stack frame (save, large): "[d]subu sp,sp,at" insn,
    //  which usually follows an "ori at, 0, XX" insn,
    //  with possibly a lui preceeding the ori
    else if ((rtype.ops == SUBUops || rtype.ops == DSUBUops) &&
             rtype.rd == REG_SP &&
             rtype.rs == REG_SP &&
             noStackFrame == true)
    {
      instruction tempInsn;
      tempInsn.raw = owner->get_instruction(start + off - INSN_SIZE);
      struct fmt_itype &tempItype = tempInsn.itype;

      if ( tempItype.op == ORIop && 
           rtype.rt == tempItype.rt )
      {
        int totFrameSize = tempItype.simm16 & 0xffff;

        noStackFrame = false;
        sp_mod = off;

        tempInsn.raw = owner->get_instruction(start + off - (2*INSN_SIZE));
        if ( tempItype.op == LUIop && 
             rtype.rt == tempItype.rt )
        {
          totFrameSize |= (tempItype.simm16 << 16);
        }

        frame_size = totFrameSize;
      }
    }
    // stack frame (restore): "[d]addiu sp,sp,<frame_size>" insn
    else if ((itype.op == ADDIUop || itype.op == DADDIUop) && 
             itype.rs == REG_SP &&
             itype.rt == REG_SP &&
             itype.simm16 == frame_size &&
             noStackFrame == false)
    {
      lastRestore = off;
      foundRest = true;
    }
    // stack frame (restore from $fp): "move sp,s8" insn
    else if (rtype.op == SPECIALop &&
             rtype.ops == ORops &&
             rtype.rt == REG_ZERO &&
             rtype.rs == REG_S8 &&
             rtype.rd == REG_SP &&
             uses_fp == true)
    {
      lastRestore = off;
      foundRest = true;
    }

    // frame pointer #1: "[d]addiu s8,sp,<frame_size>" insn
    else if ((itype.op == ADDIUop || itype.op == DADDIUop) &&
             itype.rs == REG_SP &&
             itype.rt == REG_S8 &&
             itype.simm16 == frame_size)
    {
      fp_mod = off;
      uses_fp = true;
    }
    // frame pointer #2: "move s8,sp" insn
    else if (rtype.op == SPECIALop &&
             rtype.ops == ORops &&
             rtype.rt == REG_ZERO &&
             rtype.rs == REG_SP &&
             rtype.rd == REG_S8 &&
             frame_size == 0)
    {
      fp_mod = off;
      uses_fp = true;
    }

    // frame pointer #3: "daddu s8,sp, at" insn
    else if (rtype.op == SPECIALop &&
             rtype.ops == DADDUops &&
             rtype.rs == REG_SP &&
             rtype.rd == REG_S8)
    {
      fp_mod = off;
      uses_fp = true;
    }

    // register aliasing: "move R2,R1" insn
    else if (rtype.op  == SPECIALop &&
             rtype.ops == ORops &&
             rtype.rt  == REG_ZERO &&
             rtype.rs  != REG_ZERO)
    {
      int r_src = aliases[rtype.rs];
      // check if register has been saved yet
      if (reg_saves[r_src].slot == -1) {
        int r_dst = rtype.rd;
        /*
          if (aliases[r_dst] != r_dst) {
          fprintf(stderr, "!!! <0x%016lx:\"%s\" multiple aliasing\n",
          iaddr, prettyName().string_of());
          } */
        aliases[r_dst] = r_src;
      }
    }

    // register save #1: "sd/sw RR,XX(sp)" insn
    else if ((itype.op == SDop || itype.op == SWop) &&
             itype.rs == REG_SP &&
             itype.simm16 >= 0)
    {
      assert(isAligned(itype.simm16));
      int r = aliases[itype.rt];
      regSave_t &save = reg_saves[r];
      // check if register has been saved yet
      if (save.slot == -1) {
        // convert positive $sp offset to negative $fp offset
        save.slot = itype.simm16 - frame_size;
        save.dword = (itype.op == SDop);
        save.insn = off;
      }
    }
    // register save #2: "sd/sw RR,-XX(s8)" insn
    else if ((itype.op == SDop || itype.op == SWop) &&
             uses_fp && itype.rs == REG_S8 &&
             itype.simm16 < 0)
    {
      int r = aliases[itype.rt];
      regSave_t &save = reg_saves[r];
      // check if register has been saved yet
      if (save.slot == -1) {
        save.slot = itype.simm16; // negative $fp offset
        save.dword = (itype.op == SDop);
        save.insn = off;
      }
    }
    //  return : "jr ra" insn
    else if ( rtype.op == SPECIALop && rtype.ops == JRops ) 
    {
      if ( frame_size > 0 )
      {
        instruction tempInsn;
        struct fmt_itype &itype = tempInsn.itype;
        struct fmt_rtype &rtype = tempInsn.rtype;
        Address fn_addr = owner->getObject().get_base_addr() + getAddress(0);
        tempInsn.raw = owner->get_instruction(start + off + INSN_SIZE);

        //  check if frame pop is in delay slot
        //  if so, don't save pop/return offsets to
        //  identify inactive frame range

        if ( ((itype.op == ADDIUop || itype.op == DADDIUop) && 
              itype.rs == REG_SP &&
              itype.rt == REG_SP &&
              itype.simm16 == frame_size &&
              noStackFrame == false) || 

             (rtype.op == SPECIALop &&
              rtype.ops == ORops &&
              rtype.rt == REG_ZERO &&
              rtype.rs == REG_S8 &&
              rtype.rd == REG_SP &&
              uses_fp == true) )
        {
          off += INSN_SIZE;
        }
        else
        {
          if ( foundRest )
          {
            //fprintf(stderr, "\n\n  in function %s, saving pop %d and ret %d\n", prettyName().string_of(), lastRestore, off);
            ifr.popOffset = lastRestore;
            ifr.retOffset = off;
            inactiveRanges += ifr;
          }
        }

        foundRest = false;
      }
    }
  }
  
  TRACE_E( "pd_Function::findStackFrame" );

  // default return value (entry point = first insn of fn)
  return (noStackFrame) ? (0) : (sp_mod);
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/

void pd_Function::setVectorIds()
{
  TRACE_B( "pd_Function::setVectorIds" );

  //fprintf(stderr, ">>> pd_Function::setIDS()\n");
  for (unsigned i = 0; i < calls.size(); i++) 
    calls[i]->vectorId = i;
  for (unsigned i = 0; i < funcReturns.size(); i++) 
    funcReturns[i]->vectorId = i;

  TRACE_E( "pd_Function::setVectorIds" );
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/

/* compare instPoints by Address: used in checkInstPoints() */
static int cmpByAddr(const void *A, const void *B)
{
  TRACE_B( "cmpByAddr" );

  instPoint *ptA = *(instPoint **)const_cast<void*>(A);
  instPoint *ptB = *(instPoint **)const_cast<void*>(B);
  Offset offA = ptA->offset();
  Offset offB = ptB->offset();
  if (offA < offB)
    {
      TRACE_E( "cmpByAddr" );

      return -1;
    }
  if (offA > offB)
    {
      TRACE_E( "cmpByAddr" );

      return 1;
    }

  TRACE_E( "cmpByAddr" );

  return 0;
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/

/* checkInstPoints(): check for special instPoint conditions...
 * - overlapping instPoints
 * - first instPoint is not an entry point
 */
bool pd_Function::checkInstPoints()
{
  TRACE_B( "pd_Function::checkInstPoints" );

  //fprintf(stderr, ">>> pd_Function::checkInstPoints()\n");
  bool ret = true;

  // resolve call targets of "_start"
  // (looking for calls to "main")
  if (symTabName() == "_start" ||
      symTabName() == "__start") 
  {
    for (unsigned i = 0; i < calls.size(); i++) {
      findTarget(calls[i]);
    }
  }

  /* no entry point */
  if (!funcEntry_) {
    UNINSTR("no entry point");
    ret = false;
  } 

  /* no exit points */
  if (funcReturns.size() == 0 && symTabName() != "main") {
    UNINSTR("no exit points");
    ret = false;
  }

  /* sort all instPoints by address */
  vector<instPoint*> pts;
  if (funcEntry_) pts += funcEntry_;
  for (unsigned i = 0; i < funcReturns.size(); i++) {
    pts += funcReturns[i];
  }
  for (unsigned i = 0; i < calls.size(); i++) {
    pts += calls[i];
  }
  pts.sort(cmpByAddr);

  /* first instPoint not an entry point */
  if (pts[0]->type() != IPT_ENTRY) {
    UNINSTR("1st inst pt not entry");
    //print_inst_pts(pts, this);
    ret = false;
  }

  /* check for overlapping instPoints */
  for (unsigned i = 0; i < pts.size() - 1; i++) {
    instPoint *p = pts[i];
    instPoint *p2 = pts[i+1];
    if (p2->offset() < p->offset() + p->size()) {
      relocatable_ = true;
      isTrap = true; // alias for relocatable_ (TODO)
      p2->flags |= IP_Overlap;
    }
  }
  // TODO: function relocation not yet implemented
  if (relocatable_) {
    ret = false;
  }
  
  TRACE_E( "pd_Function::checkInstPoints" );

  //if (!ret) fprintf(stderr, ">>> uninstrumentable: \"%s\"\n", prettyName().string_of());
  return ret;
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/

/* checkCallPoints():
 * Determine if callee is a "library" or "user" function.  
 * This cannot be done until all functions have been seen.  
 */
void pd_Function::checkCallPoints() 
{
  TRACE_B( "pd_Function::checkCallPoints" );

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
      // NOTE: (target == fnStart) => optimized recursive call
      if (!tgt_fn && tgt_addr > fnStart && tgt_addr < fnStart + size()) {
	// target is inside same function (i.e. branch)
	delete ip;
	continue;
      }
    }
     
    /* fallthrough cases (call sites are kept):
     * - target is unknown (trap)
     * - target is external, but callee function cannot be found
     */
    calls2 += ip;
  }
  calls = calls2;
  setVectorIds();

  TRACE_E( "pd_Function::checkCallPoints" );
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/

/* findTarget(): calculate target of call point
 * return of 0 means unknown 
 */
Address pd_Function::findTarget(instPoint *p)
{
  TRACE_B( "pd_Function::findTarget" );

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

  TRACE_E( "pd_Function::findTarget" );

  return ret;
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/

Address pd_Function::findBranchTarget(instPoint *p, instruction i)
{
  TRACE_B( "pd_Function::findBranchTarget" );

  // PC-relative branch
  Address base = p->address() + INSN_SIZE;
  signed off = i.itype.simm16 << 2;
  Address ret = base + off;
  //fprintf(stderr, ">>> pd_Function::findBranchTarget() => 0x%08x\n", ret);

  TRACE_E( "pd_Function::findBranchTarget" );

  return ret;
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/

Address pd_Function::findJumpTarget(instPoint *p, instruction i)
{
  TRACE_B( "pd_Function::findJumpTarget" );

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

  TRACE_E( "pd_Function::findJumpTarget" );

  return ret;
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/

void print_sequence(int targetReg, vector<int> &baseAdjusts,
		    vector<int> &adjusts, int n, char * /*pre*/ = NULL)
{
  TRACE_B( "print_sequence" );

  if ((unsigned)n == baseAdjusts.size()) {
    fprintf(stderr, "%s", reg_names[targetReg]);

    TRACE_E( "print_sequence" );

    return;
  }

  fprintf(stderr, "[");
  print_sequence(targetReg, baseAdjusts, adjusts, n+1);
  if (baseAdjusts[n] != 0) fprintf(stderr, "%+i", baseAdjusts[n]);
  fprintf(stderr, "]");
  if (adjusts[n] != 0) fprintf(stderr, "%+i", adjusts[n]);

  TRACE_E( "print_sequence" );
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/

uint32_t get_word(const Object &elf, Address addr)
{
  TRACE_E( "get_word" );

  uint32_t ret = 0;
  if (addr >= elf.code_off() && addr < elf.code_off() + (elf.code_len() << 2)) {
    char *base = (char *)const_cast<Word*>(elf.code_ptr());
    char *ptr = base + (addr - elf.code_off());
    ret = *(uint32_t *)ptr;
  } else if (addr >= elf.data_off() && addr < elf.data_off() + (elf.data_len() << 2)) {
    char *base = (char *)const_cast<Word*>(elf.data_ptr());
    char *ptr = base + (addr - elf.data_off());
    ret = *(uint32_t *)ptr;
  }

  TRACE_E( "get_word" );

  return ret;
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/

uint64_t get_dword(const Object &elf, Address addr)
{
  TRACE_B( "get_dword" );

  uint64_t ret = 0;
  if (addr >= elf.code_off() && addr < elf.code_off() + (elf.code_len() << 2)) {
    char *base = (char *)const_cast<Word*>(elf.code_ptr());
    char *ptr = base + (addr - elf.code_off());
    uint64_t hi = *(uint32_t *)ptr;
    uint64_t lo = *(uint32_t *)(ptr + sizeof(uint32_t));
    ret = (hi << 32) | lo;
  } else if (addr >= elf.data_off() && addr < elf.data_off() + (elf.data_len() << 2)) {
    char *base = (char *)const_cast<Word*>(elf.data_ptr());
    char *ptr = base + (addr - elf.data_off());
    uint64_t hi = *(uint32_t *)ptr;
    uint64_t lo = *(uint32_t *)(ptr + sizeof(uint32_t));
    ret = (hi << 32) | lo;
  }

  TRACE_E( "get_dword" );

  return ret;
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/

Address pd_Function::findIndirectJumpTarget(instPoint *ip, instruction i)
{
  TRACE_B( "pd_Function::findIndirectJumpTarget" );

  /*
  fprintf(stderr, ">>> pd_Function::findIndirectJumpTarget <0x%016lx: %s>\n", 
	  file_->exec()->getObject().get_base_addr() + ip->address(), 
	  prettyName().string_of());
  */

  assert(i.rtype.op == SPECIALop);
  assert(i.rtype.ops == JALRops || i.rtype.ops == JRops);
  
  // look for the following instruction sequence(s):
  // lw          R1,XX(gp)
  // [[d]addiu]  R1,R1,XX
  // j[al]r      R2,R1

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
    // analysis stops when $gp or $sp encountered
    // TODO: decode $sp sequences using stack frame info
    if (targetReg == REG_GP) break;
    if (targetReg == REG_SP) break;

    i2.raw = owner->get_instruction(start+off);

    // parse indirect jump sequence
 
    // daddu t9,t9,gp
    if (isInsnType(i2, DADDUmask, DADDUmatch) &&
	i2.rtype.rs == REG_T9 &&
	i2.rtype.rt == REG_GP &&
	i2.rtype.rd == REG_T9)
    {
      targetReg = REG_GP;
      // debug
      insns += i2.raw;
      insnAddrs += start+off;
    }
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
    // addiu  R1,R1,X
    // daddiu R1,R1,X
    if ((isInsnType(i2, ADDIUmask, ADDIUmatch) || 
	 isInsnType(i2, DADDIUmask, DADDIUmatch)) &&
	i2.itype.rs == targetReg &&
	i2.itype.rt == targetReg) 
    {
      adjust += i2.itype.simm16;
      // debug
      insns += i2.raw;
      insnAddrs += start+off;
    }
    // lw R2,X(R1)
    // ld R2,X(R1)
    if ((isInsnType(i2, LWmask, LWmatch) || 
	 isInsnType(i2, LDmask, LDmatch)) &&
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
  if (n_loads < 1) return 0;
  assert(baseAdjusts.size() == n_loads);
  assert(adjusts.size() == n_loads);
  //assert(n_loads > 0); // jump to $a1
  //assert(adjust == 0);
  
  // debug: target arithmetic
  /*
  fprintf(stderr, ">>> <0x%016lx:%s>", 
	  owner->getObject().get_base_addr() + ip->address(),
	  prettyName().string_of());
  fprintf(stderr, ": ");
  print_sequence(targetReg, baseAdjusts, adjusts, 0, NULL);
  fprintf(stderr, "\n");
  */

  // debug: insn sequence
  /*
  instruction i3;
  for (int i = insns.size()-1; i >= 0; i--) {
    i3.raw = insns[i];
    dis(&i3, (void *)insnAddrs[i], 1, "  ");
  }
  */

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
  bool is_elf64 = elf.is_elf64();

  // special case: function call via GOT entry
  if (baseRegs.size() == 1 && 
      targetReg == REG_GP &&
      adjusts[0] == 0) 
  {
    /* NOTE: We do not resolve the GOT entry yet.  While the static
       entry may appear to resolve to a local symbol, it can be
       preempted at runtime.  The Fortran function "MAIN__" is one
       such case. */

    Address got_entry_off = target + baseAdjusts[0] - obj_base;    

    // check for calls to "main"
    const char *callee_name = elf.got_entry_name(got_entry_off);
    if (callee_name && !strcmp("main", callee_name)) {
      owner->main_call_addr_ = ip->address();
    }
    
    // wait for runtime value of GOT entry
    ip->hint_got_ = got_entry_off;
    return 0;
  }
  
  // debug: target arithmetic
  /*
  fprintf(stderr, ">>> findIndirectJumpTarget <0x%016lx: %s>\n", 
	  ip->address() + obj_base, prettyName().string_of());
  fprintf(stderr, "  => ");
  print_sequence(targetReg, baseAdjusts, adjusts, 0, NULL);
  fprintf(stderr, "\n");
  */

  // calculate jump target
  for (int i = baseRegs.size()-1; i >= 0; i--) {
    Address vaddr = target + baseAdjusts[i];
    Address vaddr_rel = vaddr - obj_base;
    // address-in-memory
    Address addr_in_mem = (is_elf64)
      ? (get_dword(elf, vaddr_rel))
      : (get_word(elf, vaddr_rel));
    if (addr_in_mem == 0) return 0;
    target = addr_in_mem + adjusts[i];
  }
  target -= obj_base; // relative addressing

  TRACE_E( "pd_Function::findIndirectJumpTarget" );

  return target;
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/

bool doNotOverflow(int value)
{
  TRACE_B( "doNotOverflow" );

  //fprintf(stderr, ">>> doNotOverflow()\n");
  if (value >= MIN_IMM16 && value <= MAX_IMM16)
    {
      TRACE_E( "doNotOverflow" );

      return true;
    }

  TRACE_E( "doNotOverflow" );

  return false;
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/

void generateNoOp(process *proc, Address addr) {
  TRACE_B( "generateNoOp" );

  //fprintf(stderr, ">>> generateNoOp()\n");
  proc->writeTextWord((caddr_t)addr, NOP_INSN);

  TRACE_E( "generateNoOp" );
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/

bool branchWithinRange(Address branch, Address target)
{
  TRACE_B( "branchWithinRange" );

  //fprintf(stderr, ">>> branchWithinRange(0x%08x,0x%08x)\n", branch, target);
  Address slot = branch + INSN_SIZE; // delay slot insn

  // PC-region jump
  if (region_num(slot) == region_num(target))
    {
      TRACE_E( "branchWithinRange" );

      return true;  
    }

  // PC-relative branch
  RegValue offset = target - slot;
  if (offset >= BRANCH_MIN && offset <= BRANCH_MAX)
    {
      TRACE_E( "branchWithinRange" );

      return true;
    }

  TRACE_E( "branchWithinRange" );

  return false;
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/

/* generateBranch():
   called by inst.C for instrumentation jumps
   - instPoint to basetramp
   - basetramp to minitramp
   - minitramp to minitramp
   - minitramp to basetramp
   - basetramp to instPoint
*/
void generateBranch(process *p, Address branch, Address target)
{
  TRACE_B( "generateBranch" );

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
  p->writeTextWord((caddr_t)branch, i.raw);
  //fprintf(stderr, ">>> generateBranch(): 0x%016lx to 0x%016lx\n", branch, target);
  //disDataSpace(p, (void *)branch, 1, "  ");

  TRACE_E( "generateBranch" );
}


/****************************************************************************/
/****************************************************************************/
/****************************************************************************/

void genRtype(instruction *insn, int ops, reg rs, reg rt, 
	      reg rd, int sa) 
{
  TRACE_B( "genRtype" );

  struct fmt_rtype *i = &insn->rtype;
  i->op = SPECIALop;
  i->rs = rs;
  i->rt = rt;
  i->rd = rd;
  i->sa = sa;
  i->ops = ops;

  TRACE_E( "genRtype" );
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/

void genItype(instruction *insn, int op, reg rs, reg rt, signed short imm)
{
  TRACE_B( "genItype" );

  struct fmt_itype *i = &insn->itype;
  i->op = op;
  i->rs = rs;
  i->rt = rt;
  i->simm16 = imm;

  TRACE_E( "genItype" );
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/

void genJtype(instruction *insn, int op, unsigned imm)
{
  TRACE_B( "genJtype" );

  struct fmt_jtype *i = &insn->jtype;
  i->op = op;
  i->imm26 = imm;

  TRACE_E( "genJtype" );
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/

void genBranch(instruction *insn, Address branch, Address target) {
  TRACE_B( "genBranch" );

  Address slot = branch + INSN_SIZE;
  RegValue disp_ = (target - slot) >> 2;
  assert(disp_ <= MAX_IMM16 && disp_ >= MIN_IMM16);
  signed short disp = (signed short)disp_;
  genItype(insn, BEQop, REG_ZERO, REG_ZERO, disp);

  TRACE_E( "genBranch" );
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/

bool genJump(instruction *insn, Address branch, Address target) {
  TRACE_B( "genJump" );

  Address slot = branch + INSN_SIZE;
  // try PC-region branch
  if (region_num(slot) == region_num(target)) {
    genJtype(insn, Jop, (target & REGION_OFF_MASK) >> 2);

    TRACE_E( "genJump" );

    return true;
  }
  // try PC-relative branch
  RegValue disp_ = (target - slot) >> 2;
  if (disp_ <= MAX_IMM16 && disp_ >= MIN_IMM16) {
    genBranch(insn, branch, target);

    TRACE_E( "genJump" );

    return true;
  }
  // one-insn branch failed
  assert(false); // TODO

  TRACE_E( "genJump" );

  return false;
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/

void genNop(instruction *insn) {
  TRACE_B( "genNop" );

  insn->raw = NOP_INSN;

  TRACE_E( "genNop" );
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/

void genTrap(instruction *insn) {
  TRACE_B( "genTrap" );

  insn->raw = TRAP_INSN;

  TRACE_E( "genTrap" );
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/

void genIll(instruction *insn) {
  TRACE_B( "genIll" );

  insn->raw = ILLEGAL_INSN;

  TRACE_E( "genIll" );
}
// "mov rd,rs" emitted as "or rd,rs,r0"

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/

void genMove(instruction *insn, reg rs, reg rd) {
  TRACE_B( "genMove" );

  genRtype(insn, ORops, rs, REG_ZERO, rd);

  TRACE_E( "genMove" );
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/

#define IMM_NBITS ((Address)0x10)
#define IMM_MASK  ((Address)0xffff)
// next two must be same type as getImmField() return
#define IMM_ZERO  ((UnsignedImm)0)
#define IMM_ONES  ((UnsignedImm)~(UnsignedImm)0)
#define bit(n,v) ((v >> n) & 0x1)

UnsignedImm getImmField(Address val, int n)
{
  TRACE_B( "getImmField" );

  Address offset = n * IMM_NBITS;
  Address mask = IMM_MASK << offset;
  UnsignedImm ret = (val & mask) >> offset;

  TRACE_E( "getImmField" );

  return ret;
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/

void genLoadNegConst(reg dst, Address imm, char *code, Address &base, bool /*noCost*/)
{
  TRACE_B( "genLoadNegConst" );

  //fprintf(stderr, ">>> genLoadNegConst(0x%016lx)\n", imm);
  //Address base_orig = base; // debug

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
    genItype(insn, DADDIUop, REG_ZERO, dst, imm);
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
  /*
  for (Address i = base_orig; i < base; i += INSN_SIZE) {
    dis((instruction *)(code + i), NULL, 1, "  ");
  }
  */

  TRACE_E( "genLoadNegConst" );
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/

void genLoadConst(reg dst, RegValue imm, char *code, Address &base, bool noCost)
{
  TRACE_B( "genLoadConst" );

  // if negative, use genLoadNegConst()
  if (imm < 0) {
    genLoadNegConst(dst, (Address)imm, code, base, noCost);

    TRACE_E( "genLoadConst" );

    return;
  }

  //fprintf(stderr, ">>> genLoadConst(0x%lx)\n", imm);
  //Address base_orig = base; // debug

  int nbits = sizeof(RegValue) * 8; // N-bit addresses
  int nimm = (nbits/IMM_NBITS); // # of immediate (16-bit) fields
  int nonzero = 0; // most significant nonzero field
  for (nonzero = nimm - 1; nonzero > 0; nonzero--) {
    if (getImmField(imm, nonzero) != IMM_ZERO) break;
  }

  instruction *insn = (instruction *)(code + base);
  switch (nonzero) {
  case 0: 
    {
      genItype(insn, ORIop, REG_ZERO, dst, getImmField(imm, 0));
      base += INSN_SIZE;
    } break;
  default:
    {
      // load most significant nonzero field
      UnsignedImm field = getImmField(imm, nonzero);
      if (bit(IMM_NBITS-1, field)) {
	// MSB of first nonzero field is one 
	// (i.e. sign-extending would be bad)
	// => ori, dsll
	genItype(insn, ORIop, REG_ZERO, dst, field);
	base += INSN_SIZE;
	genRtype(insn+1, DSLLops, 0, dst, dst, IMM_NBITS);
	base += INSN_SIZE;
      } else {
	// MSB of first nonzero field is zero
	// => lui
	genItype(insn, LUIop, 0, dst, field);
	base += INSN_SIZE;
      }
      // load next field
      if (getImmField(imm, nonzero-1) != IMM_ZERO) {
	genItype(insn+1, ORIop, dst, dst, getImmField(imm, nonzero-1));
	base += INSN_SIZE;
      }
      // load remaining fields
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
  }

  // debug
  /*
  for (Address i = base_orig; i < base; i += INSN_SIZE) {
    dis((instruction *)(code + i), NULL, 1, "  ");
  }
  */

  TRACE_E( "genLoadConst" );
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/

// HERE BE DRAGONS

// return Address
// [ifOp, branchOp, trampPreamble, trampTrailer]
// TODO: "dst" is a Register, should be RegValue (holds branch offset)
Address emitA(opCode op, Register src1, Register /*src2*/, Register dst, 
	      char *code, Address &base, bool /*noCost*/)
{
  TRACE_B( "emitA" );

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

  TRACE_E( "emitA" );

  return ret;
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/

// return Register
// [getParamOp, getSysParamOp, getRetValOp, getSysRetValOp]
Register emitR(opCode op, Register src1, Register /*src2*/, Register dst, 
	       char *code, Address &base, bool /*noCost*/)
{
  TRACE_B( "emitR" );

  Register ret = REG_ZERO;
  int frame_off = 0;

  switch (op) {

  case getParamOp:
    // "src1"     : argument number [0,7]
    // "dst"      : allocated return register
    // return val : argument register
    // TODO: extract >8 parameters from stack (need to know stack frame size)
    //fprintf(stderr, ">>> emit(getParamOp): %i\n", src1);
    if ( src1 < 8 )
    {
      ret = dst;
      frame_off = 216 - (BYTES_PER_ARG * src1); // see tramp-mips.S
      genItype((instruction *)(code + base), LDop, REG_SP, ret, frame_off);
      base += INSN_SIZE;
    }
    else
    {
      ret = dst;
      frame_off = 512 + (BYTES_PER_ARG * (src1-8)); 
      genItype((instruction *)(code + base), LDop, REG_SP, ret, frame_off);
      base += INSN_SIZE;
    }
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

  TRACE_E( "emitR" );

  return ret;
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/

// return void
// [loadIndirOp, storeIndirOp, noOp, saveRegOp,
//  plusOp, minusOp, timesOp, divOp,
//  orOp, andOp, eqOp, neOp, lessOp, leOp, greaterOp, geOp]
void emitV(opCode op, Register src1, Register src2, Register dst, 
	   char *code, Address &base, bool /*noCost*/, int /* size */)
{
  TRACE_B( "emitV" );

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
    genRtype(insn, DADDUops, src1, src2, dst);
    base += INSN_SIZE;
    break;
  case minusOp:
    //fprintf(stderr, ">>> emit(minusOp)\n");
    genRtype(insn, DSUBUops, src1, src2, dst);
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
    genRtype(insn, DMULTUops, src1, src2, 0);
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
    genRtype(insn, DDIVUops, src1, src2, 0);
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

  TRACE_E( "emitV" );
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/

// [loadConstOp, loadOp]
void emitVload(opCode op, Address src1, Register /*src2*/, Register dst, 
	       char *code, Address &base, bool noCost, int size)
{
  TRACE_B( "emitVload" );

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
    if (size == sizeof(uint32_t)) {
      // 32-bit load (use "loadIndirOp")
      emitV(loadIndirOp, dst, 0, dst, code, base, noCost);
    } else if (size == sizeof(uint64_t)) {
      // 64-bit load
      genItype((instruction *)(code + base), LDop, dst, dst, 0);
      base += INSN_SIZE;
    } else {
      // bogus pointer size
      assert(0);
    }
    break;

  default: 
    assert(0);
  }

  TRACE_E( "emitVload" );
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/

// [storeOp]
void emitVstore(opCode op, Register src1, Register src2, Address dst, 
                char *code, Address &base, bool noCost, int size)
{
  TRACE_B( "emitVstore" );

  assert(op == storeOp);
  // "src1" : value register (from)
  // "src2" : scratch address register (to)
  // "dst"  : address value (to)
  //fprintf(stderr, ">>> emit(storeOp)\n");
  genLoadConst(src2, dst, code, base, noCost);
  if (size == sizeof(uint32_t)) {
    // 32-bit store (use "storeIndirOp")
    emitV(storeIndirOp, src1, 0, src2, code, base, noCost);
  } else if (size == sizeof(uint64_t)) {
    // 64-bit store
    genItype((instruction *)(code + base), SDop, src2, src1, 0);
    base += INSN_SIZE;
  } else {
    // bogus pointer size
    assert(0);
  }

  TRACE_E( "emitVstore" );
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/

// [updateCostOp]
void emitVupdate(opCode op, RegValue src1, Register /*src2*/, Address dst, 
		 char *code, Address &base, bool noCost)
{
  TRACE_B( "emitVupdate" );

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

  TRACE_E( "emitVupdate" );
}


/****************************************************************************/
/****************************************************************************/
/****************************************************************************/

/* emitImm(): This function is complicated because of the MIPS RISC
   architecture.  Specifically, the only immediate instruction
   primitives are "add" and "set less than". 
*/
// TODO - signed operations
// TODO - immediate insns use 32- or 64-bit operands?
void emitImm(opCode op, Register src, RegValue imm, Register dst, 
	     char *code, Address &base, bool noCost)
{
  TRACE_B( "emitImm" );

  //fprintf(stderr, ">>> emitImm(): op %s,%s,%i\n", reg_names[dst], reg_names[src], imm);
  instruction *insn = (instruction *)(code + base);
  int n;

  // immediate value should fit in immediate field
  switch (op) {
    
    /* arithmetic operands */
    
  case plusOp:
    if (!doNotOverflow(imm)) break;
    genItype(insn, DADDIUop, src, dst, imm);
    base += INSN_SIZE;

    TRACE_E( "emitImm" );

    return;
  case minusOp:
    if (!doNotOverflow(-imm)) break;
    genItype(insn, DADDIUop, src, dst, -imm);
    base += INSN_SIZE;

    TRACE_E( "emitImm" );

    return;
  case timesOp:
    if (!isPowerOf2(imm, n) || (n >= 64)) break;
    // use left shift for powers of 2
    if (n < 32) genRtype(insn, DSLLops, 0, src, dst, n);
    else genRtype(insn, DSLL32ops, 0, src, dst, n-32);
    base += INSN_SIZE;

    TRACE_E( "emitImm" );

    return;
  case divOp:
    if (!isPowerOf2(imm, n) || n >= 64) break;
    // use right shift for powers of 2
    if (n < 32) genRtype(insn, DSRAops, 0, src, dst, n);
    else genRtype(insn, DSRA32ops, 0, src, dst, n-32);
    base += INSN_SIZE;

    TRACE_E( "emitImm" );

    return;
    
    /* relational operands */
    
  case lessOp:
    if (!doNotOverflow(imm)) break;
    genItype(insn, SLTIUop, src, dst, imm);
    base += INSN_SIZE;

    TRACE_E( "emitImm" );

    return; 
  case eqOp:
    if (!doNotOverflow(-imm)) break;
    // saves one register over emitV()
    genItype(insn, DADDIUop, src, dst, -imm);
    genItype(insn+1, BEQLop, dst, REG_ZERO, 0x2);
    genItype(insn+2, ORIop, REG_ZERO, dst, 0x1);
    genItype(insn+3, ORIop, REG_ZERO, dst, 0x0);
    base += 4 * INSN_SIZE;

    TRACE_E( "emitImm" );

    return;
  case neOp:
    if (!doNotOverflow(-imm)) break;
    // saves one register over emitV()
    genItype(insn, DADDIUop, src, dst, -imm);
    genItype(insn+1, BNELop, dst, REG_ZERO, 0x2);
    genItype(insn+2, ORIop, REG_ZERO, dst, 0x1);
    genItype(insn+3, ORIop, REG_ZERO, dst, 0x0);
    base += 4 * INSN_SIZE;

    TRACE_E( "emitImm" );

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

    TRACE_E( "emitImm" );

    return;
  case andOp:
    if (!doNotOverflow(imm)) break;
    genItype(insn, ANDIop, src, dst, imm);
    base += INSN_SIZE;

    TRACE_E( "emitImm" );

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

  TRACE_E( "emitImm" );

  return;
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/

bool process::emitInferiorRPCheader(void *code_, Address &base)
{
  TRACE_B( "process::emitInferiorRPCheader" );

  //fprintf(stderr, ">>> process::emitInferiorRPCheader()\n");
  char *code = (char *)code_;
  instruction *insn = (instruction *)(code + base);

  // TODO: why 512 bytes? not using basetramp code, right?
  genItype(insn, DADDIUop, REG_SP, REG_SP, -512); // daddiu sp,sp,-512
  base += INSN_SIZE;

  TRACE_E( "process::emitInferiorRPCheader" );

  return true;
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/

// TODO: should offset parameters be Address's?
bool process::emitInferiorRPCtrailer(void *code_, Address &base,
				     unsigned &breakOffset,
				     bool stopForResult,
				     unsigned &stopForResultOffset,
				     unsigned &justAfter_stopForResultOffset)
{
  TRACE_B( "process::emitInferiorRPCtrailer" );

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
  // daddiu sp,sp,512
  genItype(insn, DADDIUop, REG_SP, REG_SP, 512);
  base += INSN_SIZE;
  // trap insn
  genTrap(insn+1);
  breakOffset = base;
  base += INSN_SIZE;
  // illegal insn
  genIll(insn+2);
  base += INSN_SIZE;

  TRACE_E( "process::emitInferiorRPCtrailer" );

  return true;
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/

int getInsnCost(opCode op)
{
  TRACE_B( "getInsnCost" );

  //fprintf(stderr, ">>> getInsnCost()\n");
  switch(op) {

  case plusOp:
  case minusOp: 
  case timesOp:
  case divOp:
  case orOp:
  case andOp:

    TRACE_E( "getInsnCost" );

    return 1;

  case lessOp:
  case greaterOp:

    TRACE_E( "getInsnCost" );

    return 1;

  case leOp:
  case geOp:
  case eqOp:
  case neOp:

    TRACE_E( "getInsnCost" );

    return 3;

  case noOp:

    TRACE_E( "getInsnCost" );

    return 1;

  case loadConstOp:

    TRACE_E( "getInsnCost" );

    return 3; // average = 3.0625

  case getAddrOp:
    // usually generates "loadConstOp" above

    TRACE_E( "getInsnCost" );

    return 3;

  case loadOp:
  case storeOp:

    TRACE_E( "getInsnCost" );

    return 4; // average = 4.0625

  case ifOp:
  case branchOp:

    TRACE_E( "getInsnCost" );

    return 2;

  case callOp:
    // assume 2 parameters
    // mov, mov, lui, ori, jalr, nop

    TRACE_E( "getInsnCost" );

    return 6;

  case trampPreamble:

    TRACE_E( "getInsnCost" );

    return 0;

  case trampTrailer:
    // padded in case indirect jump needed

    TRACE_E( "getInsnCost" );

    return 8;

  case getRetValOp:
  case getSysRetValOp: 
  case getParamOp:
  case getSysParamOp:      

    TRACE_E( "getInsnCost" );

    return 1;

  case loadIndirOp:
  case storeIndirOp:

    TRACE_E( "getInsnCost" );

    return 1;

  case updateCostOp:
    // padded for two constant loads (cost value and address)

    TRACE_E( "getInsnCost" );

    return 15;

  case saveRegOp:
    // not used on this platform

    TRACE_E( "getInsnCost" );

    assert(0);
    
  case loadFrameAddr:
  case storeFrameRelativeOp:
    // TODO: not implemented on this platform
    assert(0);

  default:
    assert(0);
  }

  TRACE_E( "getInsnCost" );

  return 0;
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/

// baseTramp assembly code symbols
extern "C" void baseTramp();
extern "C" void baseTramp_savePreInsn();
extern "C" void baseTramp_skipPreInsn();
extern "C" void baseTramp_globalPreBranch();
extern "C" void baseTramp_localPreBranch();
extern "C" void baseTramp_localPreReturn();
extern "C" void baseTramp_updateCostInsn();
extern "C" void baseTramp_restorePreInsn();
extern "C" void baseTramp_emulateInsn();
extern "C" void baseTramp_skipPostInsn();
extern "C" void baseTramp_savePostInsn();
extern "C" void baseTramp_globalPostBranch();
extern "C" void baseTramp_localPostBranch();
extern "C" void baseTramp_localPostReturn();
extern "C" void baseTramp_restorePostInsn();
extern "C" void baseTramp_returnInsn();
extern "C" void baseTramp_endTramp();

extern "C" void baseNonRecursiveTramp();
extern "C" void baseNonRecursiveTramp_savePreInsn();
extern "C" void baseNonRecursiveTramp_skipPreInsn();
extern "C" void baseNonRecursiveTramp_guardOnPre_begin();
extern "C" void baseNonRecursiveTramp_guardOnPre_end();
extern "C" void baseNonRecursiveTramp_globalPreBranch();
extern "C" void baseNonRecursiveTramp_localPreBranch();
extern "C" void baseNonRecursiveTramp_localPreReturn();
extern "C" void baseNonRecursiveTramp_guardOffPre_begin();
extern "C" void baseNonRecursiveTramp_guardOffPre_end();
extern "C" void baseNonRecursiveTramp_updateCostInsn();
extern "C" void baseNonRecursiveTramp_restorePreInsn();
extern "C" void baseNonRecursiveTramp_emulateInsn();
extern "C" void baseNonRecursiveTramp_skipPostInsn();
extern "C" void baseNonRecursiveTramp_savePostInsn();
extern "C" void baseNonRecursiveTramp_guardOnPost_begin();
extern "C" void baseNonRecursiveTramp_guardOnPost_end();
extern "C" void baseNonRecursiveTramp_globalPostBranch();
extern "C" void baseNonRecursiveTramp_localPostBranch();
extern "C" void baseNonRecursiveTramp_localPostReturn();
extern "C" void baseNonRecursiveTramp_guardOffPost_begin();
extern "C" void baseNonRecursiveTramp_guardOffPost_end();
extern "C" void baseNonRecursiveTramp_restorePostInsn();
extern "C" void baseNonRecursiveTramp_returnInsn();
extern "C" void baseNonRecursiveTramp_endTramp();

extern "C" void conservativeTramp();
extern "C" void conservativeTramp_savePreInsn();
extern "C" void conservativeTramp_skipPreInsn();
extern "C" void conservativeTramp_globalPreBranch();
extern "C" void conservativeTramp_localPreBranch();
extern "C" void conservativeTramp_localPreReturn();
extern "C" void conservativeTramp_updateCostInsn();
extern "C" void conservativeTramp_restorePreInsn();
extern "C" void conservativeTramp_emulateInsn();
extern "C" void conservativeTramp_skipPostInsn();
extern "C" void conservativeTramp_savePostInsn();
extern "C" void conservativeTramp_globalPostBranch();
extern "C" void conservativeTramp_localPostBranch();
extern "C" void conservativeTramp_localPostReturn();
extern "C" void conservativeTramp_restorePostInsn();
extern "C" void conservativeTramp_returnInsn();
extern "C" void conservativeTramp_endTramp();

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/

void initTramps()
{
  TRACE_B( "initTramps" );

  static bool inited = false;
  if (inited)
    {
      TRACE_E( "initTramps" );

      return;
    }
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

    if (i == (Address)baseTramp_savePreInsn)
      baseTemplate.savePreInsOffset = off;

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

    if (i == (Address)baseTramp_restorePreInsn)
      baseTemplate.restorePreInsOffset = off;

    if (i == (Address)baseTramp_emulateInsn)
      baseTemplate.emulateInsOffset = off;

    if (i == (Address)baseTramp_skipPostInsn)
      baseTemplate.skipPostInsOffset = off;

    if (i == (Address)baseTramp_savePostInsn)
      baseTemplate.savePostInsOffset = off;

    if (i == (Address)baseTramp_globalPostBranch)
      baseTemplate.globalPostOffset = off;

    if (i == (Address)baseTramp_localPostBranch)
      baseTemplate.localPostOffset = off;
    if (i == (Address)baseTramp_localPostReturn)
      baseTemplate.localPostReturnOffset = off;

    if (i == (Address)baseTramp_restorePostInsn)
      baseTemplate.restorePostInsOffset = off;

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

  // base non recursive trampoline template
  base = ( Address )baseNonRecursiveTramp;
  for( i = base;
       i < ( Address )baseNonRecursiveTramp_endTramp;
       i += INSN_SIZE )
    {
      Address off = i - base;
      // note: these should not be made into if..else blocks
      // (some of the label values are the same)

      if( i == ( Address )baseNonRecursiveTramp_savePreInsn )
	nonRecursiveBaseTemplate.savePreInsOffset = off;

      if( i == ( Address )baseNonRecursiveTramp_skipPreInsn )
	nonRecursiveBaseTemplate.skipPreInsOffset = off;

      if( i == ( Address )baseNonRecursiveTramp_globalPreBranch )
	nonRecursiveBaseTemplate.globalPreOffset = off;

      if( i == ( Address )baseNonRecursiveTramp_localPreBranch )
	nonRecursiveBaseTemplate.localPreOffset = off;
      if( i == ( Address )baseNonRecursiveTramp_localPreReturn )
	nonRecursiveBaseTemplate.localPreReturnOffset = off;

      if( i == ( Address )baseNonRecursiveTramp_updateCostInsn )
	nonRecursiveBaseTemplate.updateCostOffset = off;

      if( i == ( Address )baseNonRecursiveTramp_restorePreInsn )
	nonRecursiveBaseTemplate.restorePreInsOffset = off;

      if( i == ( Address )baseNonRecursiveTramp_emulateInsn )
	nonRecursiveBaseTemplate.emulateInsOffset = off;

      if( i == ( Address )baseNonRecursiveTramp_skipPostInsn )
	nonRecursiveBaseTemplate.skipPostInsOffset = off;

      if( i == ( Address )baseNonRecursiveTramp_savePostInsn )
	nonRecursiveBaseTemplate.savePostInsOffset = off;

      if( i == ( Address )baseNonRecursiveTramp_globalPostBranch )
	nonRecursiveBaseTemplate.globalPostOffset = off;

      if( i == ( Address )baseNonRecursiveTramp_localPostBranch )
	nonRecursiveBaseTemplate.localPostOffset = off;
      if( i == ( Address )baseNonRecursiveTramp_localPostReturn )
	nonRecursiveBaseTemplate.localPostReturnOffset = off;

      if( i == ( Address )baseNonRecursiveTramp_restorePostInsn )
	nonRecursiveBaseTemplate.restorePostInsOffset = off;

      if( i == ( Address )baseNonRecursiveTramp_returnInsn )
	nonRecursiveBaseTemplate.returnInsOffset = off;

      if( i == ( Address )baseNonRecursiveTramp_guardOnPre_begin )
	{
	  DEBUG( "baseNonRecursiveTramp_guardOnPre_begin offset initialized." );
	  nonRecursiveBaseTemplate.guardOnPre_beginOffset = off;
	}
      if( i == ( Address )baseNonRecursiveTramp_guardOffPre_begin )
	{
	  DEBUG( "baseNonRecursiveTramp_guardOffPre_begin offset initialized." );
	  nonRecursiveBaseTemplate.guardOffPre_beginOffset = off;
	}
      if( i == ( Address )baseNonRecursiveTramp_guardOnPost_begin )
	{
	  DEBUG( "baseNonRecursiveTramp_guardOnPost_begin offset initialized." );
	  nonRecursiveBaseTemplate.guardOnPost_beginOffset = off;
	}
      if( i == ( Address )baseNonRecursiveTramp_guardOffPost_begin )
	{
	  DEBUG( "baseNonRecursiveTramp_guardOffPost_begin offset initialized." );
	  nonRecursiveBaseTemplate.guardOffPost_beginOffset = off;
	}
      if( i == ( Address )baseNonRecursiveTramp_guardOnPre_end )
	{
	  DEBUG( "baseNonRecursiveTramp_guardOnPre_end offset initialized." );
	  nonRecursiveBaseTemplate.guardOnPre_endOffset = off;
	}
      if( i == ( Address )baseNonRecursiveTramp_guardOffPre_end )
	{
	  DEBUG( "baseNonRecursiveTramp_guardOffPre_end offset initialized." );
	  nonRecursiveBaseTemplate.guardOffPre_endOffset = off;
	}
      if( i == ( Address )baseNonRecursiveTramp_guardOnPost_end )
	{
	  DEBUG( "baseNonRecursiveTramp_guardOnPost_end offset initialized." );
	  nonRecursiveBaseTemplate.guardOnPost_endOffset = off;
	}
      if( i == ( Address )baseNonRecursiveTramp_guardOffPost_end )
	{
	  DEBUG( "baseNonRecursiveTramp_guardOffPost_end offset initialized." );
	  nonRecursiveBaseTemplate.guardOffPost_endOffset = off;
	}
    }
  nonRecursiveBaseTemplate.trampTemp = ( void * )baseNonRecursiveTramp;
  // TODO: include endTramp insns? (2 nops)
  nonRecursiveBaseTemplate.size =
    ( Address )baseNonRecursiveTramp_endTramp - ( Address )baseNonRecursiveTramp;
  nonRecursiveBaseTemplate.cost = 8;           // cost if both pre- and post- skipped
  nonRecursiveBaseTemplate.prevBaseCost = 135; // cost of [global_pre_branch, update_cost)
  nonRecursiveBaseTemplate.postBaseCost = 134; // cost of [global_post_branch, return_insn)
  nonRecursiveBaseTemplate.prevInstru = false;
  nonRecursiveBaseTemplate.postInstru = false;

  /* For the conservative version of the base tramp. */
  // register space
  conservativeRegSpace = new registerSpace(nDead, Dead, 0, NULL);
  assert(conservativeRegSpace);

  // conservative base trampoline template
  base = (Address)conservativeTramp;
  for (i = base; i < (Address)conservativeTramp_endTramp; i += INSN_SIZE) {
    Address off = i - base;
    // note: these should not be made into if..else blocks
    // (some of the label values are the same)
    if (i == (Address)conservativeTramp_savePreInsn)
      conservativeTemplate.savePreInsOffset = off;
    if (i == (Address)conservativeTramp_skipPreInsn)
      conservativeTemplate.skipPreInsOffset = off;
    if (i == (Address)conservativeTramp_globalPreBranch)
      conservativeTemplate.globalPreOffset = off;
    if (i == (Address)conservativeTramp_localPreBranch)
      conservativeTemplate.localPreOffset = off;
    if (i == (Address)conservativeTramp_localPreReturn)
      conservativeTemplate.localPreReturnOffset = off;
    if (i == (Address)conservativeTramp_updateCostInsn)
      conservativeTemplate.updateCostOffset = off;
    if (i == (Address)conservativeTramp_restorePreInsn)
      conservativeTemplate.restorePreInsOffset = off;
    if (i == (Address)conservativeTramp_emulateInsn)
      conservativeTemplate.emulateInsOffset = off;
    if (i == (Address)conservativeTramp_skipPostInsn)
      conservativeTemplate.skipPostInsOffset = off;
    if (i == (Address)conservativeTramp_savePostInsn)
      conservativeTemplate.savePostInsOffset = off;
    if (i == (Address)conservativeTramp_globalPostBranch)
      conservativeTemplate.globalPostOffset = off;
    if (i == (Address)conservativeTramp_localPostBranch)
      conservativeTemplate.localPostOffset = off;
    if (i == (Address)conservativeTramp_localPostReturn)
      conservativeTemplate.localPostReturnOffset = off;
    if (i == (Address)conservativeTramp_restorePostInsn)
      conservativeTemplate.restorePostInsOffset = off;
    if (i == (Address)conservativeTramp_returnInsn)
      conservativeTemplate.returnInsOffset = off;
  }
  conservativeTemplate.trampTemp = (void *)conservativeTramp;
  // TODO: include endTramp insns? (2 nops)
  conservativeTemplate.size = (Address)conservativeTramp_endTramp -
			      (Address)conservativeTramp;
  // XXX These costs are copied from the normal base tramp, they
  //     probably need to be increased.
  // cost if both pre- and post- skipped
  conservativeTemplate.cost = 8;
  // cost of [global_pre_branch, update_cost)
  conservativeTemplate.prevBaseCost = 135;
  // cost of [global_post_branch, return_insn)
  conservativeTemplate.postBaseCost = 134;
  conservativeTemplate.prevInstru = false;
  conservativeTemplate.postInstru = false;

  TRACE_E( "initTramps" );
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/

Register emitFuncCall(opCode op, registerSpace *rs, char *code, Address &base, 
		      const vector<AstNode *> &params, const string &calleeName,
		      process *p, bool noCost, const function_base *callee)
{
  TRACE_B( "emitFuncCall" );

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
    genItype (insn, DADDIUop, REG_SP, REG_SP, -stackBytes);
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
    genItype(insn, DADDIUop, REG_SP, REG_SP, stackBytes);
    base += INSN_SIZE;
  }

  // debug
  //fprintf(stderr, "  emitFuncCall code:\n");
  //dis(code+base_orig, NULL, (base-base_orig)/INSN_SIZE);

  TRACE_E( "emitFuncCall" );

  return REG_V0;
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/

void returnInstance::installReturnInstance(process *p)
{
  TRACE_B( "returnInstance::installReturnInstance" );

  //fprintf(stderr, ">>> returnInstance::installReturnInstance(%0#10x, %i bytes)\n", addr_, instSeqSize);
  p->writeTextSpace((void *)addr_, instSeqSize, instructionSeq);
  //disDataSpace(p, (void *)addr_, 4, "!!! jump to basetramp: ");
  installed = true;

  TRACE_E( "returnInstance::installReturnInstance" );
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/

/***********************/
/*** HERE BE DRAGONS ***/
/***********************/

int relocateInstruction(instruction *insn, Address origAddr, 
			 Address relocAddr, int numInsns, process *p)
{
  TRACE_B( "relocateInstruction" );

  //fprintf(stderr, "!!! relocateInstruction(0x%08x): ", relocAddr);
  //dis(insn, (void *)origAddr, 1, ">>> reloc ");

  if (isBranchInsn(*insn)) {
    Address target = (origAddr + INSN_SIZE) + (insn->itype.simm16 << 2);
    RegValue newOffset = target - (relocAddr + INSN_SIZE);

    if (newOffset >= BRANCH_MIN && newOffset <= BRANCH_MAX) {
      insn->itype.simm16 = newOffset >> 2;
    } else {
      /*
       * The branch is too far, so we need to branch to another location
       * and hopefully we can use a jump from there.
       */
      Address extra = inferiorMalloc(p, 2 * INSN_SIZE, anyHeap, relocAddr);
      if (!extra) {
	TRACE_E("relocateInstruction");
	return -1;
      }

      if (region_num(extra + INSN_SIZE) != region_num(target)) {
	TRACE_E("relocateInstruction");
	return -1;
      }

      /* Fix up the branch to go to our jump. */
      newOffset = extra - (relocAddr + INSN_SIZE);

      if (newOffset >= BRANCH_MIN && newOffset <= BRANCH_MAX) {
	  /* We can just redirect the branch to a jump. */
	  insn->itype.simm16 = newOffset >> 2;

	  /* Generate the jump. */
	  instruction jumpCode[2];
	  genJump(jumpCode, extra, target);
	  genNop(&jumpCode[1]);

	  /* Write the jump into the inferior. */
	  p->writeDataSpace((caddr_t)extra, sizeof(jumpCode),
			    (caddr_t)jumpCode);
        } else {
	  /*
	   * The extra space is too far away to redirect the branch, so
	   * we need to replace it with a jump and move the branch and it's
	   * delay slot to the extra space.  We'll need more space for this,
	   * so we'll reallocate extra.
	   */

	  /* We'll need to move the delay slot, too. */
	  assert(numInsns > 1);

	  vector<addrVecType> pointsToCheck;
	  inferiorFree(p, extra, pointsToCheck);
	  Address extra = inferiorMalloc(p, 6 * INSN_SIZE, anyHeap, relocAddr);
	  if (!extra) {
	    TRACE_E("relocateInstruction");
	    return -1;
	  }

	  assert(region_num(extra + (5*INSN_SIZE)) == region_num(target));
	  assert(region_num(relocAddr + INSN_SIZE) == region_num(extra));
	  assert(!isBranchInsn(insn[1]) && !isJumpInsn(insn[1]));

	  instruction extraCode[6];
	  extraCode[0].raw = insn[0].raw;
	  extraCode[0].itype.simm16 = 3;
	  extraCode[1].raw = insn[1].raw;
	  genJump(&extraCode[2],
		  extra + (2*INSN_SIZE), relocAddr + (2*INSN_SIZE));
	  genNop(&extraCode[3]);
	  genJump(&extraCode[4],
		  extra + (4*INSN_SIZE), target);
	  genNop(&extraCode[5]);

	  /* Write the jump into the inferior. */
	  p->writeDataSpace((caddr_t)extra, sizeof(extraCode),
			    (caddr_t)extraCode);
	  /* Change the instructions back in the base tramp to a jump
	     to our new code. */
	  genJump(&insn[0], relocAddr, extra);
	  genNop(&insn[1]);

	  TRACE_E( "relocateInstruction" );

	  return 2;
	}
      }
    } else if (isJumpInsn(*insn)) {
      switch (insn->decode.op) {
	case Jop:
	case JALop: { // PC-region branch
	  /* To calculate the address, uncomment this:
	  Address hi = (origAddr + INSN_SIZE) & (REGION_NUM_MASK);
	  Address lo = (insn->jtype.imm26 << 2) & REGION_OFF_MASK;
	  Address target = hi | lo;
	  */
	  /* XXX Is there any way to get around this restriction? */
	  assert(region_num(origAddr + INSN_SIZE) ==
		 region_num(relocAddr + INSN_SIZE));
	  } break;
	case SPECIALop: // indirect (register) jump - don't have to do anything
	  break;
	default:
	  fprintf(stderr, "relocateInstruction: bogus instruction %0#10x\n",
		  insn->raw);
	  assert(0);
      }
  }

  TRACE_E( "relocateInstruction" );

  return 1;
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/

void generate_base_tramp_recursive_guard_code( process * p,
					       char * code,
					       NonRecursiveTrampTemplate * templ )
{
  TRACE_B( "generate_base_tramp_recursive_guard_code" );

  /* prepare guard flag memory, if needed */
  Address guardFlagAddress = p->getTrampGuardFlagAddr();
  if( guardFlagAddress == 0 )
    {
	int initial_value = 1;
	DEBUG( "Attempting to allocate guard space." );
	guardFlagAddress = inferiorMalloc( p, sizeof( int ), dataHeap );
	DEBUG( "Address of guard is 0x" << setw( 16 ) << setfill( '0' ) << setbase(16)
	       << guardFlagAddress << dec << "." );
	// Zero out the new value
	p->writeDataSpace( ( void * )guardFlagAddress, sizeof( int ), & initial_value );
	DEBUG( "Initialized guardFlagAddress." );

	p->setTrampGuardFlagAddr( guardFlagAddress );
    }

  /* The 64-bit address is split into 4 16-bit chunks: A, B, C, D */
  /* A is the most significant chunk, D the least significant.    */

  unsigned short int chunk_A = ( guardFlagAddress >> 48 ) & 0x000000000000FFFF;
  unsigned short int chunk_B = ( guardFlagAddress >> 32 ) & 0x000000000000FFFF;
  unsigned short int chunk_C = ( guardFlagAddress >> 16 ) & 0x000000000000FFFF;
  unsigned short int chunk_D = ( guardFlagAddress       ) & 0x000000000000FFFF;

  DEBUG( "Using flag address 0x"
	 << setfill('0') << setw(16) << setbase(16) << guardFlagAddress );
  DEBUG( "split into 16-bit chunks: "
	 << "0x" << setfill('0') << setw(4) << setbase(16) << chunk_A << " "
	 << "0x" << setfill('0') << setw(4) << setbase(16) << chunk_B << " "
	 << "0x" << setfill('0') << setw(4) << setbase(16) << chunk_C << " "
	 << "0x" << setfill('0') << setw(4) << setbase(16) << chunk_D
	 << dec << "." );

  /* populate guardOnPre section */
  instruction * guardOnInsn = ( instruction * )( code + templ->guardOnPre_beginOffset );
  genItype ( guardOnInsn     , LUIop,  0, 12, chunk_A );
  genItype ( guardOnInsn + 1 , ORIop, 12, 12, chunk_B );
  genItype ( guardOnInsn + 2 , LUIop,  0, 13, chunk_C );
  genItype ( guardOnInsn + 3 , ORIop, 13, 13, chunk_D );

  genItype ( guardOnInsn + 7 , BEQop, 12, 0,
	     ( templ->guardOffPre_endOffset -
	       ( templ->guardOnPre_beginOffset + ( 7 + 1 ) * INSN_SIZE ) ) / INSN_SIZE );

  /* populate guardOffPre section */
  instruction * guardOffInsn = ( instruction * )( code + templ->guardOffPre_beginOffset );
  genItype ( guardOffInsn     , LUIop,  0, 12, chunk_A );
  genItype ( guardOffInsn + 1 , ORIop, 12, 12, chunk_B );
  genItype ( guardOffInsn + 2 , LUIop,  0, 13, chunk_C );
  genItype ( guardOffInsn + 3 , ORIop, 13, 13, chunk_D );

  /* populate guardOnPost section */
  guardOnInsn = ( instruction * )( code + templ->guardOnPost_beginOffset );
  genItype ( guardOnInsn     , LUIop,  0, 12, chunk_A );
  genItype ( guardOnInsn + 1 , ORIop, 12, 12, chunk_B );
  genItype ( guardOnInsn + 2 , LUIop,  0, 13, chunk_C );
  genItype ( guardOnInsn + 3 , ORIop, 13, 13, chunk_D );

  genItype ( guardOnInsn + 7 , BEQop, 12, 0,
	     ( templ->guardOffPost_endOffset -
	       ( templ->guardOnPost_beginOffset + ( 7 + 1 ) * INSN_SIZE ) ) / INSN_SIZE );

  /* populate guardOffPost section */
  guardOffInsn = ( instruction * )( code + templ->guardOffPost_beginOffset );
  genItype ( guardOffInsn     , LUIop,  0, 12, chunk_A );
  genItype ( guardOffInsn + 1 , ORIop, 12, 12, chunk_B );
  genItype ( guardOffInsn + 2 , LUIop,  0, 13, chunk_C );
  genItype ( guardOffInsn + 3 , ORIop, 13, 13, chunk_D );

  TRACE_E( "generate_base_tramp_recursive_guard_code" );
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/

trampTemplate * installBaseTramp( process * p,
				  instPoint * & ip,
				  bool trampRecursiveDesired = false )
{
  TRACE_B( "installBaseTramp" );

  // fprintf(stderr, ">>> installBaseTramp()\n");

  int ipSize = ip->size_;          // instPoint footprint size
  Address objAddr;
  p->getBaseAddress( ip->iPgetOwner(), objAddr );
  Address ipAddr = objAddr + ip->address();

  int btSize;
  char * code;    // base tramp temporary address in mutator/Paradyn
  trampTemplate * ret;

  if( ip->type() == IPT_OTHER )
    {
      assert( trampRecursiveDesired );

      btSize = conservativeTemplate.size;  // basetramp size

      // allocate basetramp buffer (local)
      code = new char[ btSize ];
      memcpy( code, ( char * )conservativeTemplate.trampTemp, btSize );

      // trampTemplate return value: copy template
      ret = new trampTemplate( conservativeTemplate );
    }
  else if( trampRecursiveDesired )
    {
      btSize = baseTemplate.size;  // basetramp size  

      // allocate basetramp buffer (local)
      code = new char[ btSize ];
      memcpy( code, ( char * )baseTemplate.trampTemp, btSize );

      // trampTemplate return value: copy template
      ret = new trampTemplate( baseTemplate );
    }
  else
    {
      DEBUG( "Using the base non-recursive trampoline." );

      btSize = nonRecursiveBaseTemplate.size;  // basetramp size  

      // allocate basetramp buffer (local)
      code = new char[ btSize ];
      memcpy( code, ( char * )nonRecursiveBaseTemplate.trampTemp, btSize );

      // NonRecursiveTrampTemplate return value: copy template
      ret = new NonRecursiveTrampTemplate( nonRecursiveBaseTemplate );
    }

  // btAddr : base tramp address in inferior process
  // allocate basetramp buffer (inferior)
  Address btAddr = inferiorMalloc( p, btSize, anyHeap, ipAddr );
  assert( btAddr );
  // TODO: if inferiorMalloc fails, try again w/o address constraints

  ret->baseAddr = btAddr;
  ret->costAddr = btAddr + ret->updateCostOffset;
  
  DEBUG( "Base trampoline address: 0x" <<
	 setfill('0') << setbase(16) << setw(16) << btAddr << setbase(10) );

  /*** populate basetramp slots ***/
  Address toAddr, fromAddr;

  /* populate emulateInsn slot */
  // fprintf(stderr, "  instPoint footprint: %i insns\n", ipSize/INSN_SIZE);
  int insnOff = 0;
  while( insnOff < ipSize )
    {
      int btOff = ret->emulateInsOffset + insnOff;
      instruction *insn = ( instruction * )( code + btOff );
      toAddr = btAddr + btOff;
      fromAddr = ipAddr + insnOff;
      // copy original insn and perform relocation transformation
      insn->raw = ip->owner_->get_instruction( fromAddr - objAddr );
      int insnsRelocated = relocateInstruction( insn, fromAddr, toAddr,
						ipSize - insnOff, p );
      assert(insnsRelocated > 0);
      insnOff += insnsRelocated * INSN_SIZE;
    }
  
  /* populate returnInsn slot */
  instruction * returnInsn = ( instruction * )( code + ret->returnInsOffset );
  fromAddr = btAddr + ret->returnInsOffset;
  toAddr = ipAddr + ipSize;
  // TODO: if can't do single-insn jump, build multi-insn jump
  genJump( returnInsn, fromAddr, toAddr );
  genNop( returnInsn + 1 ); // delay slot

  /* populate skipPreInsn slot */
  instruction * skipPreInsn = ( instruction * )( code + ret->skipPreInsOffset );
  fromAddr = btAddr + ret->skipPreInsOffset;
  toAddr = btAddr + ret->updateCostOffset;
  genBranch( skipPreInsn, fromAddr, toAddr );
  genNop( skipPreInsn + 1 ); // delay slot

  /* populate skipPostInsn slot */
  instruction * skipPostInsn = ( instruction * )( code + ret->skipPostInsOffset );
  fromAddr = btAddr + ret->skipPostInsOffset;
  toAddr = btAddr + ret->returnInsOffset;
  genBranch( skipPostInsn, fromAddr, toAddr );
  genNop( skipPostInsn + 1 ); // delay slot

  if( ! trampRecursiveDesired )
    {
      generate_base_tramp_recursive_guard_code( p,
						code,
						( NonRecursiveTrampTemplate * )ret );
      /* mihai Mon Feb 21 15:28:57 CST 2000: note that the cast above is safe
	 due to the fact that we know that is trampRecursiveDesired is false,
	 than we are using the base non recursive tramp. */
    }

  /*
    implicitly populated fields (NOPs):
    - updateCostInsn
    - globalPreBranch
    - localPreBranch
    - globalPostBranch
    - localPostBranch
  */

  /* debug */
  // fprintf( stderr, "  base trampoline code (%ld insns):\n", btSize / INSN_SIZE );
  // dis( code, ( void * )btAddr, btSize / INSN_SIZE, "  " );

  // copy basetramp to application
  p->writeDataSpace( ( void * )btAddr, btSize, code );
  delete[] code;

  TRACE_E( "installBaseTramp" );

  return ret;
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/

trampTemplate *findAndInstallBaseTramp(process *p, 
				       instPoint *&ip,
				       returnInstance *&retInst,
				       bool trampRecursiveDesired,
				       bool /*noCost*/)
{
  TRACE_B( "findAndInstallBaseTramp" );

  //fprintf(stderr, ">>> findAndInstallBaseTramp(): "); ip->print();
  retInst = NULL;
  trampTemplate *ret = NULL;

  // check if base tramp already exists
  if (p->baseMap.find((const instPoint *)ip, ret))
    { 
      TRACE_E( "findAndInstallBaseTramp" );

      return ret;
    }

  pd_Function *fn = (pd_Function *)ip->func_;
  if (fn->isTrapFunc()) { 
    // TODO: relocate function to instrument?
  }

  // runtime address of instPoint
  Address objAddr = 0;
  p->getBaseAddress(ip->iPgetOwner(), objAddr);
  Address ipAddr = objAddr + ip->address(p);

  ret = installBaseTramp(p, ip, trampRecursiveDesired );

  // debug --csserra
  /*
  char buf[1024];
  sprintf(buf, "!!! installed basetramp (%i insns): ", ret->size/INSN_SIZE);
  ip->print(stderr, buf);
  disDataSpace(p, (void *)ret->baseAddr, ret->size/INSN_SIZE, "  ");
  */

  // generate jump from instPoint to basetramp
  instruction *insn = new instruction[2];
  genJump(insn, ipAddr, ret->baseAddr);
  genNop(insn+1);
  retInst = new returnInstance(insn, 2*INSN_SIZE, ipAddr, 2*INSN_SIZE);
  
  if (ret) p->baseMap[ip] = ret;

  TRACE_E( "findAndInstallBaseTramp" );

  return ret;
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/

void installTramp(instInstance *inst, char *code, int codeSize)
{
  TRACE_B( "installTramp" );

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
  /*
  char buf[1024];
  sprintf(buf, "!!! installed minitramp (%i insns): ", codeSize/INSN_SIZE);
  inst->location->print(stderr, buf);
  disDataSpace(inst->proc, (void *)inst->trampBase, codeSize/INSN_SIZE, "  ");
  */

  TRACE_E( "installTramp" );
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/

void instWaitingList::cleanUp(process *p, Address pc)
{
  TRACE_B( "instWaitingList::cleanUp" );

  fprintf(stderr, ">>> instWaitingList::cleanUp()\n");
  p->writeTextSpace((void *)pc, INSN_SIZE, &relocatedInstruction);
  p->writeTextSpace((void *)addr_, instSeqSize, instructionSeq);

  TRACE_E( "instWaitingList::cleanUp" );
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/

// parse backwards to find "ld t9,-XX(gp)" insn (or analogue)
static int got_ld_off(const image *owner,
		      Address start,
		      int last_off,
		      Register jump_reg)
{

  TRACE_B( "got_ld_off" );

  instruction i2;
  for (int off = last_off; off >= 0; off -= INSN_SIZE) {
    i2.raw = owner->get_instruction(start+off);
    // "lw <reg>,X(gp)"
    // "ld <reg>,X(gp)"
    if ((isInsnType(i2, LWmask, LWmatch) || 
	 isInsnType(i2, LDmask, LDmatch)) &&
	i2.itype.rs == REG_GP &&
	i2.itype.rt == jump_reg)
    {
      TRACE_E( "got_ld_off" );

      return off;
    }
  }

  TRACE_E( "got_ld_off" );

  return -1;
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/

static int pdcmp_got_name(const char *got_name_, const string &pd_name)
{
  TRACE_B( "pdcmp_got_name" );

  string got_name = got_name_;
  if (pd_name == (got_name) ||
      pd_name == ("_" + got_name) ||
      pd_name == (got_name + "_") ||
      pd_name == ("__" + got_name))
  {
    TRACE_E( "pdcmp_got_name" );

    return 0;
  }

  TRACE_E( "pdcmp_got_name" );

  return 1;
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/

// replaceFunctionCall(): two requirements --
// (1) "ip" must be a call site
// (2) "ip" must not be instrumented yet
// NOTE: modifying $t9 in the delay slot of "jalr ra,t9" does not work
bool process::replaceFunctionCall(const instPoint *ip, 
				  const function_base *newFunc)
{
  TRACE_B( "process::replaceFunctionCall" );

  // runtime address of instPoint
  Address pt_base  = 0;
  getBaseAddress(ip->iPgetOwner(), pt_base);
  Address pt_addr = pt_base + ip->address(0);

  /*
  fprintf(stderr, ">>> replaceFunctionCall(): <0x%016lx:%s> to \"%s\"\n",
	  pt_addr, 
	  ip->iPgetFunction()->prettyName().string_of(),
	  (newFunc) ? (newFunc->prettyName().string_of()) : ("NOP"));
  */

  // requirement #1
  if (ip->type() != IPT_CALL)
    {
      TRACE_E( "process::replaceFunctionCall" );

      return false;
    }
  // requirement #2
  if (baseMap.defines(ip))
    {
      TRACE_E( "process::replaceFunctionCall" );

      return false;
    }

  // if "newFunc" is null, stomp existing call with NOP
  if (newFunc == NULL) {
    generateNoOp(this, pt_addr);

    TRACE_E( "process::replaceFunctionCall" );

    return true;
  }

  // resolve new callee
  pd_Function *dst2_pdf = (pd_Function *)const_cast<function_base*>(newFunc);
  Address dst2_base = 0;
  getBaseAddress(dst2_pdf->file()->exec(), dst2_base);
  Address dst2_addr = dst2_base + dst2_pdf->getAddress(0);

  /* NOTE: Calling conventions require that $t9 contain the callee
     address.  This holds even for some non-"jalr ra,t9" call insns.
     To correctly replace the function call, we need to parse
     backwards to check for an "ld t9,-XX(gp)" insn and, if present,
     modify it to point to the new callee's GOT entry.  One
     complication is that "jalr"-type calls may use a non-standard
     jump register.  */
  // check for non-default jump register (i.e. "jalr ra,RR")
  Register jump_reg = REG_T9;
  if (isInsnType(ip->origInsn_, JALRmask, JALRmatch)) {
    jump_reg = ip->origInsn_.rtype.rs;
  }
  // parsing stuff
  const Object &elf = ip->iPgetOwner()->getObject();
  const image *owner = ip->iPgetOwner();
  pd_Function *ip_pdf = (pd_Function *)ip->iPgetFunction();
  // parse backwards to check for "ld RR,-XX(gp)" insn
  Address fn_start = ip_pdf->getAddress(0);
  int ld_off = got_ld_off(owner, fn_start, ip->offset()+INSN_SIZE, jump_reg);
  Address ld_addr;
  instruction ld_insn;
  int gp_disp1, gp_disp2 = -1;
  if (ld_off != -1) {
    // fetch "ld RR,-XX(gp)" insn
    ld_addr = pt_base + fn_start + ld_off;
    ld_insn.raw = owner->get_instruction(fn_start + ld_off);
    assert(ld_insn.itype.op == LDop ||
	   ld_insn.itype.op == LWop);
    assert(ld_insn.itype.rs == REG_GP);
    // old GOT entry
    gp_disp1 = ld_insn.itype.simm16;
    // new GOT entry (modify insn)
    gp_disp2 = elf.got_gp_disp(dst2_pdf->prettyName().string_of());
    ld_insn.itype.simm16 = gp_disp2;
  }

  /* NOTE: At this point, things differ depending on whether this is a
     direct or indirect function call. */
  if (isInsnType(ip->origInsn_, JALRmask, JALRmatch)) {
    // indirect function call

    assert(ip->origInsn_.rtype.rd == REG_RA);

    if (ip->hint_got_) { 
      // GOT-based function call
      // --- sequence #1 ---
      //   ld    RR,-XX(gp)
      //   ...
      //   jalr  ra,RR
      
      // requirement: must know location of "ld RR,-XX(gp)" insn
      if (ld_off == -1)
	{
	  TRACE_E( "process::replaceFunctionCall" );

	  return false;
	}
      // requirement: new callee must have GOT entry
      if (gp_disp2 == -1)
	{
	  TRACE_E( "process::replaceFunctionCall" );

	  return false;
	}

      // write modified insn back
      bool ret = writeTextSpace((void *)ld_addr, INSN_SIZE, &ld_insn);
      if (!ret)
	{
	  TRACE_E( "process::replaceFunctionCall" );

	  return false;
	}

      TRACE_E( "process::replaceFunctionCall" );

      return true;
    } else { 
      // pointer-based function call: unresolvable

      TRACE_E( "process::replaceFunctionCall" );

      return false;
    }
    
  } else {
    // direct function call
    // --- sequence #1 ---
    //   jal   XX
    // --- sequence #2 ---
    //   b<op> XX
    // --- sequence #3 ---
    //   ld    t9,-XX(gp)
    //   ...
    //   jal   XX
    // --- sequence #4 ---
    //   ld    t9,-XX(gp)
    //   ...
    //   b<op> XX
    
    /* check if need to modify "ld t9,-XX(gp)" insn: conditions are
       (a) "ld" insn is present and (b) old GOT entry corresponds to
       old callee */
    bool use_got_ld = false;
    if (ld_off != -1) {

      // resolve old callee
      function_base *dst1_fn;
      instPoint *ip2 = const_cast<instPoint *>(ip);
      findCallee(*ip2, dst1_fn);
      /* since these are "direct" function calls (i.e. absolute jump
         or relative branch), it should always be possible to resolve
         the original callee */
      assert(dst1_fn != NULL);

      // resolve GOT entry
      Address got_entry_off = elf.get_gp_value() + gp_disp1 - pt_base;
      const char *got_name = elf.got_entry_name(got_entry_off);

      // check that GOT entry actually corresponds to callee
      if (got_name && 
	  pdcmp_got_name(got_name, dst1_fn->prettyName()) == 0)
      {
	// requirement: new callee must have GOT entry
	if (gp_disp2 == -1)
	  {
	    TRACE_E( "process::replaceFunctionCall" );

	    return false;
	  }
	use_got_ld = true;
      }
    }
    
    // generate call 
    instruction pt_insn;
    signed long dst2_disp = (dst2_addr - (pt_addr + INSN_SIZE)) >> 2;
    if (use_got_ld) {
      // use "jalr ra,RR"
      genRtype(&pt_insn, JALRops, jump_reg, 0, REG_RA);
    } else if (region_num(pt_addr + INSN_SIZE) == region_num(dst2_addr)) {
      // use "jal <dst2_addr>" insn
      pt_insn.jtype.op = JALop;
      pt_insn.jtype.imm26 = (dst2_addr & REGION_OFF_MASK) >> 2;
    } else if (dst2_disp >= MIN_IMM16 && dst2_disp <= MAX_IMM16) {
      // use "bgezal zero,<dst2_addr>" insn
      pt_insn.regimm.op = REGIMMop;
      pt_insn.regimm.opr = BGEZALopr;
      pt_insn.regimm.rs = REG_ZERO;
      pt_insn.regimm.simm16 = dst2_disp;
    } else {
      // new callee unreachable

      TRACE_E( "process::replaceFunctionCall" );

      return false;
    }

    // write modified "ld t9,-XX(gp)" insn back, if used
    if (use_got_ld) {
      //disDataSpace(this, (void *)ld_addr, 1, "    ");
      bool ret1 = writeTextSpace((void *)ld_addr, INSN_SIZE, &ld_insn);
      if (!ret1)
	{
	  TRACE_E( "process::replaceFunctionCall" );

	  return false;
	}
      //disDataSpace(this, (void *)ld_addr, 1, "    ");
    }	

    // write modified call insn back
    // TODO: cleanup "ld t9,-XX(gp)" insn
    //disDataSpace(this, (void *)pt_addr, 1, "    ");
    bool ret2 = writeTextSpace((void *)pt_addr, INSN_SIZE, &pt_insn);
    if (!ret2)
      {
	TRACE_E( "process::replaceFunctionCall" );

	return false;
      }
    //disDataSpace(this, (void *)pt_addr, 1, "    ");

    TRACE_E( "process::replaceFunctionCall" );

    return true;
  }

  // return false; /* commented out by Mihai. Not needed, generates a compiler warning. */ 
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/

// Emit code to jump to function CALLEE without linking.  (I.e., when
// CALLEE returns, it returns to the current caller.)
void emitFuncJump(opCode /*op*/, 
		  char * /*i*/, Address & /*base*/, 
		  const function_base * /*callee*/, 
		  process * /*proc*/)
{
  TRACE_B( "emitFuncJump" );

     /* Unimplemented on this platform! */
     assert(0);

  TRACE_E( "emitFuncJump" );
}

/* TODO: This function is supposed to mirror the name resolution
   algorithm of the runtime linker (rld).  This is problematic, so we
   settle for a weak approximation.  Namely, search each module in
   order, starting with the executable. */
/* findFunctionLikeRld(): The reason for the underscore kludges here
   is that the function "names" come from the .dynstr table, by way of
   .got entries.  These strings do not necessarily match paradynd's
   list of symbol names. */
static function_base *findFunctionLikeRld(process *p, const string &fn_name)
{
  TRACE_B( "findFunctionLikeRld" );

  function_base *ret = NULL;
  string name;

  // pass #1: unmodified
  name = fn_name;
  ret = p->findOneFunctionFromAll(name);
  if (ret)
    {
      TRACE_E( "findFunctionLikeRld" );

      return ret;
    }

  // pass #2: leading underscore (C)
  name = "_" + fn_name;
  ret = p->findOneFunctionFromAll(name);
  if (ret)
    {
      TRACE_E( "findFunctionLikeRld" );

      return ret;
    }

  // pass #3: trailing underscore (Fortran)
  name = fn_name + "_";
  ret = p->findOneFunctionFromAll(name);
  if (ret)
    {
      TRACE_E( "findFunctionLikeRld" );

      return ret;
    }

  // pass #4: two leading underscores (libm)
  name = "__" + fn_name;
  ret = p->findOneFunctionFromAll(name);
  if (ret)
    {
      TRACE_E( "findFunctionLikeRld" );

      return ret;
    }

  TRACE_E( "findFunctionLikeRld" );

  return NULL;
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/

bool process::findCallee(instPoint &ip, function_base *&target)
{
  TRACE_B( "process::findCallee" );

  /*
  fprintf(stderr, ">>> <0x%016lx:\"%s\"> => ", 
	  ip.iPgetOwner()->getObject().get_base_addr() + ip.address(),
	  ip.iPgetFunction()->prettyName().string_of());
  */

  // sanity check
  assert(ip.type() == IPT_CALL);

  // check if callee was already resolved
  if (ip.callee_) {
    //fprintf(stderr, "\"%s\" (static)\n", callee->prettyName().string_of());
    target = ip.callee_;

    TRACE_E( "process::findCallee" );

    return true;
  }
  
  /* GOT-based calls are partially resolved by checkCallPoints().  Now
     we find the runtime address of the GOT entry, read the entry
     value, and figure out which function it corresponds to . */
  if (ip.hint_got_) {
    const image *owner = ip.iPgetOwner();
    const Object &elf = owner->getObject();

    // runtime address of GOT entry
    Address got_entry_base = 0;
    getBaseAddress(owner, got_entry_base);
    Address got_entry_off = ip.hint_got_;
    Address got_entry_addr = got_entry_base + got_entry_off;

    // read GOT entry from process address space
    void *tgt_ptr;
    // address-in-memory
    if (sizeof(void *) == sizeof(uint64_t) && !elf.is_elf64()) 
    {
      // 32-bit application, 64-bit paradynd
      tgt_ptr = NULL;
      char *tgt_ptr_adj = ((char *)&tgt_ptr) + sizeof(uint32_t);
      readDataSpace((void *)got_entry_addr, sizeof(uint32_t), tgt_ptr_adj, true);
    } else {
      readDataSpace((void *)got_entry_addr, sizeof(void *), &tgt_ptr, true);
    }

    // lookup function by runtime address
    Address tgt_addr = (Address)tgt_ptr;
    pd_Function *pdf = (pd_Function *)findFunctionIn(tgt_addr);
    if (pdf) {
      Address fn_base;
      getBaseAddress(pdf->file()->exec(), fn_base);
      assert(fn_base + pdf->getAddress(0) == tgt_addr);

      //fprintf(stderr, "\"%s\" (GOT)\n", pdf->prettyName().string_of());
      target = pdf;

      TRACE_E( "process::findCallee" );

      return true;
    } else {
      // check if GOT entry points to .MIPS.stubs
      Address tgt_vaddr = tgt_addr - got_entry_base + elf.get_base_addr();
      if (tgt_vaddr >= elf.MIPS_stubs_addr_ &&
	  tgt_vaddr < elf.MIPS_stubs_addr_ + elf.MIPS_stubs_size_)
      {	
	const char *fn_name_ = elf.got_entry_name(got_entry_off);
	string fn_name = fn_name_;
	// TODO: rld might resolve to a different fn
	pdf = (pd_Function *)findFunctionLikeRld(this, fn_name);
	if (pdf) {
	  //fprintf(stderr, "\"%s\" (stub)\n", pdf->prettyName().string_of());
	  target = pdf;

// 	  TRACE_E( "process::findCallee" );

	  return true;
	}
      }
    }
  }

  /* TODO: We're hosed if we get to this point.  Likely reasons for
     this happening are: (A) a call was made through a function
     pointer, (B) "findIndirectJumpTarget" got confused due to a lack
     of dataflow analysis, or (C) a new call sequence was
     encountered and not parsed correctly. */

  //fprintf(stderr, "unknown\n");
  target = NULL;

  TRACE_E( "process::findCallee" );

  return false;
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/

void emitLoadPreviousStackFrameRegister(Address register_num,
					 Register dest,
					 char *insn,
					 Address &base,
					 int size,
					 bool noCost){
  TRACE_B( "emitLoadPreviousStackFrameRegister" );

  int offset = ((31 - register_num) * 8)+4;
  emitImm(plusOp ,(Register) REG_SP, (RegValue) offset, dest, insn, 
	  base, noCost);
  //Load the value stored on the stack at address dest into register dest
  emitV(loadIndirOp, dest, 0, dest, insn, base, noCost, size);

  TRACE_E( "emitLoadPreviousStackFrameRegister" );
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/

#ifndef BPATCH_LIBRARY

bool process::isDynamicCallSite(instPoint *callSite){
  TRACE_B( "process::isDynamicCallSite" );

  function_base *temp;
  if(!findCallee(*(callSite),temp)){
    TRACE_E( "process::isDynamicCallSite" );

    return true;
  }

  TRACE_E( "process::isDynamicCallSite" );

  return false;
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/

bool process::MonitorCallSite(instPoint *callSite){
  TRACE_B( "process::MonitorCallSite" );

  instruction i = callSite->origInsn_;
  vector<AstNode *> the_args(2);
  //IS the instruction of type "jalr ra,RR"?
  if(isCall1(i)){
    the_args[0] = 
      new AstNode(AstNode::PreviousStackFrameDataReg,
		  (void *) i.rtype.rs);
    the_args[1] = new AstNode(AstNode::Constant,
			      (void *) callSite->iPgetAddress());
    AstNode *func = new AstNode("DYNINSTRegisterCallee", 
				the_args);
    addInstFunc(this, callSite, func, callPreInsn,
		orderFirstAtPoint,
		true,                              /* noCost flag                */
		false);                            /* trampRecursiveDesired flag */
  }
  else
    {
      TRACE_E( "process::MonitorCallSite" );

      return false;
    }
 
  TRACE_E( "process::MonitorCallSite" );

  return true;
}
#endif

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/

/* initDefaultPointFrequencyTable() - define the expected call
   frequency of procedures.  Currently we just define several one
   shots with a frequency of one, and provide a hook to read a file
   with more accurate information. */
void initDefaultPointFrequencyTable()
{
  TRACE_B( "initDefaultPointFrequencyTable" );

    funcFrequencyTable["main"] = 1;
    funcFrequencyTable["DYNINSTsampleValues"] = 1;
    funcFrequencyTable[EXIT_NAME] = 1;

    // try to read file.
    FILE *fp = fopen("freq.input", "r");
    if (!fp)
      {
	TRACE_E( "initDefaultPointFrequencyTable" );

	return;
      }
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

    TRACE_E( "initDefaultPointFrequencyTable" );
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/

// measurements taken on 180MHz MIPS R10000
// costs are in units of cycles
void initPrimitiveCost()
{
  TRACE_B( "initPrimitiveCost" );

  //fprintf(stderr, ">>> initPrimitiveCost()\n");

    // Values (in cycles) benchmarked on an R10000 180MHz
    // Level 1 - Hardware Level
    primitiveCosts["DYNINSTstartWallTimer"] = 156;
    primitiveCosts["DYNINSTstopWallTimer"] = 155;
    primitiveCosts["DYNINSTstartProcessTimer"] = 4276;
    primitiveCosts["DYNINSTstopProcessTimer"] = 4210;

    /* Level 2 - Software Level
    // Implementation still needs to be added to handle start/stop
    // timer costs for multiple levels
    primitiveCosts["DYNINSTstartWallTimer"] = 1423;
    primitiveCosts["DYNINSTstopWallTimer"] = 1440;
    primitiveCosts["DYNINSTstartProcessTimer"] = 5590;
    primitiveCosts["DYNINSTstopProcessTimer"] = 5737;
    */

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

  TRACE_E( "initPrimitiveCost" );
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/

//
// paradynd-only methods
//

string process::getProcessStatus() const 
{
  TRACE_B( "process::getProcessStatus" );

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

  TRACE_E( "process::getProcessStatus" );

  return(ret);
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/

bool returnInstance::checkReturnInstance(const vector<Address> &stack, u_int &index) 
{
  TRACE_B( "returnInstance::checkReturnInstance" );

  // if unsafe (ret=false), set "index" to first unsafe call stack index
  for (u_int i=0; i < stack.size(); i++) {
    index = i;
    if (stack[i] >= addr_ && stack[i] < addr_+size_) 
      {
	TRACE_E( "returnInstance::checkReturnInstance" );

	return false;
      }
  }  

  TRACE_E( "returnInstance::checkReturnInstance" );

  return true;
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/

void returnInstance::addToReturnWaitingList(Address pc, process *proc) 
{
  TRACE_B( "returnInstance::addToReturnWaitingList" );

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

  TRACE_E( "returnInstance::addToReturnWaitingList" );
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/

//
// return cost in cycles of executing at this point.  This is the cost
//   of the base tramp if it is the first at this point or 0 otherwise.
//
int getPointCost(process *proc, const instPoint *point)
{
  TRACE_B( "getPointCost" );

  if (proc->baseMap.defines(point)) {
    TRACE_E( "getPointCost" );

    return 0;
  } else {
    // worst case for base tramp is 299 cycles

    TRACE_E( "getPointCost" );

    return(299);
  }
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/

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
  TRACE_B( "getPointFrequency" );

  pd_Function *func;
  
  if (point->callee_) {
    func = (pd_Function *)point->callee_;
  } else {
    func = (pd_Function *)point->func_;
  }

  if (!funcFrequencyTable.defines(func->prettyName())) {
    // TODO: this value needs to be tuned

    TRACE_E( "getPointFrequency" );

    return(50);       
  } else {
    TRACE_E( "getPointFrequency" );

    return (funcFrequencyTable[func->prettyName()]);
  }
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/

/*
 * Define the various classes of library functions to inst. 
 *
 */
void initLibraryFunctions()
{
  TRACE_B( "initLibraryFunctions" );

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

  TRACE_E( "initLibraryFunctions" );
}


#ifdef BPATCH_LIBRARY
/*
 * createInstructionInstPoint
 *
 * Create a BPatch_point instrumentation point at the given address, which
 * is guaranteed not be one of the "standard" inst points.
 *
 * proc		The process in which to create the inst point.
 * address	The address for which to create the point.
 */
BPatch_point *createInstructionInstPoint(process *proc, void *address)
{
    int i;

    function_base *func = proc->findFunctionIn((Address)address);

    if (!isAligned((Address)address))
	return NULL;

    if (func != NULL) {
	instPoint *entry = const_cast<instPoint *>(func->funcEntry(proc));
	assert(entry);
	if ((entry->iPgetAddress() == (Address)address-INSN_SIZE) ||
		 (entry->iPgetAddress() == (Address)address+INSN_SIZE)) {
	    BPatch_reportError(BPatchSerious, 117,
			       "instrumentation point conflict");
	    return NULL;
	}

	const vector<instPoint*> &exits = func->funcExits(proc);
	for (i = 0; i < exits.size(); i++) {
	    assert(exits[i]);
	    if ((exits[i]->iPgetAddress() == (Address)address-INSN_SIZE) ||
		(exits[i]->iPgetAddress() == (Address)address+INSN_SIZE)) {
		BPatch_reportError(BPatchSerious, 117,
				   "instrumentation point conflict");
		return NULL;
	    }
	}

	const vector<instPoint*> &calls = func->funcCalls(proc);
	for (i = 0; i < calls.size(); i++) {
	    assert(calls[i]);
    	    if ((calls[i]->iPgetAddress() == (Address) - INSN_SIZE) ||
		(calls[i]->iPgetAddress() == (Address) + INSN_SIZE)) {
		BPatch_reportError(BPatchSerious, 117,
				   "instrumentation point conflict");
		return NULL;
	    }
	}
    }

    /* Check for conflict with a previously created inst point. */
    if (proc->instPointMap.defines((Address)address - INSN_SIZE)) {
	BPatch_reportError(BPatchSerious,117,"instrumentation point conflict");
	return NULL;
    } else if (proc->instPointMap.defines((Address)address + INSN_SIZE)) {
	BPatch_reportError(BPatchSerious,117,"instrumentation point conflict");
	return NULL;
    }

    /* Check for instrumentation where the delay slot of the jump to the
       base tramp would be a branch target from elsewhere in the function. */
    BPatch_function *bpfunc = proc->PDFuncToBPFuncMap[func];
    /* XXX Should create here with correct module, but we don't know it. */
    if (bpfunc == NULL) bpfunc = new BPatch_function(proc, func, NULL);

    BPatch_flowGraph *cfg = bpfunc->getCFG();
    BPatch_Set<BPatch_basicBlock*>* allBlocks = cfg->getAllBasicBlocks();
    BPatch_basicBlock** belements = new BPatch_basicBlock*[allBlocks->size()];
    allBlocks->elements(belements);
    for(i=0; i< allBlocks->size(); i++) {
	void *startAddress, *endAddress;
	if (belements[i]->getAddressRange(startAddress, endAddress)) {
	    if ((Address)address + INSN_SIZE == (Address)startAddress) {
		delete[] belements;
		BPatch_reportError(BPatchSerious, 118,
				   "point uninstrumentable");
		return NULL;
	    }
	}
    }
    delete[] belements;

    /* Check for instrumenting just before or after a branch. */

    if ((Address)address > func->getEffectiveAddress(proc)) {
	instruction prevInstr;
	proc->readTextSpace((char *)address - INSN_SIZE,
			    sizeof(instruction),
			    &prevInstr.raw);
	if (isBranchInsn(prevInstr) || isJumpInsn(prevInstr)) {
	    BPatch_reportError(BPatchSerious, 118, "point uninstrumentable");
	    return NULL;
	}
    }

    if ((Address)address + INSN_SIZE <
	func->getEffectiveAddress(proc) + func->size()) {
	instruction nextInstr;
	proc->readTextSpace((char *)address + INSN_SIZE,
			    sizeof(instruction),
			    &nextInstr.raw);
	if (isBranchInsn(nextInstr) || isJumpInsn(nextInstr)) {
	    BPatch_reportError(BPatchSerious, 118, "point uninstrumentable");
	    return NULL;
	}
    }

    instPoint *newpt = new instPoint((pd_Function *)func,
				     (Address)address - func->getAddress(proc),
				     IPT_OTHER,
				     0);

    return proc->findOrCreateBPPoint(bpfunc, newpt, BPatch_instruction);
}

/*
 * BPatch_point::getDisplacedInstructions
 *
 * Returns the instructions to be relocated when instrumentation is inserted
 * at this point.  Returns the number of bytes taken up by these instructions.
 *
 * maxSize      The maximum number of bytes of instructions to return.
 * insns        A pointer to a buffer in which to return the instructions.
 */
int BPatch_point::getDisplacedInstructions(int maxSize, void *insns)
{
    if (maxSize >= sizeof(instruction))
        memcpy(insns, &point->origInsn_.raw, sizeof(instruction));

    return sizeof(instruction);
}

#endif
