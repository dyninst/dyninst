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

#include "dyninstAPI/src/inst-sparc.h"
#include "dyninstAPI/src/instPoint.h"

instruction newInstr[1024];
static dictionary_hash<string, unsigned> funcFrequencyTable(string::hash);

trampTemplate baseTemplate;
registerSpace *regSpace;

#if defined(SHM_SAMPLING) && defined(MT_THREAD)
// registers 8 to 15: out registers 
// registers 16 to 22: local registers
int deadList[] = {16, 17, 18, 19, 20, 21, 22 };
#else
int deadList[] = {16, 17, 18, 19, 20, 21, 22, 23 };
#endif

int deadListSize = sizeof(deadList);

// Constructor for the class instPoint. This one defines the 
// instPoints for the relocated function. Since the function reloated
// to the heap don't need to worry about that jump could be out of
// reach, the instructions to be moved are the one instruction at that
// point plus others if necessary(i.e. instruction in the delayed
// slot and maybe the aggregate instuction ).    
instPoint::instPoint(pd_Function *f, const instruction &instr, 
		     const image *owner, Address &adr, bool delayOK, 
		     instPointType pointType, Address &oldAddr)
: addr(adr), originalInstruction(instr), inDelaySlot(false), isDelayed(false),
  callIndirect(false), callAggregate(false), callee(NULL), func(f), 
  ipType(pointType), image_ptr(owner), firstIsConditional(false), 
  relocated_(false), isLongJump(false)
{
  assert(f->isTrapFunc() == true);  

  isBranchOut = false;
  delaySlotInsn.raw = owner->get_instruction(oldAddr+4);
  aggregateInsn.raw = owner->get_instruction(oldAddr+8);

  // If the instruction is a DCTI, the instruction in the delayed 
  // slot is to be moved.
  if (IS_DELAYED_INST(instr))
    isDelayed = true;

  // If this function call another function which return an aggregate
  // value, move the aggregate instruction, too. 
  if (ipType == callSite) {
      if (!IS_VALID_INSN(aggregateInsn) && aggregateInsn.raw != 0) {
	  callAggregate = true;
	  adr += 8;
	  oldAddr += 8;
      }
  }

  if (owner->isValidAddress(oldAddr-4)) {
    instruction iplus1;
    iplus1.raw = owner->get_instruction(oldAddr-4);
    if (IS_DELAYED_INST(iplus1) && !delayOK) {
      // ostrstream os(errorLine, 1024, ios::out);
      // os << "** inst point " << func->file->fullName << "/"
      //  << func->prettyName() << " at addr " << addr <<
      //        " in a delay slot\n";
      // logLine(errorLine);
      inDelaySlot = true;
    }
  }
}

// Add the astNode opt to generate one instruction to get the 
// return value for the compiler optimazed case
void
AstNode::optRetVal(AstNode *opt) {

    if (oType == ReturnVal) {
	cout << "Optimazed Return." << endl;
	if (loperand == 0) {
	    loperand = opt;
	    return;
	} else if (opt == 0) {
	    delete loperand;
	    loperand = NULL;
	    return; 
	}
    }
    if (loperand) loperand->optRetVal(opt);
    if (roperand) roperand->optRetVal(opt);
    for (unsigned i = 0; i < operands.size(); i++) 
	operands[i] -> optRetVal(opt);
}

bool 
processOptimaRet(instPoint *location, AstNode *&ast) {

    // For optimazed return code
    if (location -> ipType == functionExit) {
	if ((isInsnType(location -> originalInstruction, RETmask, RETmatch)) ||
	    (isInsnType(location -> originalInstruction, RETLmask, RETLmatch)))
	{
	    if (isInsnType(location -> delaySlotInsn, 
			   RESTOREmask, RESTOREmatch)&&
		((location->delaySlotInsn.raw | 0xc1e82000) != 0xc1e82000)) 
	    {
		/* cout << "Optimazed Retrun Value:  Addr " << hex << 
		    location->addr << " in "
			<< location -> func -> prettyName() << endl; */
		AstNode *opt = new AstNode(AstNode::Constant,
					   (void *)location->delaySlotInsn.raw);
		ast -> optRetVal(opt);
		return true;
	    }
	}
    }
    return false;
}

