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

#ifndef _BPatch_snippet_h_
#define _BPatch_snippet_h_

#include "BPatch_dll.h"
#include "BPatch_Vector.h"
#include "BPatch_sourceObj.h"
#include "BPatch_point.h"
#include "BPatch_type.h"
#include "BPatch_module.h"
#include "BPatch_function.h"
#include "BPatch_eventLock.h"

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
    BPatch_fieldref,
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

#ifdef DYNINST_CLASS_NAME
#undef DYNINST_CLASS_NAME
#endif
#define DYNINST_CLASS_NAME BPatch_snippet

class BPATCH_DLL_EXPORT BPatch_snippet : public BPatch_eventLock {
    friend class BPatch_thread;
    friend class BPatch_arithExpr;
    friend class BPatch_boolExpr;
    friend class BPatch_funcCallExpr;
    friend class BPatch_variableExpr;
    friend class BPatch_ifExpr;
    friend class BPatch_ifMachineConditionExpr;
    friend class BPatch_sequence;
    friend AstNode *generateArrayRef(const BPatch_snippet &lOperand, 
                                     const BPatch_snippet &rOperand);
    friend AstNode *generateFieldRef(const BPatch_snippet &lOperand, 
                                     const BPatch_snippet &rOperand);

    public:

    AstNode *PDSEP_ast() {return ast;} // This will go away
    int PDSEP_astMinCost(); // This will go away too

    //  BPatch_snippet::BPatch_snippet
    //  Default constructor

    BPatch_snippet() : ast(NULL) {};

    //  BPatch_snippet::BPatch_snippet
    //  Copy constructor

    public:  BPatch_snippet(const BPatch_snippet &src) : BPatch_eventLock(src)
             { LOCK_FUNCTION_V(BPatch_snippetInt,(src)); }
    private: void BPatch_snippetInt(const BPatch_snippet &src);

    //  BPatch_snippet:operator=
    //  Assign one BPatch_snippet to another
    API_EXPORT_OPER(_equals, (src),

    BPatch_snippet &,operator=,(const BPatch_snippet &src));

    //  BPatch_snippet::~BPatch_snippet
    //  Destructor, decrements reference count to snippet, deleting when none are left
    API_EXPORT_DTOR(_dtor, (),
    
    ~,BPatch_snippet,());

    //  BPatch_snippet::getCost
    //  Returns an estimated cost of executing the snippet, in seconds.
    API_EXPORT(Int, (),

    float,getCost,());

    //  BPatch_snippet::is_trivial
    //  allows users to check to see if
    //  a snippet operation failed (leaving ast NULL)
    API_EXPORT(Int, (),

    bool,is_trivial,());

    protected:
    AstNode	*ast; 

};

#ifdef DYNINST_CLASS_NAME
#undef DYNINST_CLASS_NAME
#endif
#define DYNINST_CLASS_NAME BPatch_arithExpr

class BPATCH_DLL_EXPORT BPatch_arithExpr: public BPatch_snippet {

    //  BPatch_arithExpr::BPatch_arithExpr (Binary Arithmetic Operation)
    //  
    API_EXPORT_CTOR(Bin, (op, lOperand, rOperand),
    BPatch_arithExpr,(BPatch_binOp op,
                      const BPatch_snippet &lOperand,
                      const BPatch_snippet &rOperand));

    //  BPatch_arithExpr::BPatch_arithExpr (Unary Arithmetic Operation)
    //  
    API_EXPORT_CTOR(Un, (op, lOperand),
    BPatch_arithExpr,(BPatch_unOp op, const BPatch_snippet &lOperand));
};

#ifdef DYNINST_CLASS_NAME
#undef DYNINST_CLASS_NAME
#endif
#define DYNINST_CLASS_NAME BPatch_boolExpr

class BPATCH_DLL_EXPORT BPatch_boolExpr : public BPatch_snippet {

    //  BPatch_boolExpr::BPatch_boolExpr
    //  Creates a representation of boolean operation
    API_EXPORT_CTOR(Int, (op, lOperand, rOperand),
    BPatch_boolExpr,(BPatch_relOp op, const BPatch_snippet &lOperand,
                     const BPatch_snippet &rOperand));
};

