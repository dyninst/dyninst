#ifndef TERMWIN_H
#define TERMWIN_H

#include "tcl.h"
#include "common/h/headers.h"
#include "pdthread/h/thread.h"
#include "visiClients/termWin/src/clientConn.h"
#include "termWin.xdr.h"
#include "termWin.xdr.SRVR.h"


class TermWin : public termWin
{
private:
    PDSOCKET lsock;
    PDSOCKET stdin_sock;
    bool shouldExit;
    pdvector<clientConn*> conns;
    thread_t stdin_tid;
    thread_t lsock_tid;

protected:
    void SetShouldExitFlag( void )                  { shouldExit = true; }

public:
    TermWin( PDSOCKET _lsock, PDSOCKET _stdin_sock );
    virtual ~TermWin( void );

    virtual bool Init( void );
    virtual bool IsDoneHandlingEvents( void ) const     { return shouldExit; }
    virtual bool DispatchEvent( thread_t mtid, tag_t mtag );
    virtual void ShowError( const char* msg ) = 0;
    virtual void Print( char* buf, int num, bool isFromDaemon ) = 0;
    virtual bool DoPendingWork( void )                 { return true; }

    // igen interface
    virtual void shutdown( void );
};

#endif // TERMWIN_H

