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

#ifndef _BPatch_snippet_h_
#define _BPatch_snippet_h_

#include "BPatch_dll.h"
#include "BPatch_Vector.h"
#include "BPatch_sourceObj.h"
#include "BPatch_point.h"
#include "BPatch_type.h"
#include "BPatch_module.h"
#include "BPatch_function.h"

class AstNode;
class process;

typedef enum {
    BPatch_lt,
    BPatch_eq,
    BPatch_gt,
    BPatch_le,
    BPatch_ne,
    BPatch_ge,
    BPatch_and,
    BPatch_or
} BPatch_relOp;

typedef enum {
    BPatch_assign,
    BPatch_plus,
    BPatch_minus,
    BPatch_divide,
    BPatch_times,
    BPatch_mod,
    BPatch_ref,
    BPatch_seq,
    BPatch_bit_and,		// not supported yet
    BPatch_bit_or,		// not supported yet
    BPatch_bit_xor,		// not supported yet
    BPatch_left_shift,		// not supported yet
    BPatch_right_shift		// not supported yet
} BPatch_binOp;

// for backwards compatability
#define BPatch_addr BPatch_address

typedef enum {
    BPatch_negate,
    BPatch_address,
#ifdef IBM_BPATCH_COMPAT
    BPatch_deref,
    BPatch_bit_compl		// not supported yet
#else
    BPatch_deref
#endif
} BPatch_unOp;

class BPATCH_DLL_EXPORT BPatch_snippet {
public:
// The following members are for  internal use by the library only:
    AstNode	*ast; /* XXX It would be better if this was protected */
// End members for internal use only.

    BPatch_snippet() : ast(NULL) {};
    BPatch_snippet(const BPatch_snippet &);
    BPatch_snippet &operator=(const BPatch_snippet &);

    ~BPatch_snippet();

    float	getCost();
};

class BPATCH_DLL_EXPORT BPatch_arithExpr: public BPatch_snippet {
public:
    BPatch_arithExpr(BPatch_binOp op,
		     const BPatch_snippet &lOperand,
		     const BPatch_snippet &rOperand);
    BPatch_arithExpr(BPatch_unOp op, const BPatch_snippet &lOperand);
};

class BPATCH_DLL_EXPORT BPatch_boolExpr : public BPatch_snippet {
public:
    BPatch_boolExpr(BPatch_relOp op, const BPatch_snippet &lOperand,
		    const BPatch_snippet &rOperand);
};

class BPATCH_DLL_EXPORT BPatch_constExpr : public BPatch_snippet {
public:
#ifdef IBM_BPATCH_COMPAT
    BPatch_constExpr(long long value);
    BPatch_constExpr(float value);
#endif
    BPatch_constExpr(int value);
#ifdef BPATCH_NOT_YET
    BPatch_constExpr(float value);
#endif
    BPatch_constExpr(const char *value);
    BPatch_constExpr(const void *value);
};

class BPATCH_DLL_EXPORT BPatch_funcCallExpr : public BPatch_snippet {
public:
    BPatch_funcCallExpr(const BPatch_function& func,
			const BPatch_Vector<BPatch_snippet *> &args);
};

class BPATCH_DLL_EXPORT BPatch_funcJumpExpr : public BPatch_snippet {
public:
     BPatch_funcJumpExpr(const BPatch_function& func);
};

class BPATCH_DLL_EXPORT BPatch_ifExpr : public BPatch_snippet {
public:
    BPatch_ifExpr(const BPatch_boolExpr &conditional,
		  const BPatch_snippet &tClase);
    BPatch_ifExpr(const BPatch_boolExpr &conditional,
		  const BPatch_snippet &tClase,
		  const BPatch_snippet &fClause);
};

class BPATCH_DLL_EXPORT BPatch_nullExpr : public BPatch_snippet {
public:
    BPatch_nullExpr();
};

