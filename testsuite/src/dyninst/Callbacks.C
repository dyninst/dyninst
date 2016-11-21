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

// $Id: Callbacks.C,v 1.1 2008/10/30 19:17:03 legendre Exp $

#if !defined(COMPLIB_DLL_BUILD)
#define COMPLIB_DLL_BUILD
#endif

#include "Callbacks.h"
#include "dyninst_comp.h"

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

void errorFunc(BPatchErrorLevel level, int num, const char * const *params)
{
    if (num == 0) {
        // conditional reporting of warnings and informational messages
        if (errorPrint) {
            if ((level == BPatchInfo) || (level == BPatchWarning) )
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

void createInstPointError(BPatchErrorLevel level, int num, const char * const *params)
{
    if (num != 117 && num != 118)
	errorFunc(level, num, params);
}

void setErrorPrint(int error) {
   errorPrint = error;
}

