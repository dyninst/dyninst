/* -*- Mode: C; indent-tabs-mode: true; tab-width: 4 -*- */

#include <stdio.h>

// debugging... is there a Paradyn Standard for this?
#define TRACE(expr)     fprintf( stderr, "At line %d of %s in %s: %s.\n", __LINE__, __PRETTY_FUNCTION__,  __FILE__, __STRING(expr) );

#include "inst-ia64.h"
#include "linux.h"
#include "process.h"
#include "dyn_lwp.h"

/* For emitInferiorRPC*(). */
#include "rpcMgr.h"

#include <sys/ptrace.h>
#include <asm/ptrace.h>
#include <asm/ptrace_offsets.h>
#include <dlfcn.h>			// DLOPEN_MODE


extern unsigned enable_pd_inferior_rpc_debug;

#if ENABLE_DEBUG_CERR == 1
#define inferiorrpc_cerr if (enable_pd_inferior_rpc_debug) cerr
#else
#define inferiorrpc_cerr if (0) cerr
#endif /* ENABLE_DEBUG_CERR == 1 */

/* Required by linuxDL.C */
Address getSP( int /* pid */ ) {
	assert( 0 );
	return 0;
	} /* end getSP */

bool changeSP( int /* pid */, Address /* loc */ ) {
	assert( 0 );
	return false;
	} /* end changeSP */

/* Required by linux.C */
void generateBreakPoint( instruction & insn ) {
	assert( 0 );
	} /* end generateBreakPoint() */

/* Require by insertTrapAtEntryPointOfMain() */
IA64_bundle generateTrapBundle() {
	/* Note: we're using 0x80000 as our break.m immediate,
	   which is defined to be a debugger breakpoint.  If this
	   gets flaky, anything up to 0x0FFFFF will generate a SIGTRAP. */

	/* Actually, what we're going to do is generate
	   a SIGILL, (0x40000) because SIGTRAP does silly things. */

	return IA64_bundle( MIIstop, TRAP_M, NOP_I, NOP_I );
	} /* end generateTrapBundle() */

/* dyn_lwp::getRegisters()
 * 
 * Entire user state can be described by struct pt_regs
 * and struct switch_stack.  It's tempting to try and use
 * pt_regs only, but only syscalls are that well behaved.
 * We must support running arbitrary code.
 */
bool dyn_lwp::getRegisters_(struct dyn_saved_regs *regs)
{
	/* (Almost) All the state preservation is done mutatee-side,
	   except for the syscall handler's predicate registers,
	   so we don't need to anything except the PC. */
	errno = 0;
	regs->pc = P_ptrace( PTRACE_PEEKUSER, proc_->getPid(), PT_CR_IIP, 0 );
	assert( ! errno );
	regs->restorePredicateRegistersFromStack =
	   needToHandleSyscall( proc_, &regs->pcMayHaveRewound );

	// /* DEBUG */ fprintf( stderr, "-*- dyn_lwp::getRegisters()\n" );
	return true;
}

bool changePC( int pid, Address loc ) { 
	/* We assume until further notice that all of our jumps
	   are properly (bundle) aligned, because we should be
	   getting their destinations from code we didn't generate. */
	assert( loc % 16 == 0 );

	return (P_ptrace( PTRACE_POKEUSER, pid, PT_CR_IIP, loc ) != -1);
	} /* end changePC() */

bool dyn_lwp::changePC( Address loc, dyn_saved_regs * regs ) {
	if( regs != NULL ) { restoreRegisters( *regs ); }

	return ::changePC( proc_->getPid(), loc );
	} /* end changePC() */

void printRegs( void * /* save */ ) {
	assert( 0 );
	} /* end printReg[isters, should be]() */

/* The iRPC header/trailer generators handle the special
   case of being in a system call when it's time to run the iRPC. */
bool dyn_lwp::executingSystemCall() {
	return false;
	} /* end executingSystemCall() */

bool dyn_lwp::restoreRegisters_( const struct dyn_saved_regs &regs )
{
	/* Restore the PC. */
	if( ! regs.pcMayHaveRewound ) {
		changePC( regs.pc, NULL );
		}
	else {
		/* The flag could have only been set if the ipsr.ri at
		   construction-(and therefore run-)time ispr.ri was 0.  If it's 0
		   after the syscall trailer completes, then there we were not in a
		   syscall and the original regs->pc is correct.  If it's 2, then we
		   really want the last instruction of the _previous_ bundle and need
		   to adjust the PC.  Otherwise, we abort because of system
		   corruption. */
        errno = 0;
		uint64_t ipsr = P_ptrace( PTRACE_PEEKUSER, proc_->getPid(), PT_CR_IPSR, 0 );
		assert( ! errno );

		uint64_t ipsr_ri = (ipsr & 0x0000060000000000) >> 41;
		assert( ipsr_ri <= 2 );
		
		switch( ipsr_ri ) {
			case 0:
				changePC( regs.pc, NULL );
				break;
			case 1:
				assert( 0 );
				break;
			case 2:
				changePC( regs.pc - 0x10, NULL );
				break;
			default:
				assert( 0 );
				break;
			} /* end ispr_ri switch */
		}

	if( regs.restorePredicateRegistersFromStack ) {
		errno = 0;
		Address stackPointer = P_ptrace( PTRACE_PEEKUSER, proc_->getPid(), PT_R12, 0 );
		assert( ! errno );
		stackPointer -= 48;

		uint64_t predicateRegisters = P_ptrace( PTRACE_PEEKDATA, proc_->getPid(), stackPointer, 0 );
		assert( ! errno );

		assert( P_ptrace( PTRACE_POKEUSER, proc_->getPid(), PT_PR, (Address) & predicateRegisters ) != -1 );
		} /* end predicate register restoration. */

	// /* DEBUG */ fprintf( stderr, "-*- dyn_lwp::restoreRegisters()\n" );
    return true;
}

Address getPC( int pid ) {
	
	/* worry: 0x0000000100000000 is the IT offset of the PSR, (PT_CR_IPSR)
	   which will tell us if we've gotten a virtual or a
	   real address as the PC.  If we got a virtual address,
	   I think we're kind of screwed. */

	errno = 0;
	Address pc = P_ptrace( PTRACE_PEEKUSER, pid, PT_CR_IIP, 0 );
	assert( ! errno );

	return pc;
	} /* end getPC() */

bool process::handleTrapAtEntryPointOfMain() {
	/* Try to find main(). */
	function_base * f_main = NULL;

	pdvector<pd_Function *> * pdfv = NULL;
	if( NULL == ( pdfv = symbols->findFuncVectorByPretty("main") ) || pdfv->size() == 0 ) {
		return false;
  		}
  
	if (pdfv->size() > 1) {
		cerr << __FILE__ << __LINE__ << ": found more than one main! using the first" << endl;
		}
	
	f_main = (function_base *)(* pdfv)[0];
	assert( f_main );

	/* Replace the original code. */
	Address addr = f_main->addr();	
	InsnAddr iAddr = InsnAddr::generateFromAlignedDataAddress( addr, this );
	iAddr.writeMyBundleFrom( savedCodeBuffer );

	// /* DEBUG */ fprintf( stderr, "* Handled trap at entry point of main().\n" );
	return true;
} /* end handleTrapAtEntryPointOfMain() */

bool process::insertTrapAtEntryPointOfMain() {
	/* Try to find main(). */
	function_base * f_main = NULL;

	pdvector<pd_Function *> * pdfv = NULL;
	if( NULL == ( pdfv = symbols->findFuncVectorByPretty("main") ) || pdfv->size() == 0 ) {
		return false;
  		}
  
	if (pdfv->size() > 1) {
		cerr << __FILE__ << __LINE__ << ": found more than one main! using the first" << endl;
		}
	
	f_main = (function_base *)(* pdfv)[0];
	assert( f_main );
	
	/* Save the original code and replace it with a trap bundle. */
	Address addr = f_main->addr();
	InsnAddr iAddr = InsnAddr::generateFromAlignedDataAddress( addr, this );
	iAddr.saveMyBundleTo( savedCodeBuffer );
	iAddr.replaceBundleWith( generateTrapBundle() );
	
	main_brk_addr = addr;
	// /* DEBUG */ fprintf( stderr, "* Inserted trap at entry point of main() : 0x%lx.\n", main_brk_addr );
	return true;
} /* end insertTrapAtEntryPointOfMain() */

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

/* Fixme: from inst-ia64.C; stash in a common header. */
#define BIT_0_6		0x000000007F

Address dyn_lwp::readRegister( Register reg ) {
	/* I can't find anything in the docs saying that ptrace()d
	   programs will always have a flushrs executed in their
	   context before the debugger gains control, but GDB seems
	   to think that this is the case, and we can't do anything
	   else to read registers anyway. */

	/* Acquire the BSP. */
	errno = 0;
	Address bsp = P_ptrace( PTRACE_PEEKUSER, proc_->getPid(), PT_AR_BSP, 0 );
	assert( ! errno );

	/* Acquire the CFM. */
	uint64_t currentFrameMarker = P_ptrace( PTRACE_PEEKUSER, proc_->getPid(), PT_CFM, 0 );
	assert( ! errno );

	/* Extract the SOF. */
	int soFrame = (currentFrameMarker & BIT_0_6 );

	/* Calculate the address of the register. */
	Address addressOfReg = calculateRSEOffsetFromBySlots( bsp, (-1 * soFrame) + (reg - 32) );

	/* Acquire and return the value of the register. */
	Address value = P_ptrace( PTRACE_PEEKTEXT, proc_->getPid(), addressOfReg, 0 );
	assert( ! errno );
	return value;
	} /* end readRegister */

#include <libunwind.h>

/* Refactored from getActiveFrame() and getCallerFrame(). */
Frame createFrameFromUnwindCursor( unw_cursor_t * unwindCursor, dyn_lwp * dynLWP, pid_t pid, bool isActiveFrame ) {
	Address ip = 0, sp = 0, fp = 0, tp = 0;
	bool isLeaf = false, isUppermost = false, isSignalFrame = false, isTrampoline = false;
	int status = 0;

	/* Use the unwind cursor to fill in the frame. */
 	status = unw_get_reg( unwindCursor, UNW_IA64_IP, &ip );
	assert( status == 0 );
 	status = unw_get_reg( unwindCursor, UNW_IA64_SP, &sp );
 	assert( status == 0 );
	status = unw_get_reg( unwindCursor, UNW_IA64_TP, &tp );
	assert( status == 0 );
	
	/* Unfortunately, libunwind is a little _too_ helpful:
	   it'll encode the slotNo into the ip.  Take it back
	   out, since we don't rely on it, and it confuses the
	   rest of Dyninst. */
	ip = ip - (ip % 16);
	
	status = unw_is_signal_frame( unwindCursor );
	if( status > 0 ) { isSignalFrame = true; }

	/* I could've just ptrace()d the ip, sp, and tp, but
	   I couldn't have gotten the frame pointer.  With libunwind,
	   the frame pointer is simply the stack pointer of the previous frame. */
	status = unw_step( unwindCursor );
  
	if( status == -UNW_ENOINFO ) {
		/* Most likely, we're trying to unwind from inside a trampoline.
	  	   FIXME: This should not occur once dynamic unwind information is implemented. */
	  	fprintf( stderr, "createFrameFromUnwindCursor(): FIXME: no unwind information available for this frame.\n" );
		isUppermost = true;
		}
	else if( status == 0 ) {
	  	/* This is the uppermost frame. */
	  	// /* DEBUG */ fprintf( stderr, "createFrameFromUnwindCursor(): unwind information indicates that this is the uppermost frame.\n" );
	  	isUppermost = true;
	  	}
	else if( status > 0 ) {
	  	/* The cursor is now one frame up. */
	 	isUppermost = false;
	  	status = unw_get_reg( unwindCursor, UNW_IA64_SP, & fp );
		assert( status == 0 );
		}
		else {
	  	/* Something erroneous happened.. */
	    assert( status >= 0 );
	  	}

	/* FIXME: multithread implementation. */
	dyn_thread * dynThread = NULL;
    
	/* FIXME: determine if this a trampoline frame.  (That is, was the information dynamically constructed?) */
	Frame currentFrame( ip, fp, sp, pid, dynThread, dynLWP, isUppermost, isLeaf, isSignalFrame, isTrampoline );
	
	// /* DEBUG */ fprintf( stderr, "createFrameFromUnwindCursor(): ip = 0x%lx, fp = 0x%lx, sp = 0x%lx, tp = 0x%lx\n", ip, fp, sp, tp );
	return currentFrame;
	} /* end createFrameFromUnwindCursor() */

