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

// $Id: test2_1.C,v 1.1 2008/10/30 19:20:09 legendre Exp $
/*
 * #Name: test_pt_ls
 * #Desc: Run parseThat on ls
 * #Dep: 
 * #Arch: all but windows
 * #Notes:
 */

#include <cstdio>
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
	  runmode = (create_mode_t) param["createmode"]->getInt();
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
    const char *binedit_dir = get_binedit_dir();
    std::string prefix = std::string(binedit_dir) + std::string("/test_pt_ls");
    std::string bin_outfile;
    ParseThat parseThat;
    parseThat.pt_output_redirect(prefix + std::string("_output1"));
    parseThat.cmd_stdout_redirect(prefix + std::string("_stdout1"));
    parseThat.cmd_stderr_redirect(prefix + std::string("_stderr1"));
    if (monitor) parseThat.measure_usage();

    ParseThat parseThat2;
    parseThat2.pt_output_redirect(prefix + std::string("_output2"));
    parseThat2.cmd_stdout_redirect(prefix + std::string("_stdout2"));
    parseThat2.cmd_stderr_redirect(prefix + std::string("_stderr2"));

    switch (runmode) {
    case CREATE:
        break;
    case USEATTACH:
        return SKIPPED;
    case DISK:
        bin_outfile = prefix + std::string("_mutatee_out");
        parseThat2.use_rewriter(bin_outfile);
        break;
    default:
        fprintf(stderr, "%s[%d]:  bad runmode %d\n", FILE__, __LINE__,
                runmode);
        return FAILED;
    };

    std::string cmd("/bin/ls");

    std::vector<std::string> args;
    args.push_back(std::string("/"));

    test_results_t res;
    if (runmode == CREATE) {
        res = parseThat(cmd, args);
    } else if (runmode == DISK) {
        res = parseThat2(cmd, args);

        if (res == PASSED) {
            // parseThat2 _should_ execute the rewritten binary, but we
            // also want to sanity check the execution.  At this (early)
            // point, this is really just a crash detector since the
            // parseThat output will not be present in the execution of
            // the cmd when run w/out parseThat

            std::string stdout3(prefix + std::string("_stdout3"));
            std::string stderr3(prefix + std::string("_stderr3"));
            res = ParseThat::sys_execute(bin_outfile, args, stdout3, stderr3);
        }
    }

    if (res == PASSED && monitor) {
        // Memory and CPU from parseThat went to stdout, which is now
        // saved in XXXX__output1.  Read the file, get the values, and
        // store them in the variable "test".
        std::string filename = prefix + std::string("_output1");
        FILE *fp = fopen(filename.c_str(), "r");
        if (fp) {
            timeval c;
            unsigned long m;
            char buf[4096] = {0}, *ptr;

            // We only need to check the tail end of the output file.
            fseek(fp, 0, SEEK_END);
            if (fseek(fp, -(sizeof(buf)-1), SEEK_CUR) != 0) rewind(fp);
            fread(buf, sizeof(char), sizeof(buf)-1, fp);

            ptr = strstr(buf, "CPU:");
            if (ptr && sscanf(ptr, "CPU: %ld.%ld", &c.tv_sec, &c.tv_usec)==2) {
                monitor->set(c);
                monitor->complete();
            }

            ptr = strstr(buf, "MEMORY:");
            if (ptr && sscanf(ptr, "MEMORY: %lu", &m)==1) {
                monitor->set(m);
                monitor->complete();
            }

            fclose(fp);
        }
    }

    return res;
}
