/*
 * Copyright (c) 1996-2004 Barton P. Miller
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

/* be sure to -I the dir where these files are... */
#include <pthread_sync.h>
#include <hashtbl.C>
#include <stdio.h>

using namespace std;

int main(int c, char* v[]) {
    hashtbl<int, const char*, pthread_sync> h;
    h.put(17, "seventeen");
    h.put(37, "thirty-seven");
    h.put(42, "forty-two");
    h.put(32, "thirty-two");
    h.put(22, "twenty-two");

    fprintf(stderr, "%d -> \"%s\"\n", 17, h.get(17));
    fprintf(stderr, "%d -> \"%s\"\n", 37, h.get(37));
    fprintf(stderr, "%d -> \"%s\"\n", 42, h.get(42));
    fprintf(stderr, "%d -> \"%s\"\n", 32, h.get(32));
    fprintf(stderr, "%d -> \"%s\"\n", 22, h.get(22));
    
    h.put(17, "Seventeen");
    h.put(37, "Thirty-Seven");
    h.put(42, "Forty-Two");
    h.put(32, "Thirty-Two");
    h.put(22, "Twenty-Two");
    
    fprintf(stderr, "%d -> \"%s\"\n", 17, h.get(17));
    fprintf(stderr, "%d -> \"%s\"\n", 37, h.get(37));
    fprintf(stderr, "%d -> \"%s\"\n", 42, h.get(42));
    fprintf(stderr, "%d -> \"%s\"\n", 32, h.get(32));
    fprintf(stderr, "%d -> \"%s\"\n", 22, h.get(22));

    return 0;
}
