/***********************************************************************
 * Copyright © 2003-2004 Dorian C. Arnold, Philip C. Roth, Barton P. Miller *
 *                  Detailed MRNet usage rights in "LICENSE" file.     *
 **********************************************************************/

#include <iostream>
#include <assert.h>
#include <unistd.h>

#include "mrnet/MRNet.h"
#include "mrnet/tests/microbench.h"

using namespace MRN;

void BeDaemon( void );


int
main( int argc, char* argv[] )
{
    int ret = 0;
    Stream* stream;
    int tag;
    char* buf = NULL;


    // become a daemon process
    BeDaemon();

    // join the MRNet network
    Network * network = new Network( argv[argc-5],
                                     argv[argc-4],
                                     argv[argc-3],
                                     argv[argc-2],
                                     argv[argc-1] );
    if( network->fail() )
    {
        std::cerr << argv[0] << "init_Backend() failed" << std::endl;
        return -1;
    }

    // participate in the broadcast/reduction roundtrip latency experiment
    bool done = false;
    while( !done )
    {
        // receive the broadcast message
        tag = 0;
        int rret = network->recv( &tag, (void**)&buf, &stream );
        if( rret == -1 )
        {
            std::cerr << "BE: Stream::recv() failed" << std::endl;
            return -1;
        }

        if( tag == MB_ROUNDTRIP_LATENCY )
        {
            // extract the value and send it back
            int ival = 0;
            stream->unpack( buf, "%d", &ival );

#if READY
            std::cout << "BE: roundtrip lat, received val " << ival << std::endl;
#else
#endif // READY
            
            // send our value for the reduction
            if( (stream->send( tag, "%d", ival ) == -1) ||
                (stream->flush() == -1) )
            {
                std::cerr << "BE: roundtrip exp reduction failed" << std::endl;
                return -1;
            }
        }
        else
        {
            // we're done with the roundtrip latency experiment
            done = true;
            if( tag != MB_RED_THROUGHPUT )
            {
                std::cerr << "BE: unexpected tag " << tag 
                        << " seen to end roundtrip experiments"
                        << std::endl;
            }
        }
    }


    //
    // participate in the reduction throughput experiment
    //

    // determine the number of reductions required
    assert( tag == MB_RED_THROUGHPUT );
    assert( buf != NULL );
    assert( stream != NULL );
    int nReductions = 0;
    int ival;
    stream->unpack( buf, "%d %d", &nReductions, &ival );

#if READY
    std::cout << "BE: received throughput exp start message"
        << ", nReductions = " << nReductions
        << ", ival = " << ival
        << std::endl;
#else
#endif // READY

    // do the reductions
    for( int i = 0; i < nReductions; i++ )
    {
        // send a value for the reduction
        if( (stream->send( MB_RED_THROUGHPUT, "%d", ival ) == -1 ) ||
            (stream->flush() == -1) )
        {
            std::cerr << "BE: reduction throughput exp reduction failed" 
                    << std::endl;
            return -1;
        }
    }
    // cleanup
    // receive a go-away message
    tag = 0;
    int rret = network->recv( &tag, (void**)&buf, &stream );
    if( (rret != -1) && (tag != MB_EXIT) )
    {
        std::cerr << "BE: received unexpected go-away tag " << tag << std::endl;
    }
    if( tag != MB_EXIT )
    {
        std::cerr << "BE: received unexpected go-away tag " << tag << std::endl;
    }

    return ret;
}

void
BeDaemon( void )
{
    // become a background process
    pid_t pid = fork();
    if( pid > 0 )
    {
        // we're the parent - we want our child to be the real job,
        // so we just exit
        exit(0);
    }
    else if( pid < 0 )
    {
        std::cerr << "BE: fork failed to put process in background" << std::endl;
        exit(-1);
    }
    
    // we're the child of the original fork
    pid = fork();
    if( pid > 0 )
    {
        // we're the parent in the second fork - exit
        exit(0);
    }
    else if( pid < 0 )
    {
        std::cerr << "BE: second fork failed to put process in background" << std::endl;
        exit(-1);
    }

    // we were the child of both forks - we're the one who survives
}

