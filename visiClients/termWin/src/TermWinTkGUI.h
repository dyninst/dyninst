#ifndef TERMWINTKGUI_H
#define TERMWINTKGUI_H

#include "common/h/String.h"
#include "visiClients/termWin/src/TermWin.h"

class TermWinTkGUI : public TermWin
{
private:
    // behaviors we can exhibit when Paradyn exits
    // (We explicitly specify numeric values because they must
    // match the values used in the termWin.tcl file.)
    enum CloseMode
    {
        CloseModeCloseOnParadynExit = 0,
        CloseModePersistent = 1
    };

    Tcl_Interp* interp;
    pdstring progName;
    CloseMode closeMode;

    static int CloseModeCmd( ClientData cd, Tcl_Interp* interp,
                                int argc, TCLCONST char** argv );

protected:
    thread_t tkTid;

    void SetCloseMode( CloseMode m )        { closeMode = m; }
    Tcl_Interp* GetInterp( void )           { return interp; }

public:
    TermWinTkGUI( pdstring progName, PDSOCKET lsock, PDSOCKET stdin_sock );
    virtual ~TermWinTkGUI( void );

    virtual bool Init( void );
    virtual bool IsDoneHandlingEvents( void ) const;
    virtual bool DispatchEvent( thread_t mtid, tag_t mtag );
    virtual void ShowError( const char* msg );
    virtual void Print( char* buf, int num, bool isFromDaemon );
    virtual bool DoPendingWork( void );

    // igen calls
    virtual void shutdown( void );
};

#endif // TERMWINTKGUI_H
