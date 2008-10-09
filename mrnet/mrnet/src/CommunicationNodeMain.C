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

#include "mrnet/MRNet.h"
#include "xplat/NetUtils.h"
#include "Message.h"
#include "InternalNode.h"
#include "utils.h"

using namespace MRN;
using namespace XPlat;

void BeDaemon( void );

int main(int argc, char **argv)
{
    InternalNode *comm_node;
    int i, status;

    if(argc != 6){
        fprintf(stderr, "Usage: %s local_hostname local_rank parent_hostname parent_port parent_rank\n",
                argv[0]);
        fprintf(stderr, "Called with (%d) args: ", argc);
        for(i=0; i<argc; i++){
            fprintf(stderr, "%s ", argv[i]);
        }
        return -1;
    }

    Network * network = new Network;

    // become a daemon
    BeDaemon();

    std::string hostname;
    std::string parent_hostname;
    XPlat::NetUtils::GetHostName( argv[1], hostname );
    Rank rank = (Rank)strtoul( argv[2], NULL, 10 );
    setrank( rank );
    XPlat::NetUtils::GetHostName( argv[3], parent_hostname );
    Port parent_port = (Port)strtoul( argv[4], NULL, 10 );
    Rank parent_rank = (Rank)strtoul( argv[5], NULL, 10 );

    //TLS: setup thread local storage for internal node
    //I am "CommNodeMain(hostname:port)"

    std::string name("COMM(");
    name += hostname;
    name += ":";
    name += argv[2];
    name += ")";

    tsd_t * local_data = new tsd_t;
    local_data->thread_id = XPlat::Thread::GetId();
    local_data->thread_name = strdup(name.c_str());

    if( (status = tsd_key.Set( local_data )) != 0){
        fprintf(stderr, "XPlat::TLSKey::Set(): %s\n", strerror(status)); 
        return -1;
    }

    mrn_dbg( 5, mrn_printf(FLF, stderr,
                           "InternalNode(local[%u]:\"%s\", parent[%u]:\"%s:%u\")\n",
                           rank, hostname.c_str(),
                           parent_rank, parent_hostname.c_str(), parent_port ));
    comm_node = new InternalNode( network, hostname, rank,
                                  parent_hostname, parent_port, parent_rank );

    comm_node->waitfor_NetworkTermination();

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
