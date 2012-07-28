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

// $Id: test_stack_3.C,v 1.1 2008/10/30 19:22:22 legendre Exp $
/*
 * #Name: test8_3
 * #Desc: getCallStack through instrumentation
 * #Dep: 
 * #Arch: !(arch_alpha && os_osf_test)
 * #Notes:
 */

#include "BPatch.h"
#include "BPatch_Vector.h"
#include "BPatch_thread.h"
#include "BPatch_snippet.h"
#include "BPatch_point.h"

#include "test_lib.h"

#include "dyninst_comp.h"
class test_stack_3_Mutator : public DyninstMutator {
private:
  BPatch *bpatch;

public:
  virtual bool hasCustomExecutionPath() { return true; }
  virtual test_results_t setup(ParameterDict &param);
  virtual test_results_t executeTest();
};
extern "C" DLLEXPORT TestMutator *test_stack_3_factory() {
  return new test_stack_3_Mutator();
}

#if defined( DEBUG )
#include <sys/ptrace.h>
#endif
// static int mutatorTest( BPatch_thread * appThread, BPatch_image * appImage ) {
test_results_t test_stack_3_Mutator::executeTest() {
  bool passedTest;

  BPatch::bpatch->setInstrStackFrames(true);
  appProc->continueExecution();
  static const frameInfo_t correct_frame_info[] = {
	
#if defined( os_linux_test ) && (defined( arch_x86_test ) || defined( arch_x86_64_test ))
    { true, true, BPatch_frameNormal, "_dl_sysinfo_int80" },
#endif
#if defined( os_aix_test ) && defined( arch_power_test )
    /* AIX uses kill(), but the PC of a process in a syscall can
       not be correctly determined, and appears to be the address
       to which the syscall function will return. */
#elif defined( os_windows_test ) && (defined( arch_x86 ) || defined( arch_x86_64_test ))
    /* Windows/x86 does not use kill(), so its lowermost frame will be 
       something unidentifiable in a system DLL. */
    { false, false, BPatch_frameNormal, NULL },
#else
    { true, false, BPatch_frameNormal, "kill" },	
#endif
#if ! defined( os_windows_test )
    /* Windows/x86's stop_process_() calls DebugBreak(); it's 
       apparently normal to lose this frame. */
    { true, false, BPatch_frameNormal, "stop_process_" },
#endif
    { true, false, BPatch_frameNormal, "test_stack_3_func3" },
    { true, false, BPatch_frameTrampoline, NULL },
    /* On AIX and x86 (and others), if our instrumentation fires
       before frame construction or after frame destruction, it's 
       acceptable to not report the function (since, after all, it
       doesn't have a frame on the stack. */
    { true, true, BPatch_frameNormal, "test_stack_3_func2" },
    { true, false, BPatch_frameNormal, "test_stack_3_func1" },
    { true, false, BPatch_frameNormal, "test_stack_3_mutateeTest" },
    { true, false, BPatch_frameNormal, "main" }
  };
	
  /* Wait for the mutatee to stop in test_stack_3_func1(). */
  if (waitUntilStopped( bpatch, appProc, 1, "getCallStack through instrumentation") < 0) {
      appProc->terminateExecution();
    return FAILED;
  }
	
  /* Instrument test_stack_3_func2() to call test_stack_3_func3(), which will trip another breakpoint. */
  BPatch_Vector<BPatch_function *> instrumentedFunctions;	
  const char *fName = "test_stack_3_func2";
  appImage->findFunction(fName, instrumentedFunctions );
  if (instrumentedFunctions.size() != 1) {
    // FIXME Print out a useful error message
    logerror("**Failed** test_stack_3\n");
    logerror("    Unable to find function '%s'\n", fName);
    appProc->terminateExecution();
    return FAILED;
  }
	
  BPatch_Vector<BPatch_point *> * functionEntryPoints = instrumentedFunctions[0]->findPoint( BPatch_entry );
  if (functionEntryPoints->size() != 1) {
    // FIXME Print out a useful error message
    logerror("**Failed** test_stack_3\n");
    logerror("    Unable to find entry point to function '%s'\n", fName);
    appProc->terminateExecution();
    return FAILED;
  }
	
  BPatch_Vector<BPatch_function *> calledFunctions;
  const char *fName2 = "test_stack_3_func3";
  appImage->findFunction(fName2, calledFunctions );
  if (calledFunctions.size() != 1) {
    //FIXME Print out a useful error message
    logerror("**Failed** test_stack_3\n");
    logerror("    Unable to find function '%s'\n", fName2);
    appProc->terminateExecution();
    return FAILED;
  }
	
  BPatch_Vector<BPatch_snippet *> functionArguments;
  BPatch_funcCallExpr functionCall( * calledFunctions[0], functionArguments );
	
  appProc->insertSnippet( functionCall, functionEntryPoints[0] );

  /* Repeat for all three types of instpoints. */
  BPatch_Vector<BPatch_point *> * functionCallPoints = instrumentedFunctions[0]->findPoint( BPatch_subroutine );
  if (functionCallPoints->size() != 1) {
    logerror("**Failed** test_stack_3\n");
    logerror("    Unable to find subroutine call points in '%s'\n", fName);
    appProc->terminateExecution();
    return FAILED;
  }
  appProc->insertSnippet( functionCall, functionCallPoints[0] );
	
  BPatch_Vector<BPatch_point *> * functionExitPoints = instrumentedFunctions[0]->findPoint( BPatch_exit );
  if (functionExitPoints->size() != 1) {
    logerror("**Failed** test_stack_3\n");
    logerror("    Unable to find exit points in '%s'\n", fName);
    appProc->terminateExecution();
    return FAILED;
  }
  appProc->insertSnippet( functionCall, functionExitPoints[0] );

#if defined( DEBUG )
  for( int i = 0; i < 80; i++ ) { ptrace( PTRACE_SINGLESTEP, appThread->getPid(), NULL, NULL ); }
	
  for( int i = 80; i < 120; i++ ) {
    ptrace( PTRACE_SINGLESTEP, appThread->getPid(), NULL, NULL );
		
    BPatch_Vector<BPatch_frame> stack;
    appThread->getCallStack( stack );
		
    dprintf("single-step stack walk, %d instructions after stop for instrumentation.\n", i );
    for( unsigned i = 0; i < stack.size(); i++ ) {
      char name[ 40 ];
      BPatch_function * func = stack[i].findFunction();
		
      if( func == NULL ) { strcpy( name, "[UNKNOWN]" ); }
      else { func->getName( name, 40 ); }
			
      dprintf("  %10p: %s, fp = %p\n", stack[i].getPC(), name, stack[i].getFP() );
    } /* end stack walk dumper */
    dprintf("end of stack walk.\n" );
  } /* end single-step iterator */
#endif /* defined( DEBUG ) */		

  /* After inserting the instrumentation, let it be called. */
  appProc->continueExecution();
	  
  /* Wait for the mutatee to stop because of the instrumentation we just inserted. */
  if (waitUntilStopped( bpatch, appProc, 1, "getCallStack through instrumentation (entry)") < 0) {
      appProc->terminateExecution();
    return FAILED;
  }

  passedTest = true;
  if( !checkStack( appThread, correct_frame_info,
		   sizeof(correct_frame_info)/sizeof(frameInfo_t),
		   3, "getCallStack through instrumentation (entry)" ) ) {
    passedTest = false;
  }

  /* Repeat for other two types of instpoints. */
  appProc->continueExecution();

  /* Wait for the mutatee to stop because of the instrumentation we just inserted. */
  if (waitUntilStopped( bpatch, appProc, 1, "getCallStack through instrumentation (call)") < 0) {
      appProc->terminateExecution();
    return FAILED;
  }

  if( !checkStack( appThread, correct_frame_info,
		   sizeof(correct_frame_info)/sizeof(frameInfo_t),
		   3, "getCallStack through instrumentation (call)" ) ) {
    passedTest = false;
  }
    	
  appProc->continueExecution();

  /* Wait for the mutatee to stop because of the instrumentation we just inserted. */
  if (waitUntilStopped( bpatch, appProc, 1, "getCallStack through instrumentation (exit)") < 0) {
      appProc->terminateExecution();
    return FAILED;
  }

  if( !checkStack( appThread, correct_frame_info,
		   sizeof(correct_frame_info)/sizeof(frameInfo_t),
		   3, "getCallStack through instrumentation (exit)" ) ) {
    passedTest = false;
  }

  if (passedTest)
    logerror("Passed test #3 (unwind through base and mini tramps)\n");

  /* Return the mutatee to its normal state. */
  appProc->continueExecution();

  while (!appProc->isTerminated()) {
    // Workaround for issue with pgCC_high mutatee
    bpatch->waitForStatusChange();
  }

  if (passedTest)
    return PASSED;
  return FAILED;
} /* end mutatorTest3() */

// External Interface
test_results_t test_stack_3_Mutator::setup(ParameterDict &param) {
  DyninstMutator::setup(param);
  bpatch = (BPatch *)(param["bpatch"]->getPtr());
  return PASSED;
}
