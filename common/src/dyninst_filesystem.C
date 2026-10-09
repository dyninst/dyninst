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
#include <deque>
#include <string>

#ifdef os_windows

namespace Dyninst {

  static std::string expand_tilde(std::string path_name) {
    return path_name;
  }

}

#else

#include <pwd.h>
#include <sys/stat.h>
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
      if(getpwuid_r(geteuid(), &pwd, buf.data(), size, &result) != 0) {
        return {};
      }
    } else {
      if(getpwnam_r(username.c_str(), &pwd, buf.data(), size, &result) != 0) {
        return {};
      }
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

#ifdef os_linux
  // Check whether two paths resolve to different underlying files or directories.
  static bool different_file_identity(std::string const& lhs, std::string const& rhs) {
    struct stat lhs_stat{};
    struct stat rhs_stat{};
    return stat(lhs.c_str(), &lhs_stat) == 0 && stat(rhs.c_str(), &rhs_stat) == 0 &&
           (lhs_stat.st_dev != rhs_stat.st_dev || lhs_stat.st_ino != rhs_stat.st_ino);
  }
#endif

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

bool has_distinct_filesystem_view(int pid) {
#ifdef os_linux
  auto const proc = "/proc/" + std::to_string(pid);
  return different_file_identity(proc + "/ns/mnt", "/proc/self/ns/mnt") ||
         different_file_identity(proc + "/root", "/");
#else
  (void)pid;
  return false;
#endif
}

std::string canonicalize(std::string path, int pid) {
  if(!has_distinct_filesystem_view(pid)) {
    return canonicalize(std::move(path));
  }
  // The target has its own filesystem view (a container or a chroot). Paths
  // under /proc/<pid>/ already open the target's files. Other absolute paths
  // are the target's: resolve them inside /proc/<pid>/root. canonical() cannot
  // do that, because it follows the procfs links and absolute symlinks back to
  // this side of the mount namespace. If resolution fails, still open them
  // through /proc/<pid>/root: the same path on this side may be a different file.
  auto const proc = "/proc/" + std::to_string(pid) + "/";
  if(path.empty() || path[0] != '/' || path.compare(0, proc.size(), proc) == 0) {
    return path;
  }
  auto const root = proc + "root";
  auto resolved = resolve_in_root_fs(path, root);
  return resolved.empty() ? root + path : resolved;
}

// Resolve an absolute target path under root (e.g. /proc/<pid>/root), expanding
// symlinks component by component: canonical() would follow absolute links on the host.
// Return the resolved path with its root prefix, or an empty string on failure.
std::string resolve_in_root_fs(std::string const& path, std::string const& root) {
  namespace bf = boost::filesystem;
  bf::path const target(path), root_path(root);
  boost::system::error_code ec;
  if(!target.is_absolute() || !root_path.is_absolute() || !bf::is_directory(root_path, ec))
    return {};

  bf::path resolved = root_path;
  std::deque<bf::path> pending;
  // Put a symlink target before the path components still to visit.
  auto prepend_components = [&](bf::path const& value) {
    auto relative = value.relative_path();
    pending.insert(pending.begin(), relative.begin(), relative.end());
  };
  prepend_components(target);

  // Match Linux's symlink-following limit to bound cycles and excessively long chains.
  constexpr unsigned max_symlinks = 40;
  unsigned links = 0;

  while(!pending.empty()) {
    auto component = pending.front();
    pending.pop_front();

    if(component == ".")
      continue;

    // Process ".." after expanding links, and never walk above the target root.
    if(component == "..") {
      if(resolved != root_path)
        resolved = resolved.parent_path();
      continue;
    }

    // Inspect this component without following its link on the host.
    auto next = resolved / component;
    auto status = bf::symlink_status(next, ec);
    if(ec || !bf::exists(status))
      return {};

    if(bf::is_symlink(status)) {
      if(++links > max_symlinks)
        return {};

      auto destination = bf::read_symlink(next, ec);
      if(ec)
        return {};

      // Absolute links restart at the target root, not the tracer's '/'.
      if(destination.is_absolute())
        resolved = root_path;
      prepend_components(destination);
      continue;
    }

    // A nonfinal component must be a directory before resolving its children.
    if(!pending.empty() && !bf::is_directory(status))
      return {};
    resolved = next;
  }

  return resolved.string();
}

bool exists(std::string const& path) {
  return boost::filesystem::exists(path);
}

std::string strip_all_extensions(std::string const& path) {
  auto p = boost::filesystem::path(path);

  if(p.extension().empty()) {
    return path;
  }

  auto fn = p.parent_path();
  for(; !p.extension().empty(); p = p.stem());
  return (fn/p).string();
}

std::string replace_extension(std::string const& path, std::string const& val) {
  return boost::filesystem::path(path).replace_extension(val).string();
}

std::string append_filename_suffix(std::string const& path, std::string const& val) {
  auto p = boost::filesystem::path(path);

  if(!p.has_extension()) {
    return path + val;
  }

  auto filename = p.filename().string();
  auto loc = filename.find('.');
  auto &&new_filename = filename.substr(0, loc) + val + filename.substr(loc);
  return (p.parent_path() / new_filename).string();
}

}}