Frame dyn_lwp::getActiveFrame() {
	// /* DEBUG */ fprintf( stderr, "getActiveFrame(): getting active frame.\n" );

	int status = 0;

	/* Initialize the unwinder. */
	unw_addr_space * unwindAddressSpace = unw_create_addr_space( & _UPT_accessors, 0 );
	assert( unwindAddressSpace != NULL );
	// /* DEBUG */ fprintf( stderr, "Creating unwind context with pid %d\n", proc_->getPid() );
	void * unwindProcessArg = _UPT_create( proc_->getPid() );
	assert( unwindProcessArg != NULL );

	/* Initialize it to the active frame. */
	unw_cursor_t unwindCursor;
	status = unw_init_remote( & unwindCursor, unwindAddressSpace, unwindProcessArg );
	assert( status == 0 );
	
	/* Generate a Frame from the unwinder. */
	Frame currentFrame = createFrameFromUnwindCursor( & unwindCursor, this, proc_->getPid(), true );
	
	/* Shut down the unwinder. */
	_UPT_destroy( unwindProcessArg );
	unw_destroy_addr_space( unwindAddressSpace );

	/* Return the result. */
	return currentFrame;
	} /* end getActiveFrame() */

#define DLOPEN_MODE		(RTLD_NOW | RTLD_GLOBAL)
#define DLOPEN_CALL_LENGTH	4

const char DYNINST_LOAD_HIJACK_FUNCTIONS[][15] = {
	/* This will probably be the lowest function in the address space,
	   since GNU ld prefers to put .init before .text.  Other compilers
	   may require us to look at all of them and pick the lowest.  (Our
	   problem is that main() may high enough in the address space that
	   the 8KB (!) that we need to loadDYNINSTlib() tries to read/write
	   from an unmapped page.) */
	"_init",
	"_start",
	"main"
	};
const int N_DYNINST_LOAD_HIJACK_FUNCTIONS = 3;

/* Defined in process.C; not sure why it isn't in a header. */
extern unsigned enable_pd_attach_detach_debug;

#if ENABLE_DEBUG_CERR == 1
#define attach_cerr if (enable_pd_attach_detach_debug) cerr
#else
#define attach_cerr if (0) cerr
#endif /* ENABLE_DEBUG_CERR == 1 */

Address findFunctionToHijack( image * symbols, process * p ) {
	for( int i = 0; i < N_DYNINST_LOAD_HIJACK_FUNCTIONS; i++ ) {
		Symbol s;
		if( symbols->symbol_info(DYNINST_LOAD_HIJACK_FUNCTIONS[i], s) ) {
		        attach_cerr << "Inserting dlopen call in " << DYNINST_LOAD_HIJACK_FUNCTIONS[i] << " at " << s.addr() << endl;
			attach_cerr << "Process at " << (void*)getPC( p->getPid() ) << endl;
			return s.addr();
			}
		} /* end hijacking search loop. */

	attach_cerr << "Couldn't find a point to insert dlopen call" << endl;
	return 0;
	} /* end findFunctionToHijack() */

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
                                + " has not been defined for process " ) + pdstring( pid );
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

