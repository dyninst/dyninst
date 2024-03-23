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

// $Id: ast.h,v 1.107 2008/05/12 22:12:47 giri Exp $

#ifndef AST_HDR
#define AST_HDR

#include <assert.h>
#include <utility>
#include <vector>
#include <stdio.h>
#include <string>
#include <unordered_map>

#include "dyn_register.h"
#include "Point.h"

#include "BPatch_snippet.h"

#include "BPatch_type.h"

#include "arch-forward-decl.h" // instruction

class AddressSpace;
class instPoint;
class func_instance;
class int_variable;
class codeGen;
class image_variable;





#include "opcode.h"

class AstNode;
typedef boost::shared_ptr<AstNode> AstNodePtr;
class AstMiniTrampNode;
typedef boost::shared_ptr<AstMiniTrampNode> AstMiniTrampNodePtr;

typedef enum {
   cfj_unset = 0,
   cfj_none = 1,
   cfj_jump = 2,
   cfj_call = 3
} cfjRet_t;

class registerSpace;

class regTracker_t {
public:
	class commonExpressionTracker {
	public:
		Dyninst::Register keptRegister;
		int keptLevel;
		commonExpressionTracker() : keptRegister(Dyninst::Null_Register), keptLevel(-1) {}
	};

	int condLevel;
	
	static unsigned astHash(AstNode * const &ast);

  regTracker_t() : condLevel(0) {}

	std::unordered_map<AstNode *, commonExpressionTracker> tracker;

	void addKeptRegister(codeGen &gen, AstNode *n, Dyninst::Register reg);
	void removeKeptRegister(codeGen &gen, AstNode *n);
	Dyninst::Register hasKeptRegister(AstNode *n);
	bool stealKeptRegister(Dyninst::Register reg);

	void reset();

	void increaseConditionalLevel();
	void decreaseAndClean(codeGen &gen);
	void cleanKeptRegisters(int level);
	void debugPrint();
	

};

class dataReqNode;
class AstNode : public Dyninst::PatchAPI::Snippet {
 public:
   enum nodeType { sequenceNode_t, opCodeNode_t, operandNode_t, callNode_t, scrambleRegisters_t};
   enum class operandType { Constant, 
                      ConstantString,
                      DataReg,
                      DataIndir,
                      Param,
                      ParamAtCall,
                      ParamAtEntry,
                      ReturnVal, 
                      ReturnAddr,
                      DataAddr,
                      FrameAddr,
                      RegOffset,
                      //PreviousStackFrameDataReg,
                      //RegValue, // A possibly spilled, possibly saved register.
                      // Both the above are now: origRegister 
                      origRegister,
                      variableAddr,
                      variableValue,
                      undefOperandType };



   enum memoryType {
      EffectiveAddr,
      BytesAccessed };

   enum MSpecialType{
       GENERIC_AST,
       CANARY_AST
   };

  protected:
   int lineNum;
   int columnNum;
   char *snippetName;

   bool lineInfoSet;
   bool columnInfoSet;
   bool snippetNameSet;

  public:
   virtual std::string format(std::string indent);
   std::string convert(operandType type);
   std::string convert(opCode op);

   int getLineNum();
   int getColumnNum();
   char *getSnippetName();

   void setLineNum(int ln);
   void setColumnNum(int cn);
   void setSnippetName(char *n);

   bool hasLineInfo();
   bool hasColumnInfo();
   bool hasNameInfo();
   
   AstNode();

   static AstNodePtr nullNode();

   static AstNodePtr stackInsertNode(int size, MSpecialType type = GENERIC_AST);
   static AstNodePtr stackRemoveNode(int size, MSpecialType type);
   static AstNodePtr stackRemoveNode(int size, MSpecialType type, func_instance* func, bool canaryAfterPrologue, long canaryHeight);
   static AstNodePtr stackGenericNode();
   bool allocateCanaryRegister(codeGen& gen, bool noCost, Dyninst::Register& reg, bool& needSaveAndRestore);

   static AstNodePtr labelNode(std::string &label);

   static AstNodePtr operandNode(operandType ot, void *arg);
   static AstNodePtr operandNode(operandType ot, AstNodePtr ast);
   static AstNodePtr operandNode(operandType ot, const image_variable* iv);

   static AstNodePtr memoryNode(memoryType ot, int which, int size = 8);

