/* -*- Mode: C; indent-tabs-mode: true -*- */

#include <stdio.h>

// debugging... is there a Paradyn Standard for this?
#define TRACE(expr)     fprintf( stderr, "At line %d of %s in %s: %s.\n", __LINE__, __PRETTY_FUNCTION__,  __FILE__, __STRING(expr) );

#include "inst-ia64.h"
#include "linux.h"
#include "process.h"
#include "dyn_lwp.h"

#include <sys/ptrace.h>
#include <asm/ptrace.h>
#include <asm/ptrace_offsets.h>
#include <dlfcn.h>			// DLOPEN_MODE

extern debug_ostream inferiorrpc_cerr;

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
	insn = instruction( );
	} /* end generateBreakPoint() */

/* Require by linux.C's insertTrapAtEntryPointOfMain() */
IA64_bundle generateTrapBundle() {
	/* Note: we're using 0x80000 as our break.m immediate,
	   which is defined to be a debugger breakpoint.  If this
	   gets flaky, anything up to 0x0FFFFF will generate a SIGTRAP. */

	/* Actually, what we're going to do is generate
	   a SIGILL, (0x40000) because SIGTRAP does silly things. */

	return IA64_bundle( MIIstop, TRAP_M, NOP_I, NOP_I );
	} /* end generateBreakBundle() */

/* dyn_lwp::getRegisters()
 * 
 * Entire user state can be described by struct pt_regs
 * and struct switch_stack.  It's tempting to try and use
 * pt_regs only, but only syscalls are that well behaved.
 * We must support running arbitrary code.
 */
dyn_saved_regs *dyn_lwp::getRegisters()
{
    int i;
    long int *memptr;
    dyn_saved_regs *result;

    // Bad things happen if you use ptrace on a running process.
    assert(proc_->status_ == stopped);

    // Allocate memory.
    result = (dyn_saved_regs *)malloc(sizeof(dyn_saved_regs));
    assert(result);

    // Save struct pt_regs.
    memptr = (long int *)(&result->pt);
    for (i = PT_CR_IPSR; i < PT_F9 + 16; i += 8) {
	*memptr = P_ptrace(PTRACE_PEEKUSER, proc_->getPid(), i, 0);
	++memptr;
    }

    // Save struct switch_stack.
    memptr = (long int *)(&result->ss);
    for (i = PT_NAT_BITS; i < PT_AR_LC + 8; i += 8) {
	*memptr = P_ptrace(PTRACE_PEEKUSER, proc_->getPid(), i, 0);
	++memptr;
    }
    return result;
}

bool changePC( int pid, Address loc ) { 
	/* We assume until further notice that all of our jumps
	   are properly (bundle) aligned, because we should be
	   getting their destinations from code we didn't generate. */
	assert( loc % 16 == 0 );

	return P_ptrace( PTRACE_POKEUSER, pid, PT_CR_IIP, loc );
	} /* end changePC() */

bool dyn_lwp::changePC( Address loc, dyn_saved_regs * regs ) {
	if( regs != NULL ) { restoreRegisters( regs ); }

	return ::changePC( proc_->getPid(), loc );
	} /* end changePC() */

void printRegs( void * /* save */ ) {
	assert( 0 );
	} /* end printReg[isters, should be]() */

/* dyn_lwp::executingSystemCall()
 *
 * For now, this function simply reads the previous instruction
 * (in the previous bundle if necessary) and returns true if
 * it is a break 0x100000.
 *
 * This is a slow and terrible hack, so if there is a better way, please
 * re-write this function.  Otherwise, it seems to work pretty well.
 */
bool dyn_lwp::executingSystemCall()
{
    const uint64_t SYSCALL_MASK = 0x01000000000 >> 5;
    uint64_t iip, instruction, rawBundle[2];
    int64_t ipsr_ri, pr;
    IA64_bundle origBundle;

    // Bad things happen if you use ptrace on a running process.
    assert(proc_->status_ == stopped);
    errno = 0;

    // Find the correct bundle.
    iip = getPC(proc_->getPid());
    ipsr_ri = P_ptrace(PTRACE_PEEKUSER, proc_->getPid(), PT_CR_IPSR, 0);
    if (errno && (ipsr_ri == -1)) {
	// Error reading process information.  Should we assert here?
	assert(0);
    }
    ipsr_ri = (ipsr_ri & 0x0000060000000000) >> 41;
    if (ipsr_ri == 0) iip -= 16;  // Get previous bundle, if necessary.

    // Read bundle data
    if (!proc_->readDataSpace((void *)iip, 16, (void *)rawBundle, true)) {
	// Could have gotten here because the mutatee stopped right
	// after a jump to the beginning of a memory segment (aka,
	// no previous bundle).  But, that can't happen from a syscall.
	return false;
    }

    // Isolate previous instruction.
    origBundle = IA64_bundle(rawBundle[0], rawBundle[1]);
    ipsr_ri = (ipsr_ri + 2) % 3;
    instruction = origBundle.getInstruction(ipsr_ri)->getMachineCode();
    instruction = instruction >> ALIGN_RIGHT_SHIFT;

    // Determine predicate register and remove it from instruction.
    pr = P_ptrace(PTRACE_PEEKUSER, proc_->getPid(), PT_PR, 0);
    if (errno && pr == -1) assert(0);
    pr = (pr >> (0x1F & instruction)) & 0x1;
    instruction = instruction >> 5;

    return (pr && instruction == SYSCALL_MASK);
} /* end executingSystemCall() */

