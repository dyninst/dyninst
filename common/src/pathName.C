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
#include <boost/filesystem.hpp>
#include <boost/algorithm/string/replace.hpp>
#include <boost/algorithm/string/trim.hpp>

#if defined(os_windows) //ccw 20 july 2000 : 29 mar 2001
	#define S_ISDIR(x) ((x) & _S_IFDIR)
#else
	#include <sys/types.h>
	#include <sys/stat.h>
	#include <unistd.h>
	#include <pwd.h>
#endif

std::string expand_tilde_pathname(const std::string &dir) {
#ifdef os_windows
	// no-op on Windows
	return dir;
#else
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

   std::string result = std::string(pwPtr->pw_dir) + (ptr ? ptr : "");
   endpwent();
   return result;
#endif
}

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

std::string extract_pathname_tail(const std::string &path)
{
    boost::filesystem::path p(path);
    return p.filename().string();
}

std::string resolve_file_path(char const* path) {
	return resolve_file_path(std::string{path});
}
std::string resolve_file_path(std::string path) {
	namespace ba = boost::algorithm;
	namespace bf = boost::filesystem;

	// Remove all leading and trailing spaces in-place.
	ba::trim(path);

#ifndef os_windows
	// On Linux-like OSes, collapse doubled directory separators
	// similar to POSIX `realpath`. On Windows, '//' is a (possibly)
	// meaningful separator, so don't change it.
	ba::replace_all(path, "//", "/");
#endif

	// If it has a tilde, expand tilde pathname
	// This is a no-op on Windows
	if(path.find("~") != std::string::npos) {
		path = expand_tilde_pathname(path);
	}

	// Convert to a boost::filesystem::path
	// This makes a copy of `path`.
	auto boost_path = bf::path(path);

	// bf::canonical (see below) requires that the path exists.
	if(!bf::exists(boost_path)) {
		return {};
	}

	/* Make the path canonical
	 *
	 * This converts the path to an absolute path (relative to the
	 * current working directory) that has no symbolic links, '.',
	 * or '..' elements and strips trailing directory separator.
	 *
	 * NOTE: makes a copy of the path.
	 */
	boost::system::error_code ec;
	auto canonical_path = bf::canonical(boost_path, ec);
	if(ec != boost::system::errc::success) {
		return {};
	}

	/* This is a bit strange, but is most optimal as it is the only
	 * member string-conversion function that does not inhibit moving
	 * the return value out (required here by C++11).
	 */
	return canonical_path.string<std::string>();
}
