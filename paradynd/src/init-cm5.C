
/*
 * $Log: init-cm5.C,v $
 * Revision 1.9  1996/05/08 23:52:32  mjrg
 * changed function called on exit
 *
 * Revision 1.8  1996/03/01 22:31:57  mjrg
 * Replaced calls at the exit point by a call to DYNINSTexit
 *
 * Revision 1.7  1995/08/24 15:03:52  hollings
 * AIX/SP-2 port (including option for split instruction/data heaps)
 * Tracing of rexec (correctly spawns a paradynd if needed)
 * Added rtinst function to read getrusage stats (can now be used in metrics)
 * Critical Path
 * Improved Error reporting in MDL sematic checks
 * Fixed MDL Function call statement
 * Fixed bugs in TK usage (strings passed where UID expected)
 *
 * Revision 1.6  1995/05/18  10:33:47  markc
 * Removed resource predicate definitions
 * Removed metric defintions
 *
 * Revision 1.5  1995/02/16  08:53:10  markc
 * Corrected error in comments -- I put a "star slash" in the comment.
 *
 * Revision 1.4  1995/02/16  08:33:17  markc
 * Changed igen interfaces to use strings/vectors rather than char igen-arrays
 * Changed igen interfaces to use bool, not Boolean.
 * Cleaned up symbol table parsing - favor properly labeled symbol table objects
 * Updated binary search for modules
 * Moved machine dependnent ptrace code to architecture specific files.
 * Moved machine dependent code out of class process.
 * Removed almost all compiler warnings.
 * Use "posix" like library to remove compiler warnings
 *
 * Revision 1.3  1994/11/10  22:59:25  jcargill
 * Corrected slight "oops" in last commit with number of metrics defined
 *
 * Revision 1.2  1994/11/10  21:17:36  jcargill
 * Removed active_process from the list of all metrics; it's now an internal
 *
 * Revision 1.1  1994/11/01  16:55:52  markc
 * Environment specific initialization. (pvm, cm5, sun sequential)
 *
 */

#include "metric.h"
#include "internalMetrics.h"
#include "inst.h"
#include "init.h"
#include "ast.h"
#include "util.h"
#include "os.h"

// NOTE - the tagArg integer number starting with 0.  
static AstNode tagArg(Param, (void *) 1);

// for rexec
static AstNode cmdArg(Param, (void *) 4);

bool initOS() {

  sym_data sd;
  sd.name = "TRACELIBcurrPtr"; sd.must_find = true; syms_to_find += sd;
  sd.name = "TRACELIBfreePtr"; sd.must_find = true; syms_to_find += sd;
  sd.name = "TRACELIBendPtr"; sd.must_find = true; syms_to_find += sd;
  sd.name = "TRACELIBtraceBuffer"; sd.must_find = true; syms_to_find += sd;


  // TODO - are main, exit correct
  // assume no underscores
  // TODO - no CMRT ?

  initialRequests += new instMapping("main", "DYNINSTinit", FUNC_ENTRY);
  initialRequests += new instMapping("main", "DYNINSTexit", FUNC_EXIT);
  
  initialRequests += new instMapping(EXIT_NAME, "DYNINSTexit", FUNC_ENTRY);
  initialRequests += new instMapping("DYNINSTsampleValues", "DYNINSTreportNewTags",
				 FUNC_ENTRY);
  initialRequests += new instMapping("CMMD_send", "DYNINSTrecordTag",
				 FUNC_ENTRY | FUNC_ARG, &tagArg);
  initialRequests += new instMapping("CMMD_receive", "DYNINSTrecordTag", 
			 FUNC_ENTRY | FUNC_ARG, &tagArg);
  initialRequests += new instMapping("CMMD_receive_block", "DYNINSTrecordTag",
			 FUNC_ENTRY | FUNC_ARG, &tagArg);
  initialRequests += new instMapping("CMMD_send_block", "DYNINSTrecordTag",
			 FUNC_ENTRY | FUNC_ARG, &tagArg);
  initialRequests += new instMapping("CMMD_send_async", "DYNINSTrecordTag",
			 FUNC_ENTRY | FUNC_ARG, &tagArg);
  initialRequests += new instMapping("CMMD_receive_async", "DYNINSTrecordTag",
			  FUNC_ENTRY | FUNC_ARG, &tagArg);
  initialRequests += new instMapping("CMPE_CMCOM_pe_init", "DYNINSTinit", FUNC_ENTRY);
  initialRequests += new instMapping("pe_main_default", "DYNINSTexit", FUNC_EXIT);

  return true;
};