bool dyn_lwp::restoreRegisters( dyn_saved_regs *regs )
{
    int i;
    const long int *memptr;

    // Bad things happen if you use ptrace on a running process.
    assert(proc_->status_ == stopped);

    memptr = (const long int *)(&regs->pt);
    for (i = PT_CR_IPSR; i < PT_F9 + 16; i += 8) {
	P_ptrace(PTRACE_POKEUSER, proc_->getPid(), i, *memptr);
	++memptr;
    }

    memptr = (const long int *)(&regs->ss);
    for (i = PT_NAT_BITS; i < PT_AR_LC + 8; i += 8) {
	P_ptrace(PTRACE_POKEUSER, proc_->getPid(), i, *memptr);
	++memptr;
    }

    // Should we free regs structure?
    return true;
}

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

	Address addr = f_main->addr();
	InsnAddr iAddr = InsnAddr::generateFromAlignedDataAddress( addr, this );
	iAddr.saveMyBundleTo( savedCodeBuffer );
	iAddr.replaceBundleWith( generateTrapBundle() );
  
	main_brk_addr = addr;
	fprintf( stderr, "*** Inserted trap at entry point of main() : 0x%lx.\n", main_brk_addr );
	} /* end insertTrapAtEntryPointOfMain() */

Address dyn_lwp::readRegister( Register ) {
	assert( 0 );
	return 0;
	} /* end readRegister */

/* Ye flipping bits only knows if this actually works. */
Frame dyn_lwp::getActiveFrame() {
  Address pc, fp, sp, tp;
  
  /* FIXME: check for errors (errno) */
  pid_t pid = proc_->getPid();
  pc = P_ptrace( PTRACE_PEEKUSER, pid, PT_CR_IIP, 0 );
  sp = P_ptrace( PTRACE_PEEKUSER, pid, PT_R12, 0 );
  tp = P_ptrace( PTRACE_PEEKUSER, pid, PT_R13, 0 );
  
  return Frame( pc, fp, sp, proc_->getPid(), NULL, this, true );
} /* end getActiveFrame() */

#define DLOPEN_MODE		(RTLD_NOW | RTLD_GLOBAL)
#define DLOPEN_CALL_LENGTH	4

const char DYNINST_LOAD_HIJACK_FUNCTIONS[][15] = {
	"main",
	"_init",
	"_start"
	};
const int N_DYNINST_LOAD_HIJACK_FUNCTIONS = 3;