   static AstNodePtr sequenceNode(std::vector<AstNodePtr > &sequence);
        
   static AstNodePtr variableNode(std::vector<AstNodePtr>&ast_wrappers_, std::vector<std::pair<Dyninst::Offset, Dyninst::Offset> > *ranges = NULL);

   static AstNodePtr operatorNode(opCode ot, 
                                  AstNodePtr l = AstNodePtr(), 
                                  AstNodePtr r = AstNodePtr(), 
                                  AstNodePtr e = AstNodePtr());

   static AstNodePtr funcCallNode(const std::string &func, std::vector<AstNodePtr > &args, AddressSpace *addrSpace = NULL);
   static AstNodePtr funcCallNode(func_instance *func, std::vector<AstNodePtr > &args);
   static AstNodePtr funcCallNode(func_instance *func);
   static AstNodePtr funcCallNode(Dyninst::Address addr, std::vector<AstNodePtr > &args);

   static AstNodePtr threadIndexNode();

   static AstNodePtr scrambleRegistersNode();

#if 0
   static AstNodePtr saveStateNode();
   static AstNodePtr restoreStateNode();
   static AstNodePtr trampGuardNode();
#endif

   static AstNodePtr miniTrampNode(AstNodePtr tramp);

   static AstNodePtr originalAddrNode();
   static AstNodePtr actualAddrNode();
   static AstNodePtr dynamicTargetNode();

   static AstNodePtr snippetNode(Dyninst::PatchAPI::SnippetPtr snip);

   AstNode(AstNodePtr src);
   //virtual AstNode &operator=(const AstNode &src);
        
   virtual ~AstNode();
        
   virtual bool generateCode(codeGen &gen, 
                             bool noCost, 
                             Dyninst::Address &retAddr,
                             Dyninst::Register &retReg);

   virtual bool generateCode(codeGen &gen, 
                             bool noCost);

   virtual bool generateCode(codeGen &gen, 
                             bool noCost, 
                             Dyninst::Register &retReg) {
      Dyninst::Address unused = Dyninst::ADDR_NULL;
      return generateCode(gen, noCost,  unused, retReg);
   }

   virtual bool generateCode_phase2(codeGen &gen,
                                    bool noCost,
                                    Dyninst::Address &retAddr,
                                    Dyninst::Register &retReg);

   virtual bool initRegisters(codeGen &gen);
        
   virtual void setVariableAST(codeGen &) {}

   unsigned getTreeSize();

   bool decRefCount();

   bool previousComputationValid(Dyninst::Register &reg,
                                 codeGen &gen);
   void cleanRegTracker(regTracker_t *tracker, int level);

   virtual AstNodePtr operand() const { return AstNodePtr(); }



   virtual bool containsFuncCall() const = 0;
   virtual bool usesAppRegister() const = 0;

   enum CostStyleType { Min, Avg, Max };
   int minCost() const {  return costHelper(Min);  }
   int avgCost() const {  return costHelper(Avg);  }
   int maxCost() const {  return costHelper(Max);  }

	virtual int costHelper(enum CostStyleType) const { return 0; }	

   int referenceCount;
   int useCount;
   void setUseCount();
   int getSize() { return size; }
   void cleanUseCount(void);
   bool checkUseCount(registerSpace*, bool&);
   void printUseCount(void);

   virtual const std::vector<AstNodePtr> getArgs() { return std::vector<AstNodePtr>(); }


   virtual void setChildren(std::vector<AstNodePtr > &children);
   virtual AstNodePtr deepCopy() { return AstNodePtr(this);}
   

	void decUseCount(codeGen &gen);

	void fixChildrenCounts();

	virtual bool canBeKept() const = 0;

	Dyninst::Register allocateAndKeep(codeGen &gen, bool noCost);

   bool stealRegister(Dyninst::Register reg);

	bool subpath(const std::vector<AstNode*> &path1, 
                const std::vector<AstNode*> &path2) const;

	virtual void getChildren(std::vector<AstNodePtr> &); 

	virtual bool accessesParam(void);

	virtual void setOValue(void *) { assert(0); }
	virtual const void *getOValue() const { assert(0); return NULL; }
	virtual const image_variable* getOVar() const {
      return NULL;
	}
	
