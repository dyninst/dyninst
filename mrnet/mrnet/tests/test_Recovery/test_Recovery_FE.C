/****************************************************************************
 * Copyright © 2003-2007 Dorian C. Arnold, Philip C. Roth, Barton P. Miller *
 *                  Detailed MRNet usage rights in "LICENSE" file.          *
 ****************************************************************************/

#include "mrnet/MRNet.h"
#include "mrnet/FailureManagement.h"
#include "test_Recovery.h"
#include "test_Recovery_aux.h"
#include "ThroughputExperiment.h"

#include <sys/stat.h>
#include <string>
#include <set>
#include <list>
#include <stdio.h>

using namespace std;
using namespace MRN;

char RecoveryStatsFilename[256],
    ThroughputStatsFilename[256],
    TopologyFileBasename[256];

static int run_ThroughputTest( Network * inetwork,
                               uint32_t iduration,
                               const char * iso_filename );

int main(int argc, char **argv)
{
    uint32_t norphans=0, extra_internal_procs=0;
    uint32_t duration=10; //experiment duration (secs)
    uint32_t procs_per_node=1;
    uint32_t nfailures=0;
    uint32_t failure_frequency=30;
    uint32_t job_id=0;
    string hosts, hosts_format, backend_exe, so_file, benchmark_type;

    benchmark_type = "M";

    proc_ArgList( argc, argv, benchmark_type, duration, backend_exe, so_file,
                  hosts, hosts_format, procs_per_node, nfailures, failure_frequency,
                  norphans, extra_internal_procs, job_id );

    //setup output tatistic filenames
    snprintf( RecoveryStatsFilename, sizeof( RecoveryStatsFilename ),
              "Recovery-latency-%03u-%u.stat", norphans, job_id );
    snprintf( ThroughputStatsFilename, sizeof( ThroughputStatsFilename ),
              "Throughput-%03u-%u.stat", norphans, job_id );
    snprintf( TopologyFileBasename, sizeof( TopologyFileBasename ),
              "Fanout-%u.%u", norphans, job_id );

    fprintf( stderr, "norphans: %u, duration: %u sec, procs/node: %u, extra: %u "
             "nfailures: %u\n failure_freq: %u, be: \"%s\", so: \"%s\"\n",
             norphans, duration, procs_per_node, extra_internal_procs, nfailures,
             failure_frequency, backend_exe.c_str(), so_file.c_str() );

    char top_filename[256];
    if( hosts_format == "topology_file" ) {
        fprintf( stderr, "Using Topology file: \"%s\"\n", hosts.c_str() );
        snprintf( top_filename, sizeof(top_filename), "%s", hosts.c_str() );
    }
    else{
        //get process list for mrnet tree
        list<string> process_list;
        char top[256];
        if( hosts_format == "hostfile" ) {
            fprintf( stderr, "Getting host list from \"%s\" ... ", hosts.c_str() );
            if( !get_HostListFromFile( hosts, process_list ) ) {
                fprintf( stderr, "get_HostListFromHostFile() failed\n");
                exit(-1);
            }
            fprintf( stderr, "Done!\n\n");
        }
        else if (hosts_format == "slurmlist" ){
            fprintf( stderr, "Creating topology from slurm list: \"%s\"\n", hosts.c_str() );
            if( !get_HostListFromSLURMFormattedString( hosts, process_list ) ){
                fprintf( stderr, "get_HostListFromSLURMFormattedString() failed\n");
                exit(-1);
            }
        }

        fprintf( stderr, "Creating tree from list of %u processes\n",
                 process_list.size() );
        list<string>::const_iterator iter;
        uint32_t i=0;
        for( iter=process_list.begin(); iter!=process_list.end(); i++,iter++ ) {
            fprintf( stderr, "\tprocess[%u]: %s\n", i, (*iter).c_str() );
        }

        if( benchmark_type == "micro" ) {
            //topology 1x1xn where n is norphans
            snprintf( top, sizeof(top), "%d:%d,%dx1", extra_internal_procs+1, norphans,
                      extra_internal_procs );
            string top_str = top;
            GenericTree * tree = new GenericTree( top_str, process_list);

            //create mrnet topology file
            snprintf( top_filename, sizeof(top_filename), "%d.top", norphans);
            FILE * fp = fopen( top_filename, "w");
            fprintf( stderr, "Creating topology file: \"%s\" ... ", top_filename );
            tree->create_TopologyFile( fp );
            fprintf( stderr, "Done!\n\n");
            fclose(fp);
        }
        else {
            //For macro experiment, tree is balanced 2-deep
            snprintf( top, sizeof(top), "%d^2", norphans );
            string top_str = top;
            BalancedTree * tree = new BalancedTree( top_str, process_list,
                                                    1, procs_per_node, 1 );

            //create mrnet topology file
            snprintf( top_filename, sizeof(top_filename), "%d.top", norphans);
            FILE * fp = fopen( top_filename, "w");
            fprintf( stderr, "Creating topology file: \"%s\" ... ", top_filename );
            tree->create_TopologyFile( fp );
            fprintf( stderr, "Done!\n\n");
            fclose(fp);
        }
    }

    const char * dummy_argv=NULL;
    fprintf( stderr, "Creating MRNet network: ... " );
    Network * network = new Network( top_filename, backend_exe.c_str(),
                                     &dummy_argv );
    if( network->has_Error() ){
        fprintf(stderr, "Network Initialization failure\n");
        network->print_error(argv[0]);
        exit(-1);
    }
    fprintf( stderr, "Done!\n\n");

    fprintf( stderr, "Front-end EDT on ***%s:%d***\n\n",
             network->get_NetworkTopology()->get_Root()->get_HostName().c_str(),
             network->get_NetworkTopology()->get_Root()->get_Port() );

    if( nfailures > 0 ) {
        fprintf(stderr, "FE: Starting failure manager ...\n");
        set_NumFailures( nfailures );
        set_FailureFrequency( failure_frequency );
        if( start_FailureManager( network ) == -1 ){
            fprintf(stderr, "start_FailureManager() failed\n");
            delete network;
            exit(-1);
        }
    }

    fprintf(stderr, "FE: Starting experiments ...\n");
    if( run_ThroughputTest( network, duration, so_file.c_str() ) == -1 ){
        fprintf(stderr, "run_ThroughputTest() failed\n");
        delete network;
        exit(-1);
    }

    if( nfailures > 0 ) {
        fprintf(stderr, "FE: Stoping failure manager ...\n");
        if( stop_FailureManager( ) == -1 ){
            fprintf(stderr, "stop_FailureManager() failed\n");
            delete network;
            exit(-1);
        }

        fprintf(stderr, "FE: Waiting for failure manager ...\n");
        if( waitFor_FailureManager( ) == -1 ){
            fprintf(stderr, "waitFor_FailureManager() failed\n");
            delete network;
            exit(-1);
        }

        fprintf(stderr, "FE: Dumping stats ...\n"); fflush(stderr);
        FailureEvent::dump_FailureRecoveryStatistics( RecoveryStatsFilename );
    }

    fprintf( stderr, "Done!\n");
    //delete network;
    return 0;
}

