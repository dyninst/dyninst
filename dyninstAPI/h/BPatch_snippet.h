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

#ifndef _BPatch_snippet_h_
#define _BPatch_snippet_h_

#include <string>
#include "BPatch_dll.h"
#include "BPatch_Vector.h"
#include "BPatch_sourceObj.h"
#include "BPatch_type.h"
#include "BPatch_module.h"
//#include "BPatch_addressSpace.h"
//#include "BPatch_function.h"
#include "BPatch_callbacks.h"
#include "BPatch_instruction.h" // for register type
#include "BPatch_enums.h"
#include "boost/shared_ptr.hpp"

class AstNode;
// Don't include the boost shared_ptr library
class BPatch_snippet;

typedef boost::shared_ptr<AstNode> AstNodePtr;
namespace boost {
   template< typename T > class shared_ptr;
   template<> class shared_ptr<AstNode *>;
}

namespace Dyninst {
   namespace PatchAPI {
      class Snippet;
      typedef boost::shared_ptr<Snippet> SnippetPtr;
      BPATCH_DLL_EXPORT SnippetPtr convert(const BPatch_snippet *);
   }
}


class AstNode;
class BPatch_process;
class BPatch_function;
class BPatch_point;
class BPatch_addressSpace;
class int_variable;
class mapped_object;


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
    BPatch_xor,
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
    BPatch_deref
} BPatch_unOp;

class BPATCH_DLL_EXPORT BPatch_snippet {

    friend class BPatch_process;
    friend class BPatch_binaryEdit;
    friend class BPatch_addressSpace;
    friend class BPatch_thread;
    friend class BPatch_arithExpr;
    friend class BPatch_boolExpr;
    friend class BPatch_funcCallExpr;
    friend class BPatch_variableExpr;
    friend class BPatch_ifExpr;
    friend class BPatch_ifMachineConditionExpr;
    friend class BPatch_sequence;
    friend class BPatch_insnExpr;
    friend class BPatch_stopThreadExpr;
    friend class BPatch_shadowExpr;
    friend class BPatch_utilExpr;
    friend AstNodePtr generateArrayRef(const BPatch_snippet &lOperand, 
                                       const BPatch_snippet &rOperand);
    friend AstNodePtr generateFieldRef(const BPatch_snippet &lOperand, 
                                       const BPatch_snippet &rOperand);
    friend AstNodePtr generateVariableBase(const BPatch_snippet &lOperand);
    friend Dyninst::PatchAPI::SnippetPtr convert(const BPatch_snippet *snip);

    public:

    int PDSEP_astMinCost(); // This will go away too


    //  BPatch_snippet::BPatch_snippet
    //  Default constructor

    BPatch_snippet();
    BPatch_snippet(const AstNodePtr& ast);

    //  BPatch_snippet::BPatch_snippet
    //  Copy constructor

 public:  BPatch_snippet(const BPatch_snippet &src);


  public:
    //DynC internal use only
    //  BPatch_snippet::getType
    //  Returns the type of the underlying AST
    BPatch_type * getType();

  public: 

    //  BPatch_snippet:operator=
    //  Assign one BPatch_snippet to another

    BPatch_snippet & operator=(const BPatch_snippet &src);

    //  BPatch_snippet::~BPatch_snippet
    //  Destructor, decrements reference count to snippet, deleting when none are left
    
    virtual ~BPatch_snippet();

    //  BPatch_snippet::is_trivial
    //  allows users to check to see if
    //  a snippet operation failed (leaving ast NULL)

    bool is_trivial();

    // BPatch_snippet::checkTypesAtPoint
    // Type checking for inserting a particular
    // snippet at a particular point
    // 
    // Currently: check return exprs against the existence
    // of return values
    // Called at insertion time, but can be used to check in advance

    bool checkTypesAtPoint(BPatch_point* p) const;
    
    //    protected:
    //AstNodePtr *ast_wrapper; 

    AstNodePtr ast_wrapper;

};

class BPATCH_DLL_EXPORT BPatch_arithExpr: public BPatch_snippet {
 public:
    //  BPatch_arithExpr::BPatch_arithExpr (Binary Arithmetic Operation)
    //  
    BPatch_arithExpr(BPatch_binOp op,
                      const BPatch_snippet &lOperand,
                      const BPatch_snippet &rOperand);

    //  BPatch_arithExpr::BPatch_arithExpr (Unary Arithmetic Operation)
    //  
    BPatch_arithExpr(BPatch_unOp op, const BPatch_snippet &lOperand);
};

class BPATCH_DLL_EXPORT BPatch_boolExpr : public BPatch_snippet {
 public:
    //  BPatch_boolExpr::BPatch_boolExpr
    //  Creates a representation of boolean operation
    BPatch_boolExpr(BPatch_relOp op, const BPatch_snippet &lOperand,
                     const BPatch_snippet &rOperand);
};

