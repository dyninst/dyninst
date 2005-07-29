/****************************************************************************/
/****************************************************************************/
/****************************************************************************/

// used for sorting inst points - typecast void *s to instPoint **s, then
//  do {-1, 0, 1} comparison by address....
int sort_inst_points_by_address(const void *arg1, const void *arg2) {
    instPoint * const *a = static_cast<instPoint* const *>(arg1);
    instPoint * const *b = static_cast<instPoint* const *>(arg2);
    if ((*a)->pointAddr() > (*b)->pointAddr()) {
        return 1;
    } else if ((*a)->pointAddr() < (*b)->pointAddr()) {
        return -1;
    }
    return 0;
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/

/*
  First pass, handle cases where general-purpose re-writing needs to be
  done to preserve paradynd/dyninstAPI's assumptions about function
  structure.  
    In current sparc-solaris version, this is ONLY the following cases:
      If a call site and return point overlap and are located directly
       next to eachother, and the return point is located on a 
       restore operation, then a TailCallPA is applied whose footprint
       covers the call and the restore.
       The TailCallPA should rewrite the call; restore, and update the 
       locations of both inst points as necessary.
      If the 2nd instruction in a function is a CALL, then a single nop
       is inserted before the call - to clear out the delay slot of the
       branch which is inserted at the first instruction (for entry point
       instrumentation).
       If the dyninstAPI is seperated from paradyn, or when converting this 
       code to support instrumentation at arbitrary points (instead of assuming
       entry, exit, and call site instrumentation), then the check should be 
       changed from:
         look at 2nd insn, see if its a call
        TO
	 look at all inst points, see if any of them have a call in what
	 corresponds to the delay slot of the instruction which is going to
	 get stomped by a branch/call to tramp....
      If the code has any calls to address (of call) + 8, replace the call with
        a sequence which sets the 07 register to the (original) address....
  Function:
    Attaches LocalAlterations related to general rewrites of function
     (by adding them to LocalAlterationSet p).
    returns boolean value indicating whether it was able to figure out
    sequence of LocalAlterations to apply to perform specified rewrites....
 */
bool int_function::PA_attachGeneralRewrites( const image *owner,
                                            LocalAlterationSet *p, 
                                            Address baseAddress, 
                                            Address firstAddress, 
                                            instruction loadedCode[], 
                                            unsigned /* numInstructions */,
                                            int codeSize ) {

#ifdef DEBUG_PA_INST
    cerr << "int_function::PA_attachGeneralRewrites called" <<endl;
    cerr << " prettyName = " << prettyName() << endl;
#endif

    assert((codeSize % instruction::size()) == 0);

    // Iterate over function instruction by instruction, looking for calls
    // made to set the pc
    for(unsigned i=0; i < codeSize / instruction::size(); i++) {

        //  want CALL %address, NOT CALL %register
        if (isTrueCallInsn(loadedCode[i])) {

	    // figure out destination of call....
	    if (is_set_O7_call(loadedCode[i], codeSize, 
                               i * instruction::size())) {

	        SetO7 *seto7 = new SetO7(this, i * instruction::size());
		p->AddAlteration(seto7);

#ifdef DEBUG_PA_INST
	        cerr << " detected call designed to set 07 register at offset "
		     <<  i * instruction::size() << endl;
#endif

	    } else {

  	        // Check for a call to a location outside of the function, 
	        // where the target of the call is a retl instruction. This 
	        // sequence is used to set the o7 register with the PC.

	        // Get target of call instruction
                Address callAddress = firstAddress + i*instruction::size();
       	        Address callTarget  = callAddress + 
                                      (loadedCode[i].call.disp30 << 2);

                // If call is to location outside of function              
                if ( (callTarget < firstAddress) || 
                     (callTarget > firstAddress + get_size()) ) { 

                  // get target instruction
                  instruction tmpInsn;
                  tmpInsn.raw = owner->get_instruction(callTarget - baseAddress);
                  // If call target instruction is a retl instruction
                  if((tmpInsn.raw & 0xfffff000) == 0x81c3e000) {

                    // Retrieve the instruction in the delay slot of the retl,
                    // so that it can be copied into the relocated function
                    tmpInsn.raw = 
                    owner->get_instruction( (callTarget - baseAddress) + instruction::size() );

                    RetlSetO7 *retlSetO7 = 
                        new RetlSetO7(this, i * instruction::size(), tmpInsn);
                    p->AddAlteration(retlSetO7);

#ifdef DEBUG_PA_INST
                    cerr << " detected call to retl instruction"
                         << " designed to set 07 register at offset " 
                         <<  i * instruction::size() << endl;
#endif
		  }
		}
	    }
        }
    }


    // Special cases for leaf functions
    if (hasNoStackFrame()) {

      // Since we are already relocating the function, add a nop after the
      // return point so that we don't have to use prior instructions to
      // instrument the exit.
      for(unsigned i=0; i < funcReturns.size(); i++) {

        instPoint *point = funcReturns[i];

        if (!JmpNopTC(point->firstInstruction, point->secondInstruction,
                      point->pointAddr() - instruction::size(), this) &&
            !CallRestoreTC(point->firstInstruction, point->secondInstruction)&&
	    !MovCallMovTC(point->firstInstruction, point->secondInstruction)) {

            // Offset to insert nop after
            int offset;
 
            offset = ( point->pointAddr() + baseAddress + 
                       instruction::size() ) - firstAddress;

            // Insert a single nop after the exit to the function
            InsertNops *exitNops = 
                new InsertNops(this, offset, instruction::size());

            p->AddAlteration(exitNops);

#ifdef DEBUG_PA_INST
            cerr << "Function: " << prettyName() << " added a single nop "
                 << " after exit at offset " << offset << "." << endl;
#endif

	}
      }
    }

    return true;
}


bool int_function::PA_attachTailCalls(LocalAlterationSet *p) {
    instruction instr, nexti;
    TailCallOptimization *tail_call;

    if (!call_points_have_been_checked)
      checkCallPoints();
    // previously referred to calls[i] directly, but gdb seems to be having
    //  trouble with templated types with the new compiler - making debugging
    //  difficult - so, directly assign calls[i] to the_call so can use gdb
    //  to get at info about it....
    instPoint *the_call;

#ifdef DEBUG_PA_INST
    cerr << "int_function::PA_tailCallOptimizations called" <<endl;
    cerr << " prettyName = " << prettyName() << endl;
#endif


    // Look for an instPoint in funcCalls where the instruction is 
    //  a call instruction and the next instruction is a restore op
    //  or where the instruction is a jmp (out of function?), and the next 
    //  instruction is a nop....
    for(unsigned i=0; i < calls.size(); i++) {

        // this should return the offset at which the FIRST instruction which
        //  is ACTUALLY OVEWRITTEN BY INST POINT is located....
        the_call = calls[i];
        int offset = (the_call->pointAddr() - getAddress(0));
        instr = the_call->insnAtPoint();
        nexti = the_call->insnAfterPoint();
        if (CallRestoreTC(instr, nexti)) {
           tail_call = new CallRestoreTailCallOptimization(this, offset, 
                                   offset + 2 * instruction::size(), instr);
           p->AddAlteration(tail_call);

#ifdef DEBUG_PA_INST
	    cerr << " detected call-restore tail-call optimization at offset "
		 << offset << endl;
#endif

	}
	else if (MovCallMovTC(instr, nexti)) {
	    tail_call = new MovCallMovTailCallOptimization(
                             this, offset, offset + 2 * instruction::size());
	    p->AddAlteration(tail_call);

#ifdef DEBUG_PA_INST
	    cerr << " detected mov-call-mov tail-call optimization at offset "
		 << offset << endl;
#endif

	}
	else if (JmpNopTC(instr, nexti, the_call->pointAddr(), this)) {
	    tail_call = new JmpNopTailCallOptimization(this, offset, 
                                       offset + 2 * instruction::size());
	    p->AddAlteration(tail_call);

#ifdef DEBUG_PA_INST
	    cerr << " detected jmp-nop tail-call optimization at offset " 
		 << offset << endl;
#endif

	}
    }

    return true;
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/

/* 
  Second pass, handle cases where inst points overlap eachother (and
  thus prevent the normal instrumentation technique from being used).
    In current sparc-solaris version, the following algorithm is used:
     Check all inst points.  If any overlap, figure out the instruction
     that each is trying to get and insert nops so that they no longer
     overlap.  
*/
bool int_function::PA_attachOverlappingInstPoints(
		        LocalAlterationSet *p, Address /* baseAddress */, 
                        Address /* firstAddress */,
	                instruction loadedCode[], int /* codeSize */) {

    instruction instr, nexti;

#ifdef DEBUG_PA_INST
    cerr << "int_function::PA_attachOverlappingInstPoints called" <<endl;
    cerr << " prettyName = " << prettyName() << endl;
#endif

    // Make a list of all inst-points attached to function, and sort
    //  by address.  Then check for overlaps....
    pdvector<instPoint*> foo;    
    sorted_ips_vector(foo);

    // should hopefully have inst points for fn sorted by address....
    // check for overlaps....
    for (unsigned i=0;i<foo.size()-1;i++) {
        instPoint *this_inst_point = foo[i];
        instPoint *next_inst_point = foo[i+1];
        
        if ((this_inst_point->getPointType() == callSite) && 
	          (next_inst_point->getPointType() == functionExit)) {
          instr = this_inst_point->insnAtPoint();
          nexti = this_inst_point->insnAfterPoint();
          if (CallRestoreTC(instr, nexti) || 
	      JmpNopTC(instr, nexti, this_inst_point->pointAddr(), this) ||
	      MovCallMovTC(instr, nexti)) {

             // This tail call optimization will be rewritten, eliminating the
             // overlap, so we don't have to worry here about rewriting this
             // as overlapping instPoints.
             continue;
          }
	}

	// check if inst point overlaps with next inst point....
	int overlap = ((this_inst_point->pointAddr() + 
 	  this_inst_point->Size()) - next_inst_point->pointAddr());
 	if (overlap > 0) {

	    // Inst point overlaps with next one.  Insert 
	    //  InsertNops into PA Set AFTER instruction pointed to
	    //  by inst point (Making sure that this does NOT break up 
	    //  an insn and its delay slot).....
	    // ALRT ALRT : This is NOT designed to handle the case where
	    //  2 inst points are located at exactly the same place or 
	    //  1 is located in the delay slot of the other - it will NOT
	    //  break up the 2 inst points in that case....

 	    int offset = (this_inst_point->insnAddress() - getAddress(0));
            
            if (isDCTI(loadedCode[offset/instruction::size()])) {
              offset += instruction::size();
            } 
 	    InsertNops *nops = new InsertNops(this, offset, overlap);
 	    p->AddAlteration(nops);

#ifdef DEBUG_PA_INST
            cerr << " detected overlapping inst points : offset " << offset <<
 	      " overlap " << overlap << endl;
#endif
	}
    }
    return true;
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/

/*
  Third pass, handle cases where inst points overlap branch/jump targets.
    In current sparc-solaris version, the following algorithm is used:
     Check all branch/jump operations.  If they go into footprint of 
     any inst points, then try to insert nops so that the branch targets
     move outside of the the footprint....
  Note that the algorithm given below should be attached AFTER alterations
   which unwind tail-call optimizations and seperate overlapping inst points
   have been BOTH attached and applied....
 */
bool int_function::PA_attachBranchOverlaps(
			     LocalAlterationSet *p, Address /* baseAddress */, 
			     Address firstAddress, instruction loadedCode[],
                             unsigned /* numberOfInstructions */, 
                             int codeSize)  {

#ifdef DEBUG_PA_INST
    cerr << "int_function::PA_attachBranchOverlaps called" <<endl;
    cerr << " prettyName = " << prettyName() << endl;
#endif

    // Make a list of all inst-points attached to function, and sort
    //  by address.  Then check for overlaps....
    pdvector<instPoint*> foo;
    foo.push_back(funcEntry_);
    VECTOR_APPEND(foo,funcReturns);
    VECTOR_APPEND(foo,calls);

    // Sort inst points list by address....
    VECTOR_SORT(foo, sort_inst_points_by_address);

    // Iterate over function instruction by instruction....
    assert((codeSize % instruction::size()) == 0);
    for(unsigned i=0;i<(codeSize/instruction::size());i++) {
        // looking for branch instructions inside function....
        if (!branchInsideRange(loadedCode[i], 
			       firstAddress + (i * instruction::size()),
			       firstAddress, 
			       firstAddress + get_size())) {
	    continue;
	}
							      
        int disp = loadedCode[i].branch.disp22;
        Address target = firstAddress + (i * instruction::size())
							    + (disp << 2);

	// branch insn inside function.  Check target versus known inst points....
	instPoint *overlap = find_overlap(foo, target);
	if (overlap == NULL) continue;

	if (target <= overlap->pointAddr()) {
	    InsertNops *nops = new InsertNops(this, 
                               (target - firstAddress) - instruction::size(), 
			       target - overlap->firstAddress());
	    p->AddAlteration(nops);

#ifdef DEBUG_PA_INST
          cerr << " detected overlap between branch target and inst point:"
               << " offset "  << target - firstAddress 
               << " # bytes " << target - overlap->firstAddress() << endl;
#endif

	} else {
	    InsertNops *nops = new InsertNops(this, 
                                 (target - firstAddress) - instruction::size(),
				 overlap->followingAddress() - target);
	    p->AddAlteration(nops);

#ifdef DEBUG_PA_INST
	  cerr << " detected overlap between branch target and inst point:"
               << " offset " << (target - firstAddress) - instruction::size()
               << " # bytes " << overlap->firstAddress() - target << endl;
#endif

	}
    }

    return true;
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
#ifdef BPATCH_LIBRARY
bool int_function::PA_attachBasicBlockEndRewrites(LocalAlterationSet *p,
                                                 Address baseAddress,
                                                 Address firstAddress,
                                                 process *proc)
#else
bool int_function::PA_attachBasicBlockEndRewrites(LocalAlterationSet *,
                                                 Address, Address,
                                                 process *)
#endif
{
#ifdef BPATCH_LIBRARY
   //registerNewFunction will return an existing one if it already exists.
   BPatch_function *bpfunc = proc->registerNewFunction(this);

   BPatch_flowGraph *cfg = bpfunc->getCFG();
   BPatch_Set<BPatch_basicBlock*> allBlocks;
   cfg->getAllBasicBlocks(allBlocks);

   BPatch_basicBlock** belements =
      new BPatch_basicBlock*[allBlocks.size()];
   allBlocks.elements(belements);
   
   BPatch_Set<Address> blockEnds;
   for (unsigned i = 0; i < allBlocks.size(); i++) {
      void *bbsa, *bbea;
      belements[i]->getAddressRange(bbsa, bbea);
      blockEnds.insert((Address) bbea);
   }
   delete[] belements;
   
   pdvector<instPoint*> ips;
   sorted_ips_vector(ips);
   
   instPoint *entry = const_cast<instPoint*>(funcEntry_);
   Address entry_begin, entry_end;
   entry_begin = entry->pointAddr();
   entry_end = entry_begin + entry->Size();
   
   for (unsigned i = 0; i < ips.size(); i++) {
      Address curr_addr = ips[i]->pointAddr();
      // Skip inst points inside entry block
      if (entry_begin <= curr_addr && curr_addr < entry_end)
         continue;
      
      // We do a special check to see if we're the restore in a tail-call
      // optimization.  This should be probably be addressed in
      // fixOverlappingAlterations
      if (blockEnds.contains(curr_addr) &&
          !isInsnType(ips[i]->insnAtPoint(), RESTOREmask, RESTOREmatch)) {
         
         InsertNops *blockNop = 
            new InsertNops(this, curr_addr + baseAddress +
                           instruction::size() - firstAddress, 
                           instruction::size());
         p->AddAlteration(blockNop);
      }
   }   
#endif // BPATCH_LIBRARY

   return true;
}



/****************************************************************************/
/****************************************************************************/
/****************************************************************************/

#ifdef NOT_DEFINED
#define UPDATE_ADDR(point) \
    if (point != NULL) { \
        offset = point->Addr() - oldAdr; \
	shift = p->GetInstPointShift();  \
        if (shift != 0) {                \
	    p->SetAddr(p->Addr() = shift); \
	} \
    }
bool int_function::applyAlterationsToInstPoints(LocalAlterationSet *p, 
        relocatedFuncInfo *info, Address oldAdr) {
    instPoint *point;
    int offset, shift, i;    
    if (!call_points_have_been_checked)
      checkCallPoints();
    // try to handle entry point....
    point = info->funcEntry();
    UPDATE_ADDR(point);

    // ditto for calls....
    for(i=0;i<funcCalls().size();i++) {
        point = funcCalls[i];
	UPDATE_ADDR(point);
    }

    // and for exit points....
    for(i=0;i<funcReturns().size();i++) {
        point = funcReturns[i];
	UPDATE_ADDR(point);
    }
}
#endif

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/

// Fill in relocatedFuncInfo <func> with inst points from function.
//  Notes:
//    Assumes that function inst points are : 1 entry, multiple call site, and
//     multiple exit points, and that all should be translated into inst
//     points in relocated function (in reloc_info).  This probably shld get 
//     changed if instrumenting arbitrary points in function....
//    When this function finishes, inst points in reloc info should have their
//     NEW (relocated) address, including changes made by alterations applied
//     to function.
//    3 LocalAlterationSet s are used because that is the number which a relatively
//     straight-forward coding of the logic to find alterations used - other
//     platforms which have different logic for attaching alterations will 
//     probably have more or less than 3 alteration sets....
//    This function preserves the (probably memory leaking) semantics of Tia's original
//     code for filling in reloc_info in findNewInstPoints - it does NOT delete 
//     created inst points on error....
#define CALC_OFFSETS(ip) \
    originalOffset = ((ip->insnAddress() + imageBaseAddr) - mutatee); \
    newOffset = alteration_set.getInstPointShift(originalOffset + 1); \
    assert(((originalOffset + newOffset) % instruction::size()) == 0); \
    arrayOffset = (originalOffset + newOffset) / instruction::size(); \
    tmp = (newAdr + originalOffset + newOffset) - imageBaseAddr; \
    tmp2 = ip->pointAddr();

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/

void int_function::addArbitraryPoint(instPoint* location,
				    process* proc,
				    relocatedFuncInfo* reloc_info)
{
    if(!hasBeenRelocated(proc))
       return;

    instPoint *point;
    int originalOffset, newOffset;

    const image* owner = location->getOwner();

    Address imageBaseAddr;
    if (!proc->getBaseAddress(owner,imageBaseAddr))
        abort();
    originalOffset = (location->pointAddr() - getAddress(NULL));
    assert((originalOffset % instruction::size()) == 0);
    newOffset = originalOffset + reloc_info->getAlterationSet().getShift(originalOffset);
    assert((newOffset % instruction::size()) == 0);
    Address newArrayOffset = newOffset / instruction::size();
    Address newAdr = reloc_info->get_address() + newOffset;
    // the inst point wants this relative the the start of the image....
    newAdr -= imageBaseAddr;
    unsigned int orig_id = location->getID();

    point = new instPoint(orig_id, this, relocatedInstructions, newArrayOffset,
                          newAdr, true, otherPoint);

    reloc_info->addArbitraryPoint(point);
}


bool int_function::fillInRelocInstPoints(
                            const image *owner, process *proc, 
                            instPoint *&location, 
                            relocatedFuncInfo *reloc_info, Address mutatee,
                            Address /* mutator */ ,instruction /* oldCode */[],
                            Address newAdr, instruction newCode[], 
                            LocalAlterationSet &alteration_set) { 

    unsigned retId = 0;
    unsigned callId = 0; 
    unsigned tmpId = 0;
    unsigned arbitraryId = 0;
    int originalOffset, newOffset, arrayOffset;
    instPoint *point;
    Address tmp, tmp2;
    if (!call_points_have_been_checked)
      checkCallPoints();

    assert(reloc_info);

#ifdef DEBUG_PA_INST
    cerr << "int_function::fillInRelocInstPoints called" <<endl;
    cerr << " mutatee = " << mutatee << " newAdr = " << newAdr << endl;
#endif
 
    Address imageBaseAddr;
    if (!proc->getBaseAddress(owner, imageBaseAddr))
        abort();

    alteration_set.Collapse();

    //  Add inst point corresponding to func entry....
    //   Assumes function has single entry point  
    if (funcEntry_ != NULL) {

        //  figure out how far entry inst point is from beginning of function
        originalOffset = ((funcEntry_->pointAddr() + imageBaseAddr) - 
                                                              mutatee);
        arrayOffset = originalOffset / instruction::size();
        tmp         = (newAdr + originalOffset) - imageBaseAddr;
        tmp2        = funcEntry_->pointAddr();

        unsigned int orig_id = funcEntry_->getID();
        point = new instPoint(orig_id, this, newCode, arrayOffset,
                              tmp, true, functionEntry);

#ifdef DEBUG_PA_INST
        cerr << " added entry point at originalOffset " << originalOffset
	     << " newOffset " << originalOffset << endl;
#endif

	assert(point != NULL);

	if (location == funcEntry_) {
	    location = point;
	}

	reloc_info->addFuncEntry(point);
        assert(reloc_info->funcEntry());
    }

    // Add inst points corresponding to func exits....
    for(retId=0;retId < funcReturns.size(); retId++) {
        CALC_OFFSETS(funcReturns[retId])
        unsigned int orig_id = funcReturns[retId]->getID();
        point = new instPoint(orig_id, this, newCode, arrayOffset,
                              tmp, false, functionExit);
#ifdef DEBUG_PA_INST
        cerr << " added return point at originalOffset " << originalOffset
	     << " newOffset " << newOffset << endl;
#endif
        assert(point != NULL);

	if (location == funcReturns[retId]) {
	    location = point;
	}
	reloc_info->addFuncReturn(point);
    } 

    // Add inst points corresponding to func call sites....
    for(callId=0;callId<calls.size();callId++) {
        CALC_OFFSETS(calls[callId])
        tmpId = callId;
        unsigned int orig_id = calls[callId]->getID();
        point = new instPoint(orig_id, this, newCode, arrayOffset, tmp,
                              false, callSite);
        addCallPoint(newCode[arrayOffset], tmpId, reloc_info, point, 
                     const_cast<const instPoint *&> (location));

#ifdef DEBUG_PA_INST
        cerr << " added call site at originalOffset " << originalOffset
	     << " newOffset " << newOffset << endl;
#endif

    }

    for(arbitraryId=0;arbitraryId<arbitraryPoints.size();arbitraryId++){
       CALC_OFFSETS(arbitraryPoints[arbitraryId]);
       unsigned int orig_id = arbitraryPoints[arbitraryId]->getID();
       point = new instPoint(orig_id, this, newCode, arrayOffset,
                             tmp, true, otherPoint);
       
       assert(point != NULL);
       
       if (location == arbitraryPoints[arbitraryId]) 
          location = point;
       
       reloc_info->addArbitraryPoint(point);
    }

    return true;    
}

