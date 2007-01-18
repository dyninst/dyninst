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

// $Id: test1_41.C,v 1.3 2007/01/18 07:53:57 jaw Exp $
/*
 * #Name: test1_41
 * #Desc: Tests whether we lose line information running a mutatee twice
 * #Dep: 
 * #Arch:
 * #Notes:useAttach does not apply
 */

#include "BPatch.h"
#include "BPatch_Vector.h"
#include "BPatch_thread.h"
#include "BPatch_snippet.h"
#include "BPatch_statement.h"

// This test uses some Dyninst-internal stuff..
#include "../src/LineInformation.h"

#include "test_lib.h"

static BPatch_exitType expectedSignal = ExitedNormally;

static int debugPrint;

static const int iterations = 2;

static int mutatorTest(char *pathname, BPatch *bpatch)
{
    unsigned int n=0;
    const char *child_argv[5];
    child_argv[n++] = pathname;
    if (debugPrint) child_argv[n++] = const_cast<char*>("-verbose");
    child_argv[n++] = const_cast<char*>("-run");
    child_argv[n++] = const_cast<char*>("41");       // run test41 in mutatee
    child_argv[n++] = NULL;

    int counts[iterations];

    // Run the mutatee twice, querying line info each time & store the info
    for (n = 0; n < iterations; n++) {
        dprintf("Starting \"%s\"\n", pathname);
	BPatch_thread *thread = bpatch->createProcess(pathname, child_argv,
						      NULL);
        if (!thread) {
            logerror("*ERROR*: unable to create handle for executable\n", n);
            logerror("**Failed** test #41 (repeated line information)\n");
            return -1;
        }
        dprintf("Mutatee started, pid=%d\n", n, thread->getPid());

	BPatch_image *image = thread->getImage();
	if (!image) {
	  logerror("*ERROR*: unable to get image from thread\n");
	  logerror("**Failed** test #41 (repeated line information)\n");
	  return -1;
	}
	if (isMutateeFortran(image)) {
	  thread->getProcess()->terminateExecution();
	  logerror("Skipped test #41 (repeated line information)\n");
	  return 0;
	}

	BPatch_module *module = image->findModule("test1.mutatee.c", true);
	if (!module) {
	  logerror("*ERROR*: unable to get module from image\n");
	  logerror("**Failed** test #41 (repeated line information)\n");
	  return -1;
	}

	char buffer[16384]; // FIXME ugly magic number; No module name should be that long..
	module->getName(buffer, sizeof(buffer));

#if 0
	int statement_count = 0;
	for (LineInformation::const_iterator i = module->getLineInformation().begin();
	     i != module->getLineInformation().end();
	     i++, statement_count++)
	  { /* empty loop */ }
#endif
        BPatch_Vector<BPatch_statement> statements;
        bool res = module->getStatements(statements);
        if (!res) {
           fprintf(stderr, "%s[%d]:  getStatements()\n", __FILE__, __LINE__);
           abort();
        }

	counts[n] = statements.size();
	dprintf("Trial %d: found %d statements\n", n, statements.size());

	thread->getProcess()->terminateExecution();
    }

    // Make sure we got the same info each time we ran the mutatee
    int last_count = -1;
    for (int i = 0; i < iterations; i++) {
      if ((last_count >= 0) && (last_count != counts[i])) {
	logerror("*ERROR*: statement counts didn't match: %d vs. %d\n", last_count, counts[i]);
	logerror("**Failed** test #41 (repeated line information)\n");
	return -1;
      }
      last_count = counts[i];
    }

    logerror("Passed test #41 (repeated line information)\n");
    return 0;
}

extern "C" TEST_DLL_EXPORT int test1_41_mutatorMAIN(ParameterDict &param)
{
    BPatch *bpatch;
    char *pathname = param["pathname"]->getString();
    bpatch = (BPatch *)(param["bpatch"]->getPtr());
    debugPrint = param["debugPrint"]->getInt();

    // Get log file pointers
    FILE *outlog = (FILE *)(param["outlog"]->getPtr());
    FILE *errlog = (FILE *)(param["errlog"]->getPtr());
    setOutputLog(outlog);
    setErrorLog(errlog);

#if defined (sparc_sun_solaris2_4)
    // we use some unsafe type operations in the test cases.
    bpatch->setTypeChecking(false);
#endif

    return mutatorTest(pathname, bpatch);
}