class BPATCH_DLL_EXPORT BPatch_constExpr : public BPatch_snippet {
 public:
    //  BPatch_constExpr::BPatch_constExpr
    //  Creates a representation of a (signed int) value
    BPatch_constExpr(signed int value);

    //  Creates a representation of an (unsigned int) value
    BPatch_constExpr(unsigned int value);
    
    //  Creates a representation of a (signed long) value
    BPatch_constExpr(signed long value);
    
    //  Creates a representation of an (unsigned long) value
    BPatch_constExpr(unsigned long value);

	//  Creates a representation of an (unsigned long long) value
	BPatch_constExpr(unsigned long long value);

    //  BPatch_constExpr::BPatch_constExpr
    //  Creates a representation of a (char *) value
    BPatch_constExpr(const char *value);

    //  BPatch_constExpr::BPatch_constExpr
    //  Creates a representation of a (void *) value
    BPatch_constExpr(const void *value);

    //  BPatch_constExpr::BPatch_constExpr
    //  Creates a representation of a (long long) value
    BPatch_constExpr(long long value);

    // Should _always_ have a default constructor. This
    // one produces a 0
    BPatch_constExpr() : BPatch_snippet() {}

};

class BPATCH_DLL_EXPORT BPatch_whileExpr : public BPatch_snippet {
  public:
   // BPatch_whileExpr::BPatch_whileExpr (while loop)
   BPatch_whileExpr(const BPatch_snippet &condition,
                    const BPatch_snippet &body);
};

class BPATCH_DLL_EXPORT BPatch_funcCallExpr : public BPatch_snippet {
 public:
    //  BPatch_funcCallExpr::BPatch_funcCallExpr
    //  Creates a representation of a function call
    BPatch_funcCallExpr(const BPatch_function& func,
                         const BPatch_Vector<BPatch_snippet *> &args);
};

class BPATCH_DLL_EXPORT BPatch_ifExpr : public BPatch_snippet {
 public:
    //  BPatch_ifExpr::BPatch_ifExpr
    //  Creates a conditional expression "if <conditional> tClause;"
    BPatch_ifExpr(const BPatch_boolExpr &conditional,
                   const BPatch_snippet &tClause);

    //  BPatch_ifExpr::BPatch_ifExpr
    //  Creates a conditional expression 
    //  "if <conditional> tClause; else fClause;"
    BPatch_ifExpr(const BPatch_boolExpr &conditional,
                   const BPatch_snippet &tClause,
                   const BPatch_snippet &fClause);
};

class BPATCH_DLL_EXPORT BPatch_nullExpr : public BPatch_snippet {
 public:
    //  BPatch_nullExpr::BPatch_nullExpr
    //  Creates a null snippet that can be used as a placeholder.
    BPatch_nullExpr();

};

class BPATCH_DLL_EXPORT BPatch_paramExpr : public BPatch_snippet {
 public:
    //  BPatch_paramExpr::BPatch_paramExpr
    //  Represents a parameter of a function (used in creating funcCallExpr)
    //  n    is the index of the parameter that should be retrieved
    //  loc  indicates whether the parameter lookup will be added at the call,
    //       at the function's entry point, or whether Dyninst should guess
    //       based on the instPoint type, which is error-prone and deprecated
    BPatch_paramExpr(int n, BPatch_ploc loc=BPatch_ploc_guess);
};

class BPATCH_DLL_EXPORT BPatch_retExpr : public BPatch_snippet {
 public:
    //  BPatch_retExpr::BPatch_retExpr
    //  Represents the return value from the function in which the 
    //  snippet is inserted
    BPatch_retExpr();
};

class BPATCH_DLL_EXPORT BPatch_retAddrExpr : public BPatch_snippet {
 public:
    //  BPatch_retAddrExpr::BPatch_retAddrExpr
    //  Represents the return address from the function in which the
    //  snippet is inserted
    BPatch_retAddrExpr();
};

class BPATCH_DLL_EXPORT BPatch_registerExpr : public BPatch_snippet {
 public:
    friend class BPatch_addressSpace;
    //  BPatch_registerExpr::BPatch_registerExpr
    //  Represents the return value from the function in which the 
    //  snippet is inserted

                    BPatch_registerExpr(BPatch_register reg);
                    BPatch_registerExpr(Dyninst::MachRegister reg);
};

class BPATCH_DLL_EXPORT BPatch_sequence : public BPatch_snippet {
 public:
    //  BPatch_sequence::BPatch_sequence
    //  Represents a sequence of statements
    BPatch_sequence(const BPatch_Vector<BPatch_snippet *> &items);
};

class BPATCH_DLL_EXPORT BPatch_variableExpr : public BPatch_snippet 
{
    friend class BPatch_process;
    friend class BPatch_addressSpace;
    friend class BPatch_binaryEdit;
    friend class BPatch_image;
    friend class BPatch_function;

