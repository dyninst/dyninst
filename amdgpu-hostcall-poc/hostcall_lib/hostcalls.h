// Shared mailbox ABI for the GPU->CPU hostcall mechanism.
// Included by BOTH the device library (hostcall_lib.cpp) and the host-side server,
// so the struct layout must be identical on both sides. Plain POD — no HIP/host types.
//
// DESIGN: a bounded MPMC ring of independent slots (NOT a single shared cell behind a
// FIFO ticket lock). A single shared cell + FIFO ticket lock DEADLOCKS at high wave
// counts: only a few thousand of e.g. 16384 waves are GPU-resident at once, and a
// resident wave spinning for its strict-FIFO turn can be blocked by a non-resident
// predecessor that can never be scheduled (the resident waves spin forever holding the
// CUs). Here the only global atomic is `enqueue_pos` (wait-free); each wave then owns
// its OWN slot and the CPU services slots OUT OF ORDER, so a resident wave always makes
// progress. Slot reuse (waves > slots) is gated by a per-slot generation `turn`.

#ifndef HOSTCALLS_H
#define HOSTCALLS_H

#include <stdint.h>

#define HC_OP_NONE     0
#define HC_OP_FOPEN    1
#define HC_OP_FREAD    2
#define HC_OP_FWRITE   3
#define HC_OP_FCLOSE   4
#define HC_OP_WRITE_ID 5   // log a per-site scalar id (demonstrates argument passing)

#define HC_PATH_SIZE 128
#define HC_MODE_SIZE 8
#define HC_DATA_SIZE 512

#define HC_NSLOTS    256   // ring size; MUST be a power of two (index = pos & (N-1))

// One request/response cell. Owned exclusively by one wave per generation.
// status: 0 idle -> 1 request-ready(GPU) -> 2 done(CPU) -> 0 idle(GPU release)
// turn:   generation gate (GPU-only). slot i starts turn=i; a producer with ticket
//         `pos` may write only when turn==pos; on release it sets turn=pos+HC_NSLOTS.
struct HostcallSlot {
    uint32_t turn;                 // generation gate (producer waits for turn==pos)
    int32_t  status;               // request/response handshake (see above)
    int32_t  opcode;               // HC_OP_*
    int32_t  _pad0;
    int64_t  handle;               // fopen: OUT handle; fread/fwrite/fclose: IN handle
    int32_t  size;                 // fread/fwrite: IN requested byte count
    int32_t  retval;               // OUT: bytes transferred / error (<0)
    int32_t  arg;                  // WRITE_ID: IN per-site scalar id
    int32_t  _pad1;
    char     path[HC_PATH_SIZE];   // fopen: filename
    char     mode[HC_MODE_SIZE];   // fopen: mode ("w","r","a"...)
    char     data[HC_DATA_SIZE];   // fwrite: IN payload; fread: OUT payload
};

// The shared queue. Host allocates one in fine-grained memory and defines it as the
// `mailbox` symbol; init: enqueue_pos=0, slots[i].turn=i, slots[i].status=0.
struct HostcallQueue {
    uint32_t     enqueue_pos;      // producer cursor (atomicAdd handout)
    uint32_t     _pad[15];         // isolate the hot cursor from the slot array
    HostcallSlot slots[HC_NSLOTS];
};

// Back-compat alias: the symbol/type used to be `HostcallMailbox` (a single cell).
typedef HostcallQueue HostcallMailbox;

#endif // HOSTCALLS_H
