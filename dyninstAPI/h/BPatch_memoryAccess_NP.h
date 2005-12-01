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

/* $Id: BPatch_memoryAccess_NP.h,v 1.17 2005/12/01 00:56:24 jaw Exp $ */

#ifndef _MemoryAccess_h_
#define _MemoryAccess_h_

#include <BPatch_Vector.h>
#include <stdlib.h>
//#include <BPatch_point.h>
#include <BPatch_instruction.h>

class BPatch_point;

/* Pseudoregisters definitions */
#define POWER_XER2531	9999

#define IA32_EMULATE	1000
#define IA32_ESCAS	1000
#define IA32_NESCAS	1001
#define IA32_ECMPS	1002
#define IA32_NECMPS	1003

#define IA32prefetchNTA  0
#define IA32prefetchT0   1
#define IA32prefetchT1   2
#define IA32prefetchT2   3
#define IA32AMDprefetch  100
#define IA32AMDprefetchw 101

extern void initOpCodeInfo();

/* This is believed to be machine independent, modulo register numbers of course */
class BPATCH_DLL_EXPORT BPatch_addrSpec_NP
{
  // the formula is regs[0] + 2 ^ scale * regs[1] + imm
  int imm;      // immediate
  unsigned int scale;
  int regs[2];  // registers: -1 means none, 0 is 1st, 1 is 2nd and so on
                // some pseudoregisters may be used, see the documentation

public:
  BPatch_addrSpec_NP(int _imm, int _ra = -1, int _rb = -1, int _scale = 0);
  BPatch_addrSpec_NP();

  int getImm() const;
  int getScale() const;
  int getReg(unsigned i) const;

  bool equals(const BPatch_addrSpec_NP& ar) const;
};

typedef BPatch_addrSpec_NP BPatch_countSpec_NP;
class BPATCH_DLL_EXPORT BPatch_memoryAccess : public BPatch_instruction
{
  friend class BPatch_function;
  friend class AstNode;
  friend class InstrucIter;

 public:

  // Utility function to filter out the points that don't have a 2nd memory 
  //  access on x86
  static BPatch_Vector<BPatch_point*>* filterPoints(
                    const BPatch_Vector<BPatch_point*> &points,
                    unsigned int numMAs);
  
 private:
  BPatch_addrSpec_NP *start;
  BPatch_countSpec_NP *count;

 protected:
  const BPatch_addrSpec_NP *getStartAddr(int which = 0) const;
  const BPatch_countSpec_NP *getByteCount(int which = 0) const;

  // initializes only the first access - general case
  void set1st(bool _isLoad, bool _isStore,
              int _imm_s, int _ra_s, int _rb_s, unsigned int _scale_s,
              int _imm_c, int _ra_c, int _rb_c, unsigned int _scale_c,
              int _preFcn, int _cond, bool _nt);

  // initializes only the first access - no scale for count
  void set1st(bool _isLoad, bool _isStore,
              int _imm_s, int _ra_s, int _rb_s,
              int _imm_c, int _ra_c = -1, int _rb_c = -1,
              unsigned int _scale_s = 0, int _preFcn = -1,
              int _cond = -1, bool _nt = false);

 public:
  static BPatch_memoryAccess* const none;
  static BPatch_memoryAccess* init_tables();

  // initializes only the first access; #bytes is a constant
  BPatch_memoryAccess(const void *buf, int _sz, Address _addr,
		      bool _isLoad, bool _isStore, unsigned int _bytes,
            int _imm, int _ra, int _rb, unsigned int _scale = 0,
            int _cond = -1, bool _nt = false);

  // initializes only the first access; #bytes is an expression w/scale
  BPatch_memoryAccess(const void *buf, int _sz, Address _addr,
                      bool _isLoad, bool _isStore,
                      int _imm_s, int _ra_s, int _rb_s, unsigned int _scale_s,
                      int _imm_c, int _ra_c, int _rb_c, unsigned int _scale_c,
                      int _cond, bool _nt, int _preFcn = -1);

  // initializes only the first access; #bytes is an expression
  BPatch_memoryAccess(const void *buf, int _sz, Address _addr,
		      bool _isLoad, bool _isStore, bool _isPrefetch,
            int _imm_s, int _ra_s, int _rb_s,
            int _imm_c, int _ra_c, int _rb_c,
            unsigned short _preFcn);

  // initializes only the first access; #bytes is an expression & not a prefetch
  BPatch_memoryAccess(const void *buf, int _sz, Address _addr,
	       bool _isLoad, bool _isStore, int _imm_s, int _ra_s, int _rb_s,
          int _imm_c, int _ra_c, int _rb_c);

  // sets 2nd access; #bytes is constant
  void set2nd(bool _isLoad, bool _isStore, unsigned int _bytes,
              int _imm, int _ra, int _rb, unsigned int _scale = 0);

  // sets 2nd access; #bytes is an expression w/scale
  void set2nd(bool _isLoad, bool _isStore,
              int _imm_s, int _ra_s, int _rb_s, unsigned int _scale_s,
              int _imm_c, int _ra_c, int _rb_c, unsigned int _scale_c,
              int _cond, bool _nt);

  // initializes both accesses; #bytes is a constant
  BPatch_memoryAccess(const void *buf, int _sz, Address _addr,
                      bool _isLoad, bool _isStore, unsigned int _bytes,
                      int _imm, int _ra, int _rb, unsigned int _scale,
                      bool _isLoad2, bool _isStore2, unsigned int _bytes2,
                      int _imm2, int _ra2, int _rb2, unsigned int _scale2);

  // initializes both accesses; #bytes is an expression & not a prefetch
  BPatch_memoryAccess(const void *buf, int _sz, Address _addr, bool _isLoad, bool _isStore,
                      int _imm_s, int _ra_s, int _rb_s, unsigned int _scale_s,
                      int _imm_c, int _ra_c, int _rb_c, unsigned int _scale_c,
                      bool _isLoad2, bool _isStore2, int _imm2_s, int _ra2_s, 
                      int _rb2_s, unsigned int _scale2_s, int _imm2_c, 
                      int _ra2_c, int _rb2_c, unsigned int _scale2_c);

  virtual ~BPatch_memoryAccess();

  bool equals(const BPatch_memoryAccess* mp) const;
  bool equals(const BPatch_memoryAccess& rp) const;

  BPatch_addrSpec_NP getStartAddr_NP(int which = 0) const;
  BPatch_countSpec_NP getByteCount_NP(int which = 0) const;
};

#endif
