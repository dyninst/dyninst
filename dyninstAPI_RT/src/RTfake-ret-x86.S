/*
 * Copyright (c) 1996 Barton P. Miller
 * 
 * We provide the Paradyn Parallel Performance Tools (below
 * described as "Paradyn") on an AS IS basis, and do not warrant its
 * validity or performance.  We reserve the right to update, modify,
 * or discontinue this software at any time.  We shall have no
 * obligation to supply such updates or modifications or any other
 * form of support to you.
 * 
 * This license is for research uses.  For such uses, there is no
 * charge. We define "research use" to mean you may freely use it
 * inside your organization for whatever purposes you see fit. But you
 * may not re-distribute Paradyn or parts of Paradyn, in any form
 * source or binary (including derivatives), electronic or otherwise,
 * to any other organization or entity without our permission.
 * 
 * (for other uses, please contact us at paradyn@cs.wisc.edu)
 * 
 * All warranties, including without limitation, any warranty of
 * merchantability or fitness for a particular purpose, are hereby
 * excluded.
 * 
 * By your use of Paradyn, you understand and agree that we (or any
 * other person or entity with proprietary rights in Paradyn) are
 * under no obligation to provide either maintenance services,
 * update services, notices of latent defects, or correction of
 * defects for Paradyn.
 * 
 * Even if advised of the possibility of such damages, under no
 * circumstances shall we (or any other person or entity with
 * proprietary rights in the software licensed hereunder) be liable
 * to you or any third party for direct, indirect, or consequential
 * damages of any character regardless of type of action, including,
 * without limitation, loss of profits, loss of use, loss of good
 * will, or computer failure or malfunction.  You agree to indemnify
 * us (and any other person or entity with proprietary rights in the
 * software licensed hereunder) for any and all liability it may
 * incur to third parties resulting from your use of Paradyn.
 */


/*
 * void *DYNINSTdlopen_fake_ret(const char *filename, int flag,
 *				const char *fake_ret);
 * dlopen on Suse 9.1 has a "security" check in it so that only
 * registered modules can call it. We fool this check around by
 * calling _dl_open(filename, flag, NULL) and pretending it was
 * called from libc (by pushing fake_ret on the stack). The
 * fake_ret argument should point to a ret instruction somewhere
 * in libc -- ret will pop another word from the stack and return to us.
 */
	.text
	.globl DYNINSTdlopen_fake_ret
DYNINSTdlopen_fake_ret:
	push	%ebp
	mov	%esp, %ebp
	push	$1f	 /* Push the proper ret addr on the stack */
	mov	0x8(%ebp),%eax
	mov	0xc(%ebp),%edx
	mov	0x10(%ebp),%ecx
	push	%ecx	 /* Push the provided addr of a ret insn in libc */
	xor	%ecx,%ecx
	jmp	_dl_open /* Will return to the pushed addr of ret when done */
1:		
	leave
	ret
