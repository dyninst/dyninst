#include <stdio.h>
#include <stdlib.h>

/* These are copied in test1.mutatee.c and libtestA.c */
#define MAGIC22_1   2200100
#define MAGIC22_2   2200200
#define MAGIC22_3   2200300
#define MAGIC22_4   2200400
#define MAGIC22_5A  2200510
#define MAGIC22_5B  2200520
#define MAGIC22_6   2200600
#define MAGIC22_7   2200700

extern int globalVariable22_1;
extern int globalVariable22_2;
extern int globalVariable22_3;
extern int globalVariable22_4;

void call22_5(int x)
{
     globalVariable22_3 += x;
     globalVariable22_3 += MAGIC22_5B;
}
/* function to make regex (test 21)search non-trivial */
void cbll21_1()
{
     printf("This function was not meant to be called!\n");
}
/* function to make regex (test 21)search non-trivial */
void cbll22_1()
{
     printf("This function was not meant to be called!\n");
}

/* function to make regex (test21) search non-trivial */
void acbll22_1()
{
     printf("This function was not meant to be called!\n");
}

/* Keep this function at the end of this file to kludgily ensure that
   its base address differs from its counterpart in libtestA.c */
void call21_1()
{
     printf("This function was not meant to be called!\n");
}

