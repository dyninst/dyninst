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

// $Id: test2_1.C,v 1.1 2008/10/30 19:20:09 legendre Exp $
/*
 * #Name: test_pt_ls
 * #Desc: Run parseThat on ls
 * #Dep: 
 * #Arch: all but windows
 * #Notes:
 */

#include "BPatch.h"
#include "BPatch_Vector.h"
#include "BPatch_thread.h"
#include "BPatch_snippet.h"

#include "test_lib.h"
#include "Callbacks.h"
#include "dyninst_comp.h"
#include "ParseThat.h"

class test_pt_ls_Mutator : public DyninstMutator 
{
  virtual bool hasCustomExecutionPath() { return true; }

  virtual test_results_t setup(ParameterDict &param)
  {
	  runmode = (create_mode_t) param["useAttach"]->getInt();
	  return PASSED;
  }

  virtual test_results_t executeTest();
};

extern "C" DLLEXPORT TestMutator *test_pt_ls_factory() 
{
  return new test_pt_ls_Mutator();
}

test_results_t test_pt_ls_Mutator::executeTest() 
{

	std::string prefix = std::string(BINEDIT_DIR) + std::string("/test_pt_ls");
	std::string bin_outfile;
	ParseThat parseThat;
	parseThat.pt_output_redirect(prefix + std::string("_output1"));
	parseThat.cmd_stdout_redirect(prefix + std::string("_stdout1"));
	parseThat.cmd_stderr_redirect(prefix + std::string("_stderr1"));

	ParseThat parseThat2;
	parseThat2.pt_output_redirect(prefix + std::string("_output2"));
	parseThat2.cmd_stdout_redirect(prefix + std::string("_stdout2"));
	parseThat2.cmd_stderr_redirect(prefix + std::string("_stderr2"));

	switch (runmode)
	{
		case CREATE:
			break;
		case USEATTACH:
			return SKIPPED;
		case DISK:
			bin_outfile = prefix + std::string("_mutatee_out");
			parseThat2.use_rewriter(bin_outfile);
			break;
		default:
			fprintf(stderr, "%s[%d]:  bad runmode %s\n", FILE__, __LINE__, runmode);
			return FAILED;
	};

	std::string cmd("/bin/ls");

	std::vector<std::string> args;
	args.push_back(std::string("/"));

	test_results_t res = parseThat(cmd, args);

	if (res != PASSED)
		return res;

	if (runmode == DISK)
		res = parseThat2(cmd, args);

	return res;
}

