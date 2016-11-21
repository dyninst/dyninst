#include<stdio.h>
#include<pthread.h>


void func3(int i, int tid){
    printf("(%d)func3 %d\n", tid, i);
    if( i > 5 ) {
        while(1){
        };
        return ;
    }

    func3(++i, tid);
    return ;
}

void func2(int i, int tid){
    printf("(%d)func2 %d\n",tid, i);
    func3(++i, tid);
    return ;
}

void *thread(void *arg){
    int i = *(int *)arg;
    func2( i+2, i+1);
}

#define NUM_THREADS 2

void func1(int i){
    printf("func1 %d\n",i);

    pthread_t threads[NUM_THREADS];

    printf("spawn threads...\n");
    for( int m = 0; m < NUM_THREADS; m++){
        int res = pthread_create(&threads[m], NULL, thread, (void*)&m);
        if( res)
            printf("error\n");
    }

    func2(++i, 0);

    printf("join threads...\n");
    for( int m = 0; m < NUM_THREADS; m++){
        pthread_join(threads[m], NULL);
    }

    return ;
}

int main(){
    printf("call func1\n");
    func1(0);
    return 0;
}
