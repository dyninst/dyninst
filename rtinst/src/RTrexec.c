
/*
 * Code to trap rexec call and munge command.
 *
 */
#include <sys/types.h>
#include <stdio.h>
#include <malloc.h>
#include <stdlib.h>

char *realCommand;

int DYNINSTrexec(char *cmd)
{
     char *pdArgs;

     pdArgs = (char *) getenv("PARADYN_MASTER_INFO");
     if (!pdArgs) {
	 printf("unable to get PARADYN_MASTER_INFO\n");
	 return (-1);
     }
     realCommand = (char *)  malloc(strlen(pdArgs)+strlen(cmd)+20);
     sprintf(realCommand, "paradynd %s -runme %s", pdArgs, cmd);
     printf("CMD = %s\n", realCommand);

     /* XXX - HACK !!! */
     /* Get Back previous argument list */
     /* We need to load the 5th argument to rexec with our version of cmd */
#if defined(sparc_sun_sunos4_1_3)
     asm("restore");
     asm("sethi	%hi(_realCommand), %l0");
     asm("or	%l0, %lo(_realCommand), %l0");
     asm("ld	[%l0], %i4");
     asm("save  %sp, -144, %sp");
#endif
#if defined(rs6000_ibm_aix3_2)
     {
	 register int temp asm("r10");
	 register volatile char *cmdPtr asm("r9") = realCommand;
	 asm("oriu  10, 1, 0");
	 asm("cal 1, 80(1)");	/* restore old stack pointer */
	 asm("ai 1, 1, 184");	/* add previous offset back */
	 asm("st 9, -36(1)");	/* get correct value */
	 asm("oriu  1, 10, 0");
      }
#endif
     return(0);
}
