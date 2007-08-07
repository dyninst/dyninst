#include <stdio.h>
#include <omp.h>
#include <stdlib.h> 
#include <unistd.h>

#define CHUNKSIZE 100
#define N 1000

void bar(int x)
{
    /*printf("In bar for thread %d\n",x);*/
}

void fooSections()
{
  int i;
  float a[N], b[N], c[N];

  for (i = 0; i < N; i++)
    a[i] = b[i] = i * 1.0;

#pragma omp parallel shared(a,b,c) private(i)
  {
#pragma omp sections nowait
    {
#pragma omp section
      for (i = 0; i < N/2; i++)
	c[i] = a[i] + b[i];
#pragma omp section
      for (i = N/2; i < N; i++)
	c[i] = a[i] + b[i];
      
    } /* end of sections */
  } /* end of parallel section */
}

void fooFor()
{
  int i, chunk;
  float a[N], b[N], c[N];

  for (i = 0; i < N; i++)
    a[i] = b[i] = i + 1.0;
  chunk = CHUNKSIZE;

#pragma omp parallel shared(a,b,c,chunk) private(i)
  {
#pragma omp for schedule(dynamic, chunk) nowait
    for (i = 0; i < N; i++)
      {
	c[i] = a[i] + b[i];
	/*printf("At part %d of loop in thread %d\n", i, omp_get_thread_num());*/
      }
  }
}

void fooSingle()
{
  
  int i;
  float a[N], b[N], c[N];

  for (i = 0; i < N; i++)
    a[i] = b[i] = i * 1.0;
   
#pragma omp parallel shared(a,b,c) private(i)
  {

#pragma omp single nowait 
     for (i = 0; i < N/2; i++)
	c[i] = a[i] + b[i];

  } 
}

void fooCritical()
{
  int x;
  x = 0;
  
#pragma omp parallel shared(x)
  {
#pragma omp critical
    x = x + 1;
    
  } /* end of parallel section */
}

void fooForCombined()
{

int i, chunk;
float a[N], b[N], c[N];

/* Some initializations */
for (i=0; i < N; i++)
  a[i] = b[i] = i * 1.0;
chunk = CHUNKSIZE;

#pragma omp parallel for \
   shared(a,b,c,chunk) private(i) \
   schedule(static,chunk)
  for (i=0; i < N; i++)
    c[i] = a[i] + b[i];

}

void fooSecCombined()
{
  
 int i;
  float a[N], b[N], c[N];

  for (i = 0; i < N; i++)
    a[i] = b[i] = i * 1.0;

#pragma omp parallel sections shared(a,b,c) private(i)
  {
#pragma omp section
      for (i = 0; i < N/2; i++)
	c[i] = a[i] + b[i];
#pragma omp section
      for (i = N/2; i < N; i++)
	c[i] = a[i] + b[i];
      
  } /* end of parallel section */
  
}


void fooMasterBarrier()
{
  int a[5], i;
  
#pragma omp parallel
  {
      /* Perform some computation.*/
#pragma omp for
    for (i = 0; i < 5; i++)
      a[i] = i * i;
    
    /*Print intermediate results.*/
#pragma omp master
    for (i = 0; i < 5; i++)
      {
          /*printf("a[%d] = %d\n", i, a[i]);*/
      }
    /* Wait.*/
#pragma omp barrier
    
    /* Continue with the computation.*/
#pragma omp for
    for (i = 0; i < 5; i++)
      a[i] += i;
  } 
}

void fooAtomic()
{
  int count = 0;
#pragma omp parallel
  {
#pragma omp atomic
    count++;
  }
  /*printf("Number of threads: %d\n", count);*/
}

void fooFlush()
{
  int sharedVar;


#pragma omp parallel shared(sharedVar)
  {
#pragma omp sections nowait
    {
#pragma omp section
      {
	sharedVar++;
#pragma omp flush(sharedVar)
      }
#pragma omp section
      {
	sharedVar++;
#pragma omp flush(sharedVar)
      }
      
    } /* end of sections */
  } /* end of parallel section */ 
}



void test(int first, int last) 
{
    int i;
#pragma omp for schedule(static) ordered
  for (i = first; i <= last; ++i) {
      /* Do something here.*/
    if (i % 2) 
      {
#pragma omp ordered 
	{	
	  int x;
	  x++;
	  /*printf("test() iteration %d\n", i);*/
	}
      }
  }
}

void test2(int iter) 
{
#pragma omp ordered
    iter++; /*printf("test2() iteration %d\n", iter);*/

}

void fooOrdered()
{
  int i;
#pragma omp parallel
  {
    test(1, 8);
#pragma omp for ordered
    for (i = 0 ; i < 5 ; i++)
      test2(i);
  }
}


int main()
{
  int nthreads, tid;
  
  /* Fork a team of threads giving them their own copies of variables */
#pragma omp parallel private(tid)
  {
    
    /* Obtain and print thread id */
    tid = omp_get_thread_num();
    printf("Hello World from thread = %d\n", tid);
    
    /* Only master thread does this */
    if (tid == 0) 
      {
	nthreads = omp_get_num_threads();
	printf("Number of threads = %d\n", nthreads);

      }
    bar(tid);
  }  /* All threads join master thread and terminate */ 
  
  fooSections();    
  fooFor();
  fooSingle();
  fooCritical();
  fooForCombined();
  fooSecCombined();
  fooMasterBarrier();
  fooAtomic();
  fooFlush();
  fooOrdered();
}
