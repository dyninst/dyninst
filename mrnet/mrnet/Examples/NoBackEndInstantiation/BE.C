#include "mrnet/MRNet.h"
#include "header.h"

using namespace MRN;

int main(int argc, char **argv) {

    // Command line args are: parent_hostname, parent_port, parent_rank, my_rank
    // these are used to instantiate the backend network object

    const char* parHostname = argv[argc-4];
    Port parPort = (Port)strtoul( argv[argc-3], NULL, 10 );
    Rank parRank = (Rank)strtoul( argv[argc-2], NULL, 10 );
    Rank myRank = (Rank)strtoul( argv[argc-1], NULL, 10 );
				
    int32_t recv_int=0;
    int tag;   
    PacketPtr p;
    Stream* stream;
    Network * network = NULL;

    char myHostname[64];
    while( gethostname(myHostname, 64) == -1 );

    fprintf( stdout, "Backend %s[%d] connecting to %s:%d\n",
             myHostname, myRank, parHostname, parPort );

    network = new Network(parHostname, parPort, parRank, myHostname, myRank);

    do{
        if ( network->recv(&tag, p, &stream) != 1){
            printf("receive failure\n");
            return -1;
        }

        switch( tag ) {
        case PROT_INT:
            if( p->unpack( "%d", &recv_int) == -1 ){
                printf("stream::unpack(%%d) failure\n");
                return -1;
            }

            printf("BE received int = %d\n", recv_int);

            if ( (stream->send(PROT_INT, "%d", recv_int) == -1) ||
                 (stream->flush() == -1 ) ){
                printf("stream::send(%d) failure\n");
                return -1;
            }
            break;
        default:
            break;
        }
    } while ( tag != PROT_EXIT );

    // FE delete of the network will causes us to exit, wait for it
    sleep(5);

    return 0;
}
