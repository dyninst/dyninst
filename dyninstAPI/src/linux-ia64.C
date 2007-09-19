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

/* -*- Mode: C; indent-tabs-mode: true; tab-width: 4 -*- */

#include <stdio.h>

// debugging... is there a Paradyn Standard for this?
#define TRACE(expr)     fprintf( stderr, "At line %d of %s in %s: %s.\n", __LINE__, __PRETTY_FUNCTION__,  __FILE__, __STRING(expr) );

#include "linux.h"
#include "inst-ia64.h"
#include "process.h"
#include "dyn_lwp.h"
#include "debuggerinterface.h"
#include "function.h"
#include "signalgenerator.h"
#include "mapped_object.h"

// For relocationEntry
#include "symtab.h"

/* For emitInferiorRPC*(). */
#include "rpcMgr.h"

#include "ast.h"
#include "registerSpace.h"

#include <asm/ptrace_offsets.h>
#include <dlfcn.h>

bool dyn_lwp::changePC( Address loc, dyn_saved_regs * regs ) {
  if( regs != NULL ) { restoreRegisters( *regs ); }

  /* We can't change the PC to a slot number other than 0. */  
  assert( loc % 16 == 0 );

  // /* DEBUG */ fprintf( stderr, "%s[%d]: (lwp %d) changing PC to 0x%lx\n", __FILE__, __LINE__, get_lwp_id(), loc );
  return (getDBI()->ptrace( PTRACE_POKEUSER, get_lwp_id(), PT_CR_IIP, loc, -1, & errno ) != -1);
} /* end changePC() */

/* This should really be printRegisters().  I also have to admit
   to some puzzlement as to the type of the parameter... */
void printRegs( void * /* save */ ) {
  assert( 0 );
} /* end printRegs() */

/* dyn_lwp::getRegisters()
 * 
 * Entire user state can be described by struct pt_regs
 * and struct switch_stack.  It's tempting to try and use
 * pt_regs only, but only syscalls are that well behaved.
 * We must support running arbitrary code.
 */
bool dyn_lwp::getRegisters_( struct dyn_saved_regs *regs, bool includeFP ) {
	assert( status_ != running );

	errno = 0;
	regs->pc = getDBI()->ptrace( PTRACE_PEEKUSER, get_lwp_id(), PT_CR_IIP, 0, -1, & errno );
   if (errno != 0)
      return false;

	/* If the PC may have rewound, handle it intelligently. */
	needToHandleSyscall( this, & regs->pcMayHaveRewound );
	
	/* We use the basetramp preservation code in our inferior RPCs, so we
	   don't have to preserve anything.  The only exception is the predicate
	   registers, because we may need them to handle system calls correctly.
	   (We predicate break instructions based on if the kernel attempted to
	   restart the system call.) */
	regs->pr = getDBI()->ptrace( PTRACE_PEEKUSER, get_lwp_id(), PT_PR, 0, -1, & errno );
   if (errno != 0)
      return false;

	return true;
} /* end getRegisters_() */

bool dyn_lwp::restoreRegisters_( const struct dyn_saved_regs &regs, bool includeFP ) {
  /* Restore the PC. */
  if( ! regs.pcMayHaveRewound ) {
    changePC( regs.pc, NULL );
  }
  else {
    /* The flag could have only been set if the ipsr.ri at
       construction-(and therefore run-)time ispr.ri was 0.  If it's 0
       after the syscall trailer completes, then the original regs->pc
       is correct.  If it's 2, then the PC rewound and we adjust
       adjust regs->pc appropriately.  No other cases are possible. */
    errno = 0;
    uint64_t ipsr = getDBI()->ptrace( PTRACE_PEEKUSER, get_lwp_id(), PT_CR_IPSR, 0, -1, & errno );
    if (errno != 0)
       return false;

	// /* DEBUG */ fprintf( stderr, "%s[%d]: pcMayHaveRewound.\n", FILE__, __LINE__ );

    uint64_t ipsr_ri = (ipsr & 0x0000060000000000) >> 41;
    assert( ipsr_ri <= 2 );
		
    switch( ipsr_ri ) {
    case 0:
	  // /* DEBUG */ fprintf( stderr, "%s[%d]: PC did not rewind.\n", FILE__, __LINE__ );
      changePC( regs.pc, NULL );
      break;
    case 1:
      assert( 0 );
      break;
    case 2:
	  /* DEBUG */ fprintf( stderr, "%s[%d]: PC rewound.\n", FILE__, __LINE__ );
      changePC( regs.pc - 0x10, NULL );
      break;
    default:
      assert( 0 );
      break;
    } /* end ispr_ri switch */
  } /* end if pcMayHaveRewound */

  /* Restore the predicate registers. */
  int status = getDBI()->ptrace( PTRACE_POKEUSER, get_lwp_id(), PT_PR, regs.pr );
  if (status != 0)
     return false;

  return true;
} /* end restoreRegisters_() */

void dyn_lwp::dumpRegisters()
{
   dyn_saved_regs regs;
   if (!getRegisters(&regs)) {
     fprintf(stderr, "%s[%d]:  registers unavailable\n", FILE__, __LINE__);
     return;
   }

   fprintf(stderr, "pc:   %lx\n", regs.pc);
   fprintf(stderr, "pr:   %lx\n", regs.pr);
}

bool process::handleTrapAtEntryPointOfMain(dyn_lwp * /*dontcare*/) {
  InsnAddr iAddr = InsnAddr::generateFromAlignedDataAddress( main_brk_addr, this );
  iAddr.writeMyBundleFrom( savedCodeBuffer );
    
  main_brk_addr = 0;
  return true;
} /* end handleTrapAtEntryPointOfMain() */