#ifdef DYNINST_CLASS_NAME
#undef DYNINST_CLASS_NAME
#endif
#define DYNINST_CLASS_NAME BPatch_constExpr

class BPATCH_DLL_EXPORT BPatch_constExpr : public BPatch_snippet {

    //  BPatch_constExpr::BPatch_constExpr
    //  Creates a representation of an (int) value
    API_EXPORT_CTOR(Int, (value),
    BPatch_constExpr,(int value));

    //  BPatch_constExpr::BPatch_constExpr
    //  Creates a representation of a (char *) value
    API_EXPORT_CTOR(CharStar, (value),
    BPatch_constExpr,(const char *value));

    //  BPatch_constExpr::BPatch_constExpr
    //  Creates a representation of a (void *) value
    API_EXPORT_CTOR(VoidStar, (value),
    BPatch_constExpr,(const void *value));

#ifdef IBM_BPATCH_COMPAT
#if defined(ia64_unknown_linux2_4)

    API_EXPORT_CTOR(Long, (value),
    BPatch_constExpr,(long value));

#else

    API_EXPORT_CTOR(LongLong, (value),
    BPatch_constExpr,(long long value));

#endif

    API_EXPORT_CTOR(Float, (value),
    BPatch_constExpr,(float value));

#endif

};

#ifdef DYNINST_CLASS_NAME
#undef DYNINST_CLASS_NAME
#endif
#define DYNINST_CLASS_NAME BPatch_regExpr

class BPATCH_DLL_EXPORT BPatch_regExpr : public BPatch_snippet {

    //  BPatch_regExpr::BPatch_regExpr
    //  Creates a representation of the contents of a particular register
    //  specified by <value>
    API_EXPORT_CTOR(Int, (value),
    BPatch_regExpr,(unsigned int value));
};

#ifdef DYNINST_CLASS_NAME
#undef DYNINST_CLASS_NAME
#endif
#define DYNINST_CLASS_NAME BPatch_funcCallExpr

class BPATCH_DLL_EXPORT BPatch_funcCallExpr : public BPatch_snippet {

    //  BPatch_funcCallExpr::BPatch_funcCallExpr
    //  Creates a representation of a function call
    API_EXPORT_CTOR(Int, (func, args),
    BPatch_funcCallExpr,(const BPatch_function& func,
                         const BPatch_Vector<BPatch_snippet *> &args));
};

#ifdef DYNINST_CLASS_NAME
#undef DYNINST_CLASS_NAME
#endif
#define DYNINST_CLASS_NAME BPatch_funcJumpExpr

class BPATCH_DLL_EXPORT BPatch_funcJumpExpr : public BPatch_snippet {

    //  BPatch_funcJumpExpr::BPatch_funcJumpExpr
    //  Creates a representation of a jump to a function without linkage
    API_EXPORT_CTOR(Int, (func),
    BPatch_funcJumpExpr,(const BPatch_function& func));
};

#ifdef DYNINST_CLASS_NAME
#undef DYNINST_CLASS_NAME
#endif
#define DYNINST_CLASS_NAME BPatch_ifExpr

class BPATCH_DLL_EXPORT BPatch_ifExpr : public BPatch_snippet {

    //  BPatch_ifExpr::BPatch_ifExpr
    //  Creates a conditional expression "if <conditional> tClause;"
    API_EXPORT_CTOR(Int, (conditional, tClause),
    BPatch_ifExpr,(const BPatch_boolExpr &conditional,
                   const BPatch_snippet &tClause));

    //  BPatch_ifExpr::BPatch_ifExpr
    //  Creates a conditional expression 
    //  "if <conditional> tClause; else fClause;"
    API_EXPORT_CTOR(WithElse, (conditional, tClause, fClause),
    BPatch_ifExpr,(const BPatch_boolExpr &conditional,
                   const BPatch_snippet &tClause,
                   const BPatch_snippet &fClause));
};

#ifdef DYNINST_CLASS_NAME
#undef DYNINST_CLASS_NAME
#endif
#define DYNINST_CLASS_NAME BPatch_nullExpr

class BPATCH_DLL_EXPORT BPatch_nullExpr : public BPatch_snippet {

    //  BPatch_nullExpr::BPatch_nullExpr
    //  Creates a null snippet that can be used as a placeholder.
    API_EXPORT_CTOR(Int, (),
    BPatch_nullExpr,());

};