unsigned 
emitOptReturn(unsigned instr, reg src, char *insn, unsigned &base, bool noCost) {
    
    cout << "Handling a special case for optimazed return value." << endl;

    assert((((instr&0x3e000000)>>25) >= 0) ||
	   (((instr&0x3e000000)>>25) <= 16));

    if ((instr&0x02000)>>13)
	emitImm(plusOp, (instr&0x07c000)>>14, instr&0x01fff,
		((instr&0x3e000000)>>25)+16, insn, base, noCost);
    else
	emit(plusOp, (instr&0x07c000)>>14, instr&0x01fff,
	     ((instr&0x3e000000)>>25)+16, insn, base, noCost);
    
    return emit(getSysRetValOp, 0, 0, src, insn, base, noCost);
}


//
// initDefaultPointFrequencyTable - define the expected call frequency of
//    procedures.  Currently we just define several one shots with a
//    frequency of one, and provide a hook to read a file with more accurate
//    information.
//
void initDefaultPointFrequencyTable()
{
    FILE *fp;
    float value;
    char name[512];

    funcFrequencyTable["main"] = 1;
    funcFrequencyTable["DYNINSTsampleValues"] = 1;
    funcFrequencyTable[EXIT_NAME] = 1;

    // try to read file.
    fp = fopen("freq.input", "r");
    if (!fp) {
        return;
    } else {
        printf("found freq.input file\n");
    }
    while (!feof(fp)) {
        fscanf(fp, "%s %f\n", name, &value);
        funcFrequencyTable[name] = (int) value;
        printf("adding %s %f\n", name, value);
    }
    fclose(fp);
}

/*
 * Get an etimate of the frequency for the passed instPoint.  
 *    This is not (always) the same as the function that contains the point.
 * 
 *  The function is selected as follows:
 *
 *  If the point is an entry or an exit return the function name.
 *  If the point is a call and the callee can be determined, return the called
 *     function.
 *  else return the funcation containing the point.
 *
 *  WARNING: This code contins arbitray values for func frequency (both user 
 *     and system).  This should be refined over time.
 *
 * Using 1000 calls sec to be one SD from the mean for most FPSPEC apps.
 *	-- jkh 6/24/94
 *
 */
float getPointFrequency(instPoint *point)
{

    pd_Function *func;

    if (point->callee)
        func = point->callee;
    else
        func = point->func;

    if (!funcFrequencyTable.defines(func->prettyName())) {
      // Changing this value from 250 to 100 because predictedCost was
      // too high - naim 07/18/96
      return(100); 
      
    } else {
      return (funcFrequencyTable[func->prettyName()]);
    }
}

//
// return cost in cycles of executing at this point.  This is the cost
//   of the base tramp if it is the first at this point or 0 otherwise.
//
int getPointCost(process *proc, const instPoint *point)
{
    if (proc->baseMap.defines(point)) {
        return(0);
    } else {
        // 70 cycles for base tramp (worst case)
        return(70);
    }
}


void initATramp(trampTemplate *thisTemp, instruction *tramp)
{
    instruction *temp;

    // TODO - are these offset always positive?
    thisTemp->trampTemp = (void *) tramp;
    for (temp = tramp; temp->raw != END_TRAMP; temp++) {
	switch (temp->raw) {
	    case LOCAL_PRE_BRANCH:
		thisTemp->localPreOffset = ((void*)temp - (void*)tramp);
		thisTemp->localPreReturnOffset = thisTemp->localPreOffset 
		                                 + sizeof(temp->raw);
		break;
	    case GLOBAL_PRE_BRANCH:
		thisTemp->globalPreOffset = ((void*)temp - (void*)tramp);
		break;
	    case LOCAL_POST_BRANCH:
		thisTemp->localPostOffset = ((void*)temp - (void*)tramp);
		thisTemp->localPostReturnOffset = thisTemp->localPostOffset
		                                  + sizeof(temp->raw);
		break;
	    case GLOBAL_POST_BRANCH:
		thisTemp->globalPostOffset = ((void*)temp - (void*)tramp);
		break;
	    case SKIP_PRE_INSN:
                thisTemp->skipPreInsOffset = ((void*)temp - (void*)tramp);
                break;
	    case UPDATE_COST_INSN:
		thisTemp->updateCostOffset = ((void*)temp - (void*)tramp);
		break;
	    case SKIP_POST_INSN:
                thisTemp->skipPostInsOffset = ((void*)temp - (void*)tramp);
                break;
	    case RETURN_INSN:
                thisTemp->returnInsOffset = ((void*)temp - (void*)tramp);
                break;
	    case EMULATE_INSN:
                thisTemp->emulateInsOffset = ((void*)temp - (void*)tramp);
                break;
  	}	
    }

    // Cost with the skip branchs.
    thisTemp->cost = 14;  
    thisTemp->prevBaseCost = 20;
    thisTemp->postBaseCost = 22;
    thisTemp->prevInstru = thisTemp->postInstru = false;
    thisTemp->size = (int) temp - (int) tramp;
}


