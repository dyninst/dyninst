# ParseAPI

## Branch states

| Branch                                  | Status        | Notes                                              |
| --------------------------------------- |:-------------:|:--------------------------------------------------:|
| master                                  | stable        | No open issues                                     |

This is release 9.1 of the ParseAPI. Currently, this library is
available for the x86[_64]/Linux, x86/Windows, and
POWER/Linux platforms.

Documentation for the API can be found at

    http://www.dyninst.org/

as well as in the doc/ subdirectory.


## Prerequisites

The ParseAPI is a component of the Dyninst system, and depends on two other
components: the SymtabAPI binary format library and the InstructionAPI
disassembly library. Both of these components should have been included in the
package containing the ParseAPI; if not, they can be obtained from the
dyninst.org website.

The SymtabAPI has further dependencies depending on the platform, including
libelf and libdwarf. See the SymtabAPI documentation for details.


## Compiling the library

See the top-level INSTALL file for build information.

-----------------------------------------------------------------------------

This software is derived from the Paradyn system and therefore subject to
the same copyright.  A copy of the Paradyn copyright appears at the end of
this file.

	The Dyninst API Team
	12/9/2015

----------------------- Start of Paradyn Copyright --------------------------
See the dyninst/COPYRIGHT file for copyright information.
 
We provide the Paradyn Tools (below described as "Paradyn")
on an AS IS basis, and do not warrant its validity or performance.
We reserve the right to update, modify, or discontinue this
software at any time.  We shall have no obligation to supply such
updates or modifications or any other form of support to you.
 
By your use of Paradyn, you understand and agree that we (or any
other person or entity with proprietary rights in Paradyn) are
under no obligation to provide either maintenance services,
update services, notices of latent defects, or correction of
defects for Paradyn.

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA

