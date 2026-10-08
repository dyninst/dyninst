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

#include <string>

namespace Dyninst { namespace filesystem {

  DYNINST_EXPORT std::string extract_filename(const std::string& path);

  DYNINST_EXPORT std::string canonicalize(std::string);

  /*
   *  True when process `pid` resolves absolute paths differently from the
   *  caller: another mount namespace or another root directory (a container or
   *  a chroot). False when that cannot be determined.
   */
  DYNINST_EXPORT bool has_distinct_filesystem_view(int pid);

  /*
   *  `path`, a name read from process `pid` (its link map, PT_INTERP or
   *  /proc/<pid>/exe), as the caller can open it. Without a distinct filesystem
   *  view this is canonicalize(path). With one, names under /proc/<pid>/ are
   *  returned unchanged and other absolute names become /proc/<pid>/root<path>.
   */
  DYNINST_EXPORT std::string canonicalize(std::string, int pid);

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