void initTramps()
{
    static bool inited=false;

    if (inited) return;
    inited = true;

    initATramp(&baseTemplate, (instruction *) baseTramp);

    regSpace = new registerSpace(sizeof(deadList)/sizeof(int), deadList,					 0, NULL);
    assert(regSpace);
}

//
// For multithreaded applications and shared memory sampling, this routine 
// will compute the address where the corresponding counter/timer vector for
// level 0 is (by default). In the mini-tramp, if the counter/timer is at a
// different level, we will add the corresponding offset - naim 4/18/97
//
void generateMTpreamble(char *insn, unsigned &base, process *proc)
{
  AstNode *t1,*t2,*t3,*t4,*t5;;
  vector<AstNode *> dummy;
  unsigned tableAddr;
  int value; 
  bool err;
  reg src = -1;

  /* t3=DYNINSTthreadTable[thr_self()] */
  t1 = new AstNode("DYNINSTthreadPos", dummy);
  value = sizeof(unsigned);
  t4 = new AstNode(AstNode::Constant,(void *)value);
  t2 = new AstNode(timesOp, t1, t4);
  removeAst(t1);
  removeAst(t4);

  tableAddr = proc->findInternalAddress("DYNINSTthreadTable",true,err);
  assert(!err);
  t5 = new AstNode(AstNode::Constant, (void *)tableAddr);
  t3 = new AstNode(plusOp, t2, t5);
  removeAst(t2);
  removeAst(t5);
  src = t3->generateCode(proc, regSpace, insn, base, false);
  removeAst(t3);
  (void) emit(orOp, src, 0, REG_MT, insn, base, false);
  regSpace->freeRegister(src);
}

void generateNoOp(process *proc, int addr)
{
    instruction insn;

    /* fill with no-op */
    insn.raw = 0;
    insn.branch.op = 0;
    insn.branch.op2 = NOOPop2;

    // TODO cast
    proc->writeTextWord((caddr_t)addr, insn.raw);
}


/*
 * change the insn at addr to be a branch to newAddr.
 *   Used to add multiple tramps to a point.
 */
void generateBranch(process *proc, unsigned fromAddr, unsigned newAddr)
{
    int disp;
    instruction insn;

    disp = newAddr-fromAddr;
    generateBranchInsn(&insn, disp);

    // TODO cast
    proc->writeTextWord((caddr_t)fromAddr, insn.raw);
}

void generateCall(process *proc, unsigned fromAddr,unsigned newAddr)
{
    instruction insn; 
    generateCallInsn(&insn, fromAddr, newAddr);

    proc->writeTextWord((caddr_t)fromAddr, insn.raw);

}

void genImm(process *proc, Address fromAddr,int op, reg rs1, int immd, reg rd)
{
    instruction insn;
    genImmInsn(&insn, op, rs1, immd, rd);

    proc->writeTextWord((caddr_t)fromAddr, insn.raw);
}

/*
 *  change the target of the branch at fromAddr, to be newAddr.
 */
void changeBranch(process *proc, unsigned fromAddr, unsigned newAddr, 
		  instruction originalBranch) {
    int disp = newAddr-fromAddr;
    instruction insn;
    insn.raw = originalBranch.raw;
    insn.branch.disp22 = disp >> 2;
    proc->writeTextWord((caddr_t)fromAddr, insn.raw);
}

