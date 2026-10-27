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

/*
 * How a test reports that it cannot run: it prints "skipped: <reason>" and
 * exits with skipped_exit_code (77), which ctest reports as skipped. The
 * exit code decides; the text gives the reason. Tests are written in C++.
 *
 *   return skip_message("HOME is not set");
 */

#ifndef DYNINST_TEST_SKIP_H
#define DYNINST_TEST_SKIP_H

#include <iostream>
#include <string>

constexpr int skipped_exit_code = 77;

// Prints "skipped: <reason>"; return the result from main
inline int skip_message(std::string const& reason)
{
  std::cout << "skipped: " << reason << std::endl;
  return skipped_exit_code;
}

// For code that tests the library's Linux implementation (os_linux)
inline int skip_without_linux(std::string const& needs)
{
  return skip_message("needs Linux: " + needs);
}

#endif
