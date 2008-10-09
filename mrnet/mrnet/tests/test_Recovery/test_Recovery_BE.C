/****************************************************************************
 * Copyright © 2003-2007 Dorian C. Arnold, Philip C. Roth, Barton P. Miller *
 *                  Detailed MRNet usage rights in "LICENSE" file.          *
 ****************************************************************************/

#include <set>
#include <assert.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/stat.h>

#include "mrnet/MRNet.h"
#include "test_Recovery.h"
#include "ThroughputExperiment.h"

using namespace std;
using namespace MRN;

static void setup_handler( void );
static int run_ThroughputTest( Network * inetwork );
static Rank myrank;
int main(int argc, char **argv){
    if( argc != 6 ){
        fprintf(stderr, "Usage: %s parent_hostname parent_port parent_rank my_hostname my_rank\n",
                argv[0]);
        exit( -1 );
    }
    //my rank should be the last arg
    myrank = atoi( argv[ argc-1 ] );

    //last arg should be myrank, use to seed random number generator
    srandom( myrank+1 );
   
    //setup handler for SIGALRM
    setup_handler( );

    Network * network = new Network( argc, argv);
    if( network->has_Error() ){
        fprintf(stderr, "backend_init() failed\n");
        exit (-1);
    }

    if( run_ThroughputTest( network ) ) {
        fprintf( stderr, "run_ThroughputTest() failure\n" );
        return -1;
    }
    
    exit(0);
}

int run_ThroughputTest( Network * inetwork )
{
    PacketPtr packet;
    int prot;
    extern Stream * stream;
    extern unsigned int WavesToSend, MaxVal;
    extern ThroughputExperiment send_exp;

    if ( inetwork->recv(&prot, packet, &stream) != 1){
        fprintf(stderr, "BE[%d]: stream::recv() failure\n", myrank);
        return -1;
    }
    if( packet->unpack( "%ud %ud", &WavesToSend, &MaxVal ) == -1 ){
        fprintf( stderr, "BE[%d]: stream::unpack() failure\n", myrank );
        return -1;
    }

#define USING_SIGALRM 1
#if defined( USING_SIGALRM )
    /*
     * Set a real time interval timer to repeat every DATA_SEND_FREQ msecs
     */

    //fprintf( stderr, "BE[%d]: Installing timer ... \n", myrank );
    struct itimerval inval, outval;

    inval.it_value.tv_sec = 0;
    inval.it_value.tv_usec = DATA_SEND_FREQ * 1000;
    inval.it_interval.tv_sec = 0;
    inval.it_interval.tv_usec = DATA_SEND_FREQ * 1000;

    if( setitimer( ITIMER_REAL, &inval, &outval ) == -1 ) {
        perror( "setitimer()" );
        exit( -1 );
    }
#else
    set<uint32_t> uint_eq_class;
    unsigned int rand_uint;


    uint32_t pkts_sent=0, tries;

    Timer send_timer;
    send_exp.start();
    unsigned long sleep_time=0;
    unsigned long send_time=0;
    for( unsigned int i=0; i<WavesToSend; i++ ) {
        send_timer.start();
        fprintf( stderr, "usleep(%lu)+send(%lu) = %lu\n",
                 sleep_time, send_time, sleep_time+send_time );
        rand_uint = random() % MaxVal;

        pkts_sent++;
        int retval;
        tries=0;
        do {
            tries++;
            send_exp.insert_DataPoint( i+1 );
            retval = stream->send(PROT_INT_EQCLASS,
                                  "%aud %ud", &rand_uint, 1, pkts_sent );
            //fprintf( stderr, "BE[%d]: sent %u of %u\n", myrank, i+1, WavesToSend );
            if( retval == -1 ) {
                fprintf(stderr, "BE[%d]: send() failure\n", myrank);
            }
            //retval = stream->flush();
            //if( retval == -1 ) {
                //fprintf( stderr, "BE[%d]: flush failed %d times!\n",
                         //myrank, tries-1 );
                //usleep(500000);
            //}
            //if flush fails, we lost our parent and should repeat the send
        } while ( retval == -1 );

        //sleep for requisite time
        send_timer.stop();
        if( DATA_SEND_FREQ > 0 ) {
            send_time = (unsigned long)send_timer.get_latency_usecs();

            sleep_time = (DATA_SEND_FREQ * 1000) - send_time;

            usleep( sleep_time );
        }
    }
    send_exp.stop();

    //close this end of stream connection
    if( stream->close( ) == -1 ) {
        fprintf(stderr, "stream::close failure\n");
    }

#endif

    send_exp.dump_Statistics( "BE.send.stat" );

    if ( stream->recv(&prot, packet, true) != 1){
        fprintf(stderr, "BE[%d]: stream::recv() failure\n", myrank);
        return -1;
    }

    fprintf( stderr, "Backend[%d] exiting\n", myrank );
    return 0;
}

Stream * stream;
unsigned int WavesToSend, MaxVal;
ThroughputExperiment send_exp;
static void alarm_handler(int /* signum */)
{
    static unsigned int waves_sent=1;

    unsigned int rand_uint = random() % MaxVal;

    int retval;
    do {
        retval = stream->send(PROT_INT_EQCLASS,
                              "%aud %ud", &rand_uint, 1, waves_sent );
        //fprintf( stderr, "BE[%d]: sent %u of %u\n", myrank, waves_sent, WavesToSend );
        if( retval == -1 ) {
            fprintf(stderr, "BE[%d]: send() failure\n", myrank);
        }
    } while ( retval == -1 );
    send_exp.insert_DataPoint( waves_sent );

    waves_sent++;
    if( waves_sent == WavesToSend ) {
        //no more sends
        signal( SIGALRM, SIG_IGN );

        //close this end of stream connection
        if( stream->close( ) == -1 ) {
            fprintf(stderr, "stream::close failure\n");
        }
    }
}

void setup_handler( void )
{
    struct sigaction sa;

    //fprintf( stderr, "BE[%d]: Installing handler ... \n", myrank );
    sa.sa_handler = alarm_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;

    if (sigaction(SIGALRM, &sa, NULL) == -1) {
        perror( "sigaction()" );
        exit(-1);
    }
}