/* Defined in process.C; not sure why it isn't in a header. */
extern debug_ostream attach_cerr;

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
Address savedProcessGP;
Address savedReturnAddress;
Address savedPFS;
bool process::loadDYNINSTlib() {
	/* Look for a function we can hijack to forcibly load dyninstapi_rt. */
	Address entry = findFunctionToHijack(symbols, this);	// We can avoid using InsnAddr because we know 
								// that function entry points are aligned.
	if( !entry ) { return false; }
        
	/* FXIME: Check the glibc version.  If it's not 2.2.x, die. */

	/* Fetch the name of the run-time library. */
        const char DyninstEnvVar[]="DYNINSTAPI_RT_LIB";

	if( ! dyninstRT_name.length() ) { // we didn't get anything on the c/l
		if (getenv(DyninstEnvVar) != NULL) {
			dyninstRT_name = getenv(DyninstEnvVar);
			} else {
			string msg = string( "Environment variable " + string(DyninstEnvVar)
					+ " has not been defined for process " ) + string(pid);
			showErrorCallback(101, msg);
			return false;
			} /* end if enviromental variable not found */
		} /* end enviromental variable extraction */

	/* Locate the entry point to dlopen. */
	bool err; Address dlopenAddr = findInternalAddress( "_dl_open", true, err );
	assert( dlopenAddr );

	/* Save the function we're going to hijack. */
	InsnAddr iAddr = InsnAddr::generateFromAlignedDataAddress( entry, this );
	iAddr.saveBundlesTo( savedCodeBuffer, sizeof(savedCodeBuffer) / 16 );

	/* Save the current GP. */
	pid_t pid = getPid();
	savedProcessGP = P_ptrace( PTRACE_PEEKUSER, pid, PT_R1, 0 );
	if( savedProcessGP == (Address)-1 ) { assert( 0 ); }

	/* Save the current return address. */
	savedReturnAddress = P_ptrace( PTRACE_PEEKUSER, pid, PT_B0, 0 );
	if( savedReturnAddress == (Address)-1 ) { assert( 0 ); }

	/* Save the current AR pfs. */
	savedPFS = P_ptrace( PTRACE_PEEKUSER, pid, PT_AR_PFS, 0 );
	if( savedPFS == (Address)-1 ) { assert( 0 ); }

	/* Write _dl_open()'s GP. */
	Address dlopenGP = getTOCoffsetInfo( dlopenAddr );
	assert( dlopenGP );
	if( P_ptrace( PTRACE_POKEUSER, pid, PT_R1, dlopenGP ) == -1 ) { assert( 0 ); }

        /* Write the string into the target's address space.
	   We can't do the smart thing and write the name-string to
	   shared memory, because dyninst doesn't currently _allocate_
	   shared memory.  So we'll write it to the text space and hope
	   that the function is long enough that we don't accidentally 
	   write over something important. */
	// FIXME: DLOPEN_CALL_LENGTH should come from the code generator.  See below.
	iAddr.writeStringAtOffset( DLOPEN_CALL_LENGTH + 1, dyninstRT_name.c_str(), dyninstRT_name.length() );

	/* Insert a function call.  The first argument should be a pointer
	   to the string we just wrote; the second DLOPEN_MODE; and the   
	   the third the return address of the current frame,
	   that is, the location of the SIGILL-generating bundle we'll use
	   to handleIfDueToDyninstLib() and restore the original code.  */

	/* FIXME: replace with a call to the code generator once it's working? */
	IA64_bundle dlopenCallBundles[DLOPEN_CALL_LENGTH];
	
	/* We need three output registers. */
	InsnAddr allocAddr = InsnAddr::generateFromAlignedDataAddress( entry, this );

	IA64_instruction integerNOP( NOP_I );
	uint64_t allocatedLocal, allocatedOutput, allocatedRotate;
	assert( extractAllocatedRegisters( * allocAddr, & allocatedLocal, & allocatedOutput, & allocatedRotate ) );
	Register out0 = allocatedLocal + allocatedOutput + 32;
	Register out1 = out0 + 1; Register out2 = out1 + 1;

	/* Generate a register space for the base tramp 'by hand.' */
	Register deadRegisterList[3];
	deadRegisterList[0] = out0;
	deadRegisterList[1] = out1;
	deadRegisterList[2] = out2;
	registerSpace fnCallRegisterSpace( 3, deadRegisterList, 0, NULL );
	IA64_instruction alteredAlloc = generateAllocInstructionFor( & fnCallRegisterSpace, 0, 3, 0 );

	/* Generate the necessary instructions. */
	IA64_instruction_x setStringPointer = generateLongConstantInRegister( out0, entry + ((DLOPEN_CALL_LENGTH + 1) * 16) );
	IA64_instruction setMode = generateShortConstantInRegister( out1, DLOPEN_MODE );
	IA64_instruction_x setReturnPointer = generateLongConstantInRegister( out2, entry + (DLOPEN_CALL_LENGTH * 16) );
	IA64_instruction memoryNOP( NOP_M );
	IA64_instruction_x branchLong = generateLongCallTo( dlopenAddr - (entry + (16 * (DLOPEN_CALL_LENGTH - 1))), 0 );

	/* Bundle and insert them. */
	dlopenCallBundles[0] = IA64_bundle( MIIstop, alteredAlloc, integerNOP, integerNOP );
	dlopenCallBundles[1] = IA64_bundle( MLXstop, memoryNOP, setStringPointer );
	dlopenCallBundles[2] = IA64_bundle( MLXstop, setMode, setReturnPointer );
	dlopenCallBundles[3] = IA64_bundle( MLXstop, memoryNOP, branchLong );
	iAddr.replaceBundlesWith( dlopenCallBundles, DLOPEN_CALL_LENGTH );

	/* Generate SIGILL when _dl_open() returns. */
	InsnAddr sigAddr = InsnAddr::generateFromAlignedDataAddress( entry + ((DLOPEN_CALL_LENGTH) * 16), this );
        sigAddr.replaceBundleWith( generateTrapBundle() );

	/* Let everyone else know that we're expecting a SIGILL. */
	dyninstlib_brk_addr = entry + ((DLOPEN_CALL_LENGTH) * 16);

	/* Let them know we're working on it. */
	setBootstrapState( loadingRT );

	/* We finished successfully. */
	fprintf( stderr, "*** Hijacked function at 0x%lx to force DYNINSTLIB loading, installed SIGILL at 0x%lx\n", entry, dyninstlib_brk_addr );
	return true;
	} /* end dlopenDYNINSTlib() */

