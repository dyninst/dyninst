#include <stdio.h>
#include <stdlib.h>
#include "xdr_link.SRVR.h"
#include <util/h/list.h>
#include <signal.h>

xdr_link *client = 0;
List<xdr_link*> client_list;
int well_known_socket;
void sig_int (int signo);

void sig_int (int signo)
{
  xdr_link *temp;

  client_list.setCurrent();
  for (temp = client_list.getCurrent();
       temp;
       temp = client_list.next())  
    {
      delete (temp);
    }
  exit(0);
}

main(int argc, char *argv[])
{
  xdr_link *tp;
  xdr_link *temp = 0;
  char **arg_list= 0;
  int i, fd, eid, ret, sfd;

  if (signal(SIGINT, sig_int) == SIG_ERR)
    {
      perror ("Error setting up signals\n");
      exit(-1);
    }

  // create the well known socket, requests will be listened for here
  if ((well_known_socket = RPC_setup_socket (&sfd, AF_INET, SOCK_STREAM)) < 0)
    {
      perror ("socket creation\n");
      exit (-1);
    }
  arg_list = RPC_make_arg_list ("xdrc_pvm", AF_INET, SOCK_STREAM, well_known_socket, 1);
  
  // start the first 'paradynd'
  fd = RPCprocessCreate (&eid, "localhost", "", "xdrc_pvm", arg_list);
  if (fd < 0)
    {
      perror ("process create");
      exit (-1);
    }

  i = 0;
  while (arg_list[i])
    {
      free (arg_list[i]);
      i++;
    }
  delete (arg_list);

  tp = new xdr_link (fd, NULL, NULL);
  client_list.add(tp, (void *) tp);

  // now go into main loop
  while (1)
    {
      // check for client connection requests
      if (RPC_readReady (sfd))
	{
	  int fd;
	  fd = RPC_getConnect (sfd);
	  client = new xdr_link(fd, NULL, NULL);
	  client_list.add(client, (void *) client);
	}

      client_list.setCurrent();
      for (temp = client_list.getCurrent();
	   temp;
	   temp = client_list.next())
	{

	  ret = temp->readReady();
	  switch (ret)
	    {
	    case -1:            // client has terminated
	      // temp->closeConnect();
	      client_list.remove( (void *) temp);
	      break;
	    case 0:            // nothing there
	      break;
	    default:           // something there
	      if (temp->mainLoop() == -1)
		{
		  // temp->closeConnect();
		  client_list.remove( (void *) temp);
		}
	    }
	}
    }
}

void
xdr_link::connected(String msg)
{
  char msg_out[100];

  sprintf(msg_out, "%s::%s\n", "in xdr_link::connected   ", msg);
  printf (msg_out);
  return;
}