#ifdef DYNINST_CLASS_NAME
#undef DYNINST_CLASS_NAME
#endif
#define DYNINST_CLASS_NAME BPatch_paramExpr

class BPATCH_DLL_EXPORT BPatch_paramExpr : public BPatch_snippet {

    //  BPatch_paramExpr::BPatch_paramExpr
    //  Represents a parameter of a function (used in creating funcCallExpr)
    API_EXPORT_CTOR(Int, (n),
    BPatch_paramExpr,(int n));
};

#ifdef DYNINST_CLASS_NAME
#undef DYNINST_CLASS_NAME
#endif
#define DYNINST_CLASS_NAME BPatch_retExpr

class BPATCH_DLL_EXPORT BPatch_retExpr : public BPatch_snippet {

    //  BPatch_retExpr::BPatch_retExpr
    //  Represents the return value from the function in which the 
    //  snippet is inserted
    API_EXPORT_CTOR(Int, (),
    BPatch_retExpr,());
};

#ifdef DYNINST_CLASS_NAME
#undef DYNINST_CLASS_NAME
#endif
#define DYNINST_CLASS_NAME BPatch_sequence

class BPATCH_DLL_EXPORT BPatch_sequence : public BPatch_snippet {

    //  BPatch_sequence::BPatch_sequence
    //  Represents a sequence of statements
    API_EXPORT_CTOR(Int, (items),
    BPatch_sequence,(const BPatch_Vector<BPatch_snippet *> &items));
};

#ifdef DYNINST_CLASS_NAME
#undef DYNINST_CLASS_NAME
#endif
#define DYNINST_CLASS_NAME BPatch_variableExpr

class BPATCH_DLL_EXPORT BPatch_variableExpr : public BPatch_snippet {
    friend class BPatch_thread;
    friend class BPatch_image;
    friend class BPatch_function;

    char		*name;
    BPatch_thread	*appThread;
    void		*address;
    int			size;
    BPatch_point	*scope;
    bool		isLocal;

    BPatch_variableExpr(BPatch_thread *in_process, void *in_address,
			int in_size);
    BPatch_variableExpr(char *in_name, BPatch_thread *in_process, AstNode *_ast,
			const BPatch_type *type);
    BPatch_variableExpr(char *in_name,
                        BPatch_thread *in_process,
                        AstNode *_ast,
                        const BPatch_type *type,
                        void* in_address);
    BPatch_variableExpr(BPatch_thread *in_process, void *in_address, int in_register,
			const BPatch_type *type, 
                        BPatch_storageClass storage = BPatch_storageAddr,
			BPatch_point *scp = NULL);

    //  BPatch_variableExpr::BPatch_variableExpr
    //  Represents a variable in the target application
    //
    //  The following constructor _should_ be private, but paradyn needs
    //  some way of declaring its own variables in its shared memory
    //  (counters, etc).  Until there is a way to do this in a better way,
    //  this needs to remain public (consider this a warning, API user,
    //  avoid using this constructor, it may not be here in the future).
    API_EXPORT_CTOR(Int, (name, in_process, in_address, type),
    BPatch_variableExpr,(char *name, BPatch_thread *in_process,
                         void *in_address, const BPatch_type *type));

    // Public functions for use by users of the library:

    //  BPatch_variableExpr::getSize
    //  Returns the size (in bytes) of this variable
    API_EXPORT(Int, (),
    unsigned int,getSize,() CONST_EXPORT);

    //  BPatch_variableExpr::getType
    //  Returns the type of this variable
    API_EXPORT(Int, (),
    const BPatch_type *,getType,());

    //  BPatch_variableExpr::setType
    //  Sets the type of this variable
    //  XXX -- should this really be public?
    API_EXPORT(Int, (t),
    bool,setType,(BPatch_type *t));

    //  BPatch_variableExpr::setSize
    //  Sets the size of this variable
    //  XXX -- should this really be public?
    API_EXPORT(Int, (sz),
    bool,setSize,(int sz));

    //  BPatch_variableExpr::readValue
    //  Read the value of a variable in a thread's address space.
    //  <dst> is assumed to be the same size as the variable.
    API_EXPORT(Int, (dst),
    bool,readValue,(void *dst));

