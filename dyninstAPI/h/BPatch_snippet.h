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
    BPatch_bit_and,
    BPatch_bit_or,
    BPatch_bit_xor,
    BPatch_left_shift,
    BPatch_right_shift
} BPatch_binOp;

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

    int PDSEP_astMinCost();


    BPatch_snippet();
    BPatch_snippet(const AstNodePtr& ast);

 public:  BPatch_snippet(const BPatch_snippet &src);


  public:

    BPatch_type * getType();

  public: 

    BPatch_snippet & operator=(const BPatch_snippet &src);

    virtual ~BPatch_snippet();

    bool is_trivial();

    bool checkTypesAtPoint(BPatch_point* p) const;
    
    AstNodePtr ast_wrapper;

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
    BPatch_constExpr(signed int value);

    BPatch_constExpr(unsigned int value);
    
    BPatch_constExpr(signed long value);
    
    BPatch_constExpr(unsigned long value);

	BPatch_constExpr(unsigned long long value);

    BPatch_constExpr(const char *value);

    BPatch_constExpr(const void *value);

    BPatch_constExpr(long long value);

    BPatch_constExpr() : BPatch_snippet() {}

};

class BPATCH_DLL_EXPORT BPatch_whileExpr : public BPatch_snippet {
  public:
   BPatch_whileExpr(const BPatch_snippet &condition,
                    const BPatch_snippet &body);
};

class BPATCH_DLL_EXPORT BPatch_funcCallExpr : public BPatch_snippet {
 public:
    BPatch_funcCallExpr(const BPatch_function& func,
                         const BPatch_Vector<BPatch_snippet *> &args);
};

class BPATCH_DLL_EXPORT BPatch_ifExpr : public BPatch_snippet {
 public:
    BPatch_ifExpr(const BPatch_boolExpr &conditional,
                   const BPatch_snippet &tClause);

    BPatch_ifExpr(const BPatch_boolExpr &conditional,
                   const BPatch_snippet &tClause,
                   const BPatch_snippet &fClause);
};

class BPATCH_DLL_EXPORT BPatch_nullExpr : public BPatch_snippet {
 public:
    BPatch_nullExpr();

};

class BPATCH_DLL_EXPORT BPatch_paramExpr : public BPatch_snippet {
 public:
    BPatch_paramExpr(int n, BPatch_ploc loc=BPatch_ploc_guess);
};

class BPATCH_DLL_EXPORT BPatch_retExpr : public BPatch_snippet {
 public:
    BPatch_retExpr();
};

class BPATCH_DLL_EXPORT BPatch_retAddrExpr : public BPatch_snippet {
 public:
    BPatch_retAddrExpr();
};

class BPATCH_DLL_EXPORT BPatch_registerExpr : public BPatch_snippet {
 public:
    friend class BPatch_addressSpace;
                    BPatch_registerExpr(BPatch_register reg);
                    BPatch_registerExpr(Dyninst::MachRegister reg);
};

class BPATCH_DLL_EXPORT BPatch_sequence : public BPatch_snippet {
 public:
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

    BPatch_variableExpr(const char *in_name, 
                        BPatch_addressSpace *in_addSpace,
                        AddressSpace *as,
                        AstNodePtr ast_wrapper_,
                        BPatch_type *type, void* in_address);

    BPatch_variableExpr(BPatch_addressSpace *in_addSpace, 
                        AddressSpace *as,
                        void *in_address, 
                        int in_register, BPatch_type *type, 
                        BPatch_storageClass storage = BPatch_storageAddr,
                        BPatch_point *scp = NULL);

    BPatch_variableExpr(BPatch_addressSpace *in_addSpace,
                        AddressSpace *as,
                        BPatch_localVar *lv, BPatch_type *type,
                        BPatch_point *scp);
    
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

    unsigned int getSize() const;

    const BPatch_type * getType();

    bool setType(BPatch_type *t);

    bool setSize(int sz);

    bool readValue(void *dst);

    bool readValue(void *dst, int len);

    bool writeValue(const void *src, bool saveWorld=false);

    bool writeValue(const void *src, int len,bool saveWorld=false);

    const char * getName();

    void * getBaseAddr();

    BPatch_Vector<BPatch_variableExpr *> * getComponents();
};

class BPATCH_DLL_EXPORT BPatch_breakPointExpr : public BPatch_snippet {
 public:
    BPatch_breakPointExpr();
};

class BPATCH_DLL_EXPORT BPatch_effectiveAddressExpr : public BPatch_snippet
{
 public:
  BPatch_effectiveAddressExpr(int _which = 0, int size = 8);
};


class BPATCH_DLL_EXPORT BPatch_bytesAccessedExpr : public BPatch_snippet
{
 public:
  BPatch_bytesAccessedExpr(int _which = 0);
};

class BPATCH_DLL_EXPORT BPatch_ifMachineConditionExpr : public BPatch_snippet {
 public:
  BPatch_ifMachineConditionExpr(const BPatch_snippet &tClause);
};


class BPATCH_DLL_EXPORT BPatch_threadIndexExpr : public BPatch_snippet {
 public:
  BPatch_threadIndexExpr();
};

class BPATCH_DLL_EXPORT BPatch_tidExpr : public BPatch_snippet {
 public:
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
  BPatch_shadowExpr(bool entry, 
		    const BPatchStopThreadCallback &cb,
		    const BPatch_snippet &calculation,
		    bool useCache = false,
		    BPatch_stInterpret interp = BPatch_noInterp);
};

class BPATCH_DLL_EXPORT BPatch_stopThreadExpr : public BPatch_snippet {
 public:
  BPatch_stopThreadExpr(const BPatchStopThreadCallback &cb,
			const BPatch_snippet &calculation,
			bool useCache = false,
			BPatch_stInterpret interp = BPatch_noInterp);

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
  BPatch_originalAddressExpr();
};

class BPATCH_DLL_EXPORT BPatch_actualAddressExpr : public BPatch_snippet
{
 public:
  BPatch_actualAddressExpr();
};

class BPATCH_DLL_EXPORT BPatch_dynamicTargetExpr : public BPatch_snippet
{
 public:
  BPatch_dynamicTargetExpr();
};

class BPATCH_DLL_EXPORT BPatch_scrambleRegistersExpr : public BPatch_snippet
{

  public:
  BPatch_scrambleRegistersExpr();
};

#endif /* _BPatch_snippet_h_ */


