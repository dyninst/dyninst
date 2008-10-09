/****************************************************************************
 * Copyright © 2003-2007 Dorian C. Arnold, Philip C. Roth, Barton P. Miller *
 *                  Detailed MRNet usage rights in "LICENSE" file.          *
 ****************************************************************************/

#include "mrnet/MRNet.h"
#include "IntegerAddition.h"

using namespace MRN;

int main(int argc, char **argv)
{
    int send_val=32, recv_val=0;
    int tag, retval;
    PacketPtr p;

    if( argc != 4 ){
        fprintf(stderr, "Usage: %s <topology file> <backend_exe> <so_file>\n", argv[0]);
        exit(-1);
    }
    const char * topology_file = argv[1];
    const char * backend_exe = argv[2];
    const char * so_file = argv[3];
    const char * dummy_argv=NULL;

    // This Network() cnstr instantiates the MRNet internal nodes, according to the
    // organization in "topology_file," and the application back-end with any
    // specified cmd line args
    Network * network = new Network( topology_file, backend_exe, &dummy_argv  );

    // Make sure path to "so_file" is in LD_LIBRARY_PATH
    int filter_id = network->load_FilterFunc( so_file, "IntegerAdd" );
    if( filter_id == -1 ){
        fprintf( stderr, "Network::load_FilterFunc() failure\n");
        delete network;
        return -1;
    }

    // A Broadcast communicator contains all the back-ends
    Communicator * comm_BC = network->get_BroadcastCommunicator( );

    // Create a stream that will use the Integer_Add filter for aggregation
    Stream * stream = network->new_Stream( comm_BC, filter_id,
                                           SFILTER_WAITFORALL);

    int num_backends = comm_BC->get_EndPoints().size();

    tag = PROT_SUM;
    unsigned int num_iters=5;
    // Broadcast a control message to back-ends to send us "num_iters"
    // waves of integers
    if( stream->send( tag, "%d %d", send_val, num_iters ) == -1 ){
        fprintf( stderr, "stream::send() failure\n");
        return -1;
    }
    if( stream->flush( ) == -1 ){
        fprintf( stderr, "stream::flush() failure\n");
        return -1;
    }

    // We expect "num_iters" aggregated responses from all back-ends
    for( unsigned int i=0; i<num_iters; i++ ){
        retval = stream->recv(&tag, p);
        assert( retval != 0 ); //shouldn't be 0, either error or block till data
        if( retval == -1){
            //recv error
            return -1;
        }

        if( p->unpack( "%d", &recv_val ) == -1 ){
            fprintf( stderr, "stream::unpack() failure\n");
            return -1;
        }

        if( recv_val != num_backends * i * send_val ){
            fprintf(stderr, "Iteration %d: Success! recv_val(%d) != %d*%d*%d=%d (send_val*i*num_backends)\n",
                    i, recv_val, send_val, i, num_backends, send_val*i*num_backends );
        }
        else{
            fprintf(stderr, "Iteration %d: Success! recv_val(%d) == %d*%d*%d=%d (send_val*i*num_backends)\n",
                    i, recv_val, send_val, i, num_backends, send_val*i*num_backends );
        }
    }

    if(stream->send(PROT_EXIT, "") == -1){
        fprintf( stderr, "stream::send(exit) failure\n");
        return -1;
    }
    if(stream->flush() == -1){
        fprintf( stderr, "stream::flush() failure\n");
        return -1;
    }

    // The Network destructor will cause all internal and leaf tree nodes to exit
    delete network;

    return 0;
}