    std::string		name;
    BPatch_addressSpace     *appAddSpace;
    AddressSpace *lladdrSpace;
    void		*address;
    int			size;
    BPatch_point	*scope;
    bool		isLocal;
    BPatch_type *type;
    int_variable *intvar;
    

    AddressSpace *getAS();
    // Used to get expressions for the components of a structure
    // Used to get function pointers
    BPatch_variableExpr(const char *in_name, 
                        BPatch_addressSpace *in_addSpace,
                        AddressSpace *as,
                        AstNodePtr ast_wrapper_,
                        BPatch_type *type, void* in_address);
    // Used to get forked copies of variable expressions
    // Used by malloc & malloc_by_type
    BPatch_variableExpr(BPatch_addressSpace *in_addSpace, 
                        AddressSpace *as,
                        void *in_address, 
                        int in_register, BPatch_type *type, 
                        BPatch_storageClass storage = BPatch_storageAddr,
                        BPatch_point *scp = NULL);
    // Used for locals (naturally)
    BPatch_variableExpr(BPatch_addressSpace *in_addSpace,
                        AddressSpace *as,
                        BPatch_localVar *lv, BPatch_type *type,
                        BPatch_point *scp);
    
    //    BPatch_variableExpr(const char *name, BPatch_addressSpace *in_addSpace,
    //                    AddressSpace *ll_addSpace, void *in_address, 
    //                    BPatch_type *type);
    // Used by findOrCreateVariable
    BPatch_variableExpr(BPatch_addressSpace *in_addSpace,
			AddressSpace *ll_addSpace, int_variable *iv,
			BPatch_type *type);
 public:
    static BPatch_variableExpr* makeVariableExpr(BPatch_addressSpace* in_addSpace,
						 int_variable* v,
						 BPatch_type* type);
    static BPatch_variableExpr* makeVariableExpr(BPatch_addressSpace* in_addSpace,
						 AddressSpace* in_llAddSpace,
						 std::string name,
						 void* offset,
						 BPatch_type* type);
    
    

  public:

    // Public functions for use by users of the library:

    //  BPatch_variableExpr::getSize
    //  Returns the size (in bytes) of this variable
    unsigned int getSize() const;

    //  BPatch_variableExpr::getType
    //  Returns the type of this variable
    const BPatch_type * getType();

    //  BPatch_variableExpr::setType
    //  Sets the type of this variable
    //  XXX -- should this really be public?
    bool setType(BPatch_type *t);

    //  BPatch_variableExpr::setSize
    //  Sets the size of this variable
    //  XXX -- should this really be public?
    bool setSize(int sz);

    //  BPatch_variableExpr::readValue
    //  Read the value of a variable in a thread's address space.
    //  <dst> is assumed to be the same size as the variable.
    bool readValue(void *dst);

    //  BPatch_variableExpr::readValue
    //  Read the value of a variable in a thread's address space.
    //  Will read <len> bytes into <dst>
    bool readValue(void *dst, int len);

    //  BPatch_variableExpr::writeValue
    //  Write a value into a variable in a thread's address space.
    //  variable is assumed to be the same size as the <dst>.
    //  returns false if the type info isn't available (i.e. we don't know the size)
    bool writeValue(const void *src, bool saveWorld=false);

    //  BPatch_variableExpr::writeValue
    //  Write a value into a variable in a thread's address space.
    //  Will write <len> bytes from <src> into variable
    bool writeValue(const void *src, int len,bool saveWorld=false);

    //  BPatch_variableExpr::getName
    //  Returns the symbol table name for this variable
    const char * getName();

    //  BPatch_variableExpr::getBaseAddr
    //  Returns base address of this variable in the target's address space
    void * getBaseAddr();

    //  BPatch_variableExpr::getComponents
    //  return variable expressions for all of the fields in a struct/union
    BPatch_Vector<BPatch_variableExpr *> * getComponents();
};

class BPATCH_DLL_EXPORT BPatch_breakPointExpr : public BPatch_snippet {
 public:
    //  BPatch_breakPointExpr::BPatch_breakPointExpr
    //  Creates a representation of a break point in the target process

    BPatch_breakPointExpr();
};

// VG(11/05/01): This nullary snippet will return the effective
// address of a memory access when inserted at an instrumentation
// point that is a memory access.  In other words, the instruction at
// the point where it is inserted is the one to get effective address
// for.  Furthermore, there must be memory access information about
// the inst. point; this basically means that the point must have been
// created using a method that attaches that info to the point -
// e.g. using findPoint(const std::set<BPatch_opCode>& ops) from
// BPatch_function.

// VG(7/31/02): Since x86 can have 2 addresses per instruction, there is
// now parameter for the constructor indicating which of these you want.
// It defaults to the 1st access (#0).