bool process::insertTrapAtEntryPointOfMain() {
  int_function *f_main = 0;
  pdvector<int_function *> funcs;
    
  //first check a.out for function symbol   
  bool res = findFuncsByPretty("main", funcs);
  if (!res)
    {
      logLine( "a.out has no main function. checking for PLT entry\n" );
      //we have not found a "main" check if we have a plt entry
      res = findFuncsByPretty( "DYNINST_pltMain", funcs );
 
      if (!res) {
	logLine( "no PLT entry for main found\n" );
	return false;
      }       
    }
    
  if( funcs.size() > 1 ) {
    cerr << __FILE__ << __LINE__ 
	 << ": found more than one main! using the first" << endl;
  }
  f_main = funcs[0];
  assert(f_main);
  Address addr = f_main->getAddress();

  InsnAddr iAddr = InsnAddr::generateFromAlignedDataAddress( addr, this );
  iAddr.saveMyBundleTo( savedCodeBuffer );
  iAddr.replaceBundleWith( generateTrapBundle() );
    
  main_brk_addr = addr;
  return true;
} /* end insertTrapAtEntryPointOfMain() */


bool process::handleTrapAtLibcStartMain(dyn_lwp *)  { assert(0); return false; }
bool process::instrumentLibcStartMain() { assert(0); return false; }
bool process::decodeStartupSysCalls(EventRecord &) { assert(0); return false; }
void process::setTraceSysCalls(bool) { assert(0); }
void process::setTraceState(traceState_t) { assert(0); }
bool process::getSysCallParameters(dyn_saved_regs *, long *, int) { assert(0); return false; }
int process::getSysCallNumber(dyn_saved_regs *) { assert(0); return 0; }
long process::getSysCallReturnValue(dyn_saved_regs *) { assert(0); return 0; }
Address process::getSysCallProgramCounter(dyn_saved_regs *) { assert(0); return 0; }
bool process::isMmapSysCall(int) { assert(0); return false; }
Offset process::getMmapLength(int, dyn_saved_regs *) { assert(0); return 0;}
Address process::getLibcStartMainParam(dyn_lwp *) { assert(0); return 0;}


#define BIT_8_3		0x1F8

/* private refactoring function; account for the RNAT slots. */
Address calculateRSEOffsetFromBySlots( Address addr, int slots ) {
  /* Whenever bits 8:3 of BSPSTORE are all ones, the RSE stores 64 RNAT bits.
     We'll just do this in the stupidest possible way. */

  if( slots == 0 ) { return addr; }
  int adjust = slots < 0 ? -1 : 1;

  for( int i = 0; i < abs( slots ); i++ ) {
    addr += adjust * 8;
    if( (addr & BIT_8_3) == BIT_8_3 ) {
      addr += adjust * 8;
    } /* end if we ran into a NaT collection */
  } /* end iteration over addresses */

  return addr;
} /* end calculateRSEOffsetFromBySlots() */

Address dyn_lwp::readRegister( Register reg ) {
  /* I can't find anything in the docs saying that ptrace()d
     programs will always have a flushrs executed in their
     context before the debugger gains control, but GDB seems
     to think that this is the case, and we can't do anything
     else to read registers anyway. */

  /* Acquire the BSP. */
  errno = 0;
  Address bsp = getDBI()->ptrace( PTRACE_PEEKUSER, get_lwp_id(), PT_AR_BSP, 0, -1, & errno );
  assert( ! errno );

  /* Acquire the CFM. */
  reg_tmpl tmpl = { getDBI()->ptrace( PTRACE_PEEKUSER, get_lwp_id(), PT_CFM, 0, -1, & errno ) };
  assert( ! errno );

  /* Calculate the address of the register. */
  Address addressOfReg = calculateRSEOffsetFromBySlots( bsp, (-1 * tmpl.CFM.sof) + (reg - 32) );

  /* Acquire and return the value of the register. */
  Address value = getDBI()->ptrace( PTRACE_PEEKTEXT, get_lwp_id(), addressOfReg, 0, -1, & errno );
  assert( ! errno );
  return value;
} /* end readRegister */

#include <libunwind.h>
#include <libunwind-ptrace.h>

