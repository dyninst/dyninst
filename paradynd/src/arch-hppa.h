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

#if !defined(hppa1_1_hp_hpux)
#error "invalid architecture-os inclusion"
#endif


#if !defined(_arch_hppa_h_)
#define _arch_hppa_h_
#include "util/h/Types.h"

typedef enum { 
    noneType,
    functionEntry,
    functionExit,
    callSite
} instPointType;


//
// instruction types
//

struct load_store {
    unsigned op   : 6;
    unsigned b    : 5;
    unsigned tr   : 5;
    unsigned s    : 2;
    int      im14 : 14;
};

struct indexed_displacement_load_store {
    unsigned op      : 6;
    unsigned b       : 5;
    unsigned x_im5_r : 5;
    unsigned s       : 2;
    unsigned u_a     : 1;
    unsigned bit1    : 1;
    unsigned bit2    : 2;
    unsigned ext4    : 4;
    unsigned m       : 1;
    unsigned t_im5   : 5;
};

union memory_reference {
    load_store                      ls;
    indexed_displacement_load_store idls;
};

struct w_w1_w2_bits {
    unsigned ignore : 15;
    unsigned w      : 1;
    unsigned w1     : 5;
    unsigned w2     : 11;
};

union assemble_w_w1_w2 {
    int          raw;
    w_w1_w2_bits w_w1_w2;
};

struct w_w1_bits {
    unsigned ignore : 20;
    unsigned w      : 1;	
    unsigned w1     : 11;
};

union assemble_w_w1 {
    int       raw;   
    w_w1_bits w_w1; 
};

struct branch_instruction {
    unsigned op          : 6;
    unsigned r2_p_b_t    : 5;
    unsigned r1_im5_w1_x : 5;
    unsigned c_s_ext3    : 3;
    unsigned w1_w2       : 11;
    unsigned n           : 1;
    unsigned w           : 1;
};

struct long_immediate {
    unsigned op   : 6;
    unsigned tr   : 5;
    int      im21 : 21;
};

struct arithmetic_logical {
    unsigned op   : 6;
    unsigned r2   : 5;
    unsigned r1   : 5;
    unsigned c    : 3;
    unsigned f    : 1;
    unsigned ext7 : 7;
    unsigned t    : 5;
};

struct arithmetic_immediate {
    unsigned op   : 6;
    unsigned r    : 5;
    unsigned t    : 5;
    unsigned c    : 3;
    unsigned f    : 1;
    unsigned e    : 1;
    unsigned im11 : 11;
};

struct computation_generic {
    unsigned op   : 6;
    unsigned r2   : 5;
    unsigned r1   : 5;
    unsigned c    : 3;
    unsigned rest : 13;
};

union computation_instruction {
    arithmetic_logical   al;
    arithmetic_immediate ai;
    computation_generic  cg;
};

struct generic_instruction {
    unsigned op   : 6;
    unsigned rest : 26;
};

union instruction {
    unsigned                raw;
    memory_reference        mr;
    branch_instruction      bi;
    long_immediate          li;
    computation_instruction ci;
    generic_instruction     gi;
};


/*
 * Define the operation codes
 *
 */

#define LDILop          0x08
#define LDOop           0x0d
#define LDWop           0x12
#define STWop           0x1a

#define BLop            0x3a
#define BLext           0x0

// Added BE, LDIL, BLE
#define BEop            0x38  
#define LDILop          0x08
#define BLEop           0x39
#define BVop            0x3a

#define ADDop           0x02
#define ADDext7         0x30

#define SUBop           0x02
#define SUBext7         0x20

#define ORop            0x02
#define ORext7          0x12

#define ANDop           0x02
#define ANDext7         0x10

#define COMBTop         0x20
#define COMIBTop        0x21
#define COMBFop         0x22
#define COMIBFop        0x23
#define ADDBTop         0x28
#define ADDIBTop        0x29
#define ADDBFop         0x2a
#define ADDIBFop        0x2b
#define BVBop           0x30
#define BBop            0x31
#define MOVBop          0x32
#define MOVIBop         0x33

#define ADDIop          0x2d
#define SUBIop          0x25

#define SHDop           0x34

#define COMCLRop        0x02
#define COMCLRext7      0x44

#define COMCLR_EQ_C     1
#define COMCLR_EQ_F     0
#define COMCLR_NE_C     1
#define COMCLR_NE_F     1
#define COMCLR_LT_C     2
#define COMCLR_LT_F     0
#define COMCLR_LT_T     1
#define COMCLR_LE_C     3
#define COMCLR_LE_F     0
#define COMCLR_LE_T     1

/* mask bits for various parts of the instruction format */

#define GATEmask        0xfc00e000
#define GATEmatch       0xe8002000

#define BEmask          0xfc000000
#define BEmatch         0xe0000000

#define BLEmask         0xfc000000
#define BLEmatch        0xe4000000

#define CALLmask        0xfc00e000
#define CALLmatch       0xe8000000

#define CALLImask       0xfc00fffd
#define CALLImatch      0xe8004000

#define RETmask         0xfffffffd
#define RETmatch        0xe840c000 /* bv,n 0(%r2) */

