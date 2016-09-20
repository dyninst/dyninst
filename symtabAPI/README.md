# SymtabAPI        

## Branch states

| Branch                                  | Status        | Notes                                              |
| --------------------------------------- |:-------------:|:--------------------------------------------------:|
| master                                  | stable        | No open issues                                     |

## Prerequisites

On Linux, Symtab requires some libraries that are not installed by default
in most distributions.  The first is libelf, which is included with most
distributions and can be installed using the distribution's package manager.
The second is libdwarf.  You can get the latest source code distribution
of libdwarf from http://sourceforge.net/projects/libdwarf/.  RPM files for libdwarf
are also available from various sites.  

## Building the SymtabAPI from source

See the top-level INSTALL file for how to build.

### Changes since 9.0

- Speed and space optimizations
- Fixes for bugs reading line information
- Fixes for bugs reading exception information

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

-----------------------------------------------------------------------------

