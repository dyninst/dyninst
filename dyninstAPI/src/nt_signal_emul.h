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

// $Id: nt_signal_emul.h,v 1.3 2004/03/23 01:12:06 eli Exp $

#ifndef _nt_signal_emul_h_
#define _nt_signal_emul_h_

#define SIGEM_EVENTMASK		0xff00
#define SIGEM_SIGNALED		0x100
#define SIGEM_EXITED		0x200
#define SIGEM_STOPPED		0x300

#define SIGEM_SIGMASK		0xff

#define WIFSIGNALED(x)		(((x) & SIGEM_EVENTMASK) == SIGEM_SIGNALED)
#define WIFEXITED(x)		(((x) & SIGEM_EVENTMASK) == SIGEM_EXITED)
#define WIFSTOPPED(x)		(((x) & SIGEM_EVENTMASK) == SIGEM_STOPPED)

#define WSTOPSIG(x)	((x) & SIGEM_SIGMASK)
#define WTERMSIG(x)	((x) & SIGEM_SIGMASK)

#endif _nt_signal_emul_h_
