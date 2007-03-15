/****************************************************************************
 * Copyright © 2003-2007 Dorian C. Arnold, Philip C. Roth, Barton P. Miller *
 *                  Detailed MRNet usage rights in "LICENSE" file.          *
 ****************************************************************************/

#include <stdio.h>

#if !defined(os_windows)
#include <sys/time.h>
#include <unistd.h>
#include <sys/types.h>
#endif // !defined(os_windows)


#include <list>

#include "Message.h"
#include "InternalNode.h"
#include "utils.h"
#include "xplat/NetUtils.h"

using namespace MRN;

void BeDaemon( void );

int main(int argc, char **argv)
{
    InternalNode *comm_node;
    int i, status;
    //std::list <Packet *> packet_list;

    //set_OutputLevel(5);
    if(argc != 5){
        fprintf(stderr, "Usage: %s lhostname lport phostname pport\n",
                argv[0]);
        fprintf(stderr, "Called with (%d) args: ", argc);
        for(i=0; i<argc; i++){
            fprintf(stderr, "%s ", argv[i]);
        }
        exit(-1);
    }

    // become a daemon
    BeDaemon();


    std::string hostname;
    XPlat::NetUtils::GetHostName( argv[1], hostname );
    Port port = atoi(argv[2]);
    std::string parent_hostname;
    XPlat::NetUtils::GetHostName( argv[3], parent_hostname );
    Port parent_port = atol(argv[4]);

    //TLS: setup thread local storage for internal node
    //I am "CommNodeMain(hostname:port)"

    std::string name("CommNodeMain(");
    name += hostname;
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
    free( (char *)(local_data->thread_name) );
    delete local_data;

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
