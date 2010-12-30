#if !defined(COMMUNICATION_H_)
#define COMMUNICATION_H_

#define MAX_POSSIBLE_THREADS 512
#define DEFAULT_NUM_THREADS 8
#define DEFAULT_NUM_PROCS 8

#if defined(__cplusplus)
extern "C" {
#endif

#if defined(os_linux_test)
#include <stdint.h>
#endif

#define SEND_PID_CODE 0xBEEF0001
typedef struct {
   uint32_t code;
   pid_t pid;
} send_pid;

#define HANDSHAKE_CODE 0xBEEF0002
typedef struct {
   uint32_t code;
} handshake;

#define ALLOWEXIT_CODE 0xBEEF0003
typedef struct {
   uint32_t code;
} allow_exit;

#define SENDADDR_CODE 0xBEEF0004
typedef struct {
  uint32_t code;
  uint32_t dummy;
  uint64_t addr;
} send_addr;

#define SYNCLOC_CODE 0xBEEF0005
typedef struct {
  uint32_t code;
} syncloc;

#define FORKINFO_CODE 0xBEEF0006
typedef struct {
   uint32_t code;
   uint32_t pid;
   uint32_t is_threaded;
   uint32_t is_done;
} forkinfo;

#define THREADINFO_CODE 0xBEEF0007
typedef struct {
   uint64_t code;
   uint64_t pid;
   uint64_t lwp;
   uint64_t tid;
   uint64_t a_stack_addr;
   uint64_t initial_func;
   uint64_t tls_addr;
} threadinfo;

#if defined(__cplusplus)
}
#endif

#endif
