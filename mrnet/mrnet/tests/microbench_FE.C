/***********************************************************************
 * Copyright © 2003-2004 Dorian C. Arnold, Philip C. Roth, Barton P. Miller *
 *                  Detailed MRNet usage rights in "LICENSE" file.     *
 **********************************************************************/

#include <iostream>
#include <string>
#include <unistd.h>
#include <stdlib.h>
#include <assert.h>

#include "mrnet/MRNet.h"
#include "mrnet/tests/timer.h"
#include "mrnet/tests/microbench.h"

using namespace MRN;
const unsigned int kMaxRecvTries = 1000000;


#if READY
inline
double
TimevalDiff( struct timeval& begin, struct timeval& end )
{
    double dbegin = (double)(begin.tv_sec + ((double)begin.tv_usec)/1000000);
    double dend = (double)(end.tv_sec + ((double)end.tv_usec)/1000000);
    
    if( dend < dbegin )
    {
        dend = dbegin;
    }
    return dend - dbegin;
}
#endif // READY


Network * BuildNetwork( std::string cfgfile, std::string backend_exe );
int DoRoundtripLatencyExp( Stream* stream,
                            unsigned int nIters,
                            unsigned int nBackends );
int DoReductionThroughputExp( Stream* stream,
                            unsigned int nIters,
                            unsigned int nBackends );


int
main( int argc, char* argv[] )
{
    int ret = 0;

    if( getenv( "MRN_DEBUG_FE" ) != NULL )
    {
        fprintf( stderr, "FE: spinning, pid=%d\n", getpid() );
        bool bCont=false;
        while( !bCont )
        {
            // spin
        }
    }

    // parse the command line
    if( argc != 5 )
    {
        std::cerr << "Usage: " << argv[0]
                << "<roundtrip_iters> <thru_iters>"
                << " <topology file> <backend exe>"
                << std::endl;
        return -1;
    }
    
    unsigned long nRoundtripIters  = strtoul( argv[1], NULL, 10 );
    unsigned long nThroughputIters  = strtoul( argv[2], NULL, 10 );
    std::string topology_file = argv[3];
    std::string backend_exe = argv[4];
    
    // instantiate the network
    Network * network = BuildNetwork( topology_file, backend_exe );

    // get a broadcast communicator
    Communicator * bcComm = network->get_BroadcastCommunicator();
    Stream* stream = network->new_Stream( bcComm, TFILTER_SUM );
    assert( bcComm != NULL );
    assert( stream != NULL );
    unsigned int nBackends = bcComm->size();
    std::cout << "FE: broadcast communicator has " 
        << nBackends << " backends" 
        << std::endl;

    // perform roundtrip latency experiment
    DoRoundtripLatencyExp( stream, nRoundtripIters, nBackends );

    // perform reduction throughput experiment
    DoReductionThroughputExp( stream, nThroughputIters, nBackends );

    // tell back-ends to go away
    if( (stream->send( MB_EXIT, "%d", 0 ) == -1) ||
        (stream->flush() == -1) )
    {
        std::cerr << "FE: failed to broadcast termination message" << std::endl;
    }

    // delete the network
    std::cout << "FE: deleting network" << std::endl;
    delete network;

    return ret;
}


Network *
BuildNetwork( std::string cfgfile, std::string backend_exe )
{
    mb_time startTime;
    mb_time endTime;
	const char *argv=NULL;

    std::cout << "FE: network instantiation: ";
    startTime.set_time();
    Network * network = new Network( cfgfile.c_str(), backend_exe.c_str(), &argv );
    if( network->fail() )
    {
        std::cerr << "FE: network initialization failed" << std::endl;
        network->error_str("FE:");
        delete network;
        exit(-1);
    }
    endTime.set_time();

    // dump network instantiation latency
    double latency = (endTime - startTime).get_double_time();
    std::cout << " latency(sec): " << latency << std::endl;

    return network;
}


