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

/* $Id: RTtext-power.S,v 1.5 2006/06/05 22:30:20 bernat Exp $
 * create an array to use as a heap for new instructions.
 *
 *    This file is required since AIX has instructions and data are in
 *       different segments that are not reachable with a PC relative
 *       branch.  On other systems this would not be a good idea since it
 *       means that the dyninst code would have to be inserted at the first 
 *       link (since the data and text segments are next to each other).
 *       However, AIX permits incremental re-linking and can insert new
 *       code into a existing text segment.
 */

.csect .text[PR]
	.globl DYNINSTstaticHeap_16M_textHeap_1
	.globl .DYNINSTstaticHeap_16M_textHeap_1
DYNINSTstaticHeap_16M_textHeap_1:
.DYNINSTstaticHeap_16M_textHeap_1:
	.space	16777216
