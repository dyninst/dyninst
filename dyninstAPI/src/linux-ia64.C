#include <stdio.h>

// debugging... is there a Paradyn Standard for this?
#define TRACE(expr)     fprintf( stderr, "At line %d of %s in %s: %s.\n", __LINE__, __PRETTY_FUNCTION__,  __FILE__, __STRING(expr) );

#include "inst-ia64.h"
#include "linux.h"
#include "process.h"

#include <sys/ptrace.h>
#include <asm/ptrace.h>
#include <asm/ptrace_offsets.h>

/* Required by linuxDL.C */
Address getSP( int pid ) {
	TRACE( FIXME );
	assert( 0 );
	return NULL;
	} /* end getSP */

bool changeSP( int pid, Address loc ) {
	TRACE( FIXME );
	assert( 0 );
	return false;
	} /* end changeSP */

instruction generateTrapInstruction() {
	TRACE( FIXME );
	assert( 0 );
	return instruction( );
	} /* end generateTrapInstruction */

/* Required by linux.C */
void generateBreakPoint( instruction & insn ) {
	TRACE( FIXME );
	assert( 0 );
	insn = instruction( );
	} /* end generateBreakPoint() */

/* Require by linux.C's insertTrapAtEntryPointOfMain() */
IA64_bundle generateTrapBundle() {
	// right-aligned template ID
	uint8_t MIIstop = 0x01;

	// left-aligned instructions
	uint64_t TRAP_M =	0x0000800000000000;
	uint64_t NOP_I =	0x0004000000000000;

	/* Note: we're using 0x80000 as our break.m immediate,
	   which is defined to be a debugger breakpoint.  If this
	   gets flaky, anything up to 0x0FFFFF will generate a SIGTRAP. */

	/* Actually, what we're going to try and do is generate
	   a SIGILL, (0x40000) because SIGTRAP does silly things. */

	return IA64_bundle( MIIstop, TRAP_M, NOP_I, NOP_I );
	} /* end generateBreakBundle() */

void *process::getRegisters( unsigned ) {
	TRACE( FIXME );
	assert( 0 );
	return NULL;
	} /* end getRegisters() */

bool changePC( int pid, Address loc ) {
	/* We assume until further notice that all of our jumps
	   are properly (bundle) aligned, because we should be
	   getting their destinations from code we didn't generate. */
	assert( loc % 16 == 0 );
	return P_ptrace( PTRACE_POKEUSER, pid, PT_CR_IIP, loc );
	} /* end changePC() */

void printRegs( void *save ) {
	TRACE( FIXME );
	assert( 0 );
	} /* end printReg[isters, should be]() */

bool process::executingSystemCall( unsigned lwp ) { 
	TRACE( FIXME );
	assert( 0 );
	return false;
	} /* end executingSystemCall() */

bool process::restoreRegisters( void * buffer ) {
	TRACE( FIXME );
	assert( 0 );
	return false;
	} /* end restoreRegisters() */

Address getPC( int pid ) {
	
	/* worry: 0x0000000100000000 is the IT offset of the PSR, (PT_CR_IPSR)
	   which will tell us if we've gotten a virtual or a
	   real address as the PC.  If we got a virtual address,
	   I think we're kind of screwed. */

	errno = 0;
	Address pc = P_ptrace( PTRACE_PEEKUSER, pid, PT_CR_IIP, 0 );
	if( errno ) { perror( "getPC()" ); exit( -1 ); return 0; }
	
	return pc;
	} /* end getPC() */

void process::handleIfDueToDyninstLib() {
	TRACE( FIXME );
	assert( 0 );
	} /* end handleIfDueToDyninstLib() */

void process::handleTrapAtEntryPointOfMain() {
	function_base *f_main = findOneFunction("main");
	assert(f_main);
	Address addr = f_main->addr();

	/* See comment below in iTAEPOM(). */
	InsnAddr iAddr = InsnAddr::generateFromAlignedDataAddress( addr, this );
	iAddr.writeMyBundleFrom( savedCodeBuffer );

	fprintf( stderr, "*** Handled trap at entry point of main().\n" );
	} /* end handleTrapAtEntryPointOfMain() */

void process::insertTrapAtEntryPointOfMain() {
	function_base * f_main = 0;
	f_main = findOneFunction( "main" );

	if ( ! f_main ) {     // we can't instrument main - naim
		showErrorCallback( 108, "main() uninstrumentable" );
		extern void cleanUpAndExit( int );
		cleanUpAndExit( -1 );
		return;
		}

	assert( f_main );

	/* For now, because we know function addresses are aligned,
	   we'll just convert the function address.  FIXME:
	   function_base::addr() should return an InsnAddr. */
    
	Address addr = f_main->addr();
	InsnAddr iAddr = InsnAddr::generateFromAlignedDataAddress( addr, this );
	iAddr.saveMyBundleTo( savedCodeBuffer );
	iAddr.replaceBundleWith( generateTrapBundle() );
  
	main_brk_addr = addr;
	fprintf( stderr, "*** Inserted trap at entry point of main() : 0x%llx.\n", main_brk_addr );
	} /* end insertTrapAtEntryPointOfMain() */

Address process::read_inferiorRPC_result_register( Register ) {
	TRACE( FIXME );
	assert( 0 );
	return 0;
	} /* end read_inferiorRPC_result_register() */

