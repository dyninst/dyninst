
#ifndef RPC_UTIL_PVM
#define RPC_UTIL_PVM

#include <util/h/rpcUtil.h>

extern "C" {
#include <pvm3.h>
}

class PVMrpc {
  public:
    PVMrpc(char *where, char *program, char **argv, int flag);
    PVMrpc(int other);
    PVMrpc();
    void setNonBlock() { ; }
    inline int get_error() { return pvm_error;}
    inline int get_other_tid() { return other_tid;}
    inline void set_other_tid(int set_to) {other_tid = set_to;}
    int readReady();
  protected:
    int my_tid;
    int other_tid;
    int pvm_error;
};

#endif