int callsTrackedFuncP(instPoint *point)
{
    if (point->callIndirect) {
        return(true);
    } else {
	if (point->callee) {
	    return(true);
	} else {
	    return(false);
	}
    }
}

/*
 * return the function asociated with a point.
 *
 *     If the point is a funcation call, and we know the function being called,
 *          then we use that.  Otherwise it is the function that contains the
 *          point.
 *  
 *   This is done to return a better idea of which function we are using.
 */
pd_Function *getFunction(instPoint *point)
{
    return(point->callee ? point->callee : point->func);
}

bool process::emitInferiorRPCheader(void *insnPtr, unsigned &baseBytes) {
   instruction *insn = (instruction *)insnPtr;
   unsigned baseInstruc = baseBytes / sizeof(instruction);

   genImmInsn(&insn[baseInstruc++], SAVEop3, 14, -112, 14);

   baseBytes = baseInstruc * sizeof(instruction); // convert back
   return true;
}


bool process::emitInferiorRPCtrailer(void *insnPtr, unsigned &baseBytes,
				     unsigned &breakOffset,
				     bool stopForResult,
				     unsigned &stopForResultOffset,
				     unsigned &justAfter_stopForResultOffset) {
   // Sequence: restore, trap, illegal

   instruction *insn = (instruction *)insnPtr;
   unsigned baseInstruc = baseBytes / sizeof(instruction);

   if (stopForResult) {
      // trap insn:
      genBreakpointTrap(&insn[baseInstruc]);
      stopForResultOffset = baseInstruc * sizeof(instruction);
      baseInstruc++;
      justAfter_stopForResultOffset = baseInstruc * sizeof(instruction);
   }

   genSimpleInsn(&insn[baseInstruc++], RESTOREop3, 0, 0, 0);

   // Now that the inferior has executed the 'restore' instruction, the %in and
   // %local registers have been restored.  We mustn't modify them after this point!!
   // (reminder: the %in and %local registers aren't saved and set with ptrace
   //  GETREGS/SETREGS call)

   // Trap instruction:
   genBreakpointTrap(&insn[baseInstruc]); // ta 1
   breakOffset = baseInstruc * sizeof(instruction);
   baseInstruc++;

   // And just to make sure that we don't continue from the trap:
   genUnimplementedInsn(&insn[baseInstruc++]); // UNIMP 0

   baseBytes = baseInstruc * sizeof(instruction); // convert back

   return true; // success
}

unsigned emitImm(opCode op, reg src1, reg src2, reg dest, char *i, 
                 unsigned &base, bool noCost)
{
        instruction *insn = (instruction *) ((void*)&i[base]);
        int op3=-1;
        int result;
	switch (op) {
	    // integer ops
	    case plusOp:
		op3 = ADDop3;
                genImmInsn(insn, op3, src1, src2, dest);
		break;

	    case minusOp:
		op3 = SUBop3;
                genImmInsn(insn, op3, src1, src2, dest);
		break;

	    case timesOp:
		op3 = SMULop3;
                if (isPowerOf2(src2,result) && (result<32))
                  generateLShift(insn, src1, (reg)result, dest);           
                else 
                  genImmInsn(insn, op3, src1, src2, dest);
		break;

	    case divOp:
		op3 = SDIVop3;
                if (isPowerOf2(src2,result) && (result<32))
                  generateRShift(insn, src1, (reg)result, dest);           
                else { // needs to set the Y register to zero first
                  // Set the Y register to zero: Zhichen
                  genImmInsn(insn, WRYop3, REG_G0, 0, 0);
                  base += sizeof(instruction);
                  insn = (instruction *) ((void*)&i[base]);
                  genImmInsn(insn, op3, src1, src2, dest);
                }

		break;

	    // Bool ops
	    case orOp:
		op3 = ORop3;
                genImmInsn(insn, op3, src1, src2, dest);
		break;

	    case andOp:
		op3 = ANDop3;
                genImmInsn(insn, op3, src1, src2, dest);
		break;

	    // rel ops
	    // For a particular condition (e.g. <=) we need to use the
            // the opposite in order to get the right value (e.g. for >=
            // we need BLTcond) - naim
	    case eqOp:
		genImmRelOp(insn, BNEcond, src1, src2, dest, base);
		return(0);
		break;

            case neOp:
                genImmRelOp(insn, BEcond, src1, src2, dest, base);
                return(0);
                break;

	    case lessOp:
                genImmRelOp(insn, BGEcond, src1, src2, dest, base);
                return(0);
                break;

            case leOp:
                genImmRelOp(insn, BGTcond, src1, src2, dest, base);
                return(0);
                break;

            case greaterOp:
                genImmRelOp(insn, BLEcond, src1, src2, dest, base);
                return(0);
                break;

            case geOp:
                genImmRelOp(insn, BLTcond, src1, src2, dest, base);
                return(0);
                break;

	    default:
                reg dest2 = regSpace->allocateRegister(i, base, noCost);
                (void) emit(loadConstOp, src2, dest2, dest2, i, base, noCost);
                (void) emit(op, src1, dest2, dest, i, base, noCost);
                regSpace->freeRegister(dest2);
                return(0);
		break;
	}
	base += sizeof(instruction);
        return(0);
}

