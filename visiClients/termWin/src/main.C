/*
 * Copyright (c) 1996-2001 Barton P. Miller
 * 
 * We provide the Paradyn Parallel Performance Tools (below
 * described as Paradyn") on an AS IS basis, and do not warrant its
 * validity or performance.  We reserve the right to update, modify,
 * or discontinue this software at any time.  We shall have no
 * obligation to supply such updates or modifications or any other
 * form of support to you.
 * 
 * This license is for research uses.  For such uses, there is no
 * charge. We define "research use" to mean you may freely use it
 * inside your organization for whatever purposes you see fit. But you
 * may not re-distribute Paradyn or parts of Paradyn, in any form
 * source or binary (including derivatives), electronic or otherwise,
 * to any other organization or entity without our permission.
 * 
 * (for other uses, please contact us at paradyn@cs.wisc.edu)
 * 
 * All warranties, including without limitation, any warranty of
 * merchantability or fitness for a particular purpose, are hereby
 * excluded.
 * 
 * By your use of Paradyn, you understand and agree that we (or any
 * other person or entity with proprietary rights in Paradyn) are
 * under no obligation to provide either maintenance services,
 * update services, notices of latent defects, or correction of
 * defects for Paradyn.
 * 
 * Even if advised of the possibility of such damages, under no
 * circumstances shall we (or any other person or entity with
 * proprietary rights in the software licensed hereunder) be liable
 * to you or any third party for direct, indirect, or consequential
 * damages of any character regardless of type of action, including,
 * without limitation, loss of profits, loss of use, loss of good
 * will, or computer failure or malfunction.  You agree to indemnify
 * us (and any other person or entity with proprietary rights in the
 * software licensed hereunder) for any and all liability it may
 * incur to third parties resulting from your use of Paradyn.
 */

// $Id: main.C,v 1.4 2001/11/02 16:05:27 pcroth Exp $

#include <stdio.h>
#include <signal.h>

#include "common/h/headers.h"

#include "tcl.h"
#include "tk.h"
#include "tkTools.h"

#include "pdLogo.h"
#include "paradyn/xbm/logo.xbm"

#include "thread/h/thread.h"
#include "pdutilOld/h/rpcUtil.h"
#include "visi/h/visualization.h"

#include "clientConn.h"

#include "termWin.xdr.h"
#include "termWin.xdr.SRVR.h"

#include "common/h/Ident.h"
extern "C" const char V_libpdutilOld[];
Ident V_Uid(V_libpdutilOld,"Paradyn");

Tcl_Interp *MainInterp;
termWin* twServer = NULL;
PDSOCKET serv_sock = -1;
vector<clientConn *> conn_pool;
thread_t stdin_tid = 0;


#define PARADYN_EXIT 0
#define PERSISTENT   1
int close_mode = PARADYN_EXIT;


void
sighup_handler( int )
{
	// we get SIGHUP when Paradyn closes our connection
	// but we are persistent.
	//
	// swallow the signal
	// cerr << "termWin received SIGHUP" << endl;
}


int CloseOptionCmd(ClientData,Tcl_Interp *,int argc, char **argv)
{
    assert(argc == 2);
    const int mode = atoi(argv[1]);

    close_mode = mode;
    return TCL_OK;
}

int app_init() {
    if (Tcl_Init(MainInterp) == TCL_ERROR)
	return TCL_ERROR;

    if (Tk_Init(MainInterp) == TCL_ERROR)
	return TCL_ERROR;

    Tcl_CreateCommand(MainInterp,"close_mode",CloseOptionCmd,NULL,NULL);

    // now install "makeLogo", etc:
    pdLogo::install_fixed_logo("paradynLogo", logo_bits, logo_width,
			       logo_height);

    tcl_cmd_installer createPdLogo(MainInterp, "makeLogo", pdLogo::makeLogoCommand,
				   (ClientData)Tk_MainWindow(MainInterp));

    // now initialize_tcl_sources created by tcl2c:
   extern int initialize_tcl_sources(Tcl_Interp *);
   if (TCL_OK != initialize_tcl_sources(MainInterp))
      tclpanic(MainInterp, "phaseTable: could not initialize_tcl_sources");

//    assert(TCL_OK == Tcl_EvalFile(MainInterp, "/p/paradyn/development/tamches/core/visiClients/phaseTable/tcl/phasetbl.tcl"));
    
    return TCL_OK;
}

void processPendingTkEventsNoBlock()
{
	// We use Tk_DoOneEvent (w/o blocking) to soak up and process
	// pending tk events, if any.  Returns as soon as there are no
	// tk events to process.
	// NOTE: This includes (as it should) tk idle events.
	// NOTE: This is basically the same as Tcl_Eval(interp, "update"), but
	//       who wants to incur the expense of tcl parsing?
	while (Tk_DoOneEvent(TK_DONT_WAIT) > 0)
		;
}