/* Refactored from getActiveFrame() and getCallerFrame(). */
Frame createFrameFromUnwindCursor( unw_cursor_t * unwindCursor, dyn_lwp * dynLWP, pid_t pid ) {
  Address ip = 0, sp = 0, fp = 0, tp = 0;
  bool isUppermost = false, isSignalFrame = false, isTrampoline = false;
  int status = 0;

  /* Use the unwind cursor to fill in the frame. */
  status = getDBI()->getFrameRegister( unwindCursor, UNW_IA64_IP, &ip );
  assert( status == 0 );
  status = getDBI()->getFrameRegister( unwindCursor, UNW_IA64_SP, &sp );
  assert( status == 0 );
  status = getDBI()->getFrameRegister( unwindCursor, UNW_IA64_TP, &tp );
  assert( status == 0 );
	
  /* Unfortunately, libunwind is a little _too_ helpful:
     it'll encode the slotNo into the ip.  Take it back
     out, since we don't rely on it, and it confuses the
     rest of Dyninst. */
  ip = ip - (ip % 16);
	
  status = getDBI()->isSignalFrame( unwindCursor );
  if( status > 0 ) { isSignalFrame = true; }

  /* Determine if this is a trampoline frame. */
  codeRange * range = dynLWP->proc()->findOrigByAddr( ip );

  /* We assume here, and below, that the two stackwalking errors before
     bootstrapping are from loading the run-time library. */
  if( range == NULL && dynLWP->proc()->reachedBootstrapState( bootstrapped_bs ) ) {
    /* We don't maintain the auxv page in the code range tree, so manually
       check if we're in it. */
    dynLWP->proc()->readAuxvInfo();
    if( !( dynLWP->proc()->getVsyscallStart() <= ip && ip < dynLWP->proc()->getVsyscallEnd() )) {
      /* DEBUG */ fprintf( stderr, "%s[%d]: warning: no code range found for ip 0x%lx\n", __FILE__, __LINE__, ip );
    }
  }
  else {
    if( range->is_minitramp() != NULL || range->is_multitramp() != NULL ) {
      isTrampoline = true;
    }	
    if( range->is_function() != NULL && range->is_function()->symTabName() == "__libc_start_main" ) {
      isUppermost = true;
    }
    if( range->is_inferior_rpc() != NULL ) {
      isUppermost = true;
    }
  }

  /* Duplicate the current frame's cursor, since we'll be unwinding past it. */
  unw_cursor_t currentFrameCursor = * unwindCursor;
  status = getDBI()->stepFrameUp( unwindCursor );
  
	
  if( status < 0 ) {
    if( range->is_inferior_rpc() != NULL ) {
      // /* DEBUG */ fprintf( stderr, "%s[%d]: ignoring stackwalk failure in inferior RPC at 0x%lx.\n", __FILE__, __LINE__, ip );
    }
    else if( isUppermost ) {
      /* DEBUG */ fprintf( stderr, "%s[%d]: ignoring stackwalk failure because frame at 0x%lx is a priori uppermost.\n", __FILE__, __LINE__, ip );
    }
    else if( ! dynLWP->proc()->reachedBootstrapState( bootstrapped_bs ) ) {
      // /* DEBUG */ fprintf( stderr, "%s[%d]: ignoring stackwalk failure at 0x%lx because bootstrap is incomplete.\n", __FILE__, __LINE__, ip );
    }
    else {
      /* Should we supress this warning? */
       /*      fprintf( stderr, "%s[%d]: warning: failed to walk stack from address 0x%lx.\n", __FILE__, __LINE__, ip );*/
    }
  }
	
  if( status <= 0 ) {
    isUppermost = true;
  }
		
  /* Since the rest of dyninst ignores isUppermost for some reason,
     fake it by setting the frame pointer to 0. */
  if( ! isUppermost ) {
    /* Set the frame pointer. */
    status = getDBI()->getFrameRegister( unwindCursor, UNW_IA64_SP, & fp );
    assert( status == 0 );
  }
	
  /* FIXME: multithread implementation. */
  dyn_thread * dynThread = NULL;

  Frame currentFrame( ip, fp, sp, pid, dynLWP->proc(), dynThread, dynLWP, isUppermost );
  currentFrame.setUnwindCursor( currentFrameCursor );
  currentFrame.setRange( range );
	
  if( isTrampoline ) {
    currentFrame.frameType_ = FRAME_instrumentation;
  }
  else if( isSignalFrame ) {
    currentFrame.frameType_ = FRAME_signalhandler;
  }
  else {
    currentFrame.frameType_ = FRAME_normal;
  }
	
  return currentFrame;
} /* end createFrameFromUnwindCursor() */

Frame dyn_lwp::getActiveFrame() {
	int status = 0;
	process * proc = proc_;

	assert( status_ != running );

	// /* DEBUG */ fprintf( stderr, "%s[%d]: getActiveFrame(): working on lwp %d\n", __FILE__, __LINE__, get_lwp_id() );

	/* Initialize the unwinder. */
	if( proc->unwindAddressSpace == NULL ) {
		// /* DEBUG */ fprintf( stderr, "getActiveFrame(): Creating unwind address space for process pid %d\n", proc->getPid() );
		proc->unwindAddressSpace = (unw_addr_space *) getDBI()->createUnwindAddressSpace( & _UPT_accessors, 0 );
		assert( proc->unwindAddressSpace != NULL );
		}
    
	/* Initialize the thread-specific accessors. */
	unsigned lid = get_lwp_id();
	if( ! proc->unwindProcessArgs.defines( lid ) ) {
		proc->unwindProcessArgs[ lid ] = getDBI()->UPTcreate( lid );
		assert( proc->unwindProcessArgs[ lid ] != NULL );
		}

	/* Allocate an unwindCursor for this stackwalk. */
	unw_cursor_t * unwindCursor = (unw_cursor_t *)malloc( sizeof( unw_cursor_t ) );
	assert( unwindCursor != NULL );
		
	/* Initialize it to the active frame. */
	status = getDBI()->initFrame( unwindCursor, proc->unwindAddressSpace, proc->unwindProcessArgs[ lid ] );
	assert( status == 0 );
	
	/* Generate a Frame from the unwinder. */
	Frame currentFrame = createFrameFromUnwindCursor( unwindCursor, this, lid );
	
	/* createFrameFromUnwindCursor() copies the unwind cursor into the Frame it returns */
	free( unwindCursor );
	
	/* Return the result. */
	return currentFrame;
	} /* end getActiveFrame() */

#define DLOPEN_MODE		(RTLD_NOW | RTLD_GLOBAL)
#define DLOPEN_CALL_LENGTH	4

/* Defined in process.C; not sure why it isn't in a header. */
extern unsigned enable_pd_attach_detach_debug;

