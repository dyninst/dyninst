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


	    