	virtual void emitVariableStore(opCode, Dyninst::Register, Dyninst::Register, codeGen&,
                                  bool, registerSpace*, 
                                  int, const instPoint*, AddressSpace*)
	{
      assert(!"Never call this on anything but an operand");
	}
	virtual void emitVariableLoad(opCode, Dyninst::Register, Dyninst::Register, codeGen&,
                                 bool, registerSpace*, 
                                 int, const instPoint*, AddressSpace*)
	{
      assert(!"Never call this on anything but an operand");
	}

	bool condMatch(AstNode* a,
                  std::vector<dataReqNode*> &data_tuple1,
                  std::vector<dataReqNode*> &data_tuple2,
                  std::vector<dataReqNode*> datareqs1,
                  std::vector<dataReqNode*> datareqs2);


   virtual operandType getoType() const { return operandType::undefOperandType; }

   virtual void setConstFunc(bool) {}

 protected:
	BPatch_type *bptype;
	bool doTypeCheck;
	int size;


 public:
	//AstNode(operandType ot, int which); // for memory access
	BPatch_type *getType();
	void		  setType(BPatch_type *t);
	void		  setTypeChecking(bool x) { doTypeCheck = x; }
	virtual BPatch_type	  *checkType(BPatch_function* func = NULL);
	

        virtual bool generate(Dyninst::PatchAPI::Point *, 
                              Dyninst::Buffer &);

 private:
   static AstNodePtr originalAddrNode_;
   static AstNodePtr actualAddrNode_;
   static AstNodePtr dynamicTargetNode_;
};


class AstNullNode : public AstNode {
 public:

    AstNullNode() : AstNode() {}

   virtual std::string format(std::string indent);
    virtual bool containsFuncCall() const;
    virtual bool usesAppRegister() const;
    
    bool canBeKept() const { return true; }
 private:
    virtual bool generateCode_phase2(codeGen &gen,
                                     bool noCost,
                                     Dyninst::Address &retAddr,
                                     Dyninst::Register &retReg);
};

class AstStackInsertNode : public AstNode {
    public:
        AstStackInsertNode(int s, MSpecialType t) : AstNode(),
        size(s),
        type(t) {}

        virtual std::string format(std::string indent);
        virtual bool containsFuncCall() const;
        virtual bool usesAppRegister() const;

        bool canBeKept() const { return true; }

    private:
    virtual bool generateCode_phase2(codeGen &gen,
                                     bool noCost,
                                     Dyninst::Address &retAddr,
                                     Dyninst::Register &retReg);

    int size;
    MSpecialType type;
};

class AstStackRemoveNode : public AstNode {
    public:
        AstStackRemoveNode(int s, MSpecialType t = GENERIC_AST) : AstNode(),
        size(s),
        type(t) {}

        AstStackRemoveNode(int s, MSpecialType t, func_instance* func, bool canaryAfterPrologue, long canaryHeight) :
            AstNode(),
            size(s),
            type(t),
            func_(func),
            canaryAfterPrologue_(canaryAfterPrologue),
            canaryHeight_(canaryHeight)
    {}

        virtual std::string format(std::string indent);
        virtual bool containsFuncCall() const;
        virtual bool usesAppRegister() const;

        bool canBeKept() const { return true; }

    private:
    virtual bool generateCode_phase2(codeGen &gen,
                                     bool noCost,
                                     Dyninst::Address &retAddr,
                                     Dyninst::Register &retReg);

    int size;
    MSpecialType type;

    func_instance* func_{};
    bool canaryAfterPrologue_{};
    long canaryHeight_{};
};

class AstStackGenericNode : public AstNode {
    public: AstStackGenericNode() : AstNode() {}
            virtual std::string format(std::string indent);
            virtual bool containsFuncCall() const;
            virtual bool usesAppRegister() const;

            bool canBeKept() const { return true; }
    private:
            virtual bool generateCode_phase2(codeGen &gen,
                    bool noCost,
                    Dyninst::Address &retAddr,
                    Dyninst::Register &retReg);
};

class AstLabelNode : public AstNode {
 public:
    AstLabelNode(std::string &label) : AstNode(), label_(label), generatedAddr_(0) {}
    virtual bool containsFuncCall() const;
    virtual bool usesAppRegister() const;

