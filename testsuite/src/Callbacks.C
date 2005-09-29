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

// $Id: Callbacks.C,v 1.1 2005/09/29 20:40:01 bpellin Exp $
#include "Callbacks.h"
#include "test_lib.h"

bool expectErrors = false;
bool gotError = false;
extern int expectError;
int errorPrint;
BPatch *bpatch;

void setBPatch(BPatch *bp)
{
  bpatch = bp;
}

void setExpectErrors(bool er) {
   expectErrors = er;
}

void clearError() {
   gotError = false;
}

bool getError() {
   return gotError;
}

// Set the expectError variable.  This variable is global to this library.
// It is used by the errorFunc call back to determine if errors are expected.
void setExpectError(int error)
{
   expectError = error;
}

void errorFunc(BPatchErrorLevel level, int num, const char **params)
{
    if (num == 0) {
        // conditional reporting of warnings and informational messages
        if (errorPrint) {
            if (level == BPatchInfo)
              { if (errorPrint > 1) printf("%s\n", params[0]); }
            else
                printf("%s", params[0]);
        }
    } else {
        // reporting of actual errors
        char line[256];
        const char *msg = bpatch->getEnglishErrorString(num);
        bpatch->formatErrorString(line, sizeof(line), msg, params);

        gotError = true;
        
        if (num != expectError) {
          if ( expectErrors ) {
            dprintf("Error (expected) #%d (level %d): %s\n", num, level, line);
          } else {
            if(num != 112)
               if ( errorPrint )
                  printf("Error #%d (level %d): %s\n", num, level, line);
        
               // We consider some errors fatal.
               if (num == 101) {
                  exit(-1);
               }
          }
        } else {
            dprintf("Error (expected) #%d (level %d): %s\n", num, level, line);
        }

    }

}

void createInstPointError(BPatchErrorLevel level, int num, const char **params)
{
    if (num != 117 && num != 118)
	errorFunc(level, num, params);
}

void setErrorPrint(int error) {
   errorPrint = error;
}

