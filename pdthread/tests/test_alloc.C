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

#include <iostream>
#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include "../dllist.C"

dllist<unsigned, pthread_sync> a_list;
dllist<unsigned, pthread_sync> b_list;

void* work1(void* arg) {
   for(int i = 0; i < 100000; i++) {
       a_list.put(37);
       b_list.yank(42);
       char* f = (char*)malloc(sizeof(char) * 13);
       a_list.put(37);
       b_list.yank(42);
       char* g = (char*)malloc(sizeof(char) * 13);
       a_list.put(37);
       free(f);
       b_list.yank(42);
       a_list.put(37);
       b_list.yank(42);
       free(g);
       fprintf(stderr, "%s: iteration %d complete\n",  arg, i);
   }
}

void* work2(void* arg) {
   for(int i = 0; i < 100000; i++) {
       char* f = (char*)malloc(sizeof(char) * 13);
       a_list.yank(37);
       b_list.put(42);
       a_list.yank(37);
       char* g = (char*)malloc(sizeof(char) * 13);
       b_list.put(42);
       free(f);
       a_list.yank(37);
       b_list.put(42);
       a_list.yank(37);
       free(g);
       b_list.put(42);       
       fprintf(stderr, "%s: iteration %d complete\n",  arg, i);
   }
}

int main(int c, char *v[]) {
    pthread_t first_thr;
    pthread_t second_thr;
    
    pthread_create(&first_thr, NULL, work1, (void*)"first thread");
    pthread_create(&second_thr, NULL, work2, (void*)"second thread");
    pthread_join(first_thr, NULL);
    pthread_join(second_thr, NULL);   
}


	    