/* DEBUG code */
void printBinary( unsigned long long word, int start = 0, int end = 63 ) {
  for( int i = start; i <= end; i++ ) {
    if( i % 4 == 0 ) { fprintf( stderr,"\t" ); }
    if( ( word << i ) & 0x8000000000000000 ) {
      fprintf( stderr, "1" ); } else {
      fprintf( stderr, "0" );  
    }
  }
} /* end printBinary() */

/* FIXME: this almost certainly NOT the Right Place to keep this. */
Address savedPC;

bool process::getDyninstRTLibName() {
  if (dyninstRT_name.length() == 0) {
    // Get env variable
    if (getenv("DYNINSTAPI_RT_LIB") != NULL) {
      dyninstRT_name = getenv("DYNINSTAPI_RT_LIB");
    }
    else {
      pdstring msg = pdstring( "Environment variable " + pdstring( "DYNINSTAPI_RT_LIB" )
			       + " has not been defined for process " ) + pdstring( getPid() );
      showErrorCallback(101, msg);
      return false;
    }
  }
  // Check to see if the library given exists.
  if (access(dyninstRT_name.c_str(), R_OK)) {
    pdstring msg = pdstring("Runtime library ") + dyninstRT_name
      + pdstring(" does not exist or cannot be accessed!");
    showErrorCallback(101, msg);
    return false;
  }
  return true;
}

bool process::loadDYNINSTlib() {
  /* Look for a function we can hijack to forcibly load dyninstapi_rt. 
     This is effectively an inferior RPC with the caveat that we're
     overwriting code instead of allocating memory from the RT heap. 
     (So 'hijack' doesn't mean quite what you might think.) */
  Address codeBase = findFunctionToHijack(this);	

  if( !codeBase ) { return false; }
  
  /* glibc 2.3.4 and higher adds a fourth parameter to _dl_open().
     While we could probably get away with treating the three and four
     -argument functions the same, check the version anyway, since
     we'll probably need to later. */
  bool useFourArguments = true;
  Symbol libcVersionSymbol;
  if( getSymbolInfo( "__libc_version", libcVersionSymbol ) ) {
    char libcVersion[ sizeof( int ) * libcVersionSymbol.getSize() + 1 ];
	libcVersion[ sizeof( int ) * libcVersionSymbol.getSize() ] = '\0';
    if( ! readDataSpace( (void *) libcVersionSymbol.getAddr(), libcVersionSymbol.getSize(), libcVersion, true ) ) {
      fprintf( stderr, "%s[%d]: warning, failed to read libc version, assuming 2.3.4+\n", __FILE__, __LINE__ );
      }
    else {
      startup_printf( "%s[%d]: libcVersion: %s\n", __FILE__, __LINE__, libcVersion );

      /* We could potentially add a sanity check here to make sure we're looking at 2.3.x. */
      int microVersion = ((int)libcVersion[4]) - ((int)'0');
      if( microVersion <= 3 ) {
	    useFourArguments = false;
        }
      } /* end if we read the version symbol */
    } /* end if we found the version symbol */

  if( useFourArguments ) { startup_printf( "%s[%d]: using four arguments.\n", __FILE__, __LINE__ ); }

  /* Fetch the name of the run-time library. */
  const char DyninstEnvVar[]="DYNINSTAPI_RT_LIB";
        
  if( ! dyninstRT_name.length() ) { // we didn't get anything on the command line
    if (getenv(DyninstEnvVar) != NULL) {
      dyninstRT_name = getenv(DyninstEnvVar);
    } else {
      pdstring msg = pdstring( "Environment variable " + pdstring( DyninstEnvVar )
			       + " has not been defined for process " ) + pdstring( getPid() );
      showErrorCallback(101, msg);
      return false;
    } /* end if enviromental variable not found */
  } /* end enviromental variable extraction */
        
  /* Save the (main thread's) current PC.*/
  savedPC = getRepresentativeLWP()->getActiveFrame().getPC();	
        
  /* _dl_open() takes three arguments: a pointer to the library name,
     the DLOPEN_MODE, and the return address of the current frame
     (that is, the location of the SIGILL-generating bundle we'll use
     to handleIfDueToDyninstLib()).  We construct the first here. */

  /* Write the string to entry, and then move the PC to the next bundle. */
  codeGen gen(BYTES_TO_SAVE);
  gen.setAddrSpace(this);
  gen.setAddr(codeBase);

  gen.setRegisterSpace(registerSpace::savedRegSpace(this));
  
  Address dyninstlib_addr = gen.used() + codeBase;
  gen.copy(dyninstRT_name.c_str(), dyninstRT_name.length()+1);
        
  Address dlopencall_addr = gen.used() + codeBase;
	
  /* At this point, we use the generic iRPC headers and trailers
     around the call to _dl_open.  (Note that pre-1.35 versions
     of this file had a simpler mechanism well-suited to boot-
     strapping a new port.  The current complexity is to handle
     the attach() case, where we don't know if execution was stopped
     at the entry the entry point to a function. */

  bool ok = theRpcMgr->emitInferiorRPCheader(gen);
  if( ! ok ) { return false; }
	
  /* Generate the call to _dl_open with a large dummy constant as the
     the third argument to make sure we generate the same size code the second
     time around, with the correct "return address." (dyninstlib_brk_addr) */
  // As a quick note, we want to "return" to the beginning of the restore
  // segment, not dyninstlib_brk_addr (or we skip all the restores).
  // Of course, we're not sure what this addr represents....

  pdvector< AstNodePtr > dlOpenArguments;
  AstNodePtr dlOpenCall;
	
  dlOpenArguments.push_back(AstNode::operandNode(AstNode::Constant, (void *)dyninstlib_addr));
  dlOpenArguments.push_back(AstNode::operandNode(AstNode::Constant, (void *)DLOPEN_MODE ));
  dlOpenArguments.push_back(AstNode::operandNode(AstNode::Constant, (void *)0xFFFFFFFFFFFFFFFF ));
  if( useFourArguments ) { 
  	/* I derived the -2 as follows: from dlfcn/dlopen.c in the glibc sources, line 59,
  	   we find the call to _dl_open(), whose last argument is 'args->file == NULL ? LM_ID_BASE : NS'.
  	   Since the filename we pass in is non-null, this means we (would) pass in NS, which
  	   is defined to be __LM_ID_CALLER in the same file, line 48.  (Since glibc must be shared
  	   for us to be calling _dl_open(), we fall into the second case of the #ifdef.)  __LM_ID_CALLER
  	   is defined in include/dlfcn.h, where it has the value -2. */
      dlOpenArguments.push_back(AstNode::operandNode( AstNode::Constant, (void *)(long unsigned int)-2 ));
    }
  
  dlOpenCall = AstNode::funcCallNode( "_dl_open", dlOpenArguments );
	
  /* Remember where we originally generated the call. */
  codeBufIndex_t index = gen.getIndex();
	
  /* emitInferiorRPCheader() configures (the global) registerSpace for us. */
  dlOpenCall->generateCode( gen, true );

  // Okay, we're done with the generation, and we know where we'll be.
  // Go back and regenerate it
  Address dlopenRet = codeBase + gen.used();
  gen.setIndex(index);

  /* Clean up the reference counts before regenerating. */
	
  dlOpenArguments[ 2 ] = AstNode::operandNode( AstNode::Constant, (void *)dlopenRet );
  dlOpenCall = AstNode::funcCallNode( "_dl_open", dlOpenArguments );
	
  /* Regenerate the call at the same original location with the correct constants. */
  dlOpenCall->generateCode( gen, true );

  // Okay, that was fun. Now restore. And trap. And stuff.
        
	
  unsigned breakOffset, resultOffset, justAfterResultOffset;
  ok = theRpcMgr->emitInferiorRPCtrailer(gen, breakOffset, false, 
					 resultOffset, justAfterResultOffset );
  if( ! ok ) { return false; }					 

  /* Let everyone else know that we're expecting a SIGILL. */
  dyninstlib_brk_addr = codeBase + breakOffset;

  assert(gen.used() < BYTES_TO_SAVE);

  /* Save the function we're going to hijack. */
  InsnAddr iAddr = InsnAddr::generateFromAlignedDataAddress( codeBase, this );
  /* We need to save the whole buffer, because we don't know how big gen is
     when we do the restore.  This could be made more efficient by storing
     gen.used() somewhere. */
  iAddr.saveBundlesTo( savedCodeBuffer, sizeof( savedCodeBuffer ) / 16 );

  /* Write the call into the mutatee. */
  InsnAddr jAddr = InsnAddr::generateFromAlignedDataAddress( codeBase, this );
  jAddr.writeBundlesFrom( (unsigned char *)gen.start_ptr(), gen.used() / 16 );

  /* Now that we know where the code will start, move the (main thread's) PC there. */
  getRepresentativeLWP()->changePC( dlopencall_addr, NULL );

  /* Let them know we're working on it. */
  setBootstrapState( loadingRT_bs );

  return true;
} /* end dlopenDYNINSTlib() */

