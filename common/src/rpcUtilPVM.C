
//
// This file defines a set of utility routines for RPC services.
//
//
#include <stdlib.h>
#include <errno.h>
#include <assert.h>
#include <util/h/rpcUtilPVM.h>

//
// Starts the 'program' on 'machine' and saves its thread id
//
PVMrpc::PVMrpc(char *where, char *program, char **argv, int flag)
{
  pvm_error = 0;
  other_tid = -1;
  if ((my_tid = pvm_mytid()) < 0)
    pvm_error = -1;
  else 
    {
      if (pvm_spawn(program, argv, flag, where, 1, &other_tid) != 1)
	pvm_error = -1;
    }
}

//
// Accepts id of other thread
//
PVMrpc::PVMrpc(int other)
{
  if ((my_tid = pvm_mytid()) < 0)
    pvm_error = -1;
  else 
    pvm_error = 0;

  other_tid = other;
}

//
// parent is other
//
PVMrpc::PVMrpc()
{
  pvm_error = 0;
  if ((my_tid = pvm_mytid()) < 0)
    pvm_error = -1;
  else
    { if ((other_tid = pvm_parent()) < 0) pvm_error = -1;}
}

int
PVMrpc::readReady()
{
  int bufid, count;
  struct taskinfo *tp;

  if (pvm_error == -1) return -1;
  if ((bufid = pvm_probe(other_tid, -1)) < 0)
    return -1;
  else if (bufid >= 1)
    return 1;
  else if (other_tid == -1)
    return 0;
  else if ((pvm_tasks(other_tid, &count, &tp) < 0) ||
	   (count != 1) ||
	   (tp->ti_tid != other_tid))
    return (pvm_probe (other_tid, -1));
  else
    return 0;
}