	bool canBeKept() const { return true; }
 private:
    virtual bool generateCode_phase2(codeGen &gen,
                                     bool noCost,
                                     Dyninst::Address &retAddr,
                                     Dyninst::Register &retReg);
    std::string label_;
    Dyninst::Address generatedAddr_;
};

class AstOperatorNode : public AstNode {
 public:

    AstOperatorNode(opCode opC, AstNodePtr l, AstNodePtr r = AstNodePtr(), AstNodePtr e = AstNodePtr());

   virtual std::string format(std::string indent);
    virtual int costHelper(enum CostStyleType costStyle) const;	

    virtual BPatch_type	  *checkType(BPatch_function* func = NULL);
    virtual bool accessesParam(void);

    virtual bool canBeKept() const;

    virtual void getChildren(std::vector<AstNodePtr> &children);
    
    virtual void setChildren(std::vector<AstNodePtr> &children);
    virtual AstNodePtr deepCopy();

    virtual bool containsFuncCall() const;
    virtual bool usesAppRegister() const;
 

    virtual bool initRegisters(codeGen &gen);
    
    virtual void setVariableAST(codeGen &gen);

 private:

    virtual bool generateCode_phase2(codeGen &gen,
                                     bool noCost,
                                     Dyninst::Address &retAddr,
                                     Dyninst::Register &retReg);

    bool generateOptimizedAssignment(codeGen &gen, int size, bool noCost);

    opCode op{};
    AstNodePtr loperand;
    AstNodePtr roperand;
    AstNodePtr eoperand;
};


class AstOperandNode : public AstNode {
    friend class AstOperatorNode;
 public:

    AstOperandNode(operandType ot, void *arg);

    AstOperandNode(operandType ot, AstNodePtr l);

    AstOperandNode(operandType ot, const image_variable* iv);
    
    ~AstOperandNode() {
        //printf("at ~AstOperandNode()\n");
        //debugPrint();
        if (oType == operandType::ConstantString) free((char *)oValue);
    }

   virtual std::string format(std::string indent);

    virtual operandType getoType() const { return oType; }

    virtual void setOValue(void *o) { oValue = o; }
    virtual const void *getOValue() const { return oValue; }
    virtual const image_variable* getOVar() const 
    {
      return oVar;
    }
    

    virtual AstNodePtr operand() const { return operand_; }

    virtual int costHelper(enum CostStyleType costStyle) const;	
        
    virtual BPatch_type	  *checkType(BPatch_function* func = NULL);

    virtual bool accessesParam(void) { return (oType == operandType::Param || oType == operandType::ParamAtEntry || oType == operandType::ParamAtCall); }
    virtual bool canBeKept() const;
        
    virtual void getChildren(std::vector<AstNodePtr> &children);
    
    virtual void setChildren(std::vector<AstNodePtr> &children);
    virtual AstNodePtr deepCopy();

    virtual void setVariableAST(codeGen &gen);

    virtual bool containsFuncCall() const;

    virtual bool usesAppRegister() const;
 
    virtual void emitVariableStore(opCode op, Dyninst::Register src1, Dyninst::Register src2, codeGen& gen,
			   bool noCost, registerSpace* rs, 
			   int size, const instPoint* point, AddressSpace* as);
    virtual void emitVariableLoad(opCode op, Dyninst::Register src2, Dyninst::Register dest, codeGen& gen,
			  bool noCost, registerSpace* rs, 
			  int size, const instPoint* point, AddressSpace* as);

    virtual bool initRegisters(codeGen &gen);
        
 private:
    virtual bool generateCode_phase2(codeGen &gen,
                                     bool noCost,
                                     Dyninst::Address &retAddr,
                                     Dyninst::Register &retReg);
    int_variable* lookUpVar(AddressSpace* as);
    
    AstOperandNode(): oType(operandType::undefOperandType), oValue(NULL), oVar(NULL) {}

    operandType oType;
    void *oValue;
    const image_variable* oVar;
    AstNodePtr operand_;
};


class AstCallNode : public AstNode {
 public:

    AstCallNode(func_instance *func, std::vector<AstNodePtr>&args);
    AstCallNode(const std::string &str, std::vector<AstNodePtr>&args);
    AstCallNode(Dyninst::Address addr, std::vector<AstNodePtr> &args);
    AstCallNode(func_instance *func);
    
    ~AstCallNode() {}

   virtual std::string format(std::string indent);

