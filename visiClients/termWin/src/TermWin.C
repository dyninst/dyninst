#include <signal.h>
#include "common/h/headers.h"
#include "visiClients/termWin/src/TermWin.h"


void sighup_handler( int );



TermWin::TermWin( PDSOCKET _lsock, PDSOCKET _stdin_sock )
  : termWin( _stdin_sock, NULL, NULL, 0 ),
    lsock( _lsock ),
    stdin_sock( _stdin_sock ),
    shouldExit( false ),
    stdin_tid( THR_TID_UNSPEC ),
    lsock_tid( THR_TID_UNSPEC )
{ }


TermWin::~TermWin( void )
{
    // break our connection to Paradyn
    msg_unbind( stdin_tid );

    // close our sockets
    P_close( lsock );
    for( pdvector<clientConn*>::iterator iter = conns.begin();
                iter != conns.end();
                iter++ )
    {
        clientConn* client = *iter;
        delete client;
    }
}


bool
TermWin::Init( void )
{
    bool ret = true;


    // ensure that the thread library is aware of available input
    // on our XDR connection
    rpcSockCallback += (RPCSockCallbackFunc)clear_ready_sock;

    assert( lsock_tid == THR_TID_UNSPEC );
    int bret = msg_bind_socket( lsock,
                                1,
                                NULL,
                                NULL,
                                &lsock_tid );
    if( bret != 0 )
    {
        ShowError( "TermWin: failed to bind connection socket for input notification" );
        return false;
    }

    assert( stdin_tid == THR_TID_UNSPEC );
    bret = msg_bind_socket( stdin_sock,
                                1,
                                NULL,
                                NULL,
                                &stdin_tid );
    if( bret != 0 )
    {
        ShowError( "TermWin: failed to bind stdin for input notification" );
        return false;
    }

    // ensure that we don't die from SIGHUP when our connection
    // to Paradyn closes
    struct sigaction act;
    act.sa_handler = sighup_handler;
    act.sa_flags = 0;
    sigfillset( &act.sa_mask );
    if( sigaction( SIGHUP, &act, 0 ) == -1 )
    {
        ShowError( "TermWin: failed to install SIGHUP handler" );
        // this isn't a fatal error for us
    }

    return ret;
}


bool
TermWin::DispatchEvent( thread_t mtid, tag_t mtag )
{
    bool ret = false;

    if( mtag == MSG_TAG_SOCKET )
    {
        if( mtid == stdin_tid )
        {
            // there's input on the stdin (i.e., igen) connection
            // so we handle the message
            bool doneWithInput = false;
            while( !doneWithInput )
            {
                this->waitLoop();

                if( xdrrec_eof( this->net_obj() ) )
                {
                    // there is no more input
                    doneWithInput = true;
                }
            }
            // indicate to the thread library we've consumed input from stdin
            clear_ready_sock( thr_socket( stdin_tid ));
            ret = true;
        }
        else if( mtid == lsock_tid )
        {
            PDSOCKET client_sock = INVALID_PDSOCKET;
            thread_t ctid = THR_TID_UNSPEC;

            client_sock = RPC_getConnect( lsock );
            if (client_sock == PDSOCKET_ERROR)
            {
                ShowError( "failed to accept daemon connection" );
                return false;
            }
            int bret = msg_bind_socket( client_sock,
                                            true,
                                            NULL,
                                            NULL,
                                            &ctid );
            if( bret != 0 )
            {
                ShowError( "TermWin: failed to bind client socket for input notification" );
                return false;
            }

            conns.push_back( new clientConn( client_sock, ctid ) );
            clear_ready_sock( lsock );
            ret = true;
        }
        else
        {
            bool foundSender = false;

            for( pdvector<clientConn*>::iterator iter = conns.begin();
                        iter != conns.end();
                        iter++ )
            {
                clientConn* client = *iter;
                assert( client != NULL );

                if( (mtid == client->tid) && !(client->isDone) )
                {
                    foundSender = true;

                    char buffer[1024];
#if !defined(os_windows)
                    int num=read(client->sock,buffer,1023);
#else
                    int num=recv(client->sock,buffer,1023,0);
#endif // defined(os_windows)

                    // indicate to the thread library we've consumed
                    // input from this client connection
                    clear_ready_sock( client->sock );

                    if( num <= 0 )
                    {
                        client->isDone = true;
                        P_close(client->sock);
                        msg_unbind(client->tid);
                        break;
                    }
                    buffer[num]=0x00;

                    if( client->isReady )
                    {
                        Print( buffer, num, client->isFromDaemon );
                    }
                    else
                    {
                        char* buf_pos = NULL;
                        if( !strncmp( buffer,
                                        "from_paradynd",
                                        strlen("from_paradynd")))
                        {
                            buf_pos = buffer + strlen("from_paradynd\n");
                            client->isFromDaemon = true;
                        }
                        else 
                        {
                            buf_pos = buffer + strlen("from_app\n");
                            client->isFromDaemon = false;
                        }
                        client->isReady = true;
                        Print( buf_pos,
                                    buffer+num-buf_pos,
                                    client->isFromDaemon);
                    }

                    ret = true;
                    break;
                }
            }

            if( !foundSender )
            {
                ShowError( "WARNING: failed to find sender of socket message" );
            }
        }
    }
    else
    {
        // we expect no other type of input
    }

    return ret;
}


void
TermWin::shutdown( void )
{
    // Paradyn is telling us that it is shutting down
    shouldExit = true;
}


void
sighup_handler( int )
{
	// we get SIGHUP when Paradyn closes our connection
	// but we are persistent.
	//
	// swallow the signal
	// cerr << "termWin received SIGHUP" << endl;
}


void
termWin::shutdown( void )
{
    // should be pure virtual, but igen does not support that
    assert( false );
}