//
// All values based on Cypress0 && Cypress1 implementations as documented in
//   SPARC v.8 manual p. 291
//
int getInsnCost(opCode op)
{
    /* XXX Need to add branchOp */
    if (op == loadConstOp) {
	return(1);
    } else if (op ==  loadOp) {
	// sethi + load single
	return(1+1);
    } else if (op ==  loadIndirOp) {
	return(1);
    } else if (op ==  storeOp) {
	// sethi + store single
	// return(1+3); 
	// for SS-5 ?
	return(1+2); 
    } else if (op ==  storeIndirOp) {
	return(2); 
    } else if (op ==  ifOp) {
	// subcc
	// be
	// nop
	return(1+1+1);
    } else if (op ==  callOp) {
	int count = 0;

	// mov src1, %o0
	count += 1;

	// mov src2, %o1
	count += 1;

	// clr i2
	count += 1;

	// clr i3
	count += 1;

	// sethi
	count += 1;

	// jmpl
	count += 1;

	// noop
	count += 1;

	return(count);
    } else if (op ==  updateCostOp) {
        // sethi %hi(obsCost), %l0
        // ld [%lo + %lo(obsCost)], %l1
        // add %l1, <cost>, %l1
        // st %l1, [%lo + %lo(obsCost)]
        return(1+2+1+3);
    } else if (op ==  trampPreamble) {
	return(0);
    } else if (op ==  trampTrailer) {
	// retl
	return(2);
    } else if (op == noOp) {
	// noop
	return(1);
    } else if (op == getParamOp) {
        return(0);
    } else {
	switch (op) {
	    // rel ops
	    case eqOp:
            case neOp:
	    case lessOp:
            case leOp:
            case greaterOp:
	    case geOp:
	        // bne -- assume taken
	        return(2);
	        break;
	    default:
		return(1);
		break;
	}
    }
}

bool isReturnInsn(const image *owner, Address adr, bool &lastOne)
{
    instruction instr;
    
    instr.raw = owner->get_instruction(adr);
    lastOne = false;

    if (isInsnType(instr, RETmask, RETmatch) ||
        isInsnType(instr, RETLmask, RETLmatch)) {
        if ((instr.resti.simm13 != 8) && (instr.resti.simm13 != 12)) {
	    logLine("*** FATAL Error:");
	    sprintf(errorLine, " unsupported return\n");
	    logLine(errorLine);
	    showErrorCallback(55, "");
	    P_abort();
        }
	return true;
    }
    return false;
}


//
// called to relocate a function: when a request is made to instrument
// a system call we relocate the entire function into the heap
//
bool pd_Function::relocateFunction(process *proc, 
				 const instPoint *&location,
				 vector<instruction> &extra_instrs) {

    relocatedFuncInfo *reloc_info = 0;
    // check to see if this process already has a relocation record 
    // for this function
    for(u_int i=0; i < relocatedByProcess.size(); i++){
	if((relocatedByProcess[i])->getProcess() == proc){
	    reloc_info = relocatedByProcess[i];
	}
    }
    u_int ret = 0;
    if(!reloc_info){
        //Allocate the heap for the function to be relocated
        ret = inferiorMalloc(proc, size()+RELOCATED_FUNC_EXTRA_SPACE, \
			     textHeap);
	if(!ret)  return false;
        reloc_info = new relocatedFuncInfo(proc,ret);
	relocatedByProcess += reloc_info;
    }
    if(!(findNewInstPoints(location->image_ptr, location, ret, proc, 
			   extra_instrs, reloc_info))){
    }
    proc->writeDataSpace((caddr_t)ret, size()+RELOCATED_FUNC_EXTRA_SPACE \
			 ,(caddr_t) newInstr);
    return true;
}