    virtual int costHelper(enum CostStyleType costStyle) const;	
        
    virtual BPatch_type	  *checkType(BPatch_function* func = NULL);
    virtual bool accessesParam(); 
    virtual bool canBeKept() const;

    virtual void getChildren(std::vector<AstNodePtr> &children);
    
    virtual void setChildren(std::vector<AstNodePtr> &children);
    virtual AstNodePtr deepCopy();

    virtual void setVariableAST(codeGen &gen);
    virtual bool containsFuncCall() const; 
    virtual bool usesAppRegister() const;
 
    void setConstFunc(bool val) { constFunc_ = val; }

    virtual bool initRegisters(codeGen &gen);

 private:
    virtual bool generateCode_phase2(codeGen &gen,
                                     bool noCost,
                                     Dyninst::Address &retAddr,
                                     Dyninst::Register &retReg);

    AstCallNode(): func_addr_(0), func_(NULL), callReplace_(false), constFunc_(false) {}

    const std::string func_name_;
    Dyninst::Address func_addr_;
    
    func_instance *func_;
    std::vector<AstNodePtr> args_;

    bool callReplace_;
    bool constFunc_;
};


class AstSequenceNode : public AstNode {
 public:
    AstSequenceNode(std::vector<AstNodePtr> &sequence);

    ~AstSequenceNode() {}

   virtual std::string format(std::string indent);

    virtual int costHelper(enum CostStyleType costStyle) const;	

    virtual BPatch_type	  *checkType(BPatch_function* func = NULL);
    virtual bool accessesParam();
    virtual bool canBeKept() const;

    virtual void getChildren(std::vector<AstNodePtr> &children);
    
    virtual void setChildren(std::vector<AstNodePtr> &children);
    virtual AstNodePtr deepCopy();

    virtual void setVariableAST(codeGen &gen);
    virtual bool containsFuncCall() const;
    virtual bool usesAppRegister() const;
 

 private:
    virtual bool generateCode_phase2(codeGen &gen,
                                     bool noCost,
                                     Dyninst::Address &retAddr,
                                     Dyninst::Register &retReg);

    AstSequenceNode() {}
    std::vector<AstNodePtr> sequence_;
};

class AstVariableNode : public AstNode {
  public:
    AstVariableNode(std::vector<AstNodePtr>&ast_wrappers, std::vector<std::pair<Dyninst::Offset, Dyninst::Offset> >*ranges);

    ~AstVariableNode() {}

    virtual std::string format(std::string indent);

    virtual int costHelper(enum CostStyleType costStyle) const;	

    virtual BPatch_type	  *checkType(BPatch_function* = NULL) { return getType(); }
    virtual bool accessesParam();
    virtual bool canBeKept() const;
    virtual operandType getoType() const { return ast_wrappers_[index]->getoType(); }
    virtual AstNodePtr operand() const { return ast_wrappers_[index]->operand(); }
    virtual const void *getOValue() const { return ast_wrappers_[index]->getOValue(); }

    virtual void setVariableAST(codeGen &gen);

    virtual void getChildren(std::vector<AstNodePtr> &children);
    
    virtual void setChildren(std::vector<AstNodePtr> &children);
    virtual AstNodePtr deepCopy();

    virtual bool containsFuncCall() const;
    virtual bool usesAppRegister() const;
 

 private:
    virtual bool generateCode_phase2(codeGen &gen,
                                     bool noCost,
                                     Dyninst::Address &retAddr,
                                     Dyninst::Register &retReg);

    AstVariableNode(): ranges_(NULL), index(0) {}
    std::vector<AstNodePtr>ast_wrappers_;
    std::vector<std::pair<Dyninst::Offset, Dyninst::Offset> > *ranges_;
    unsigned index;

};


class AstMiniTrampNode : public AstNode {
 public:
    AstMiniTrampNode(AstNodePtr ast): inline_(false) {
       if (ast != AstNodePtr())
          ast->referenceCount++;
       ast_ = ast;
    }


    Dyninst::Address generateTramp(codeGen &gen,
                          int &trampCost, 
                          bool noCost);
            
    virtual ~AstMiniTrampNode() {}    

    virtual bool accessesParam(void) { return ast_->accessesParam(); } 

    virtual void getChildren(std::vector<AstNodePtr> &children);
    