bool process::loadDYNINSTlibCleanup() {
	/* We function did we hijack? */
	Address entry = findFunctionToHijack(symbols, this);	// We can avoid using InsnAddr because we know 
								// that function entry points are aligned.
	if( !entry ) { assert( 0 ); }		

	/* Restore the function we hijacked. */
	InsnAddr iAddr = InsnAddr::generateFromAlignedDataAddress( entry, this );
	iAddr.writeBundlesFrom( savedCodeBuffer, sizeof(savedCodeBuffer) / 16 );

	/* Restore the GP, return address, and AR pfs. */
	pid_t pid = getPid();
	if( P_ptrace( PTRACE_POKEUSER, pid, PT_R1, savedProcessGP ) == -1 ) { assert( 0 ); }
	if( P_ptrace( PTRACE_POKEUSER, pid, PT_B0, savedReturnAddress ) == -1 ) { assert( 0 ); }
	if( P_ptrace( PTRACE_POKEUSER, pid, PT_AR_PFS, savedPFS ) == -1 ) { assert( 0 ); }

	/* DYNINSTlib is finished loading; handle it. */
	dyn->handleDYNINSTlibLoad( this );

	/* Continue execution at the entry point. */
	changePC( pid, entry );

	fprintf( stderr, "*** Handled trap due to dyninstLib.\n" );
	return true;
	} /* end loadDYNINSTlibCleanup() */

Frame Frame::getCallerFrame( process * /* p */ ) const {
	assert( 0 );
	
	return Frame(); // zero frame
} /* end getCallerFrame() */

syscallTrap *process::trapSyscallExitInternal(Address syscall) {
    syscallTrap *trappedSyscall = NULL;

    // First, the cross-platform bit. If we're already trapping
    // on this syscall, then increment the reference counter
    // and return

    for (unsigned iter = 0; iter < syscallTraps_.size(); iter++) {
        if (syscallTraps_[iter]->syscall_id == (int) syscall) {
            trappedSyscall = syscallTraps_[iter];
            cerr << "Found previously trapped syscall at slot " << iter << endl;
            break;
        }
    }
    if (trappedSyscall) {
        // That was easy...
        trappedSyscall->refcount++;
        cerr << "Syscall refcount = " << trappedSyscall->refcount;
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
        uint64_t *savedBundle = (uint64_t *)trappedSyscall->saved_insn;
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
        newInst = IA64_instruction(0x0, 0, NULL, ipsr_ri);
        trapBundle.setInstruction(newInst);
        
        // Write modified bundle
        if (!writeDataSpace((void *)codeBase, 16, trapBundle.getMachineCodePtr()))
            return NULL;
        return trappedSyscall;
    }
    return NULL;
}

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
    fprintf(stderr, "Removing trapped syscall at %d\n",
            trappedSyscall->syscall_id);
    if (!writeDataSpace((void *)getPC(pid), 16, trappedSyscall->saved_insn))
        return false;
        
    // Now that we've reset the original behavior, remove this
    // entry from the vector
    for (unsigned iter = 0; iter < syscallTraps_.size(); iter++) {
        if (trappedSyscall == syscallTraps_[iter]) {
            syscallTraps_.removeByIndex(iter);
            break;
        }
    }
    delete trappedSyscall;
    
    return true;
}

Address dyn_lwp::getCurrentSyscall() {
    Frame active = getActiveFrame();
    return active.getPC();
}

bool dyn_lwp::stepPastSyscallTrap() {
    // Shouldn't be necessary yet
    assert(0 && "Unimplemented");
    return false;
}

int dyn_lwp::hasReachedSyscallTrap() {
    if (!trappedSyscall_) return false;
    Frame active = getActiveFrame();
    return active.getPC() == trappedSyscall_->syscall_id;
}

/* Required by linux.C */
bool process::hasBeenBound( const relocationEntry /* entry */, pd_Function * & /* target_pdf */, Address /* base_addr */ ) {
	assert( 0 );

	return false;
	} /* end hasBeenBound() */
