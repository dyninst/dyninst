#include "common/h/headers.h"
#include "common/h/String.h"

extern "C"
void
ShowNoSoloVisiMessage( const char* progName )
{
    string msg =
"This is a Paradyn visualization program.  It is designed to be started by\n"
"Paradyn, and does not support being run by itself.";

#if !defined(i386_unknown_nt4_0)
    cerr << progName << ":\n" << msg << endl;
#else
    MessageBox( NULL, msg.c_str(), progName, MB_OK );
#endif // defined(i386_unknown_nt4_0)
    exit(-1);
}