extern registerSpace * regSpace;
bool process::loadDYNINSTlib() {
	/* Look for a function we can hijack to forcibly load dyninstapi_rt. 
	   This is effectively an inferior RPC with the caveat that we're
	   overwriting code instead of allocating memory from the RT heap. 
	   (So 'hijack' doesn't mean quite what you might think.) */
	Address entry = findFunctionToHijack( symbols, this );	// We can avoid using InsnAddr because we know 
															// that function entry points are aligned.
	if( !entry ) { return false; }
        
	/* FIXME: Check the glibc version.  If it's not appropriate, die. */

	/* Fetch the name of the run-time library. */
	const char DyninstEnvVar[]="DYNINSTAPI_RT_LIB";

	if( ! dyninstRT_name.length() ) { // we didn't get anything on the command line
		if (getenv(DyninstEnvVar) != NULL) {
			dyninstRT_name = getenv(DyninstEnvVar);
			} else {
			pdstring msg = pdstring( "Environment variable " + pdstring( DyninstEnvVar )
					+ " has not been defined for process " ) + pdstring( pid );
			showErrorCallback(101, msg);
			return false;
			} /* end if enviromental variable not found */
		} /* end enviromental variable extraction */

	/* Locate the entry point to _dl_open(). */
	bool err; Address dlopenAddr = findInternalAddress( "_dl_open", true, err );
	assert( dlopenAddr );

	/* Save the function we're going to hijack. */
	InsnAddr iAddr = InsnAddr::generateFromAlignedDataAddress( entry, this );
	iAddr.saveBundlesTo( savedCodeBuffer, sizeof( savedCodeBuffer ) / 16 );

	/* Save the current PC. */
	savedPC = getPC( pid );	

	/* _dl_open() takes three arguments: a pointer to the library name,
	   the DLOPEN_MODE, and the return address of the current frame
	   (that is, the location of the SIGILL-generating bundle we'll use
	   to handleIfDueToDyninstLib()).  We construct the first here. */
	   
	/* Write the string to entry, and then move the PC to the next bundle. */
	iAddr.writeStringAtOffset( 0, dyninstRT_name.c_str(), dyninstRT_name.length() );
	Address firstInstruction = entry + dyninstRT_name.length();
	firstInstruction -= (firstInstruction % 16);
	firstInstruction += 0x10;
	Address stringAddress = entry;
	
	/* Now that we know where the code will start, move the PC there. */
	changePC( pid, firstInstruction );
	
	/* At this point, we use the generic iRPC headers and trailers
	   around the call to _dl_open.  (Note that pre-1.35 versions
	   of this file had a simpler mechanism well-suited to boot-
	   strapping a new port.  The current complexity is to handle
	   the attach() case, where we don't know if execution was stopped
	   at the entry the entry point to a function. */
	
	Address offset = 0;
	ia64_bundle_t callBundles[ CODE_BUFFER_SIZE ];
	Address maxOffset = (CODE_BUFFER_SIZE - 1) * sizeof( ia64_bundle_t );
	bool ok = theRpcMgr->emitInferiorRPCheader( callBundles, offset );
	assert( ok );
	assert( offset < maxOffset );
	
	/* Generate the call to _dl_open with a large dummy constant as the
	   the third argument to make sure we generate the same size code the second
	   time around, with the correct "return address." (dyninstlib_brk_addr) */
	pdvector< AstNode * > dlOpenArguments( 3 );
	AstNode * dlOpenCall;
	
	dlOpenArguments[ 0 ] = new AstNode( AstNode::Constant, (void *)stringAddress );
	dlOpenArguments[ 1 ] = new AstNode( AstNode::Constant, (void *)DLOPEN_MODE );
	dlOpenArguments[ 2 ] = new AstNode( AstNode::Constant, (void *)0xFFFFFFFFFFFFFFFF );
	dlOpenCall = new AstNode( "_dl_open", dlOpenArguments );
	
	/* Remember where we originally generated the call. */
	Address originalOffset = offset;
	
	/* emitInferiorRPCheader() configures (the global) registerSpace for us. */
	dlOpenCall->generateCode( this, regSpace, (char *)(& callBundles), offset, true, true );
	
	unsigned breakOffset, resultOffset, justAfterResultOffset;
	ok = theRpcMgr->emitInferiorRPCtrailer( callBundles, offset, breakOffset, false, resultOffset, justAfterResultOffset );
	assert( ok );
	assert( offset < maxOffset );

	/* Let everyone else know that we're expecting a SIGILL. */
	dyninstlib_brk_addr = firstInstruction + breakOffset;

	/* Clean up the reference counts before regenerating. */
	removeAst( dlOpenCall );
	removeAst( dlOpenArguments[ 2 ] );
	
	dlOpenArguments[ 2 ] = new AstNode( AstNode::Constant, (void *)dyninstlib_brk_addr );
	dlOpenCall = new AstNode( "_dl_open", dlOpenArguments );
	
	/* Regenerate the call at the same original location with the correct constants. */
	dlOpenCall->generateCode( this, regSpace, (char *)(& callBundles), originalOffset, true, true );

	/* Clean up the reference counting. */
	removeAst( dlOpenCall );
	removeAst( dlOpenArguments[ 0 ] );
	removeAst( dlOpenArguments[ 1 ] );
	removeAst( dlOpenArguments[ 2 ] );

	/* Write the call into the mutatee. */
	InsnAddr jAddr = InsnAddr::generateFromAlignedDataAddress( firstInstruction, this );
	jAddr.writeBundlesFrom( (unsigned char *)(& callBundles), offset / 16 );

	/* Let them know we're working on it. */
	setBootstrapState( loadingRT );

	return true;
	} /* end dlopenDYNINSTlib() */

