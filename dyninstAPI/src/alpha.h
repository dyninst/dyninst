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

#if !defined(alpha_dec_osf4_0)
#error "invalid architecture-os inclusion"
#endif

#ifndef ALPHA_PD_HDR
#define ALPHA_PD_HDR

#include <sys/param.h>
#include <sys/procfs.h>
#define EXIT_NAME "exit"

#define BYTES_TO_SAVE 512 // should be a multiple of sizeof(instruction)

#define SIGNAL_HANDLER  0

/*extern unsigned AIX_TEXT_OFFSET_HACK;
extern unsigned AIX_DATA_OFFSET_HACK;*/
typedef int handleT; // defined for compatibility with other platforms

// Register usage conventions:
//   a0-a5 are saved in the base trampoline
//   t0-t10 are scratch and do not have to be saved as long as instrumentation
//      is restricted to function entry/exit, and call points
//      these will be allocated by paradynd
//
//   t0-t7 : allocated by paradynd
//   t8: stores compare results, scratch for paradynd
//   t9,t10 : used as scratch registers by paradynd

// Return value register
#define REG_V0           0
// scratch registers -- not saved across procedure calls
#define REG_T0           1
#define REG_T1           2
#define REG_T2           3
#define REG_T3           4
#define REG_T4           5
#define REG_T5           6
#define REG_T6           7
#define REG_T7           8

// caller save registers
#define REG_S0           9
#define REG_S1          10
#define REG_S2          11
#define REG_S3          12
#define REG_S4          13
#define REG_S5          14

// argument registers
#define REG_A0          16
#define REG_A1          17
#define REG_A2          18
#define REG_A3          19
#define REG_A4          20
#define REG_A5          21

// more scratch registers
#define REG_T8          22
#define REG_T9          23
#define REG_T10         24
#define REG_T11         25

// return address register
#define REG_RA          26

// undocumented pv/t12
#define REG_PV          27
#define REG_T12		27

#define REG_GP          29
#define REG_SP          30
#define REG_ZERO        31

const unsigned Num_Registers=32;


struct dyn_saved_regs
{
    gregset_t theIntRegs;
    fpregset_t theFpRegs;
};


#endif