int
DoRoundtripLatencyExp( Stream* stream,
                            unsigned int nIters,
                            unsigned int nBackends )
{
    mb_time startTime;
    mb_time endTime;

#if READY
    struct timeval preSendTime;
    struct timeval preFlushTime;
    struct timeval postFlushTime;
    struct timeval preRecvTime;
    struct timeval postRecvTime;
#else
#endif // READY
    int sret = -1;
    int fret = -1;
    unsigned int nTries = 0;

    std::cout << "FE: roundtrip latency: " << std::flush;
    startTime.set_time();
    for( unsigned int i = 0; i < nIters; i++ )
    {
        // broadcast
#if READY
        gettimeofday( &preSendTime, NULL );
#else
#endif // READY
        sret = stream->send( MB_ROUNDTRIP_LATENCY, "%d", 1 );
#if READY
        gettimeofday( &preFlushTime, NULL );
#else
#endif // READY
        if( sret != -1 )
        {
            fret = stream->flush();
        }
#if READY
        gettimeofday( &postFlushTime, NULL );
#else
#endif // READY

        if( (sret == -1) || (fret == -1) )
        {
            std::cerr << "FE: roundtrip latency broadcast failed" << std::endl;
            return -1;
        }

        // receive reduced value
        int tag;
        Packet* buf = NULL;
        int rret = 0;
        nTries = 0;
#if READY
        gettimeofday( &preRecvTime, NULL );
#else
#endif // READY
        do
        {
            tag = 0;
            buf = NULL;
            rret = stream->recv( &tag, &buf );
            if( rret == -1 )
            {
                std::cerr << "FE: roundtrip latency recv() failed" << std::endl;
                return -1;
            }
            nTries++;
        } while( rret == 0 );
#if READY
        gettimeofday( &postRecvTime, NULL );
#else
#endif // READY
        if( tag != MB_ROUNDTRIP_LATENCY )
        {
            std::cerr << "FE: unexpected tag " << tag << " seen"
                << ", rret = " << rret
                << std::endl;
        }
        else
        {
            int ival = 0;
            stream->unpack(buf, "%d", &ival );
            if( ival != (int)nBackends )
            {
                std::cerr << "FE: unexpected reduction value " << ival << " seen, expected " << nBackends << std::endl;
            }
        }
    }
    endTime.set_time();

    // dump broadcast/reduction roundtrip latency
    double totalLatency = (endTime - startTime).get_double_time();
    double avgLatency = totalLatency / nIters;
    std::cout << "total(sec): " << totalLatency
                << ", nIters: " << nIters
                << ", avg(sec): " << avgLatency
                << std::endl;

#if READY
    std::cout << "\tsend() latency: "
        << TimevalDiff( preSendTime, preFlushTime )
        << std::endl;
    std::cout << "\tflush() latency: "
        << TimevalDiff( preFlushTime, postFlushTime )
        << std::endl;
    std::cout << "\trecv() latency: "
        << TimevalDiff( preRecvTime, postRecvTime )
        << std::endl;
    std::cout << "\trecv() tries: "
        << nTries
        << std::endl;
#else
#endif // READY

    return 0;
}





int
DoReductionThroughputExp( Stream* stream,
                            unsigned int nIters,
                            unsigned int nBackends )
{
    mb_time startTime;
    mb_time endTime;

    std::cout << "FE: reduction throughput: " << std::flush;
    // broadcast request to start throughput experiment
    // we send number of iterations to do and the value to send
    if( (stream->send( MB_RED_THROUGHPUT, "%d %d", nIters, 1 ) == -1) ||
        (stream->flush() == -1) )
    {
        std::cerr << "FE: failed to start throughput experiment" << std::endl;
        return -1;
    }

    // do the experiment
    startTime.set_time();
    for( unsigned int i = 0; i < nIters; i++ )
    {
        // receive reduced value
        int tag;
        Packet* buf;
        int rret;
        unsigned int nTries = 0;
        do
        {
            tag = 0;
            buf = NULL;
            rret = stream->recv( &tag, &buf );
            if( rret == -1 )
            {
                std::cerr << "FE: reduction throughput recv() failed" 
                        << std::endl;
                return -1;
            }
            nTries++;
        }
        while( rret == 0 );
        if( nTries == kMaxRecvTries )
        {
            std::cerr << "FE: warning! max tries reached" << std::endl;
        }
        else if( tag != MB_RED_THROUGHPUT )
        {
            std::cerr << "FE: unexpected tag " << tag << " seen during throughput experiment"
                << ", rret = " << rret
                << std::endl;
        }
        else
        {
            int ival = 0;
            stream->unpack( buf, "%d", &ival );
            if( ival != (int)nBackends )
            {
                std::cerr << "FE: unexpected reduction value " << ival << " received, expected " << nBackends << std::endl;
            }
        }
    }
    endTime.set_time();

    // dump reduction throughput
    double expLatency = (endTime - startTime).get_double_time();
    double throughput = ((double)nIters) / expLatency;
    std::cout << "nIters: " << nIters
                << ", latency(sec): " << expLatency
                << ", throughput(ops/sec): " << throughput
                << std::endl;

    return 0;
}