void print_data(char *buffer,int num, int from_paradynd)
{
	if (num <= 0)
		return ;
	char command[2048];
	if (from_paradynd)
		sprintf(command,".termwin.textarea.text insert end {%.*s} paradyn_tag",num,buffer);
	else sprintf(command,".termwin.textarea.text insert end {%.*s} app_tag",num,buffer);
	Tcl_Eval(MainInterp,command);

	sprintf(command,".termwin.textarea.text yview -pickplace end");
	Tcl_Eval(MainInterp,command);
}


int main(int argc, char **argv) {
//sigpause(0);
   if (argc == 1)
   	return -1;
   serv_sock = atoi(argv[1]);
   //int serv_port = RPC_setup_socket(serv_sock,AF_INET,SOCK_STREAM);
   //assert(serv_port != -1);
   //fprintf(stderr,"serv_port: %d\n",serv_port);

   MainInterp = Tcl_CreateInterp();
   assert(MainInterp);

   if (TCL_OK != app_init()) // formerly Tcl_AppInit()
      tclpanic(MainInterp, "PhaseTable: app_init() failed");

   thread_t xtid;
   int retVal=-1;
   thread_t stid;

#if !defined(i386_unknown_nt4_0)
   Display *UIMdisplay = Tk_Display (Tk_MainWindow(MainInterp));
   int xfd = XConnectionNumber (UIMdisplay);

   retVal = msg_bind_socket (xfd,
        1, // "special" flag --> libthread leaves it to us to manually
		                           // dequeue these messages
	NULL,
	NULL,
	&xtid
	);

  retVal = msg_bind_socket(fileno(stdin),1,NULL,NULL,&stdin_tid);
  retVal = msg_bind_socket(serv_sock,1,NULL,NULL,&stid);

#else // !defined(i386_unknown_nt4_0)
  retVal = msg_bind_wmsg( &xtid );
  //need modify: wxd
  //retVal = msg_bind_socket(fileno(stdin),1,NULL,NULL,&stdin_tid);
  retVal = msg_bind_socket(serv_sock,1,NULL,NULL,&stid);
#endif // !defined(i386_unknown_nt4_0)

	// wrap a termWin server around our connection to Paradyn
	twServer = new termWin( fileno(stdin), NULL, NULL, 0 );

	// ensure that we don't die from SIGHUP when our connection
	// to Paradyn closes
	struct sigaction act;

	act.sa_handler = sighup_handler;
	act.sa_flags = 0;
	sigfillset( &act.sa_mask );

	if( sigaction( SIGHUP, &act, 0 ) == -1 )
	{
		cerr << "Failed to install SIGHUP handler: " << errno << endl;
		// this isn't a fatal error for us
	}
   
   while (Tk_GetNumMainWindows() > 0) {
   	
      // Before we block, let's process any pending tk DoWhenIdle events.
      processPendingTkEventsNoBlock();

      unsigned msgSize = 1024;
      thread_t pollsender = THR_TID_UNSPEC;
      tag_t mtag = MSG_TAG_ANY;
      int err = msg_poll (&pollsender, &mtag, 1); // 1-->make this a blocking poll
                                            // i.e., not really a poll at all...
      if (err == THR_ERR)
      {
      	fprintf(stderr,"msg_poll error\n");
	exit(-1);
      }
      // Why don't we do a blocking msg_recv() in all cases?  Because it soaks
      // up the pending message, throwing off the X file descriptor (tk wants to
      // dequeue itself).  Plus igen feels that way too.
      // NOTE: It would be nice if the above poll could take in a TIMEOUT argument,
      //       so we can handle tk "after <n millisec> do <script>" events properly.
      //       But libthread doesn't yet support time-bounded blocking!!!

      // check for X events or commands on stdin
      if (mtag == MSG_TAG_SOCKET) {
         // Note: why don't we do a msg_recv(), to consume the pending
         //       event?  Because both of the MSG_TAG_FILE we have set
         //       up have the special flag set (in the call to msg_bind()),
         //       which indicated that we, instead of libthread, will take
         //       responsibility for that.  In other words, a msg_recv()
         //       now would not dequeue anything, so there's no point in doing it...

     if (pollsender == xtid)
            processPendingTkEventsNoBlock();
         else if (pollsender == stid)
	 {
	 	//fprintf(stderr,"accept client request stid=%d\n",stid);
#if defined(i386_unknown_nt4_0)
	 	static int first=1;
		if (first == 0)
			continue;
		first = 0;
#endif

   		PDSOCKET client_sock = -1;
   		thread_t ctid= THR_TID_UNSPEC;

		client_sock = RPC_getConnect(serv_sock);
		if (client_sock == PDSOCKET_ERROR)
		{
			fprintf(stderr,"getConnect error\n");
			continue;
		}//else fprintf(stderr,"getConnect success\n");

   		retVal = msg_bind_socket(client_sock, true, NULL, NULL, &ctid);
		
#if defined(i386_unknown_nt4_0)
		printf("ctid = %d, client_sock = %d\n",ctid,client_sock);
		msg_unbind(stid);
#endif

		conn_pool += new clientConn(client_sock,ctid);

		//if (conn_pool.size() > 1)
			//fprintf(stderr,"current conn_pool size %d\n",conn_pool.size());
     	 }else if (pollsender == stdin_tid)
	 {
	     	bool doneWithInput = false;

		// there's input on the stdin (i.e., igen) connection
		// so we handle the message
		while( !doneWithInput )
		{
			twServer->waitLoop();

			if( xdrrec_eof( twServer->net_obj() ) )
			{
				// there is no more input
				doneWithInput = true;
			}
		}
	 }
	 else {
	 	int	client_is_sender = 0;
	 	for (int i=0;i<conn_pool.size();i++)
		{
			clientConn *client = (clientConn *)conn_pool[i];
	 		if (client->done == 0 && pollsender == client->tid){
				client_is_sender = 1;
		         	char buffer[1024];
#if !defined(i386_unknown_nt4_0)
				int num=read(client->sock,buffer,1023);
#else
				int num=recv(client->sock,buffer,1023,0);
#endif
				//printf("num = %d\n",num);
		
				if (num <= 0)
				{
					client->done = 1;
					P_close(client->sock);
					msg_unbind(client->tid);
					continue;
				}
				buffer[num]=0x00;
				//printf("%s\n",buffer);
		
				if (client->ready)
					print_data(buffer,num,client->from_paradynd);
				else {
					char *buf_pos=NULL;
					if (!strncmp(buffer,"from_paradynd",strlen("from_paradynd")))
					{
						buf_pos = buffer + strlen("from_paradynd\n");
						client->from_paradynd = 1;
					}else {
						buf_pos = buffer + strlen("from_app\n");
						client->from_paradynd = 0;
					}
					client->ready = 1;
					print_data(buf_pos,buffer+num-buf_pos,client->from_paradynd);
				}
			}
		}
		if (!client_is_sender)
		{
	 		for (int i=0;i<conn_pool.size();i++)
			{
				clientConn *client = (clientConn *)conn_pool[i];
				fprintf(stderr,"tid:\t%d\n",client->tid);
			}
			fprintf(stderr,"ser_id:\t%d\n",stid);
			fprintf(stderr,"error id:\t%d\n",pollsender);
			fprintf(stderr,"stdin id:\t%d\n",stdin_tid);
			fprintf(stderr,"Xid:\t%d\n",xtid);
			cerr << "hmmm...unknown sender of a MSG_TAG_SOCKET message...ignoring" << endl;
			exit(-1);
		}
	}
     // The above processing may have created some pending tk DoWhenIdle
     // requests.  If so, process them now.
         processPendingTkEventsNoBlock();
      }
#if defined(i386_unknown_nt4_0)
        else if( mtag == MSG_TAG_WMSG )
        {
            // there are events in the Windows message queue - handle them
            processPendingTkEventsNoBlock();
        }
#endif // defined(i386_unknown_nt4_0)

#if !defined(i386_unknown_nt4_0)
	else if (mtag == MSG_TAG_FILE)
	{
     	    if (pollsender == stdin_tid)
	    {
         	char buffer[1024];
		int num=read(0,buffer,1023);
		
		if (num <= 0)
			exit(-1);
		buffer[num]=0x00;
		printf("%s\n",buffer);
	    }
	}
#endif // defined(i386_unknown_nt4_0)
  }

   return 0;
}


void
do_graceful_shutdown( void )
{
   delete twServer;
   twServer = NULL;

   Tcl_DeleteCommand(MainInterp, "close_mode");

   P_close(serv_sock);
   for(unsigned int i=0;i<conn_pool.size();i++)
   {
   	clientConn *client = (clientConn *)conn_pool[i];
	if (client->done == 0)
		P_close(client->sock);
   }
   Tcl_DeleteInterp(MainInterp);

   exit( 0 );
}


void
termWin::shutdown( void )
{
	// Paradyn is telling us that it is shutting down

	// break our connection to Paradyn
	msg_unbind(stdin_tid);

	// shut ourselves down gracefully if we aren't set to be persistent
	if( close_mode != PERSISTENT )
	{
		// shut ourselves down
		do_graceful_shutdown();
		// not reached
		assert( false );
	}
}


