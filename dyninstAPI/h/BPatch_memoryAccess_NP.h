/*
 * See the dyninst/COPYRIGHT file for copyright information.
 * 
 * We provide the Paradyn Tools (below described as "Paradyn")
 * on an AS IS basis, and do not warrant its validity or performance.
 * We reserve the right to update, modify, or discontinue this
 * software at any time.  We shall have no obligation to supply such
 * updates or modifications or any other form of support to you.
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

/* $Id: BPatch_memoryAccess_NP.h,v 1.23 2008/02/21 21:27:47 legendre Exp $ */

#ifndef _MemoryAccess_h_
#define _MemoryAccess_h_

#include "BPatch_Vector.h"
#include <stdlib.h>
#include "BPatch_instruction.h"
#include "dyntypes.h"

class BPatch_point;
class internal_instruction;

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

//extern void initOpCodeInfo();

class BPATCH_DLL_EXPORT BPatch_addrSpec_NP
{
  long imm;
  unsigned int scale;
  int regs[2];

public:
  BPatch_addrSpec_NP(long _imm, int _ra = -1, int _rb = -1, int _scale = 0);
  BPatch_addrSpec_NP();

  long getImm() const;
  int getScale() const;
  int getReg(unsigned i) const;

  bool equals(const BPatch_addrSpec_NP& ar) const;
};

typedef BPatch_addrSpec_NP BPatch_countSpec_NP;
class BPATCH_DLL_EXPORT BPatch_memoryAccess : public BPatch_instruction
{
  friend class BPatch_function;
  friend class AstMemoryNode;
  friend class BPatch_memoryAccessAdapter;
  
 public:

  static BPatch_Vector<BPatch_point*>* filterPoints(
                    const BPatch_Vector<BPatch_point*> &points,
                    unsigned int numMAs);
  
 private:
  BPatch_addrSpec_NP *start;
  BPatch_countSpec_NP *count;

 public:
  const BPatch_addrSpec_NP *getStartAddr(int which = 0) const;
  const BPatch_countSpec_NP *getByteCount(int which = 0) const;

 protected:

  void set1st(bool _isLoad, bool _isStore,
              long _imm_s, int _ra_s, int _rb_s, unsigned int _scale_s,
              long _imm_c, int _ra_c, int _rb_c, unsigned int _scale_c,
              int _preFcn, int _cond, bool _nt);

  void set1st(bool _isLoad, bool _isStore,
              long _imm_s, int _ra_s, int _rb_s,
              long _imm_c, int _ra_c = -1, int _rb_c = -1,
              unsigned int _scale_s = 0, int _preFcn = -1,
              int _cond = -1, bool _nt = false);

 public:
  static BPatch_memoryAccess* const none;
  static BPatch_memoryAccess* init_tables();

  BPatch_memoryAccess(internal_instruction *, Dyninst::Address _addr,
		      bool _isLoad, bool _isStore, unsigned int _bytes,
		      long _imm, int _ra, int _rb, unsigned int _scale = 0,
		      int _cond = -1, bool _nt = false);

  BPatch_memoryAccess(internal_instruction *insn, Dyninst::Address _addr,
                      bool _isinternal_Load, bool _isStore,
                      long _imm_s, int _ra_s, int _rb_s, unsigned int _scale_s,
                      long _imm_c, int _ra_c, int _rb_c, unsigned int _scale_c,
                      int _cond, bool _nt, int _preFcn = -1);

  BPatch_memoryAccess(internal_instruction *insn, Dyninst::Address _addr,
		      bool _isLoad, bool _isStore, bool _isPrefetch,
		      long _imm_s, int _ra_s, int _rb_s,
		      long _imm_c, int _ra_c, int _rb_c,
		      unsigned short _preFcn);

  BPatch_memoryAccess(internal_instruction *insn, Dyninst::Address _addr,
		      bool _isLoad, bool _isStore, long _imm_s, int _ra_s, int _rb_s,
		      long _imm_c, int _ra_c, int _rb_c);

  void set2nd(bool _isLoad, bool _isStore, unsigned int _bytes,
              long _imm, int _ra, int _rb, unsigned int _scale = 0);

  void set2nd(bool _isLoad, bool _isStore,
              long _imm_s, int _ra_s, int _rb_s, unsigned int _scale_s,
              long _imm_c, int _ra_c, int _rb_c, unsigned int _scale_c,
              int _cond, bool _nt);

  BPatch_memoryAccess(internal_instruction *insn, Dyninst::Address _addr,
                      bool _isLoad, bool _isStore, unsigned int _bytes,
                      long _imm, int _ra, int _rb, unsigned int _scale,
                      bool _isLoad2, bool _isStore2, unsigned int _bytes2,
                      long _imm2, int _ra2, int _rb2, unsigned int _scale2);

  BPatch_memoryAccess(internal_instruction *insn, Dyninst::Address _addr, bool _isLoad, bool _isStore,
                      long _imm_s, int _ra_s, int _rb_s, unsigned int _scale_s,
                      long _imm_c, int _ra_c, int _rb_c, unsigned int _scale_c,
                      bool _isLoad2, bool _isStore2, long _imm2_s, int _ra2_s, 
                      int _rb2_s, unsigned int _scale2_s, long _imm2_c, 
                      int _ra2_c, int _rb2_c, unsigned int _scale2_c);

  virtual ~BPatch_memoryAccess();

  bool equals(const BPatch_memoryAccess* mp) const;
  bool equals(const BPatch_memoryAccess& rp) const;

  BPatch_addrSpec_NP getStartAddr_NP(int which = 0) const;
  BPatch_countSpec_NP getByteCount_NP(int which = 0) const;
};

#endif