bool process::loadDYNINSTlibCleanup(dyn_lwp * /*ignored*/) {
  /* We function did we hijack? */
  Address entry = findFunctionToHijack(this);	// We can avoid using InsnAddr because we know 
  // that function entry points are aligned.
  if( !entry ) { assert( 0 ); }
	
  /* Restore the function we hijacked. */
  InsnAddr iAddr = InsnAddr::generateFromAlignedDataAddress( entry, this );
  iAddr.writeBundlesFrom( savedCodeBuffer, sizeof(savedCodeBuffer) / 16 );

  /* Continue (the main thread's) execution at the correct point. */
  getRepresentativeLWP()->changePC( savedPC, NULL );

  return true;
} /* end loadDYNINSTlibCleanup() */

bool Frame::setPC( Address addr ) {
  /* setPC() should only be called on a frame from a stackwalk.
     If it isn't, we can duplicate the code below in getCallerFrame()
     in order to create the necessary unwindCursor. */
  assert( this->hasValidCursor );
    
  /* Sanity-checking: ensure that the frame libunwind is setting is the same frame we are. */
  Address ip;
  getDBI()->getFrameRegister( & this->unwindCursor, UNW_IA64_IP, &ip );
  assert( ip == pc_ );
    
  /* Update the PC in the remote process. */
  int status = getDBI()->setFrameRegister( & this->unwindCursor, UNW_IA64_IP, addr );
  if( status != 0 ) {
    fprintf( stderr, "Unable to set frame's PC: libunwind error %d\n", status );
    return false;
  }
    
  /* Remember that we've done so. */
  pc_ = addr;
  range_ = NULL; // This has changed.
  return true;
} /* end Frame::setPC() */