    //  BPatch_variableExpr::readValue
    //  Read the value of a variable in a thread's address space.
    //  Will read <len> bytes into <dst>
    API_EXPORT(WithLength, (dst, len),
    bool,readValue,(void *dst, int len));

    //  BPatch_variableExpr::writeValue
    //  Write a value into a variable in a thread's address space.
    //  variable is assumed to be the same size as the <dst>.
    //  returns false if the type info isn't available (i.e. we don't know the size)
    API_EXPORT(Int, (src, saveWorld),
    bool,writeValue,(const void *src, bool saveWorld=false));

    //  BPatch_variableExpr::writeValue
    //  Write a value into a variable in a thread's address space.
    //  Will write <len> bytes from <src> into variable
    API_EXPORT(WithLength, (src, len, saveWorld),
    bool,writeValue,(const void *src, int len,bool saveWorld=false));

    //  BPatch_variableExpr::getName
    //  Returns the symbol table name for this variable
    API_EXPORT(Int, (),
    char *,getName,());

    //  BPatch_variableExpr::getBaseAddr
    //  Returns base address of this variable in the target's address space
    API_EXPORT(Int, (),
    void *,getBaseAddr,());

    //  BPatch_variableExpr::getComponents
    //  return variable expressions for all of the fields in a struct/union
    API_EXPORT(Int, (),
    BPatch_Vector<BPatch_variableExpr *> *,getComponents,());

#ifdef IBM_BPATCH_COMPAT
    API_EXPORT(WithLength, (buffer, max),
    char *,getName,(char *buffer, int max));

    API_EXPORT(Int, (),
    void *,getAddress,());

#endif

};

#ifdef DYNINST_CLASS_NAME
#undef DYNINST_CLASS_NAME
#endif
#define DYNINST_CLASS_NAME BPatch_breakPointExpr

class BPATCH_DLL_EXPORT BPatch_breakPointExpr : public BPatch_snippet {

    //  BPatch_breakPointExpr::BPatch_breakPointExpr
    //  Creates a representation of a break point in the target process

    API_EXPORT_CTOR(Int, (),
    BPatch_breakPointExpr,());
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

#ifdef DYNINST_CLASS_NAME
#undef DYNINST_CLASS_NAME
#endif
#define DYNINST_CLASS_NAME BPatch_effectiveAddressExpr

class BPATCH_DLL_EXPORT BPatch_effectiveAddressExpr : public BPatch_snippet
{

  //  BPatch_effectiveAddressExpr:: BPatch_effectiveAddressExpr
  //  Construct a snippet representing an effective address.

  API_EXPORT_CTOR(Int, (_which),
  BPatch_effectiveAddressExpr,(int _which = 0));
};

#ifdef DYNINST_CLASS_NAME
#undef DYNINST_CLASS_NAME
#endif
#define DYNINST_CLASS_NAME BPatch_bytesAccessedExpr

// Number of bytes moved
class BPATCH_DLL_EXPORT BPatch_bytesAccessedExpr : public BPatch_snippet
{

  //  BPatch_bytesAccessedExpr::BPatch_bytesAccessedExpr
  //  Construct a snippet representing the number of bytes accessed.

  API_EXPORT_CTOR(Int, (_which),
  BPatch_bytesAccessedExpr,(int _which = 0));
};

// VG(8/11/2): It is possible to have a more general expression, say 
// machineConditionExpr, then have this reimplemented as ifExpr(machineConditionExpr, ...),
// and have an optimization (fast path) for that case using the specialized
// AST that supports this class. Memory instrumentation has no need for a standalone
// machineConditionExpr, so that remains TBD...

#ifdef DYNINST_CLASS_NAME
#undef DYNINST_CLASS_NAME
#endif
#define DYNINST_CLASS_NAME BPatch_ifMachineConditionExpr

class BPATCH_DLL_EXPORT BPatch_ifMachineConditionExpr : public BPatch_snippet {

  //  BPatch_ifMachineConditionExpr::BPatch_ifMachineConditionExpr
  //  

  API_EXPORT_CTOR(Int, (tClause),
  BPatch_ifMachineConditionExpr,(const BPatch_snippet &tClause));
};

#endif /* _BPatch_snippet_h_ */