class BPATCH_DLL_EXPORT BPatch_paramExpr : public BPatch_snippet {
public:
    BPatch_paramExpr(int n);
};

class BPATCH_DLL_EXPORT BPatch_retExpr : public BPatch_snippet {
public:
    BPatch_retExpr();
};

class BPATCH_DLL_EXPORT BPatch_sequence : public BPatch_snippet {
public:
    BPatch_sequence(const BPatch_Vector<BPatch_snippet *> &items);
};

class BPATCH_DLL_EXPORT BPatch_variableExpr : public BPatch_snippet {
    char		*name;
    process		*proc;
    void		*address;
    int			size;
    BPatch_point	*scope;
public:
// The following functions are for internal use by the library only:
    BPatch_variableExpr(char *name, process *in_process, void *in_address,
			const BPatch_type *type);
    BPatch_variableExpr(process *in_process, void *in_address,
			const BPatch_type *type, bool frameRelative = false,
			BPatch_point *sc = NULL);
    BPatch_variableExpr(process *in_process, void *in_address,
			int in_size);
    BPatch_variableExpr(char *in_name, process *in_process, AstNode *_ast,
			const BPatch_type *type);
    BPatch_variableExpr(char *in_name,
                        process *in_process,
                        AstNode *_ast,
                        const BPatch_type *type,
                        void* in_address);

// Public functions for use by users of the library:
    bool readValue(void *dst);
    void readValue(void *dst, int len);
    bool writeValue(const void *src, bool saveWorld=false); //ccw 26 nov 2001
    void writeValue(const void *src, int len,bool saveWorld=false);

    char *getName() { return name; }
    void *getBaseAddr() const { return address; }

#ifdef IBM_BPATCH_COMPAT
    const char *getName(char *buffer, int max) { return strncpy(buffer, name, max); }
    long long int getAddress() const { return (long long int) address; }
#endif

    unsigned int getSize() const { return size; }
    BPatch_type *getType();
    const BPatch_type *getType() const;
    void setType(BPatch_type *);
    void setSize(int sz) {  size = sz; }
    BPatch_Vector<BPatch_variableExpr *> *getComponents();
};

class BPATCH_DLL_EXPORT BPatch_breakPointExpr : public BPatch_snippet {
public:
    BPatch_breakPointExpr();
};

// VG(11/05/01): This nullary snippet will return the effective
// address of a memory access when inserted at an instrumentation
// point that is a memory access.  In other words, the instruction at
// the point where it is inserted is the one to get effective address
// for.  Furthermore, there must be memory access information about
// the inst. point; this basically means that the point must have been
// created using a method that attaches that info to the point -
// e.g. using findPoint(const BPatch_Set<BPatch_opCode>& ops) from
// BPatch_function.

// VG(7/31/02): Since x86 can have 2 addresses per instruction, there is
// now parameter for the constructor indicating which of these you want.
// It defaults to the 1st access (#0).

// VG(8/14/02): added conditional parameter
class BPATCH_DLL_EXPORT BPatch_effectiveAddressExpr : public BPatch_snippet
{
public:
  BPatch_effectiveAddressExpr(int _which = 0);
};

// Number of bytes moved
class BPATCH_DLL_EXPORT BPatch_bytesAccessedExpr : public BPatch_snippet
{
public:
  BPatch_bytesAccessedExpr(int _which = 0);
};

// VG(8/11/2): It is possible to have a more general expression, say 
// machineConditionExpr, then have this reimplemented as ifExpr(machineConditionExpr, ...),
// and have an optimization (fast path) for that case using the specialized
// AST that supports this class. Memory instrumentation has no need for a standalone
// machineConditionExpr, so that remains TBD...
class BPATCH_DLL_EXPORT BPatch_ifMachineConditionExpr : public BPatch_snippet {
public:
  BPatch_ifMachineConditionExpr(const BPatch_snippet &tClase);
};

#endif /* _BPatch_snippet_h_ */
