/*
 * Copyright (c) 1996 Barton P. Miller
 * 
 * We provide the Paradyn Parallel Performance Tools (below
 * described as Paradyn") on an AS IS basis, and do not warrant its
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

// Contains definitions for functions which NT does not give you
// The argument given is returned here

char *optarg;
int P_getopt(int argc, char *argv[], const char *optstring)
{
  int i;
  char arg_found = (char) 0;
  static int lastArgSeen = 0;
  if (lastArgSeen >= argc) return EOF;
  for (i = lastArgSeen; i < argc; i++) {
    int opt_index = 0;
    char *nextArg = argv[i];
    if (nextArg[0] != '-') continue;
    for (opt_index = 0; opt_index < strlen(optstring); opt_index++)
      if (optstring[opt_index] == nextArg[1]) {
	arg_found = optstring[opt_index];
	// We may have an argument value
	if ((opt_index < strlen(optstring)-1) // overflow bad
	    && (optstring[opt_index+1] == ':')) {
	  // Argument. Either the last part of the string, or the 
	  // next item in the argv set
	  if (strlen(nextArg) > 2) { // Last part of the argument
	    optarg = (char *)(nextArg + 2);
	  }
	  else
	    if (i < argc - 1) {	      
	      optarg = argv[i + 1];
	    }
	    else {
	      optarg = NULL;
	    }
	} // argument found
	break;
      }
    if (opt_index == strlen(optstring)) // Nothing found
      break;
    if (arg_found)
      break;
  }
  lastArgSeen = i+1;
  return arg_found;
}
