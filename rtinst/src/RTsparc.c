
/*
 * This file contains the implementation of runtime dynamic instrumentation
 *   functions for a normal Sparc with SUNOS.
 *
 * $Log: RTsparc.c,v $
 * Revision 1.3  1994/02/02 00:46:12  hollings
 * Changes to make it compile with the new tree.
 *
 * Revision 1.2  1993/12/13  19:47:52  hollings
 * corrected rindex parameter error
 *
 * Revision 1.1  1993/08/26  19:43:28  hollings
 * Initial revision
 *
 * Revision 1.1  1993/07/02  21:49:35  hollings
 * Initial revision
 *
 *
 */
#include <stdio.h>
#include <string.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <sys/user.h>
#include <sys/proc.h>
#include <machine/vmparam.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <assert.h>
#include <unistd.h>
#include <kvm.h>

int DYNINSTmappedUarea;
int *_p_1, *_p_2;
static int _kmem = -1;

/*
 * find out if the uarea is safe for mapping.
 *    If it is, return the address of the uarea.
 *
 */
caddr_t DYNINSTprobeUarea()
{
     int pid;
     char *cmd;
     kvm_t *kd;
     char **args;
     struct user *u;
     struct proc *p;

     kd = kvm_open(NULL, NULL, NULL, O_RDONLY, NULL);
     if (!kd) return(0);
     pid = getpid();
     p = kvm_getproc(kd, pid);
     if (!p) return(0);
     u = kvm_getu(kd, p);
     if (!u) return(0);

     kvm_getcmd(kd, p, u, &args, NULL);
     if (cmd = (char *) rindex(args[0], '/')) {
	 cmd++;
     } else {
	 cmd = args[0];
     }
     if (strcmp(cmd, u->u_comm)) {
	 return(0);
     }
     kvm_close(kd);

     return((caddr_t)p->p_uarea);
}

int DYNINSTmapUarea()
{
    int ret;
    caddr_t uAddr;
    static struct user *u;

    uAddr = DYNINSTprobeUarea();

    if (!uAddr) {
	printf("WARNING: program compiled for wrong version of SPARC chip.\n");
	printf(" using getrusage for times, this may slow your program down\n");
	printf(" by a factor of ten or more.\n");
	return(0);
    }

    _kmem = open("/dev/kmem", O_RDONLY, 0);
    if (_kmem < 0) {
	return(0);
    }

    ret = (int) mmap((caddr_t) 0, sizeof(struct user), PROT_READ, 
	MAP_SHARED, _kmem, (off_t) uAddr);
    if (ret == -1) {
	return(0);
    }
    u = (struct user *) ret;
    _p_1 = (int *) &(u->u_ru.ru_utime.tv_sec);
    _p_2 = (int *) &(u->u_ru.ru_utime.tv_usec);

    return(1);
}
