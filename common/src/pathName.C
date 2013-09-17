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

// pathName.C
#include "common/src/headers.h"  // P_strrchr()
#include <ctype.h>
#include <assert.h>
#include <limits.h>
#include "common/src/pathName.h"

#if defined(os_windows) //ccw 20 july 2000 : 29 mar 2001

#define S_ISDIR(x) ((x) & _S_IFDIR)

std::string expand_tilde_pathname(const std::string &dir) {
   return dir;
}

#else

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include <pwd.h>

std::string expand_tilde_pathname(const std::string &dir) {
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
         return std::string(home_dir) + &dir_cstr[2];
      else
         return std::string(home_dir) + &dir_cstr[1];
   }

   // It's the second case.  We need to find the username.  It starts at
   // dir_cstr[1] and ends at (but not including) the first '/' or '\0'.
   std::string userName;

   const char *ptr = strchr(&dir_cstr[1], '/');
   if (ptr == NULL)
      userName = std::string(&dir_cstr[1]);
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

   std::string result = std::string(pwPtr->pw_dir) + std::string(ptr);
   endpwent();
   return result;
}
#endif

static std::string concat_pathname_components_simple(const std::string &comp1, const std::string &comp2)
{
   std::string result = (comp1.length() ? comp1 : comp2);
   return result;
}

std::string concat_pathname_components(const std::string &comp1, const std::string &comp2)
{
   if (comp1.length() == 0 || comp2.length() == 0)
      return concat_pathname_components_simple(comp1, comp2);

   bool needToAddSlash = true; // so far

   // if comp1 ends in a "/" then no need to add slash
   const char *temp = comp1.c_str();
   if (temp[comp1.length()-1] == '/')
      needToAddSlash = false;

   // if comp2 begins with a "/" then no need to add slash
   if (comp2.length() && comp2[0] == '/')
	   needToAddSlash = false;
#if 0
   if (comp2.prefixed_by("/"))
      needToAddSlash = false;
#endif

   std::string result = comp1;
   if (needToAddSlash)
      result += "/";
   result += comp2;

   return result;
}

bool extractNextPathElem(const char * &ptr, std::string &result)
{
   // assumes that "ptr" points to the value of the PATH environment
   // variable.  Extracts the next element (writing to result, updating
   // ptr, returning true) if available else returns false;

   if ( ptr == NULL )
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

   result = std::string(start_ptr, len);

   // ptr now points at a ":" or end-of-string
   assert(*ptr == ':' || *ptr == '\0');
   if (*ptr == ':')
      ptr++;

//   cerr << "extractNextPathElem returning " << result << endl;

   return true;
}

bool exists_executable(const std::string &fullpathname)
{
   struct stat stat_buffer;
   int result = stat(fullpathname.c_str(), &stat_buffer);

   if (result == -1)
      return false;

   if (S_ISDIR(stat_buffer.st_mode))
      return false; // that was a directory, not an executable

   // more checks needed to be sure this is an executable file...

   return true;
}

bool executableFromArgv0AndPathAndCwd(std::string &result,
      const std::string &i_argv0,
      const std::string &path,
      const std::string &cwd) 
{
   // return true iff successful.
   // if successful, writes to result.
   // "path" is the value of the PATH env var
   // "cwd" is the current working directory, presumably from the PWD env var

   // 0) if argv0 empty then forget it
   if (i_argv0.length() == 0)
      return false;

   const std::string &argv0 = expand_tilde_pathname(i_argv0);

   // 1) If argv0 starts with a slash then we sink or swim with argv0

   if ((argv0.c_str())[0] == '/') 
   {
      if (exists_executable(argv0)) 
      {
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
   {
      if (*ptr++ == '/') 
      {
         contains_slash = true;
         break;
      }
   }

   if (!contains_slash) 
   {
      // search the path to see what directory argv0 came from.  If found, then
      // use dir + argv0 else use argv0.
      ptr = path.c_str();
      std::string pathelem;

      while (extractNextPathElem(ptr, pathelem)) 
      {
         std::string trystr = concat_pathname_components(pathelem, argv0);

         if (exists_executable(trystr)) 
         {
            result = trystr;
            return true;
         }
      }
   }

   // well, if we've gotten this far without success: couldn't find argv0 in the
   // path and argv0 wasn't a full path.  Last resort: try current directory + argv0
   std::string trystr = concat_pathname_components(cwd, argv0);

   if (exists_executable(trystr)) 
   {
      result = trystr;
      return true;
   }
   return false;
}

#if defined(os_windows)
#define PATH_SEP ('\\')
#define SECOND_PATH_SEP ('/')
#else
#define PATH_SEP ('/')
#endif

std::string extract_pathname_tail(const std::string &path)
{
	if (!path.length())
	{
		return std::string("");
	}

   const char *path_str = path.c_str();
   if (!path_str)
	{
		return std::string("");
	}

   const char *path_sep = P_strrchr(path_str, PATH_SEP);

#if defined(SECOND_PATH_SEP)
   const char *sec_path_sep = P_strrchr(path_str, SECOND_PATH_SEP);
   if (sec_path_sep && (!path_sep || sec_path_sep > path_sep))
      path_sep = sec_path_sep;
#endif

   std::string ret = (path_sep) ? (path_sep + 1) : (path_str);
   return ret;
}

#if !defined (os_windows)
static
char *resolve_file_path_local(const char *fname, char *resolved_path)
{
   // (1) realpath doesn't always return errors when the last element
   // of fname doesn't exist -- make sure it exists first to allow
   // consistent results of this function across platforms
   struct stat stat_buf;
   if( -1 == stat(fname, &stat_buf) ) {
       return NULL;
   }

   // (2)  use realpath() to resolve any . or ..'s, or symbolic links
   if (NULL == realpath(fname, resolved_path)) {
      return NULL;
   }

   // (3) if no slashes, try CWD
   if (!strpbrk(resolved_path, "/\\")) {
      char cwd[PATH_MAX];
      if (NULL == getcwd(cwd, PATH_MAX)) {
         return NULL;
      }
      char resolved_path_bak[PATH_MAX];
      strcpy(resolved_path_bak, resolved_path);
      sprintf(resolved_path, "%s/%s", cwd, resolved_path_bak);
   }

   // (4) if it has a tilde, expand tilde pathname
   if (!strpbrk(resolved_path, "~")) {
      std::string td_pathname = std::string(resolved_path);
      std::string no_td_pathname = expand_tilde_pathname(td_pathname);
      strcpy(resolved_path, no_td_pathname.c_str());
   }

   return resolved_path;
}

std::string resolve_file_path(const char *fname) {
    char path_buf[PATH_MAX];
    char *result = resolve_file_path_local(fname, path_buf);
    if ( result == NULL ) {
        return std::string();
    }
    std::string ret = result;
    return ret;
}

#else
std::string resolve_file_path(const char *fname) {
    assert(!"UNIMPLEMENTED ON WINDOWS");
    return std::string("");
}
#endif