Frame process::getActiveFrame( unsigned lwp ) {
	Address pc, fp, sp, tp;
	pdThread * pdThreadPtr = NULL;                  /* [1] */

	/* FIXME: check for errors (errno) */
	pid_t pid = getPid();
	pc = P_ptrace( PTRACE_PEEKUSER, pid, PT_CR_IIP, 0 );
//	fp = P_ptrace( PTRACE_PEEKUSER, pid, , 0);
	sp = P_ptrace( PTRACE_PEEKUSER, pid, PT_R12, 0 );
	tp = P_ptrace( PTRACE_PEEKUSER, pid, PT_R13, 0 );

	return Frame( pc, fp, sp, this->getPid(), pdThreadPtr, lwp, true );
	} /* end getActiveFrame() */

/* 1: This was OK in the x86 version.  Ye flipping bits only know why. */


#define DLOPEN_MODE (RTLD_NOW | RTLD_GLOBAL)
#define DLOPEN_CALL_LENGTH      12              /* FIXME: verify */

const char DYNINST_LOAD_HIJACK_FUNCTIONS[][15] = {
	"main",
	"_init",
	"_start"
	};
const int N_DYNINST_LOAD_HIJACK_FUNCTIONS = 3;

/* Defined in process.C; not sure why it isn't in a header. */
extern debug_ostream attach_cerr;

bool process::dlopenDYNINSTlib() {
	/* Look for a function we can hijack to forcibly load dyninstapi_rt. */
	Address entry = 0;	// We can avoid using InsnAddr because we know 
				// that function entry points are aligned.
	int i;
	for( i = 0; i < N_DYNINST_LOAD_HIJACK_FUNCTIONS; i++ ) {
		Symbol s; bool found = false;
		found = symbols->symbol_info(DYNINST_LOAD_HIJACK_FUNCTIONS[i], s);
		if( found ) { entry = s.addr(); }
		if( entry ) { break; }
		} /* end hijacking search loop. */
                
	if( !entry || i >= N_DYNINST_LOAD_HIJACK_FUNCTIONS ) {
		attach_cerr << "Couldn't find a point to insert dlopen call" << endl;
		return false;
		}

        attach_cerr << "Inserting dlopen call in " << DYNINST_LOAD_HIJACK_FUNCTIONS[i] << " at " << (void *)entry << endl;
	attach_cerr << "Process at " << (void*)getPC( getPid() ) << endl;
        
	/* Check the glibc version.  If it's not 2.2.x, die. */

	/* Fetch the name of the run-time library. */
	#ifdef BPATCH_LIBRARY  /* dyninst API loads a different run-time library */
		const char DyninstEnvVar[]="DYNINSTAPI_RT_LIB";
	#else
		const char DyninstEnvVar[]="PARADYN_LIB";
	#endif

	if( ! dyninstName.length() ) { // we didn't get anything on the c/l
		if (getenv(DyninstEnvVar) != NULL) {
			dyninstName = getenv(DyninstEnvVar);
			} else {
			string msg = string( "Environment variable " + string(DyninstEnvVar)
					+ " has not been defined for process " ) + string(pid);
			showErrorCallback(101, msg);
			return false;
			} /* end if enviromental variable not found */
		} /* end enviromental variable extraction */

	/* Save the function we're going to hijack. */
	InsnAddr iAddr = InsnAddr::generateFromAlignedDataAddress( entry, this );
	iAddr.saveBundlesTo( savedCodeBuffer, sizeof(savedCodeBuffer) / 16 );
        
        /* Write the string into the target's address space.
	   We can't do the smart thing and write the name-string to
	   shared memory, because dyninst doesn't currently _allocate_
	   shared memory.  So we'll write it to the text space and hope
	   that the function is long enough that we don't accidentally 
	   write over something important. */

	/* FIXME: we (can) know how long the call sequence will be ahead of time,
	   but it really should be a call into or a constant defined in
	   the code generator. */
	iAddr.writeStringAtOffset( DLOPEN_CALL_LENGTH, dyninstName.c_str(), dyninstName.length() );

	/* Insert a function call.  The first argument should be a pointer
	   to the string we just wrote; the second DLOPEN_MODE; and the   
	   the third the return address of the current frame. (?)
	   The only tricky bit will be to doing the PLT thing and
	   saving/restoring the GP around the call. */

	/* FIXME: replace with a call to the code generator once it's working. */
	IA64_bundle dlopenCallBundles[DLOPEN_CALL_LENGTH];
	dlopenCallBundles[0] = IA64_bundle( /* ... */ );
	dlopenCallBundles[1] = IA64_bundle( /* ... */ );
	// ...

fprintf( stderr, "Replacing bundles at 0x%llx with %d bundles from 0x%llx.\n",
	entry, DLOPEN_CALL_LENGTH, dlopenCallBundles );
fprintf( stderr, "Generating call to object with GP of 0x%llx.\n", getTOCoffsetInfo( entry ) );
	iAddr.replaceBundlesWith( dlopenCallBundles, DLOPEN_CALL_LENGTH );

	/* FIXME: What else do we need to save to fake out the hijacked function? */
       
	} /* end dlopenDYNINSTlib() */

Frame Frame::getCallerFrame( process *p ) const {
	/* FIXME */
	assert( 0 );
	
	return Frame(); // zero frame
	} /* end getCallerFrame() */

/* Required by process.C */
bool process::set_breakpoint_for_syscall_completion() {
	/* FIXME */
	assert( 0 );

	return false;
	} /* end set_breakpoint_for_syscall_completion() */

/* Required by process.C */
void process::clear_breakpoint_for_syscall_completion() {
	/* FIXME */
	assert( 0 );

	} /* end clear_breakpoint_for_syscall_completion() */

/* Required by linux.C */
bool process::hasBeenBound( const relocationEntry entry, pd_Function * & target_pdf, Address base_addr ) {
	/* FIXME */
	assert( 0 );

	return false;
	} /* end hasBeenBound() */