#define BREAK_POINT_INSN 0x00010004  /* break */

//
// support functions
//
inline bool isLoadReturn(const instruction& insn) {
    return ((insn.mr.ls.tr == 2)&&(insn.mr.ls.b == 30)&&
            ((insn.mr.ls.op == 0x12)||(insn.mr.ls.op == 0x0D)));
}

static inline bool
isInsnType(const instruction& insn, unsigned mask, unsigned match) {
    return ((insn.raw & mask) == match);
}

static inline bool
isCallInsn(const instruction& insn) {
    return (isInsnType(insn, CALLmask, CALLmatch));
}


inline int 
w_w1_w2_to_offset(unsigned w, unsigned w1, unsigned w2) {
    assemble_w_w1_w2 x;
    x.raw = (w == 0) ? (0) : (~0); // sign extend the offset

    x.w_w1_w2.w = w;
    x.w_w1_w2.w1 = w1;
    x.w_w1_w2.w2 = ((w2&0x01)<<10) | ((w2&0x07fe)>>1); 
    // it means cat(w,w1,w2{10},w2{0..9}) 

    return ((x.raw << 2)+8);
}

inline int 
w_w1_to_offset(unsigned w, unsigned w1) {
    assemble_w_w1 x;
    x.raw = (w == 0) ? (0) : (~0);

    x.w_w1.w = w;
    x.w_w1.w1 = ((w1&0x01)<<10) | ((w1&0x07fe)>>1);
    return ((x.raw << 2)+8);
}


static inline bool
IS_CONDITIONAL_BRANCH(const instruction& insn) {
    switch (insn.gi.op) {
    case COMBTop:
    case COMIBTop:
    case COMBFop:
    case COMIBFop:
    case ADDBTop:
    case ADDIBTop:
    case ADDBFop:
    case ADDIBFop:
    case BVBop:
    case BBop:
    case MOVBop:
    case MOVIBop:
        return true;
    default:
        break;
    }
    return false;
}


// Test to see if this is a branch instruction which the target address
// is within the same function. 
static inline bool
isBranchInsnTest(const instruction& insn, Address adr, Address enter, Address ret) {
    unsigned target;
    if (isInsnType(insn, CALLmask, CALLmatch)) {
       target = adr + w_w1_w2_to_offset(insn.bi.w,
                        insn.bi.r1_im5_w1_x,
                        insn.bi.w1_w2);
       
       if ((((int)(target-enter)) * ((int)(target-ret))) < 0) { 
	   return true;
       } else 
	   return false;
    } else if (IS_CONDITIONAL_BRANCH(insn)) {
	target = adr + w_w1_to_offset(insn.bi.w, insn.bi.w1_w2);
	if ((((int)(target-enter)) * ((int)(target-ret))) < 0) { 
	    return true;
	} else
	    return false;
    }

    return false;
}

// Test to see if this is a branch instruction which jump outside the 
// the function.
static inline bool
isCallInsnTest(const instruction& insn, Address adr, Address enter, Address ret) {
    if (isInsnType(insn, CALLmask, CALLmatch)) {
       unsigned target;
       target = adr + w_w1_w2_to_offset(insn.bi.w,
                        insn.bi.r1_im5_w1_x,
                        insn.bi.w1_w2);

       if ((((int)(target-enter)) * ((int)(target-ret))) > 0) { 
          return true;
       } else 
          return false;
    }

    return false;
}

//TODO -- is this possible?           --------to do 
//isInsnType(insn, CALLImask, CALLImatch)) &&
//(insn.bi.r2_p_b_t != 0)) {
// target gr0 is a jump, and thus jump tables should
// be correctly handled

static inline bool
IS_BRANCH_INSN(const instruction& insn) {
  return ((isInsnType(insn, GATEmask, GATEmatch))
          || (isInsnType(insn, CALLmask, CALLmatch))
          || (isInsnType(insn, BLEmask, BLEmatch))
          || (isInsnType(insn, BEmask, BEmatch)));
} 	



static inline bool
IS_DELAYED_INST(const instruction& insn) {
    return (((insn.bi.op == 0x3a) /* all unconditional branches */
        || (isInsnType(insn, BEmask, BEmatch)) /* external */
        || (isInsnType(insn, BLEmask, BLEmatch)) /* external link */
        || (IS_CONDITIONAL_BRANCH(insn))) /* conditionals */
        && (!(insn.bi.n)));
}

static inline bool
IS_VALID_INSN(const instruction& insn) {
    // almost everything on PA-RISC is valid, after all
    // this is a "RISC" processor, haha
    switch (insn.gi.op) {
    case 0x07:
    case 0x14:
    case 0x15:
    case 0x16:
    case 0x17:
    case 0x1c:
    case 0x1d:
    case 0x1e:
    case 0x1f:
    case 0x27:
    case 0x2e:
    case 0x2f:
    case 0x36:
    case 0x37:
    case 0x3b:
    case 0x3c:
    case 0x3d:
    case 0x3e:
    case 0x3f:
        return false;
    default:
        break;
    }
    return true;
}


#endif /* !defined(_arch_hppa_h_) */








