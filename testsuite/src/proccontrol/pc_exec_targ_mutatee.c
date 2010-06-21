#include <unistd.h>
#include <stdlib.h>

#define EXIT_CODE 4

int pc_exec_targ_mutatee()
{
   exit(EXIT_CODE);
   return 0;
}