#include <miniTramp.h>
#include <baseTramp.h>	
#include <instPoint.h>
Frame Frame::getCallerFrame() {
	int status = 0;	
	assert( lwp_->status() != running );

	/* Initialize the unwinder. */
	if( getProc()->unwindAddressSpace == NULL ) {
		// /* DEBUG */ fprintf( stderr, "Creating unwind address space for process pid %d\n", proc->getPid() );
		getProc()->unwindAddressSpace = (unw_addr_space *)getDBI()->createUnwindAddressSpace( & _UPT_accessors, 0 );
		assert( getProc()->unwindAddressSpace != NULL );
		}
	
	/* Initialize the thread-specific accessors. */
	unsigned lid = lwp_->get_lwp_id();
	if( ! getProc()->unwindProcessArgs.defines( lid ) ) {
		getProc()->unwindProcessArgs[ lid ] = getDBI()->UPTcreate( lid );
		assert( getProc()->unwindProcessArgs[ lid ] != NULL );
		}
		
	/* Generating the synthetic frame above the instrumentation is in cross-platform code. */

	Frame currentFrame;
	if( ! this->hasValidCursor ) {
		/* DEBUG */ fprintf( stderr, "%s[%d]: no valid cursor in frame, regenerating.\n", __FILE__, __LINE__ );

		/* Allocate an unwindCursor for this stackwalk. */
		unw_cursor_t * unwindCursor = (unw_cursor_t *)malloc( sizeof( unw_cursor_t ) );
		assert( unwindCursor != NULL );

		/* Initialize it to the active frame. */
		status = getDBI()->initFrame( unwindCursor, getProc()->unwindAddressSpace, getProc()->unwindProcessArgs[ lid ] );
		assert( status == 0 );

		/* Unwind to the current frame. */
		currentFrame = createFrameFromUnwindCursor( unwindCursor, lwp_, lid );
		while( ! currentFrame.isUppermost() ) {
			if( getFP() == currentFrame.getFP() && getSP() == currentFrame.getSP() && getPC() == currentFrame.getPC() ) {
				currentFrame = createFrameFromUnwindCursor( unwindCursor, lwp_, lid );
				break;
				} /* end if we've found this frame */
			currentFrame = createFrameFromUnwindCursor( unwindCursor, lwp_, lid );
			}
			
		/* createFrameFromUnwindCursor() copies the unwind cursor into the Frame it returns. */
		free( unwindCursor );	
		} /* end if this frame was copied before being unwound. */
	else {
		/* Don't try to walk off the end of the stack. */
		assert( ! this->uppermost_ );

		/* Allocate an unwindCursor for this stackwalk. */
		unw_cursor_t * unwindCursor = (unw_cursor_t *)malloc( sizeof( unw_cursor_t ) );
		assert( unwindCursor != NULL );

		/* Initialize it to this frame. */
		* unwindCursor = this->unwindCursor;

		/* Unwind the cursor to the caller's frame. */
		int status = getDBI()->stepFrameUp( unwindCursor );
		
		/* We unwound from this frame once before to get its FP. */
		assert( status > 0 );

		/* Create a Frame from the unwound cursor. */
		currentFrame = createFrameFromUnwindCursor( unwindCursor, lwp_, lid );
		
		/* createFrameFromUnwindCursor() copies the unwind cursor into the Frame it returns. */
		free( unwindCursor );	
	} /* end if this frame was _not_ copied before being unwound. */
	
	/* Make sure we made progress. */	
	if( getFP() == currentFrame.getFP() && getSP() == currentFrame.getSP() && getPC() == currentFrame.getPC() ) {	
		/* This will forcibly terminate the stack walk. */
		currentFrame.fp_ = (Address)NULL;
		currentFrame.pc_ = (Address)NULL;
		currentFrame.sp_ = (Address)NULL;
		currentFrame.uppermost_ = false;

		fprintf( stderr, "%s[%d]: detected duplicate stack frame, aborting stack with zeroed frame.\n", __FILE__, __LINE__ );
		}

	if( thread_ != NULL ) {
		currentFrame.thread_ = thread_;
		}
                    
	/* Return the result. */
	return currentFrame;
	} /* end getCallerFrame() */


/* Required by linux.C */
bool process::hasBeenBound( const relocationEntry &entry, int_function * & target_pdf, Address base_addr ) {
  /* A PLT entry always looks up a function descriptor in the FD table in the .IA_64.pltoff section; if
     the function hasn't been bound yet, that FD's function pointer will point to another PLT entry.
     (Which will jump to the first, special PLT entry that calls the linker.) 
	   
     The relocation entry points directly to the descriptor, so only a single indirection is necessary. */
	   
  Address gotAddress = entry.rel_addr() + base_addr;
  assert( gotAddress % 16 == 0 );
  // /* DEBUG */ fprintf( stderr, "hasBeenBound(): checking entry at 0x%lx\n", gotAddress );

  Address functionAddress;
  if( ! this->readDataSpace( (const void *)gotAddress, 8, (void *) & functionAddress, true ) ) {
    fprintf( stderr, "%s: failed to read from GOT (0x%lx)\n", __FUNCTION__, gotAddress );
    return false;
  }
  // /* DEBUG */ fprintf( stderr, "hasBeenBound(): checking function address 0x%lx\n", functionAddress );
	
  /* Do the look up.  We're skipping a potential optimization here (checking to see if
     functionAddress is in the PLT first) for simplicitly. */
  target_pdf = this->findFuncByAddr( functionAddress );
  // /* DEBUG */ fprintf( stderr, "hasBeenBound(): found int_function at %p\n", target_pdf );
  return ( target_pdf != NULL );
} /* end hasBeenBound() */

