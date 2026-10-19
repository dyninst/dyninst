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

#ifndef DYNINST_COMMON_PATHNAME_H
#define DYNINST_COMMON_PATHNAME_H

#include "dyninst_visibility.h"
#include "dyntypes.h"

#include <string>

namespace Dyninst { namespace filesystem {

  DYNINST_EXPORT std::string extract_filename(const std::string& path);

  DYNINST_EXPORT std::string canonicalize(std::string);

  /*
   *  True when process `pid` resolves absolute paths differently from the
   *  caller: another mount namespace or another root directory (a container or
   *  a chroot). Also true when that cannot be determined, for example when
   *  /proc/<pid> cannot be read, so that the target's names are opened
   *  through /proc/<pid>/root rather than as paths in the caller's own
   *  filesystem view.
   */
  DYNINST_EXPORT bool has_distinct_filesystem_view(PID pid);

  /*
   *  `path`, a name read from process `pid` (its link map, PT_INTERP or
   *  /proc/<pid>/exe), as the caller can open it. Without a distinct filesystem
   *  view this is canonicalize(path). With one, names under /proc/<pid>/ are
   *  returned unchanged, and other absolute names become /proc/<pid>/root<path>
   *  with their symlinks resolved inside that root by resolve_in_root_fs()
   *  (e.g., where libc.so.6 is a link, it becomes libc-2.28.so), or unresolved if that fails.
   */
  DYNINST_EXPORT std::string canonicalize(std::string, PID pid);

  /*
   *  The name of process `pid`'s executable, as the caller can open it.
   *  Without a distinct filesystem view this is canonicalize("/proc/<pid>/exe"),
   *  the file's real path. With one it is the target's path as canonicalize()
   *  translates it (/proc/<pid>/root/usr/bin/app), so each program the pid runs
   *  gets its own name, and canonicalize(name, pid) keeps it unchanged. It falls
   *  back to /proc/<pid>/exe if the file was deleted or no such name reaches it.
   */
  DYNINST_EXPORT std::string executable_path(PID pid);

  /*
   *  True when `path` is under /proc/<pid>/ for a process with a distinct
   *  filesystem view: a name canonicalize(path, pid) or executable_path(pid)
   *  gave. Such a name already opens the target's file, and canonicalize(path)
   *  or realpath() would follow /proc/<pid>/root or /proc/<pid>/exe back into
   *  the caller's own filesystem view, where the same path may be another file.
   *
   *  Callers must keep such a name exactly as it is.
   */
  DYNINST_EXPORT bool is_rooted_target_path(std::string const& path);

  /*
   *  `path`, an absolute name in the filesystem rooted at `root`, with every
   *  symlink resolved inside `root`: absolute links restart at `root`, and ".."
   *  never goes above it. The result keeps the `root` prefix. It is empty if
   *  the name cannot be resolved: a missing component, a non-directory before
   *  the last component, or more than 40 links.
   */
  DYNINST_EXPORT std::string resolve_in_root_fs(std::string const& path, std::string const& root);

  DYNINST_EXPORT bool exists(std::string const& path);

  DYNINST_EXPORT std::string replace_extension(std::string const& path, std::string const& val);

  DYNINST_EXPORT std::string strip_all_extensions(std::string const& path);

  /*
   *  Append the suffix `val` to `path` while maintaining all extensions
   *
   *  e.g., append_filename_suffix("foo/bar.txt.tar", "_sfx") returns "foo/bar_sfx.txt.tar"
   */
  DYNINST_EXPORT std::string append_filename_suffix(std::string const& path, std::string const& val);

}}

#endif