    virtual void setChildren(std::vector<AstNodePtr> &children);
    virtual AstNodePtr deepCopy();

    virtual void setVariableAST(codeGen &gen);

    virtual bool containsFuncCall() const;
    virtual bool usesAppRegister() const;
 

    bool canBeKept() const;

    AstNodePtr getAST() { return ast_; }
 private:
    AstMiniTrampNode(): inline_(false) {}

    bool inline_;
    AstNodePtr ast_;
};

class AstMemoryNode : public AstNode {
 public:
    AstMemoryNode(memoryType mem, unsigned which, int size);
	bool canBeKept() const;

   virtual std::string format(std::string indent);
   virtual bool containsFuncCall() const;
   virtual bool usesAppRegister() const;
 

 private:
    virtual bool generateCode_phase2(codeGen &gen,
                                     bool noCost,
                                     Dyninst::Address &retAddr,
                                     Dyninst::Register &retReg);
    
    AstMemoryNode() {}
    memoryType mem_{};
    unsigned which_{};
};

class AstOriginalAddrNode : public AstNode {
 public:
    AstOriginalAddrNode() {}

    virtual ~AstOriginalAddrNode() {}


    virtual BPatch_type *checkType(BPatch_function*  = NULL) { return getType(); }
    virtual bool canBeKept() const { return true; }
    virtual bool containsFuncCall() const;
    virtual bool usesAppRegister() const;
 

 private:
    virtual bool generateCode_phase2(codeGen &gen,
                                     bool noCost,
                                     Dyninst::Address &retAddr,
                                     Dyninst::Register &retReg);
};

class AstActualAddrNode : public AstNode {
 public:
    AstActualAddrNode() {}

    virtual ~AstActualAddrNode() {}


    virtual BPatch_type *checkType(BPatch_function*  = NULL) { return getType(); }
    virtual bool canBeKept() const { return false; }
    virtual bool containsFuncCall() const;
    virtual bool usesAppRegister() const;
 
 private:
    virtual bool generateCode_phase2(codeGen &gen,
                                     bool noCost,
                                     Dyninst::Address &retAddr,
                                     Dyninst::Register &retReg);
};

class AstDynamicTargetNode : public AstNode {
 public:
    AstDynamicTargetNode() {}

    virtual ~AstDynamicTargetNode() {}


    virtual BPatch_type *checkType(BPatch_function*  = NULL) { return getType(); }
    virtual bool canBeKept() const { return false; }
    virtual bool containsFuncCall() const;
    virtual bool usesAppRegister() const;
 
 private:
    virtual bool generateCode_phase2(codeGen &gen,
                                     bool noCost,
                                     Dyninst::Address &retAddr,
                                     Dyninst::Register &retReg);
};
class AstScrambleRegistersNode : public AstNode {
 public:
    AstScrambleRegistersNode() {}

    virtual ~AstScrambleRegistersNode() {}

    virtual bool canBeKept() const { return false; }
    virtual bool containsFuncCall() const;
    virtual bool usesAppRegister() const;
 
 private:
    virtual bool generateCode_phase2(codeGen &gen,
                                     bool noCost,
                                     Dyninst::Address &retAddr,
                                     Dyninst::Register &retReg);
};


class AstSnippetNode : public AstNode {
  public:
  AstSnippetNode(Dyninst::PatchAPI::SnippetPtr snip) : snip_(snip) {}
   bool canBeKept() const { return false; }
   bool containsFuncCall() const { return false; }
   bool usesAppRegister() const { return false; }
   
  private:
    virtual bool generateCode_phase2(codeGen &gen,
                                     bool noCost,
                                     Dyninst::Address &retAddr,
                                     Dyninst::Register &retReg);
    Dyninst::PatchAPI::SnippetPtr snip_;
};

void emitLoadPreviousStackFrameRegister(Dyninst::Address register_num,
					Dyninst::Register dest,
                                        codeGen &gen,
					int size,
					bool noCost);
void emitStorePreviousStackFrameRegister(Dyninst::Address register_num,
                                         Dyninst::Register src,
                                         codeGen &gen,
                                         int size,
                                         bool noCost);

#define SCAST_AST(ast) boost::static_pointer_cast<AstNode>(ast)
#define DCAST_AST(ast) boost::dynamic_pointer_cast<AstNode>(ast)


#endif /* AST_HDR */
