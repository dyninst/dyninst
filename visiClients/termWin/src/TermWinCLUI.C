#include "visiClients/termWin/src/TermWinCLUI.h"

TermWinCLUI::TermWinCLUI( PDSOCKET lsock, PDSOCKET stdin_sock )
  : TermWin( lsock, stdin_sock )
{ }


void
TermWinCLUI::ShowError( const char* msg )
{
    cerr << "TermWin: " << msg << endl;
}

void
TermWinCLUI::Print( char* buf, int /* num */, bool isFromDaemon )
{
    if( isFromDaemon )
    {
        if( strlen( buf ) > 0 )
        {
            cout << "D: " << buf << endl;
        }
    }
    else
    {
        cout << "A: " << buf << endl;
    }
}