// modifyInstPoint: if the function associated with the process was 
// recently relocated, then the instPoint may have the old pre-relocated
// address (this can occur because we are getting instPoints in mdl routines 
// and passing these to routines that do the instrumentation, it would
// be better to let the routines that do the instrumenting find the points)
void pd_Function::modifyInstPoint(const instPoint *&location,process *proc)
{

    if(relocatable_ && !(location->relocated_)){
        for(u_int i=0; i < relocatedByProcess.size(); i++){
	    if((relocatedByProcess[i])->getProcess() == proc){
		if(location->ipType == functionEntry){
		    const instPoint *new_entry = 
				(relocatedByProcess[i])->funcEntry();
		    location = new_entry;
		} 
		else if(location->ipType == functionExit){
		    const vector<instPoint *> new_returns = 
			(relocatedByProcess[i])->funcReturns(); 
                    if(funcReturns.size() != new_returns.size()){
			printf("funcReturns = %d new_returns = %d\n",
				funcReturns.size(),new_returns.size());
                        fflush(stdout);
		    }
                    assert(funcReturns.size() == new_returns.size());
                    for(u_int j=0; j < new_returns.size(); j++){
			if(funcReturns[j] == location){
			    location = (new_returns[j]);
			    break;
			}
		    }
		}
		else {
		    const vector<instPoint *> new_calls = 
				(relocatedByProcess[i])->funcCallSites(); 
                    assert(calls.size() == new_calls.size());
                    for(u_int j=0; j < new_calls.size(); j++){
			if(calls[j] == location){
			    location = (new_calls[j]);
			    break;
			}
		    }

		}
 		break;
    } } }
}


// The exact semantics of the heap are processor specific.
//
// find all DYNINST symbols that are data symbols
//
bool process::heapIsOk(const vector<sym_data> &find_us) {
  Symbol sym;
  Address baseAddr;

  // find the main function
  // first look for main or _main
  if (!((mainFunction = findOneFunction("main")) 
        || (mainFunction = findOneFunction("_main")))) {
     string msg = "Cannot find main. Exiting.";
     statusLine(msg.string_of());
     showErrorCallback(50, msg);
     return false;
  }

  for (unsigned i=0; i<find_us.size(); i++) {
    const string &str = find_us[i].name;
    if (!getSymbolInfo(str, sym, baseAddr)) {
      string str1 = string("_") + str.string_of();
      if (!getSymbolInfo(str1, sym, baseAddr) && find_us[i].must_find) {
	string msg;
        msg = string("Cannot find ") + str + string(". Exiting");
	statusLine(msg.string_of());
	showErrorCallback(50, msg);
	return false;
      }
    }
  }

//  string ghb = GLOBAL_HEAP_BASE;
//  if (!getSymbolInfo(ghb, sym, baseAddr)) {
//    ghb = U_GLOBAL_HEAP_BASE;
//    if (!linkedFile.get_symbol(ghb, sym)) {
//      string msg;
//      msg = string("Cannot find ") + ghb + string(". Exiting");
//      statusLine(msg.string_of());
//      showErrorCallback(50, msg);
//      return false;
//    }
//  }
//  Address instHeapEnd = sym.addr()+baseAddr;
//  addInternalSymbol(ghb, instHeapEnd);

  string ihb = INFERIOR_HEAP_BASE;
  if (!getSymbolInfo(ihb, sym, baseAddr)) {
    ihb = UINFERIOR_HEAP_BASE;
    if (!getSymbolInfo(ihb, sym, baseAddr)) {
      string msg;
      msg = string("Cannot find ") + ihb + string(". Cannot use this application");
      statusLine(msg.string_of());
      showErrorCallback(50, msg);
      return false;
    }
  }
  Address curr = sym.addr()+baseAddr;

#ifdef ndef
  // Check that we can patch up user code to jump to our base trampolines
  // (Perhaps this code is no longer needed for sparc platforms, since we use full
  // 32-bit jumps)
  const Address instHeapStart = curr;
  const Address instHeapEnd = instHeapStart + SYN_INST_BUF_SIZE - 1;

  if (instHeapEnd > getMaxBranch3Insn()) {
    logLine("*** FATAL ERROR: Program text + data too big for dyninst\n");
    sprintf(errorLine, "    heap starts at %x and ends at %x; maxbranch=%x\n",
	    instHeapStart, instHeapEnd, getMaxBranch3Insn());
    logLine(errorLine);
    return false;
  }
#endif

  return true;
}



