#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

#include "mrnet/h/MR_Network.h"
#include "mrnet/src/Filter.h"
#include "mrnet/tests/timer.h"
#include "mrnet/tests/microbench.h"

using namespace MRN;

int main(int argc, char **argv){
    char *topology_file, *application, *commnode_exe;
    int num_exps, num_waves;
    Stream * stream_BC;
    int tag;
    int i, j;
    char * buf=NULL;
    mb_time *bc_start, *bc_end, *red_start, *red_lat_end, *red_thru_end;
    mb_time startup_start, startup_end;
    struct timeval tmp_tv;
    FILE * f;
    char output_filename[256];
    double avg_red_lat, avg_bc_lat, avg_red_thru;
    double total_red_lat=0, total_bc_lat=0, total_red_thru=0;
    double *bc_lat, *red_lat, *red_thru, startup_lat;
    double double_val;
    int retval;
    
    if(argc !=6){
        fprintf(stderr, "Usage: %s <num_exps> <num_waves> <topology file>"
                " <application exe>  <commnode exe>\n", argv[0]);
        exit(-1);
    }
    
    num_exps = atoi( argv[1] );
    num_waves = atoi(argv[2]);
    topology_file = argv[3];
    application = argv[4];
    commnode_exe = argv[5];
    
    // set the output filename
    sprintf( output_filename, "%s-%d.out", topology_file, getpid() );

    bc_start      = new mb_time[num_exps];
    bc_end        = new mb_time[num_exps];
    red_start     = new mb_time[num_exps];
    red_lat_end   = new mb_time[num_exps];
    red_thru_end  = new mb_time[num_exps];

    bc_lat   = new double[num_exps];
    red_lat  = new double[num_exps];
    red_thru = new double[num_exps];
    
    startup_start.set_time();
    if( Network::new_Network(topology_file, application, commnode_exe) == -1){
        fprintf(stderr, "%s: Network Initialization failed\n", argv[0]);
        Network::error_str(argv[0]);
        exit(-1);
    }
    
    Communicator * comm_BC = Communicator::get_BroadcastCommunicator();
    
    stream_BC = Stream::new_Stream(comm_BC, AGGR_FLOAT_MAX_ID);
    
    //make a broadcast request for start time, then await result
    if( stream_BC->send(MB_SEND_STARTUP_TIME, "%d", num_waves) == -1 ){
        fprintf(stderr, "%s: stream.send() failed\n", argv[0]);
        exit(-1);
    }
    if(stream_BC->flush() == -1){
        fprintf(stderr, "%s: stream.flush() failed\n", argv[0]);
        exit(-1);
    }
    while( (retval = stream_BC->recv(&tag, (void **)&buf)) == 0 ) ;
    if(retval == -1){
        fprintf(stderr, "%s: stream.recv() failed\n", argv[0]);
        exit(-1);
    }
    stream_BC->unpack(buf, "%lf", &double_val);
    fprintf(stderr, "frontend recv'd startup_end: %lf\n", double_val);
    startup_end.set_time(double_val);
    
    for( i=0; i<num_exps; i++){
        //make a broadcast request for broadcast recv time, then await
        //num_wave results
        //synchronize BEs to start waves about 10secs after broadcast
        bc_start[i].set_time();
        bc_start[i].get_time( &tmp_tv );

        //printf("FE setting bc_start: %ld.%ld\n", tmp_tv.tv_sec,
               //tmp_tv.tv_usec );
        //printf("FE setting bc_start: %lf\n", bc_start[i].get_double_time());

        tmp_tv.tv_sec += 2; tmp_tv.tv_usec = 0;
        red_start[i].set_time(tmp_tv);
        
        if( stream_BC->send(MB_SEND_BROADCAST_RECV_TIME, "%d %ld", num_waves,
                            tmp_tv.tv_sec) == -1 ){
            fprintf(stderr, "%s: stream.send() failed\n", argv[0]);
            exit(-1);
        }
        if(stream_BC->flush() == -1){
            fprintf(stderr, "FFF: stream.flush() failed\n");
            exit(-1);
        }
        
        for (j = 0; j < num_waves; ){
            Stream * stream;
            
            if(stream_BC->recv(&tag, (void **)&buf) > 0){
                if( j == 0 ){
                    stream->unpack(buf, "%lf", &double_val);
                    bc_end[i].set_time(double_val);
                    
                    red_lat_end[i].set_time();
                }
                if( j == num_waves-1 ){
                    red_thru_end[i].set_time();
                }
                j++;
            }
        }
    }
    
    //tell backends to go away
    if( stream_BC->send(MB_EXIT, "%d", num_waves) == -1 ||
        stream_BC->flush() == -1 ){
        fprintf(stderr, "%s: stream.send() failed\n", argv[0]);
        exit(-1);
    }
    Network::delete_Network();
    
    f = fopen(output_filename, "a"); assert(f);
    fprintf(f, "EXP: %s %s num_waves:%d num_exps:%d\n"
            "======================================\n", argv[1],
            topology_file, num_waves, num_exps);
    fprintf(f, "RAW:\n");
    for(i=0; i<num_exps; i++){
        //normalize start time of app to zero
        //fprintf(f, "subtracting %lf - %lf\n", bc_start[i].get_double_time(), startup_start.get_double_time() );
        //bc_start[i]     -= startup_start.get_double_time();
        //bc_end[i]       -= startup_start.get_double_time();
        //red_start[i]    -= startup_start.get_double_time();
        //red_lat_end[i]  -= startup_start.get_double_time();
        //red_thru_end[i] -= startup_start.get_double_time();
        fprintf(f, "\texp[%d]: ", i);
        fprintf(f, "bc_start: %.4lf, bc_end: %.4lf, red_start: %.4lf, "
                "red_lat_end: %.4lf, red_thru_end: %.4lf\n",
                bc_start[i].get_double_time(),
                bc_end[i].get_double_time(),
                red_start[i].get_double_time(),
                red_lat_end[i].get_double_time(),
                red_thru_end[i].get_double_time() );
        
        bc_lat[i] = (bc_end[i] - bc_start[i]).get_double_time();
        red_lat[i] = (red_lat_end[i] - red_start[i]).get_double_time();
        red_thru[i] = ( (double)num_waves) /
            ( (red_thru_end[i] - red_start[i]).get_double_time() ) ;
        //fprintf(f, "SUMMARY: bc_lat: %.4lf red_lat: %.4lf red_thru: %.4lf\n",
                //bc_lat[i], red_lat[i], red_thru[i]);
    }
    //fprintf(f, "\t\t*** init: %f  backend_start: %f\n",
            //startup_start.get_double_time(), startup_end.get_double_time() );
    //startup_end   -= startup_start.get_double_time();
    //startup_start -= startup_start.get_double_time();
    
    for(i=0; i<num_exps; i++){
        total_bc_lat += bc_lat[i];
        total_red_lat += red_lat[i];
        total_red_thru += red_thru[i];
    }
    avg_bc_lat   = total_bc_lat / ( (double)num_exps );
    avg_red_lat  = total_red_lat / ( (double)num_exps );
    avg_red_thru = total_red_thru / ( (double)num_exps );
    startup_lat = startup_end.get_double_time() - startup_start.get_double_time();

    fprintf(f, "AVERAGE: bc_lat: %.4lf red_lat: %.4lf red_thru: %.4lf start_lat: %.4lf\n\n",
            avg_bc_lat, avg_red_lat, avg_red_thru, startup_lat);
    fclose(f);
    exit(0);
}

