#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>


#include <list>

#include "mrnet/src/Message.h"
#include "mrnet/src/InternalNode.h"
#include "mrnet/src/utils.h"

using namespace MRN;

void BeDaemon( void );

int main(int argc, char **argv)
{
    InternalNode *comm_node;
    int i, status;
    std::list <Packet *> packet_list;

    // become a daemon
    BeDaemon();

    if(argc != 6){
        fprintf(stderr, "Usage: %s hostname port phostname pport pid\n",
                argv[0]);
        fprintf(stderr, "Called with (%d) args: ", argc);
        for(i=0; i<argc; i++){
            fprintf(stderr, "%s ", argv[i]);
        }
        exit(-1);
    }

    std::string hostname(argv[1]);
    unsigned short port = atoi(argv[2]);
    std::string parent_hostname(argv[3]);
    unsigned short parent_port = atol(argv[4]);
    unsigned short parent_id = atol(argv[5]);

    //TLS: setup thread local storage for internal node
    //I am "CommNodeMain(hostname:port)"
    std::string local_hostname;

    //TODO: why are we doing this, don't I know my own hostname?
    getHostName( local_hostname, hostname );

    std::string name("CommNodeMain(");
    name += local_hostname;
    name += ":";
    name += argv[2];
    name += ")";

    if( (status = pthread_key_create(&tsd_key, NULL)) != 0){
        fprintf(stderr, "pthread_key_create(): %s\n", strerror(status)); 
        exit(-1);
    }
    tsd_t * local_data = new tsd_t;
    local_data->thread_id = pthread_self();
    local_data->thread_name = strdup(name.c_str());
    if( (status = pthread_setspecific(tsd_key, local_data)) != 0){
        fprintf(stderr, "pthread_key_create(): %s\n", strerror(status)); 
        exit(-1);
    }

    comm_node = new InternalNode(hostname, port, parent_hostname,
                                 parent_port, parent_id);

    comm_node->waitLoop();

    delete comm_node;
    return 0;
}

void BeDaemon( void )
{
    // become a background process
    pid_t pid = fork();
    if( pid > 0 ) {
        // we're the parent - we want our child to be the real job,
        // so we just exit
        exit(0);
    }
    else if( pid < 0 ) {
        fprintf( stderr, "BE: fork failed to put process in background\n" );
        exit(-1);
    }
    
    // we're the child of the original fork
    pid = fork();
    if( pid > 0 ) {
        // we're the parent in the second fork - exit
        exit(0);
    }
    else if( pid < 0 ) {
        fprintf( stderr, "BE: 2nd fork failed to put process in background\n" );
        exit(-1);
    }

    // we were the child of both forks - we're the one who survives
}
