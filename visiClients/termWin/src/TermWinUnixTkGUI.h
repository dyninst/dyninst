#ifndef TERMWINUNIXTKGUI_H
#define TERMWINUNIXTKGUI_H

#include "common/h/String.h"
#include "visiClients/termWin/src/TermWinTkGUI.h"

class TermWinUnixTkGUI : public TermWinTkGUI
{
public:
    TermWinUnixTkGUI( pdstring _progName,
                            PDSOCKET _lsock,
                            PDSOCKET _stdin_sock )
      : TermWinTkGUI( _progName, _lsock, _stdin_sock )
    {
        // nothing else to do
    }
    virtual ~TermWinUnixTkGUI( void )   { }

    virtual bool Init( void );
    virtual bool DispatchEvent( thread_t mtid, tag_t mtag );
};

#endif // TERMWINUNIXTKGUI_H
