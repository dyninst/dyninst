/***********************************************************************
 * Copyright © 2003-2004 Dorian C. Arnold, Philip C. Roth, Barton P. Miller *
 *                  Detailed MRNet usage rights in "LICENSE" file.     *
 **********************************************************************/

// $Id: Process-unix.C,v 1.3 2004/06/01 18:23:52 pcroth Exp $
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <assert.h>
#include "xplat/Process.h"


namespace XPlat
{


int
Process::CreateLocal( const std::string& cmd,
                        const std::vector<std::string>& args )
{
    int ret = -1;


    // build a pipe we can use to tell us whether the 
    // child exec'd successfully
    int epipe[2];
    int pret = pipe( epipe );
    if( pret == -1 )
    {
        // we failed to create the "exec failed" pipe
        return -1;
    }

    // set the child's pipe endpoint to close automatically 
    // when it execs
    int fret = fcntl( epipe[1], F_GETFD );
    if( fret == -1 )
    {
        // we failed to retrieve the child endpoint descriptor flags 
        return -1;
    }
    fret = fcntl( epipe[1], F_SETFD, fret | FD_CLOEXEC );
    if( fret == -1 )
    {
        // we failed to set the child endpoint descriptor flags
        return -1;
    }

    // fork the child process
    pid_t pid = fork();
    if( pid > 0 )
    {
        // we're the parent

        // close the child's end of the "exec failed" pipe
        close( epipe[1] );

        // determine whether the child exec'd successfully
        int cval;
        int rret = read( epipe[0], &cval, sizeof(cval) );
        if( rret != sizeof(cval) )
        {
            // the child did not write on the "exec failed" pipe
            // so we know it exec'd OK
            ret = 0;
        }
        else
        {
            // the child wrote 'cval' on the "exec failed" pipe
            ret = cval;
        }
        close( epipe[0] );
    }
    else if( pid == 0 )
    {
        // we're the child

        // close the parent's end of the "exec failed" pipe
        close( epipe[0] );
        
        // build argument array
        char** argv = new char*[args.size() + 1];
        for( unsigned int i = 0; i < args.size(); i++ )
        {
            argv[i] = new char[args[i].length() + 1];
            strcpy( argv[i], args[i].c_str() );
        }
        argv[args.size()] = NULL;

        // exec the command
        int eret = execv( cmd.c_str(), argv );

        // the exec failed - tell our parent and bail
        write( epipe[1], (char*)&eret, sizeof(eret) );
        close( epipe[1] );
        delete[] argv;
        exit( -1 );
    }
    else
    {
        // the fork failed
        // cleanup the pipe
        close( epipe[0] );
        close( epipe[1] );
    }

    return ret;
}

int
Process::GetLastError( void )
{
    return errno;
}

int
Process::GetProcessId( void )
{
    return (int)getpid();
}

} // namespace XPlat