bool process::loadDYNINSTlibCleanup() {
	/* We function did we hijack? */
	Address entry = findFunctionToHijack( symbols, this );	// We can avoid using InsnAddr because we know 
															// that function entry points are aligned.
	if( !entry ) { assert( 0 ); }
	
	/* Restore the function we hijacked. */
	InsnAddr iAddr = InsnAddr::generateFromAlignedDataAddress( entry, this );
	iAddr.writeBundlesFrom( savedCodeBuffer, sizeof(savedCodeBuffer) / 16 );

	/* Continue execution at the correct point. */
	pid_t pid = getPid();
	changePC( pid, savedPC );

	return true;
	} /* end loadDYNINSTlibCleanup() */

/* At some point, it may be worth the trouble to add an unw_cursor_t member to Frame. */
Frame Frame::getCallerFrame( process * proc ) const {
	// /* DEBUG */ fprintf( stderr, "getCallerFrame(): getting caller's frame (ip = 0x%lx, fp = 0x%lx, sp = 0x%lx).\n", getPC(), getFP(), getSP() );

	int status = 0;

	/* Initialize the unwinder. */
	unw_addr_space * unwindAddressSpace = unw_create_addr_space( & _UPT_accessors, 0 );
	assert( unwindAddressSpace != NULL );
	void * unwindProcessArg = _UPT_create( proc->getPid() );
	assert( unwindProcessArg != NULL );

	/* Initialize it to the active frame. */
	unw_cursor_t unwindCursor;
	status = unw_init_remote( & unwindCursor, unwindAddressSpace, unwindProcessArg );
	assert( status == 0 );
	
	/* Unwind to the current frame. */
	Frame currentFrame = createFrameFromUnwindCursor( & unwindCursor, lwp_, proc->getPid(), true );
	while( ! currentFrame.isUppermost() ) {
		if( getFP() == currentFrame.getFP() && getSP() == currentFrame.getSP() && getPC() == currentFrame.getPC() ) {
			currentFrame = createFrameFromUnwindCursor( &unwindCursor, lwp_, proc->getPid(), false );
			break;		
			} /* end if we've found this frame */
		currentFrame = createFrameFromUnwindCursor( &unwindCursor, lwp_, proc->getPid(), false );
		}

	/* Shut down the unwinder. */
	_UPT_destroy( unwindProcessArg );
	unw_destroy_addr_space( unwindAddressSpace );
	
	/* Return the result. */
	return currentFrame;
	} /* end getCallerFrame() */