bool GetFrameRegisterCallback::operator()(unw_cursor_t *cp, 
                                          unw_regnum_t reg, 
                                          unw_word_t *valp)
{
  lock->_Lock(FILE__, __LINE__);
  cp_ = cp;
  reg_ = reg;
  valp_ = valp;
  getMailbox()->executeOrRegisterCallback(this);
  if (synchronous) {
    dbi_printf("%s[%d]:  waiting for completion of callback\n", FILE__, __LINE__);
    waitForCompletion();
  }
  lock->_Unlock(FILE__, __LINE__);
  return true;
}

bool GetFrameRegisterCallback::execute_real()
{
  ret = unw_get_reg(cp_, reg_, valp_);
  return true;
}

int DebuggerInterface::getFrameRegister(unw_cursor_t *cp, unw_regnum_t reg, unw_word_t *valp)
{
  getBusy();
  int ret;
  GetFrameRegisterCallback *cbp = new GetFrameRegisterCallback(&dbilock);
  GetFrameRegisterCallback &cb = *cbp;

  cb.enableDelete(false);
  cb(cp, reg, valp);
  ret = cb.getReturnValue();
  cb.enableDelete();

  releaseBusy();
  return ret;
}

int DebuggerInterface::setFrameRegister(unw_cursor_t *cp, unw_regnum_t reg, unw_word_t val)
{
  getBusy();
  int ret;
  SetFrameRegisterCallback *cbp = new SetFrameRegisterCallback(&dbilock);
  SetFrameRegisterCallback &cb = *cbp;

  cb.enableDelete(false);
  cb(cp, reg, val);
  ret = cb.getReturnValue();
  cb.enableDelete();

  releaseBusy();
  return ret;
}
bool SetFrameRegisterCallback::operator()(unw_cursor_t *cp, 
                                          unw_regnum_t reg, 
                                          unw_word_t val)
{
  lock->_Lock(FILE__, __LINE__);
  cp_ = cp;
  reg_ = reg;
  val_ = val;
  getMailbox()->executeOrRegisterCallback(this);
  if (synchronous) {
    dbi_printf("%s[%d]:  waiting for completion of callback\n", FILE__, __LINE__);
    waitForCompletion();
  }
  lock->_Unlock(FILE__, __LINE__);
  return true;
}

bool SetFrameRegisterCallback::execute_real()
{
  ret = unw_set_reg(cp_, reg_, val_);
  return true;
}

void * DebuggerInterface::createUnwindAddressSpace(unw_accessors_t *ap, int byteorder)
{
  getBusy();
  void * ret;
  CreateUnwindAddressSpaceCallback *cbp = new CreateUnwindAddressSpaceCallback(&dbilock);
  CreateUnwindAddressSpaceCallback &cb = *cbp;

  cb.enableDelete(false);
  cb(ap, byteorder);
  ret = cb.getReturnValue();
  cb.enableDelete();

  releaseBusy();
  return ret;
}

bool CreateUnwindAddressSpaceCallback::operator()(unw_accessors_t *ap, 
                                                  int byteorder) 
{
  lock->_Lock(FILE__, __LINE__);
  ap_ = ap;
  byteorder_ = byteorder;
  getMailbox()->executeOrRegisterCallback(this);
  if (synchronous) {
    dbi_printf("%s[%d]:  waiting for completion of callback\n", FILE__, __LINE__);
    waitForCompletion();
  }
  lock->_Unlock(FILE__, __LINE__);
  return true;
}

bool CreateUnwindAddressSpaceCallback::execute_real()
{
  unw_addr_space *s = unw_create_addr_space(ap_, byteorder_);
  ret = (void *) s;
  return true;
}

int DebuggerInterface::destroyUnwindAddressSpace(unw_addr_space *as)
{
  getBusy();
  int ret;
  DestroyUnwindAddressSpaceCallback *cbp = new DestroyUnwindAddressSpaceCallback(&dbilock);
  DestroyUnwindAddressSpaceCallback &cb = *cbp;

  cb.enableDelete(false);
  cb(as);
  ret = cb.getReturnValue();
  cb.enableDelete();

  releaseBusy();
  return ret;
}

bool DestroyUnwindAddressSpaceCallback::operator()(unw_addr_space *as)
{
  lock->_Lock(FILE__, __LINE__);
  as_ = as;
  getMailbox()->executeOrRegisterCallback(this);
  if (synchronous) {
    dbi_printf("%s[%d]:  waiting for completion of callback\n", FILE__, __LINE__);
    waitForCompletion();
  }
  lock->_Unlock(FILE__, __LINE__);
  return true;
}

bool DestroyUnwindAddressSpaceCallback::execute_real()
{
  unw_destroy_addr_space(as_);
  return true;
}
void *DebuggerInterface::UPTcreate(pid_t pid)
{
  getBusy();
  void * ret;
  UPTCreateCallback *cbp = new UPTCreateCallback(&dbilock);
  UPTCreateCallback &cb = *cbp;

  cb.enableDelete(false);
  cb(pid);
  ret = cb.getReturnValue();
  cb.enableDelete();

  releaseBusy();
  return ret;
}

bool UPTCreateCallback::operator()(pid_t pid)
{
  lock->_Lock(FILE__, __LINE__);
  pid_ = pid;
  getMailbox()->executeOrRegisterCallback(this);
  if (synchronous) {
    dbi_printf("%s[%d]:  waiting for completion of callback\n", FILE__, __LINE__);
    waitForCompletion();
  }
  lock->_Unlock(FILE__, __LINE__);
  return true;
}

bool UPTCreateCallback::execute_real()
{
  ret = _UPT_create(pid_);
  return true;
}