// Certain registers (i0-i7 on a SPARC) may be available to be read
// as an operand, but cannot be written.
bool registerSpace::readOnlyRegister(reg reg_number) {
  if ((reg_number < 16) || (reg_number > 23))
      return true;
  else
      return false;
}

bool returnInstance::checkReturnInstance(const vector<Address> &stack, u_int &index) {
    // If false (unsafe) is returned, then 'index' is set to the first unsafe call stack
    // index.

  //    cout << "checkReturnInstance: addr_=" << (void*)addr_ << "; size_=" <<
  //            (void*)size_ << endl;
  //    cout << "instruction sequence is:" << endl;
  //    for (unsigned i=0; i < instSeqSize/4; i ++)
  //       cout << (void*)instructionSeq[i].raw << endl;
  //    cout << endl;
  //    
  //    for (unsigned i=0; i < stack.size(); i++)
  //       cout << (void*)stack[i] << endl;

    for (u_int i=0; i < stack.size(); i++) {
	index = i;

	// Is the following check correct?  Shouldn't the ">" be changed to ">=",
	// and the "<=" be changed to "<" ??? --ari 6/11/97
        if (stack[i] > addr_ && stack[i] <= addr_+size_)
	    return false;
    }

    return true;
}

void returnInstance::installReturnInstance(process *proc) {
    proc->writeTextSpace((caddr_t)addr_, instSeqSize, 
			 (caddr_t) instructionSeq); 
}

void generateBreakPoint(instruction &insn) {
    insn.raw = BREAK_POINT_INSN;
}

void returnInstance::addToReturnWaitingList(Address pc, process *proc) {
    // if there already is a TRAP set at this pc for this process don't
    // generate a trap instruction again...you will get the wrong original
    // instruction if you do a readDataSpace
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
        generateBreakPoint(insnTrap);
        proc->readDataSpace((caddr_t)pc, sizeof(insn), (char *)&insn, true);
        proc->writeTextSpace((caddr_t)pc, sizeof(insnTrap), (caddr_t)&insnTrap);
    }
    else {
    }

    instWaitingList *instW = new instWaitingList(instructionSeq,instSeqSize,
						 addr_,pc,insn,pc,proc);
    instWList += instW;
}


bool doNotOverflow(int value)
{
  // we are assuming that we have 13 bits to store the immediate operand.
  if ( (value <= 16383) && (value >= -16384) ) return(true);
  else return(false);
}

void instWaitingList::cleanUp(process *proc, Address pc) {
    proc->writeTextSpace((caddr_t)pc, sizeof(relocatedInstruction),
		    (caddr_t)&relocatedInstruction);
    proc->writeTextSpace((caddr_t)addr_, instSeqSize, (caddr_t)instructionSeq);
}


// process::replaceFunctionCall
//
// Replace the function call at the given instrumentation point with a call to
// a different function, or with a NOOP.  In order to replace the call with a
// NOOP, pass NULL as the parameter "func."
// Returns true if sucessful, false if not.  Fails if the site is not a call
// site, or if the site has already been instrumented using a base tramp.
bool process::replaceFunctionCall(const instPoint *point,
				  const function_base *func) {
    // Must be a call site
    if (point->ipType != callSite)
	return false;

    // Cannot already be instrumented with a base tramp
    if (baseMap.defines(point))
	return false;

    // Replace the call
    if (func == NULL)
	generateNoOp(this, point->addr);
    else
    	generateCall(this, point->addr,
		     ((function_base *)func)->getAddress(this));

    return true;
}
