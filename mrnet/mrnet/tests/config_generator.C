/***********************************************************************
 * Copyright © 2003-2004 Dorian C. Arnold, Philip C. Roth, Barton P. Miller *
 *                  Detailed MRNet usage rights in "LICENSE" file.     *
 **********************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <getopt.h>

#include <vector>
#include <string>

#include "Topology.h"
#include "Tree.h"

static std::vector<std::string> get_HostsFromFile( FILE *);
static void print_usage();

int main(int argc, char **argv)
{
    std::string machine_file="", output_file="", topology="", topology_type="";
    std::vector<std::string> hosts;
    int c;
    FILE * infile, *outfile;

    while (1) {
        int option_index = 0;
        static struct option long_options[] = {
            {"balanced", 1, 0, 'b'},
            {"binomial", 1, 0, 'n'},
            {"other", 1, 0, 'o'},
            {0, 0, 0, 0}
        };

        c = getopt_long (argc, argv, "b:n:o", long_options, &option_index);
        if (c == -1)
            break;

        switch (c) {
        case 'b':
            topology_type = "balanced";
            topology = optarg;
            break;
        case 'n':
            topology_type = "binomial";
            topology = optarg;
            break;
        case 'o':
            topology_type = "other";
            topology = optarg;
            break;
        default:
            print_usage();
            break;
        }
    }

    //hack alert:if topology_type is 2nd to last arg, only infile is specified
    //           if topology_type is 3rd to last arg, outfile is also specified
    if( topology == argv[argc-2] ||
        ( std::string(argv[argc-2]).find('=') != std::string::npos ) ){
        machine_file = argv[argc-1];
    }
    else if( topology == argv[argc-3] ||
             ( std::string(argv[argc-3]).find('=') != std::string::npos ) ){
        output_file = argv[argc-1];
        machine_file = argv[argc-2];
    }
    else if( topology != argv[argc-1] ||
             ( std::string(argv[argc-1]).find('=') != std::string::npos ) ) {
        fprintf(stderr, "topology_type \"%s\" should be last, 2nd to last or 3rd to last!\n",
                topology.c_str());
        print_usage();
    }

    if( machine_file == "" ){
        infile = stdin;
    }
    else{
        infile = fopen(machine_file.c_str(), "r");
        if( !infile ){
            fprintf(stderr, "%s (reading):", machine_file.c_str() );
            perror("fopen()");
            exit(-1);
        }
    }
    if( output_file == "" ){
        outfile = stdout;
    }
    else{
        outfile = fopen(output_file.c_str(), "w");
        if( !outfile ){
            fprintf(stderr, "%s (writing): ", output_file.c_str() );
            perror("fopen()");
            exit(-1);
        }
    }

    if( topology_type == "binomial" ){
        fprintf(stderr, "binomial topologies not yet supported\n" );
        exit(-1);
    }
    else if( topology_type == "balanced" ){
        //fprintf(stderr, "Creating balanced topology \"%s\" from \"%s\" to \"%s\"\n",
                //topology.c_str(),
                //( machine_file == "" ? "stdin" : machine_file.c_str() ),
                //( output_file == "" ? "stdout" : output_file.c_str() ) );
    }
    else if( topology_type == "other" ){
        //fprintf(stderr, "Creating generic topology \"%s\" from \"%s\" to \"%s\"\n",
                //topology.c_str(),
                //( machine_file == "" ? "stdin" : machine_file.c_str() ),
                //( output_file == "" ? "stdout" : output_file.c_str() ) );
    }
    else{
        fprintf(stderr, "Error: Unknown topology \"%s\".", topology.c_str() );
        exit(-1);
    }

    hosts = get_HostsFromFile(infile);

    Tree * tree;
    if( topology_type == "balanced" ){
        tree = new BalancedTree( hosts, topology );
    }
    else if( topology_type == "other" ){
        tree = new GenericTree( hosts, topology );
    }

    tree->create_TopologyFile( outfile );

    if ( !tree->validate() ){
        if( tree->contains_Cycle() ){
            fprintf(stderr, "Tree contains cycle: check hostfile for duplicate machine specifications\n");
            exit(-1);
        }
        if( tree->contains_UnreachableNodes() ){
            fprintf(stderr, "Tree contains unreachable nodes: check hostfile for duplicate machine specifications\n");
            exit(-1);
        }
    }
    return 0;
}

std::vector<std::string> get_HostsFromFile(FILE * f)
{
    std::vector<std::string> hosts;
    char tmp_str[128], tmp_str2[128], *num_proc_str;
    char * cur_host;
    unsigned int num_procs;

    while( fscanf(f, "%s", tmp_str) != EOF ){
        cur_host = tmp_str;
        num_procs = 1;

        if( (num_proc_str = strstr(tmp_str, ":")) != NULL ){
            *(num_proc_str) = '\0'; //null terminate host name
            num_procs = atoi(num_proc_str+1);
        }

        for(unsigned int i=0; i<num_procs; i++){
            sprintf(tmp_str2, "%s:%d", cur_host, i);
            hosts.push_back( tmp_str2 ); //place in host vector
        }
    }

    return hosts;
}

void print_usage()
{
    fprintf( stderr, "\nUsage: mrnet_topgen <OPTION> [INFILE] [OUTFILE]\n\n"

             "Create a MRNet topology specification from machine list in [INFILE],\n"
             "or standard input, and writes output to [OUTFILE], or standard output.\n"
             "\t-b topology, --balanced=topology\n"
             "\t\tCreate a balanced tree using \"topology\" specification. The specification\n"
             "\t\tis in the format DxN, where D is the fan-out or out degree of each node and N is\n"
             "\t\tthe number of leaves or back-ends.\n\n"

             "\t-n topology, --binomial=topology\n"
             "\t\tCurrently unsupported\n\n"

             "\t-o topology, --othter=topology\n"
             "\t\tCreate a generic tree using \"topology\" specification. The specification\n"
             "\t\tfor this option is (the agreeably complicated) N:N,N,N:... where N specifies\n"
             "\t\tthe number of children a node has, ',' distinguishes nodes on the same level,\n"
             "\t\tand ':' separates the tree into levels.\n\n"
             "\t\tExample 1: \"2:2,2\" specifies a tree where the root has 2 children\n"
             "\t\t           and each child on the 2nd level has 2 children.\n"
             "\t\tExample 2: \"2:8,4\" specifies a tree where the root has 2 children.\n"
             "\t\t           At the 2nd level, the 1st child has 8 children, and the\n"
             "\t\t           2nd child has 4 children\n" );
    exit(-1);
}