// VG(8/14/02): added conditional parameter


class BPATCH_DLL_EXPORT BPatch_effectiveAddressExpr : public BPatch_snippet
{
 public:
  //  BPatch_effectiveAddressExpr:: BPatch_effectiveAddressExpr
  //  Construct a snippet representing an effective address.

  BPatch_effectiveAddressExpr(int _which = 0, int size = 8);
};


// Number of bytes moved
class BPATCH_DLL_EXPORT BPatch_bytesAccessedExpr : public BPatch_snippet
{
 public:
  //  BPatch_bytesAccessedExpr::BPatch_bytesAccessedExpr
  //  Construct a snippet representing the number of bytes accessed.

  BPatch_bytesAccessedExpr(int _which = 0);
};

// VG(8/11/2): It is possible to have a more general expression, say 
// machineConditionExpr, then have this reimplemented as ifExpr(machineConditionExpr, ...),
// and have an optimization (fast path) for that case using the specialized
// AST that supports this class. Memory instrumentation has no need for a standalone
// machineConditionExpr, so that remains TBD...


class BPATCH_DLL_EXPORT BPatch_ifMachineConditionExpr : public BPatch_snippet {
 public:
  //  BPatch_ifMachineConditionExpr::BPatch_ifMachineConditionExpr
  //  

  BPatch_ifMachineConditionExpr(const BPatch_snippet &tClause);
};


class BPATCH_DLL_EXPORT BPatch_threadIndexExpr : public BPatch_snippet {
 public:
  //
  // BPatch_threadIndexExpr::BPatch_threadIndexExpr
  BPatch_threadIndexExpr();
};

class BPATCH_DLL_EXPORT BPatch_tidExpr : public BPatch_snippet {
 public:
  //
  // BPatch_tidExpr::BPatch_tidExpr
  BPatch_tidExpr(BPatch_process *proc);
};

class BPatch_instruction;

typedef enum {
    BPatch_noInterp,
    BPatch_interpAsTarget,
    BPatch_interpAsReturnAddr,
} BPatch_stInterpret;

class BPATCH_DLL_EXPORT BPatch_shadowExpr : public BPatch_snippet {
 public:
  // BPatch_stopThreadExpr 
  //  This snippet type stops the thread that executes it.  It
  //  evaluates a calculation snippet and triggers a callback to the
  //  user program with the result of the calculation and a pointer to
  //  the BPatch_point at which the snippet was inserted
  BPatch_shadowExpr(bool entry, 
		    const BPatchStopThreadCallback &cb,
		    const BPatch_snippet &calculation,
		    bool useCache = false,
		    BPatch_stInterpret interp = BPatch_noInterp);
};

class BPATCH_DLL_EXPORT BPatch_stopThreadExpr : public BPatch_snippet {
 public:
  // BPatch_stopThreadExpr 
  //  This snippet type stops the thread that executes it.  It
  //  evaluates a calculation snippet and triggers a callback to the
  //  user program with the result of the calculation and a pointer to
  //  the BPatch_point at which the snippet was inserted
  BPatch_stopThreadExpr(const BPatchStopThreadCallback &cb,
			const BPatch_snippet &calculation,
			bool useCache = false,
			BPatch_stInterpret interp = BPatch_noInterp);

  // for internal use in conjunction with memory emulation and defensive 
  // mode analysis
  BPatch_stopThreadExpr(
   const BPatchStopThreadCallback &cb,
   const BPatch_snippet &calculation,
   const mapped_object &obj,
   bool useCache = false,
   BPatch_stInterpret interp = BPatch_noInterp);
};

class BPATCH_DLL_EXPORT BPatch_originalAddressExpr : public BPatch_snippet
{
 public:
  //  BPatch_originalAddressExpr
  //  Construct a snippet representing the original address of an 
  //  instruction

  BPatch_originalAddressExpr();
};

class BPATCH_DLL_EXPORT BPatch_actualAddressExpr : public BPatch_snippet
{
 public:
  //  BPatch_actualAddressExpr
  //  Construct a snippet representing the actual (relocated) address
  //  where the snippet was executed.

  BPatch_actualAddressExpr();
};

class BPATCH_DLL_EXPORT BPatch_dynamicTargetExpr : public BPatch_snippet
{
 public:
  //  BPatch_dynamicTargetExpr
  //  Construct a snippet to calculate the target of a 
  //  dynamic control transfer instruction

  BPatch_dynamicTargetExpr();
};

class BPATCH_DLL_EXPORT BPatch_scrambleRegistersExpr : public BPatch_snippet
{

  public:
  // BPatch_scrambleRegistersExpr
  // Set all GPR to flag value.

  BPatch_scrambleRegistersExpr();
};

#endif /* _BPatch_snippet_h_ */


