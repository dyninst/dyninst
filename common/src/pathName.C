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

// pathName.C

#include <pwd.h>
#include "util/h/pathName.h"

string expand_tilde_pathname(const string &dir) {
   // e.g. convert "~tamches/hello" to "/u/t/a/tamches/hello",
   // or convert "~/hello" to same.
   // In the spirit of Tcl_TildeSubst
   if (dir.length()==0)
      return dir;

   const char *dir_cstr = dir.string_of();
   if (dir_cstr[0] != '~')
      return dir;

   // Now, there are two possibilities: a tilde by itself (e.g. ~/x/y or ~), or
   // a tilde followed by a username.
   if (dir_cstr[1] == '/' || dir_cstr[1] == '\0') {
      // It's the first case.  We need to find the environment vrble HOME and use it.
      // It it doesn't exist (but it always does, I think) then I don't know what
      // to do.
      char *home_dir = getenv("HOME");
      if (home_dir == NULL)
         return dir; // yikes

      if (home_dir[strlen(home_dir)-1] == '/' && dir_cstr[1] != '\0')
         return string(home_dir) + &dir_cstr[2];
      else
         return string(home_dir) + &dir_cstr[1];
   }

   // It's the second case.  We need to find the username.  It starts at
   // dir_cstr[1] and ends at (but not including) the first '/' or '\0'.
   string userName;

   const char *ptr = strchr(&dir_cstr[1], '/');
   if (ptr == NULL)
      userName = string(&dir_cstr[1]);
   else {
      char user_name_buffer[200];
      unsigned user_name_len = ptr - &dir_cstr[1];

      for (unsigned j=0; j < user_name_len; j++)
	 user_name_buffer[j] = dir_cstr[1+j];

      user_name_buffer[user_name_len] = '\0';
      userName = user_name_buffer;
   }

   struct passwd *pwPtr = getpwnam(userName.string_of());
   if (pwPtr == NULL) {
      endpwent();
      return dir; // something better needed...
   }

   string result = string(pwPtr->pw_dir) + string(ptr);
   endpwent();
   return result;
}
