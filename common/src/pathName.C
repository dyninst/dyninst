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

#include "pathName.h"
#include <boost/algorithm/string/replace.hpp>
#include <boost/algorithm/string/trim.hpp>
#include <boost/filesystem.hpp>
#include <cstdlib>
#include <string>

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

std::string extract_pathname_tail(const std::string &path)
{
    boost::filesystem::path p(path);
    return p.filename().string();
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
