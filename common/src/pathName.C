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

// pathName.C

#include <ctype.h>
#include "common/h/pathName.h"
#include "common/h/headers.h"  // P_strrchr()

#if defined(i386_unknown_nt4_0) || defined(mips_unknown_ce2_11) //ccw 20 july 2000 : 29 mar 2001

#define S_ISDIR(x) ((x) & _S_IFDIR)

pdstring expand_tilde_pathname(const pdstring &dir) {
   return dir;
}

#else

#include <pwd.h>

pdstring expand_tilde_pathname(const pdstring &dir) {
   // e.g. convert "~tamches/hello" to "/u/t/a/tamches/hello",
   // or convert "~/hello" to same.
   // In the spirit of Tcl_TildeSubst
   if (dir.length()==0)
      return dir;

   const char *dir_cstr = dir.c_str();
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
         return pdstring(home_dir) + &dir_cstr[2];
      else
         return pdstring(home_dir) + &dir_cstr[1];
   }

   // It's the second case.  We need to find the username.  It starts at
   // dir_cstr[1] and ends at (but not including) the first '/' or '\0'.
   pdstring userName;

   const char *ptr = strchr(&dir_cstr[1], '/');
   if (ptr == NULL)
      userName = pdstring(&dir_cstr[1]);
   else {
      char user_name_buffer[200];
      unsigned user_name_len = ptr - &dir_cstr[1];

      for (unsigned j=0; j < user_name_len; j++)
	 user_name_buffer[j] = dir_cstr[1+j];

      user_name_buffer[user_name_len] = '\0';
      userName = user_name_buffer;
   }

   struct passwd *pwPtr = getpwnam(userName.c_str());
   if (pwPtr == NULL) {
      endpwent();
      return dir; // something better needed...
   }

   pdstring result = pdstring(pwPtr->pw_dir) + pdstring(ptr);
   endpwent();
   return result;
}
#endif

static pdstring concat_pathname_components_simple(const pdstring &comp1, const pdstring &comp2) {
   pdstring result = (comp1.length() ? comp1 : comp2);
   return result;
}

pdstring concat_pathname_components(const pdstring &comp1, const pdstring &comp2) {
   if (comp1.length() == 0 || comp2.length() == 0)
      return concat_pathname_components_simple(comp1, comp2);

   bool needToAddSlash = true; // so far

   // if comp1 ends in a "/" then no need to add slash
   const char *temp = comp1.c_str();
   if (temp[comp1.length()-1] == '/')
      needToAddSlash = false;

   // if comp2 begins with a "/" then no need to add slash
   if (comp2.prefixed_by("/"))
      needToAddSlash = false;

   pdstring result = comp1;
   if (needToAddSlash)
      result += "/";
   result += comp2;

   return result;
}

bool extractNextPathElem(const char * &ptr, pdstring &result) {
   // assumes that "ptr" points to the value of the PATH environment
   // variable.  Extracts the next element (writing to result, updating
   // ptr, returning true) if available else returns false;

   if( ptr == NULL )
      return false;

   while (isspace(*ptr))
      ptr++;

   if (*ptr == '\0')
      return false;
   
   // collect from "ptr" upto but not including the next ":" or end-of-string
   const char *start_ptr = ptr;

   while (*ptr != ':' && *ptr != '\0')
      ptr++;

   unsigned len = ptr - start_ptr;

   result = pdstring(start_ptr, len);

   // ptr now points at a ":" or end-of-string
   assert(*ptr == ':' || *ptr == '\0');
   if (*ptr == ':')
      ptr++;

//   cerr << "extractNextPathElem returning " << result << endl;

   return true;
}

bool exists_executable(const pdstring &fullpathname) {
   struct stat stat_buffer;
   int result = stat(fullpathname.c_str(), &stat_buffer);
   if (result == -1)
      return false;

   if (S_ISDIR(stat_buffer.st_mode))
      return false; // that was a directory, not an executable

   // more checks needed to be sure this is an executable file...

   return true;
}

bool executableFromArgv0AndPathAndCwd(pdstring &result,
				      const pdstring &i_argv0,
				      const pdstring &path,
				      const pdstring &cwd) {
   // return true iff successful.
   // if successful, writes to result.
   // "path" is the value of the PATH env var
   // "cwd" is the current working directory, presumably from the PWD env var

   // 0) if argv0 empty then forget it
   if (i_argv0.length() == 0)
      return false;

   const pdstring &argv0 = expand_tilde_pathname(i_argv0);

   // 1) If argv0 starts with a slash then we sink or swim with argv0
   
   if ((argv0.c_str())[0] == '/') {
      if (exists_executable(argv0)) {
	 result = argv0;
	 return true;
      }
   }

   // 2) search the path, trying (dir + argv0) for each path component.
   //    But only search the path if argv0 doesn't contain any slashes.
   //    Why?  Because if it does contain a slash than at least one
   //    directory component prefixes the executable name, and the path
   //    is only supposed to be searched when an executable name is specifed
   //    alone.
   bool contains_slash = false;
   const char *ptr = argv0.c_str();
   while (*ptr != '\0')
      if (*ptr++ == '/') {
	 contains_slash = true;
	 break;
      }

   if (!contains_slash) {
      // search the path to see what directory argv0 came from.  If found, then
      // use dir + argv0 else use argv0.
      ptr = path.c_str();
      pdstring pathelem;
      while (extractNextPathElem(ptr, pathelem)) {
	 pdstring trystr = concat_pathname_components(pathelem, argv0);
	 
	 if (exists_executable(trystr)) {
	    result = trystr;
	    return true;
	 }
      }
   }

   // well, if we've gotten this far without success: couldn't find argv0 in the
   // path and argv0 wasn't a full path.  Last resort: try current directory + argv0
   pdstring trystr = concat_pathname_components(cwd, argv0);
   if (exists_executable(trystr)) {
      result = trystr;
      return true;
   }

   return false;
}


#if defined(i386_unknown_nt4_0) || defined(mips_unknown_ce2_11) //ccw 20 july 2000 : 29 mar 2001
#define PATH_SEP ('\\')
#else
#define PATH_SEP ('/')
#endif

pdstring extract_pathname_tail(const pdstring &path)
{
  const char *path_str = path.c_str();
  const char *path_sep = P_strrchr(path_str, PATH_SEP);
  pdstring ret = (path_sep) ? (path_sep + 1) : (path_str);
  return ret;
}
