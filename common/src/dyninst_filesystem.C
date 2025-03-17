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

#include "dyninst_filesystem.h"

#include <boost/algorithm/string/replace.hpp>
#include <boost/algorithm/string/trim.hpp>
#include <boost/filesystem.hpp>
#include <cstdlib>
#include <string>

#ifdef os_windows

namespace Dyninst {

  static std::string expand_tilde(std::string path_name) {
    return path_name;
  }

}

#else

#include <pwd.h>
#include <sys/types.h>
#include <unistd.h>
#include <vector>

namespace Dyninst { namespace filesystem {

  static std::string get_home_dir(const std::string& username = {}) {

    const auto size = []() -> long {
      auto const s = sysconf(_SC_GETPW_R_SIZE_MAX);
      if(s == -1) {
        return 16384;
      }
      return s;
    }();

    std::vector<char> buf(size);
    passwd pwd{};
    passwd* result{};

    // If no name is given, use current effective user
    if(username.empty()) {
      getpwuid_r(geteuid(), &pwd, buf.data(), size, &result);
    } else {
      getpwnam_r(username.c_str(), &pwd, buf.data(), size, &result);
    }

    if(!result) {
      // failed to find an entry
      return {};
    }

    return pwd.pw_dir;
  }

  // Replace unix `~` in a path with $HOME
  static std::string expand_tilde(std::string path_name) {
    if(path_name.empty() || path_name[0] != '~') {
      return path_name;
    }

    // ~/x -> $HOME/x
    // NOTE: '/x' is optional
    if(path_name.length() == 1UL || path_name[1] == '/') {

      // If 'HOME' is set, use it
      if(auto* home_dir = std::getenv("HOME")) {
        return path_name.replace(0, 1, home_dir);
      }

      // Otherwise, use current user's entry in the passwd file
      auto home = get_home_dir();
      if(!home.empty()) {
        return path_name.replace(0, 1, home);
      }

      // Failed to read the passwd file, so just return unexpanded path
      return path_name;
    }

    // ~NAME/foo -> passwd(NAME).pw_dir/foo
    // Note: NAME may not be the same as $USER
    const auto idx_of_slash = path_name.find('/');
    const auto user_name = path_name.substr(1, idx_of_slash - 1);

    // Find the user's entry in the passwd file
    auto const& home = get_home_dir(user_name);

    if(home.empty()) {
      // Failed to read the passwd file, so just return unexpanded path
      return path_name;
    }

    // Everything after ~NAME
    auto trailing_path = path_name.substr(user_name.length() + 1);

    namespace fs = boost::filesystem;
    auto full_path = fs::path(home) / trailing_path;
    return full_path.string();
  }

#endif

std::string extract_filename(const std::string& path) {
  boost::filesystem::path p(path);
  return p.filename().string();
}

std::string canonicalize(std::string path) {
  namespace ba = boost::algorithm;
  namespace bf = boost::filesystem;

  // Remove all leading and trailing spaces in-place.
  ba::trim(path);

  // Collapse multiple slashes and remove terminal slashes
  boost::algorithm::replace_all(path, "//", "/");

  // If it has a tilde, expand tilde pathname
  if(path.find('~') != std::string::npos) {
    path = expand_tilde(path);
  }

  // Convert to a boost::filesystem::path
  auto boost_path = bf::path(path);

  // bf::canonical (see below) requires that the path exists.
  if(!bf::exists(boost_path)) {
    return path;
  }

  /* Make the path canonical
   *
   * This converts the path to an absolute path (relative to the
   * current working directory) that has no symbolic links, '.',
   * or '..' elements and strips trailing directory separator.
   */
  boost::system::error_code ec;
  auto canonical_path = bf::canonical(boost_path, ec);
  if(ec != boost::system::errc::success) {
    return {};
  }

  return canonical_path.string();
}

}}
