#ifndef TERMWINCLUI_H
#define TERMWINCLUI_H

#include "visiClients/termWin/src/TermWin.h"

class TermWinCLUI : public TermWin
{
public:
    TermWinCLUI( PDSOCKET lsock, PDSOCKET stdin_sock );
    virtual ~TermWinCLUI( void ) { }

#if READY
    virtual bool Init( void );
    virtual bool DispatchEvent( thread_t mtid, tag_t mtag );
#endif // READY
    virtual void ShowError( const char* msg );
    virtual void Print( char* buf, int num, bool isFromDaemon );
};

#endif // TERMWINCLUI_H
