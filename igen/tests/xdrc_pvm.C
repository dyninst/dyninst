
// xdr client pvm client/server

// xdr client, pvm server
// this is started by 'paradyn' and starts the other paradynd's
//

// xdr client, pvm client
// started by pvm server, attempts to establish socket to xdr server
// by passing messages to pvm server
//

#include <stdio.h>
#include "xdr_link.CLNT.h"
#include <util/h/list.h>
#include <pvm3.h>
#include <string.h>
#include <assert.h>

static int started_by_paradynd(char *m, int f, int t, int s);
static int get_list_of_pvm_clients(char *m, int f, int t, int s, int flag);
static xdr_linkUser *xdr_link_conn= 0;
int kidid;

main (int argc, char *argv[])
{
  // is this the first 'paradyn' --> no args passed in
  // else, this is not the first paradyn
  int family, type, well_known_socket, flag;
  char *machine;
  int tid;

  tid = pvm_parent();
  if (tid != PvmNoParent)
    {
      pvm_recv (tid, -1);
    }
  RPC_undo_arg_list (argc, argv, &machine, &family, &type, &well_known_socket, &flag);

  if (flag)
    {
      // the first paradynd started
      // start others and act as server until they all report
      pvm_perror("In first\n");
      xdr_link_conn = new xdr_linkUser (0, NULL, NULL);
      xdr_link_conn->connected("from first\n");


      if (get_list_of_pvm_clients(machine, family, type, well_known_socket, 0)
	  != 0)
	{
	  // there was an error starting the remote paradynd's, exit
	  assert (0);
	}
      pvm_initsend(0);
      pvm_send(kidid, 0);
      delete (machine);
      delete (xdr_link_conn);
      exit (0);
    }
  else
    {
      pvm_perror("In else\n");
      // talk to paradyn through 'first' paradynd
      if (started_by_paradynd(machine, family, type, well_known_socket) == -1)
	assert (0);
    }

  delete(xdr_link_conn);
  // do normal paradynd behavior here
  exit(0);
}

int
get_list_of_pvm_clients(char *machine, int family, int type,
			int well_known_socket, int flag)
{
  struct taskinfo *taskp;
  struct hostinfo *hostp;
  int narch, nhost, loop, pvmd_tid, tids, i;
  char **pvm_argv;

  if ((pvm_tasks(pvm_mytid(), &narch, &taskp) < 0) || (narch < 1))
    return -1;

  pvmd_tid = taskp[0].ti_host;
  if (pvm_config(&nhost, &narch, &hostp) < 0)
    return -1;

  pvm_argv = RPC_make_arg_list ("xdrc_pvm", family, type, well_known_socket, 0);
  for (loop=0; loop<nhost; ++loop)
    {
      assert (hostp->hi_name);
      
      // don't start a paradyn on this machine
      //if (pvmd_tid != hostp[loop].hi_tid)
	{
	  if (pvm_spawn("xdrc_pvm", pvm_argv, 1, hostp->hi_name, 1, &kidid) != 1)
	    {
	      pvm_perror("spawn failed\n");
	      pvm_exit();
	      exit(-1);
	    }
	}
    }

  i = 0;
  while (pvm_argv[i])
    {
      free (pvm_argv[i]);
      i++;
    }
  delete (pvm_argv);
  return 0;
}

int
started_by_paradynd(char *machine, int family, int type, int well_known_socket)
{
  xdr_link_conn = new xdr_linkUser(family, well_known_socket, type, machine, NULL, NULL);

  xdr_link_conn->connected("pvm client connected\n");
  return 0;
}


