
#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <machine/vmparam.h>
#include <stdio.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/proc.h>
#include <sys/types.h>
#include <sys/times.h>
#include <sys/user.h>
#include <unistd.h>
#include <kvm.h>
#include <sys/time.h>
#include <sys/resource.h>

extern void perror (const char *MESSAGE);

/* The following functions were copied from the rtinst directory */
/************************************************************************
 * caddr_t DYNINSTprobeUarea(void)
 *
 * test to see if uarea of the process can be mapped.  if so, return
 * the address of the uarea, else return null.
************************************************************************/

static caddr_t probeUarea(void) {
  kvm_t*       kd;
  struct user* u;
  struct proc* p;

  if (!(kd = kvm_open(0, 0, 0, O_RDONLY, 0))) {
    perror("kvm_open");
    return 0;
  }

  if (!(p = kvm_getproc(kd, getpid()))) {
    perror("kvm_getproc");
    return 0;
  }

  if (!(u = kvm_getu(kd, p))) {
    perror("kvm_getu");
    return 0;
  }

  kvm_close(kd);
  return (caddr_t) (p->p_uarea);
}

/************************************************************************
 * struct rusage *mapUarea(void)
 *
 * map urea of the process into its accessible address space.  if
 * such a mapping is possible, set clock pointers for fast access,
 * else use slower system calls.
************************************************************************/

struct rusage *mapUarea(void) {
  int     ret;
  int     kmem;
  caddr_t uAddr;
  static struct user* u;
  struct rusage *r;

  uAddr = probeUarea();
  if (!uAddr) return NULL;

  if ((kmem = open("/dev/kmem", O_RDONLY, 0)) == -1) {
    perror("/dev/kmem");
    return NULL;
  }

  if ((ret = (int) mmap(0, sizeof(struct user), PROT_READ, MAP_SHARED,
			kmem, (off_t) uAddr)) == -1) {
    perror("mmap");
    return NULL;
  }

  u = (struct user *) ret;
  r = &(u->u_ru);
  return r;
}