syscallTrap *process::trapSyscallExitInternal(Address syscall) {
    syscallTrap *trappedSyscall = NULL;

    // First, the cross-platform bit. If we're already trapping
    // on this syscall, then increment the reference counter
    // and return

    for (unsigned iter = 0; iter < syscallTraps_.size(); iter++) {
        if (syscallTraps_[iter]->syscall_id == syscall) {
            trappedSyscall = syscallTraps_[iter];
            break;
        }
    }
    if (trappedSyscall) {
        // That was easy...
        trappedSyscall->refcount++;
        return trappedSyscall;
    }
    else {
        // Okay, we haven't trapped this system call yet.
        // Things to do:
        // 1) Get the original value
        // 2) Place a trap
        // 3) Create a new syscallTrap object and return it

        trappedSyscall = new syscallTrap;
        trappedSyscall->refcount = 1;
        trappedSyscall->syscall_id = (int) syscall;

        uint64_t codeBase;
        assert( (Address)&(trappedSyscall->saved_insn) % 8 == 0 );
        uint64_t * savedBundle = (uint64_t *)(Address)&(trappedSyscall->saved_insn);
        int64_t ipsr_ri;
        IA64_bundle trapBundle;
        IA64_instruction newInst;
        
        // Determine exact interruption point (IIP and IPSR.ri)
        codeBase = getPC(pid);
        ipsr_ri = P_ptrace(PTRACE_PEEKUSER, pid, PT_CR_IPSR, 0);
        if (errno && (ipsr_ri == -1)) return NULL;
        ipsr_ri = (ipsr_ri & 0x0000060000000000) >> 41;
        
        // Save current bundle
        if (!readDataSpace((void *)codeBase, 16, (void *)trappedSyscall->saved_insn, true))
            return NULL;

        // Modify current bundle
        trapBundle = IA64_bundle(savedBundle[0], savedBundle[1]);
        newInst = IA64_instruction(0x0, 0, ipsr_ri);
        trapBundle.setInstruction(newInst);
        
        // Write modified bundle
        if (!writeDataSpace((void *)codeBase, 16, trapBundle.getMachineCodePtr()))
            return NULL;
        return trappedSyscall;
    }
    return NULL;
} /* trapSyscallExitInternal() */

bool process::clearSyscallTrapInternal(syscallTrap *trappedSyscall) {
    // Decrement the reference count, and if it's 0 remove the trapped
    // system call
    assert(trappedSyscall->refcount > 0);
    
    trappedSyscall->refcount--;
    if (trappedSyscall->refcount > 0) {
        fprintf(stderr, "Syscall still has refcount %d\n", 
                trappedSyscall->refcount);
        return true;
    }
    fprintf(stderr, "Removing trapped syscall at %ld\n",
            trappedSyscall->syscall_id);
    if (!writeDataSpace((void *)getPC(pid), 16, trappedSyscall->saved_insn))
        return false;
        
    // Now that we've reset the original behavior, remove this
    // entry from the vector
    pdvector<syscallTrap *> newSyscallTraps;
    for (unsigned iter = 0; iter < syscallTraps_.size(); iter++) {
        if (trappedSyscall != syscallTraps_[iter])
            newSyscallTraps.push_back(syscallTraps_[iter]);
    }
    syscallTraps_ = newSyscallTraps;

    delete trappedSyscall;
    return true;
} /* end clearSyscallTrapInternal() */

Address dyn_lwp::getCurrentSyscall() {
	Frame active = getActiveFrame();
	return active.getPC();
	}

bool dyn_lwp::stepPastSyscallTrap() {
	// Shouldn't be necessary yet
	assert(0 && "Unimplemented");
	return false;
	} /* end stepPastSyscallTrap() */

int dyn_lwp::hasReachedSyscallTrap() {
	if (!trappedSyscall_) return false;
	Frame active = getActiveFrame();
	return active.getPC() == trappedSyscall_->syscall_id;
	} /* end hasReachedSyscallTrap() */

/* Required by linux.C */
bool process::hasBeenBound( const relocationEntry /* entry */, pd_Function * & /* target_pdf */, Address /* base_addr */ ) {
	assert( 0 );

	return false;
	} /* end hasBeenBound() */
