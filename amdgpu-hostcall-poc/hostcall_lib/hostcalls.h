// Shared mailbox ABI for the GPU->CPU hostcall mechanism.
// This header is included by BOTH the device library (hostcall_lib.cpp) and the
// host-side server, so the struct layout must be identical on both sides.
// It is plain POD — no HIP/host-specific types.
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

// status transitions: 0 idle -> 1 request-ready(GPU) -> 2 done(CPU) -> 0 idle
struct HostcallMailbox {
    uint32_t ticket_next;          // GPU atomically "takes a number"
    uint32_t ticket_now;           // CPU-served ticket; GPU spins until ticket_now==mine
    int32_t  opcode;               // HC_OP_*
    int32_t  status;               // handshake flag (see above)

    int64_t  handle;               // fopen: OUT file handle; fread/fwrite: IN handle
    int32_t  size;                 // fread/fwrite: IN requested byte count
    int32_t  retval;               // OUT: bytes transferred / error (<0)
    int32_t  arg;                  // WRITE_ID: IN per-site scalar id (call argument)

    char     path[HC_PATH_SIZE];   // fopen: filename
    char     mode[HC_MODE_SIZE];   // fopen: mode string ("w","r","a"...)
    char     data[HC_DATA_SIZE];   // fwrite: IN payload; fread: OUT payload
};

#endif // HOSTCALLS_H