void DebuggerInterface::UPTdestroy(void *handle)
{
  getBusy();
  UPTDestroyCallback *cbp = new UPTDestroyCallback(&dbilock);
  UPTDestroyCallback &cb = *cbp;

  cb.enableDelete(false);
  cb(handle);
  cb.enableDelete();

  releaseBusy();
}

bool UPTDestroyCallback::operator()(void *handle)
{
  lock->_Lock(FILE__, __LINE__);
  handle_ = handle;
  getMailbox()->executeOrRegisterCallback(this);
  if (synchronous) {
    dbi_printf("%s[%d]:  waiting for completion of callback\n", FILE__, __LINE__);
    waitForCompletion();
  }
  lock->_Unlock(FILE__, __LINE__);
  return true;
}

bool UPTDestroyCallback::execute_real()
{
  _UPT_destroy(handle_);
  return true;
}

int DebuggerInterface::initFrame(unw_cursor_t *cp, unw_addr_space_t as, void *arg)
{
  getBusy();
  int ret;
  InitFrameCallback *cbp = new InitFrameCallback(&dbilock);
  InitFrameCallback &cb = *cbp;

  cb.enableDelete(false);
  cb(cp, as, arg);
  ret = cb.getReturnValue();
  cb.enableDelete();

  releaseBusy();
  return ret;
}

bool InitFrameCallback::operator()(unw_cursor_t *cp, unw_addr_space_t as, void *arg)
{
  lock->_Lock(FILE__, __LINE__);
  cp_ = cp;
  as_ = as;
  arg_ = arg;
  getMailbox()->executeOrRegisterCallback(this);
  if (synchronous) {
    dbi_printf("%s[%d]:  waiting for completion of callback\n", FILE__, __LINE__);
    waitForCompletion();
  }
  lock->_Unlock(FILE__, __LINE__);
  return true;
}

bool InitFrameCallback::execute_real()
{
  ret = unw_init_remote(cp_, as_, arg_);
  return true;
}

int DebuggerInterface::stepFrameUp(unw_cursor_t *cp)
{
  getBusy();
  int ret;
  StepFrameUpCallback *cbp = new StepFrameUpCallback(&dbilock);
  StepFrameUpCallback &cb = *cbp;

  cb.enableDelete(false);
  cb(cp);
  ret = cb.getReturnValue();
  cb.enableDelete();

  releaseBusy();
  return ret;
}

bool StepFrameUpCallback::operator()(unw_cursor_t *cp)
{
  lock->_Lock(FILE__, __LINE__);
  cp_ = cp;
  getMailbox()->executeOrRegisterCallback(this);
  if (synchronous) {
    dbi_printf("%s[%d]:  waiting for completion of callback\n", FILE__, __LINE__);
    waitForCompletion();
  }
  lock->_Unlock(FILE__, __LINE__);
  return true;
}

bool StepFrameUpCallback::execute_real()
{
  ret = unw_step(cp_);
  return true;
}

bool DebuggerInterface::isSignalFrame(unw_cursor_t *cp)
{
  getBusy();
  bool ret;
  IsSignalFrameCallback *cbp = new IsSignalFrameCallback(&dbilock);
  IsSignalFrameCallback &cb = *cbp;

  cb.enableDelete(false);
  cb(cp);
  ret = cb.getReturnValue();
  cb.enableDelete();

  releaseBusy();
  return ret;
}

bool IsSignalFrameCallback::operator()(unw_cursor_t *cp)
{
  lock->_Lock(FILE__, __LINE__);
  cp_ = cp;
  getMailbox()->executeOrRegisterCallback(this);
  if (synchronous) {
    dbi_printf("%s[%d]:  waiting for completion of callback\n", FILE__, __LINE__);
    waitForCompletion();
  }
  lock->_Unlock(FILE__, __LINE__);
  return true;
}

bool IsSignalFrameCallback::execute_real()
{
  ret = unw_is_signal_frame(cp_);
  return true;
}

int DebuggerInterface::waitpid(pid_t pid, int *status, int options)
{
  getBusy();
  int ret;
  WaitpidCallback *cbp = new WaitpidCallback(&dbilock);
  WaitpidCallback &cb = *cbp;

  cb.enableDelete(false);
  cb(pid, status, options);
  ret = cb.getReturnValue();
  cb.enableDelete();

  releaseBusy();
  return ret;
}
bool WaitpidCallback::operator()(pid_t pid, int *status, int options)
{
  lock->_Lock(FILE__, __LINE__);
  pid_ = pid;
  status_ = status;
  options_ = options;
  getMailbox()->executeOrRegisterCallback(this);
  if (synchronous) {
    dbi_printf("%s[%d]:  waiting for completion of callback\n", FILE__, __LINE__);
    waitForCompletion();
  }
  lock->_Unlock(FILE__, __LINE__);
  return true;
}

bool WaitpidCallback::execute_real()
{
  ret = waitpid(pid_, status_, options_);
  return true;
}

Frame process::preStackWalkInit(Frame startFrame)
{
   /* Do a special check for the vsyscall page.  Silently drop
      the page if it exists. The IA-64 doesn't use DWARF to unwind out of 
      the vsyscall page, so calcVsyscallFrame() is overkill.
   */
    if( getVsyscallStart() == 0x0 ) {
        if( ! readAuxvInfo() ) {
            /* We're probably on Linux 2.4; use default values. */
            setVsyscallRange( 0xffffffffffffe000, 0xfffffffffffff000 );
            setVsyscallData( NULL );
        }
    }
    Address next_pc = startFrame.getPC();
    if (next_pc >= getVsyscallStart() && next_pc < getVsyscallEnd()) {
       return startFrame.getCallerFrame();
    }
    return startFrame;
}
