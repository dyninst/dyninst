/****************************************************************************
 * Copyright © 2003-2005 Dorian C. Arnold, Philip C. Roth, Barton P. Miller *
 *                  Detailed MRNet usage rights in "LICENSE" file.          *
 ****************************************************************************/

#include <stdio.h>

#if !defined(os_windows)
#include <sys/time.h>
#include <unistd.h>
#include <sys/types.h>
#endif // !defined(os_windows)


#include <list>

#include "mrnet/src/Message.h"
#include "mrnet/src/InternalNode.h"
#include "mrnet/src/utils.h"
#include "xplat/NetUtils.h"


using namespace MRN;

void BeDaemon( void );

int main(int argc, char **argv)
{
    InternalNode *comm_node;
    int i, status;
    //std::list <Packet *> packet_list;

    //set_OutputLevel(5);
    if(argc != 4){
        fprintf(stderr, "Usage: %s port phostname pport\n",
                argv[0]);
        fprintf(stderr, "Called with (%d) args: ", argc);
        for(i=0; i<argc; i++){
            fprintf(stderr, "%s ", argv[i]);
        }
        exit(-1);
    }

    // become a daemon
    BeDaemon();

    // get our own hostname
    std::string hostname;
    getNetworkName( hostname, XPlat::NetUtils::GetNetworkName() );

    Port port = atoi(argv[1]);
    std::string parent_hostname(argv[2]);
    Port parent_port = atol(argv[3]);

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

    tsd_t * local_data = new tsd_t;
    local_data->thread_id = XPlat::Thread::GetId();
    local_data->thread_name = strdup(name.c_str());

    if( (status = tsd_key.Set( local_data )) != 0){
        fprintf(stderr, "XPlat::TLSKey::Set(): %s\n", strerror(status)); 
        exit(-1);
    }

    comm_node = new InternalNode(hostname, port,
                                parent_hostname, parent_port );

    comm_node->waitLoop();

    delete comm_node;

    return 0;
}

void BeDaemon( void )
{
#if !defined (os_windows)
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
#else
//TODO: figure out how to run in background on windows
#endif /* !defined(os_windows) */
}