int run_ThroughputTest( Network * inetwork,
                        uint32_t iduration,
                        const char * iso_filename )
{
    set<uint32_t> uint_eq_class;
    Stream * stream;
    int tag;
    PacketPtr packet;
    
    int filter_id = inetwork->load_FilterFunc( iso_filename, "uint32_EqClass" );
    if( filter_id == -1 ){
        fprintf( stderr, "load_FilterFunc() failure\n" );
        return -1;
    }

    uint32_t nbackends = inetwork->get_BroadcastCommunicator()->
        get_EndPoints().size() ;

    //duration is in secs, DATA_SEND_FREQ is in msecs
    uint32_t nwaves = ( iduration * 1000 ) / DATA_SEND_FREQ;
    uint32_t nsamples_expected = nwaves * nbackends;

    stream = inetwork->new_Stream( inetwork->get_BroadcastCommunicator(),
                                   filter_id, SFILTER_WAITFORALL );

    uint32_t max_val = nsamples_expected;

    //fprintf( stderr,
             //"ThroughputTest(): nbackends: %u, nwaves: %u, "
             //"nsamples: %u, max_val: %u\n",
             //nbackends, nwaves, nsamples_expected, max_val );

    if( ( stream->send( PROT_START, "%ud %ud", nwaves, max_val ) == -1 ) || 
        ( stream->flush() == -1) ){
        fprintf( stderr, "stream::send()/flush(() failure\n" );
        return -1;
    }

    Timer throughput_period_timer, timer;
    ThroughputExperiment thruput_exp, exp;

    //thruput_exp.start();
    exp.start();
    //throughput_period_timer.start();
    unsigned int nsamples=0;;
    timer.start();
    int retval;
    while( true ) {
        tag=0;
        retval = stream->recv( &tag, packet, true );
        if( retval == -1 ) {
            if( stream->is_Closed() ) {
                fprintf( stderr, "FE: stream closed. We're done!\n" );
                break;
            }
            fprintf( stderr, "stream::recv() failure\n" );
            return -1;
        }
        nsamples++;
        exp.insert_DataPoint( nsamples );
        //nsamples_period++;

        assert( tag == PROT_INT_EQCLASS );
        //uint32_t * uint_array;
        //uint32_t uint_array_size;
        //if( packet->unpack( "%aud %ud", &uint_array, &uint_array_size, &nsamples_recvd ) == -1 ){
            //fprintf( stderr, "stream::unpack() failure\n" );
            //return -1;
        //}

        //throughput_period_timer.stop();
        //if( throughput_period_timer.get_latency_msecs() >
            //THROUGHPUT_SAMPLING_INTERVAL ) {
            //thruput_exp.insert_DataPoint( nsamples_recvd );

        //fprintf( stderr, "FE: Received %d of %d samples\n", nsamples,
                 //nsamples_expected );
            //fprintf( stderr, "\tThroughput (last %lf secs): %lf pkts/sec\n\n",
                     //throughput_period_timer.get_latency_secs(),
                     //nsamples_period/throughput_period_timer.get_latency_secs() );
            //reset timer
            //throughput_period_timer.start();
            //nsamples_period=0;
        //}

#if defined( VALIDATE_OUTPUT )
        for( uint32_t j=0; j<uint_array_size; j++ ){
            uint_eq_class.insert( uint_array[j] );
        }
#endif
    }
    timer.stop();
    //thruput_exp.stop();
    exp.stop();

    //Dump results
    FILE * f = fopen( ThroughputStatsFilename, "a" );
    if( f == NULL ) {
        fprintf( stderr, "fopen(\"%s\") failed.\n", ThroughputStatsFilename );
        perror( "fopen()" );
        return -1;
    }

    fprintf( f, "DURATION: %lf NPKTS: %u RATE: %lf\n",
             timer.get_latency_secs(), nsamples,
             nsamples / timer.get_latency_secs() );
    fclose( f );
    exp.dump_Statistics( ThroughputStatsFilename );

    fprintf( stderr, "FE: Sending \"EXIT\" protocol\n" );
    if( ( stream->send(PROT_EXIT, "") == -1 ) || 
        ( stream->flush() == -1) ){
        fprintf( stderr, "stream::send()/flush(() failure\n" );
        return -1;
    }

#if defined( VALIDATE_OUTPUT )
    set<uint32_t> expected_uint_eq_class;
    if( !validate_Output( uint_eq_class, nbackends, nwaves, max_val, expected_uint_eq_class ) ) {
        fprintf( stderr, "FE: Failure: Invalid Output!\n" );

        //dump int_eq_class to file for validation
        char outfile[256];
        snprintf( outfile, sizeof(outfile), "eqclass-fe-recvd");
        if( dump_EqClass( uint_eq_class, outfile ) == -1 ){
            fprintf(stderr, "FE: dump_EqClass() failed!\n");
            return(-1);
        }
        snprintf( outfile, sizeof(outfile), "eqclass-fe-expected");
        if( dump_EqClass( expected_uint_eq_class, outfile ) == -1 ){
            fprintf(stderr, "FE: dump_EqClass() failed!\n");
            return(-1);
        }

        return -1;
    }

    fprintf( stderr, "FE: Output Validated! %u samples, %u backends, %u elements.\n",
             nsamples_recvd, nbackends, uint_eq_class.size() );
#endif

    return 0;
}

