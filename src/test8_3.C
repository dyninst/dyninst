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

// $Id: test8_3.C,v 1.1 2005/09/29 20:39:53 bpellin Exp $
/*
 * #Name: test1.5
 * #Desc: Mutator Side - If without else
 * #Dep: 
 * #Arch: !(arch_alpha && os_osf, arch_sparc && os_solaris)
 * #Notes:
 */

#include "BPatch.h"
#include "BPatch_Vector.h"
#include "BPatch_thread.h"
#include "BPatch_snippet.h"

#include "test_lib.h"

BPatch *bpatch;

#if defined( DEBUG )
#include <sys/ptrace.h>
#endif
int mutatorTest( BPatch_thread * appThread, BPatch_image * appImage ) {

#if (defined( arch_alpha ) && defined( os_osf )) \
 || (defined( arch_sparc ) && defined( os_solaris ))
	printf("Skipping test #3 (getCallStack through instrumentation)\n");
	printf("    unwinding through base & minitramps not implemented on this platform\n");
        return 0;
#else
        bool passedTest;
        appThread->continueExecution();
	static const frameInfo_t correct_frame_info[] = {
	
#if defined( os_linux ) && (defined( arch_x86 ) || defined( arch_x86_64 ))
	  { true, true, BPatch_frameNormal, "_dl_sysinfo_int80" },
#endif
#if defined( os_aix ) && defined( arch_power )
		/* AIX uses kill(), but the PC of a process in a syscall can
		   not be correctly determined, and appears to be the address
		   to which the syscall function will return. */
#elif defined( os_windows ) && (defined( arch_x86 ) || defined( arch_x86_64 ))
		/* Windows/x86 does not use kill(), so its lowermost frame will be 
		   something unidentifiable in a system DLL. */
		{ false, false, BPatch_frameNormal, NULL },
#else
		{ true, false, BPatch_frameNormal, "kill" },	
#endif
#if ! defined( os_windows )
		/* Windows/x86's stop_process_() calls DebugBreak(); it's 
		   apparently normal to lose this frame. */
		{ true, false, BPatch_frameNormal, "stop_process_" },
#endif
		{ true, false, BPatch_frameNormal, "func3_3" },
		{ true, false, BPatch_frameTrampoline, NULL },
		/* On AIX and x86 (and others), if our instrumentation fires
		   before frame construction or after frame destruction, it's 
		   acceptable to not report the function (since, after all, it
		   doesn't have a frame on the stack. */
		{ true, true, BPatch_frameNormal, "func3_2" },
		{ true, false, BPatch_frameNormal, "func3_1" },
		{ true, false, BPatch_frameNormal, "main" }
		};
	
	/* Wait for the mutatee to stop in func3_1(). */
	RETURNONFAIL(waitUntilStopped( bpatch, appThread, 1, "getCallStack through instrumentation"));
	
	/* Instrument func3_2() to call func3_3(), which will trip another breakpoint. */
	BPatch_Vector<BPatch_function *> instrumentedFunctions;	
	appImage->findFunction( "func3_2", instrumentedFunctions );
	assert( instrumentedFunctions.size() == 1 );
	
	BPatch_Vector<BPatch_point *> * functionEntryPoints = instrumentedFunctions[0]->findPoint( BPatch_entry );
	assert( functionEntryPoints->size() == 1 );
	
	BPatch_Vector<BPatch_function *> calledFunctions;
	appImage->findFunction( "func3_3", calledFunctions );
	assert( calledFunctions.size() == 1 );
	
	BPatch_Vector<BPatch_snippet *> functionArguments;
	BPatch_funcCallExpr functionCall( * calledFunctions[0], functionArguments );
	
	appThread->insertSnippet( functionCall, functionEntryPoints[0] );

	/* Repeat for all three types of instpoints. */
	BPatch_Vector<BPatch_point *> * functionCallPoints = instrumentedFunctions[0]->findPoint( BPatch_subroutine );
	assert( functionCallPoints->size() == 1 );
	appThread->insertSnippet( functionCall, functionCallPoints[0] );
	
	BPatch_Vector<BPatch_point *> * functionExitPoints = instrumentedFunctions[0]->findPoint( BPatch_exit );
	assert( functionExitPoints->size() == 1 );
	appThread->insertSnippet( functionCall, functionExitPoints[0] );

#if defined( DEBUG )
	for( int i = 0; i < 80; i++ ) { ptrace( PTRACE_SINGLESTEP, appThread->getPid(), NULL, NULL ); }
	
	for( int i = 80; i < 120; i++ ) {
		ptrace( PTRACE_SINGLESTEP, appThread->getPid(), NULL, NULL );
		
	    BPatch_Vector<BPatch_frame> stack;
	    appThread->getCallStack( stack );
		
		fprintf( stderr, "single-step stack walk, %d instructions after stop for instrumentation.\n", i );
		for( unsigned i = 0; i < stack.size(); i++ ) {
			char name[ 40 ];
			BPatch_function * func = stack[i].findFunction();
		
			if( func == NULL ) { strcpy( name, "[UNKNOWN]" ); }
			else { func->getName( name, 40 ); }
			
			fprintf( stderr, "  %10p: %s, fp = %p\n", stack[i].getPC(), name, stack[i].getFP() );
			} /* end stack walk dumper */
		fprintf( stderr, "end of stack walk.\n" );
		} /* end single-step iterator */
#endif /* defined( DEBUG ) */		

	/* After inserting the instrumentation, let it be called. */
	appThread->continueExecution();
	  
	/* Wait for the mutatee to stop because of the instrumentation we just inserted. */
	RETURNONFAIL(waitUntilStopped( bpatch, appThread, 1, "getCallStack through instrumentation (entry)"));

	passedTest = true;
	if( !checkStack( appThread, correct_frame_info,
			 sizeof(correct_frame_info)/sizeof(frameInfo_t),
			 3, "getCallStack through instrumentation (entry)" ) ) {
	    passedTest = false;
    	}

	/* Repeat for other two types of instpoints. */
	appThread->continueExecution();	

	/* Wait for the mutatee to stop because of the instrumentation we just inserted. */
	RETURNONFAIL(waitUntilStopped( bpatch, appThread, 1, "getCallStack through instrumentation (call)"));

	if( !checkStack( appThread, correct_frame_info,
			 sizeof(correct_frame_info)/sizeof(frameInfo_t),
			 3, "getCallStack through instrumentation (call)" ) ) {
	    passedTest = false;
    	}
    	
	appThread->continueExecution();	

	/* Wait for the mutatee to stop because of the instrumentation we just inserted. */
	RETURNONFAIL(waitUntilStopped( bpatch, appThread, 1, "getCallStack through instrumentation (exit)"));

	if( !checkStack( appThread, correct_frame_info,
			 sizeof(correct_frame_info)/sizeof(frameInfo_t),
			 3, "getCallStack through instrumentation (exit)" ) ) {
	    passedTest = false;
    	}

	if (passedTest)
	    printf("Passed test #3 (unwind through base and mini tramps)\n");

	/* Return the mutatee to its normal state. */
	appThread->continueExecution();	

        return passedTest;

#endif
} /* end mutatorTest3() */

// External Interface
extern "C" int mutatorMAIN(ParameterDict &param)
{
    bpatch = (BPatch *)(param["bpatch"]->getPtr());
    BPatch_thread *appThread = (BPatch_thread *)(param["appThread"]->getPtr());


    // Read the program's image and get an associated image object
    BPatch_image *appImage = appThread->getImage();

    // Run mutator code
    return mutatorTest(appThread, appImage);
}
