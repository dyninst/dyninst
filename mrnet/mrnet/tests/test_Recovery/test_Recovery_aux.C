#include <unistd.h>
#include <getopt.h>
#include <assert.h>
#include <sys/time.h>
#include <math.h>

#include "test_Recovery_aux.h"

string get_FrontEndNodeName( );


bool get_HostListFromFile( string &ihostfile,
                           list<string> &oprocess_list )
{
    char cur_host[256];

    FILE * fp = fopen( ihostfile.c_str(), "r" );
    if( !fp ) {
        perror("fopen()");
        return false;
    }


    while( fscanf(fp, "%s", cur_host) != EOF ){
        oprocess_list.push_back( cur_host );
    }

    return true;
}

bool get_HostListFromSLURMFormattedString( string &inodes_str,
                                           list< string > & oprocess_list )
{
    char str_num[32], node_name[256];
    unsigned int num1=0, num2=0;
    bool in_range=false;
    unsigned int start_pos, end_pos;

    //add localhost to head of hostlist
    //string fe = get_FrontEndNodeName();
    //fe += ".llnl.gov:0";
    //oprocess_list.push_front( fe );

    // Find the node list string
    // string format: xxx[0-15, 12, 23, ..., 26-35, ...]
    string::size_type openbracketpos = inodes_str.find_first_of( "[" );
    if( openbracketpos == string::npos ) {
        snprintf( node_name, sizeof(node_name), "%s.llnl.gov",
                  get_FrontEndNodeName().c_str() );
        oprocess_list.push_back( node_name );

        return true;
    }

    string::size_type closebracketpos = inodes_str.find_first_of( "]" );
    assert( closebracketpos != string::npos );
    const char * node_range = strdup( inodes_str.substr( openbracketpos+1, closebracketpos-(openbracketpos+1) ).c_str() );

    // Get the machine name
    string basename = inodes_str.substr( 0, openbracketpos );

    // Decode the node list string 
    for( unsigned int i=0; i<=strlen(node_range)-1; i++ ){
        start_pos = i;
        while( isdigit(node_range[i]) )
            i++;
        end_pos=i-1;

        memcpy( str_num, node_range+start_pos, end_pos-start_pos+1);
        str_num[end_pos-start_pos+1] = '\0';

        if( in_range ){
            in_range=false;
            num2 = atoi( str_num );

            for( unsigned int j=num1; j<= num2; j++ ){
                snprintf( node_name, sizeof(node_name), "%s%u.llnl.gov",
                          basename.c_str(), j );
                oprocess_list.push_back( node_name );
            }
        }
        else{
            num1 = atoi( str_num );
            if( node_range[i] == '-' ){
                in_range=true;
                continue;
            }

            snprintf( node_name, sizeof(node_name), "%s%u.llnl.gov",
                      basename.c_str(), num1 );
            oprocess_list.push_back( node_name );
        }
    }

    oprocess_list.unique();
    return true;
}

string get_FrontEndNodeName( )
{
    char hn[256];

    gethostname( hn, 256 );

    return string( hn );
}

bool validate_Output( set<uint32_t> & irecvd_eq_class, uint32_t inbackends,
                      uint32_t inwaves, uint32_t imax_val,
                      set<uint32_t> & oexpected_uint_eq_class )
{
    //fprintf( stderr, "validate_Output(): nbackends: %u, nwaves: %u, max: %u\n",
             //inbackends, inwaves, imax_val );

    for( unsigned int i=0; i<inbackends; i++ ) {
        srandom( i+1 ); //BEs seed on rank+1
        for( unsigned int j=0; j<inwaves; j++ ) {
            //generate random integer
            uint32_t rand_int = random() % imax_val;
            oexpected_uint_eq_class.insert( rand_int );
            //fprintf( stderr, "be[%u], wave[%u]: %u\n", i,j,rand_int );
        }
    }

    if( irecvd_eq_class == oexpected_uint_eq_class ) {
        return true;
    }

    return false;
}

void proc_ArgList( int iargc,
                   char ** iargv,
                   string &obenchmark_type,
                   uint32_t &oduration,
                   string &obackend_exe,
                   string &oso_file,
                   string &ohosts,
                   string &ohosts_format,
                   uint32_t &oprocs_per_node,
                   uint32_t &onfailures,
                   uint32_t &ofailure_frequency,
                   uint32_t &onorphans,
                   uint32_t &oextra_internal_procs,
                   uint32_t &ojob_id )
{
    static struct option long_options[] = {
        {"macro-benchmark", 0, NULL, 'm'},
        {"job-id", 1, NULL, 'j'},
        {"duration", 1, NULL, 'd'},
        {"backend-exe", 1, NULL, 'b'},
        {"so-file", 1, NULL, 's'},
        {"hostfile", 1, NULL, 'f'},
        {"hostlist", 1, NULL, 'l'},
        {"topology-file", 1, NULL, 't'},
        {"procs-per-node", 1, NULL, 'p'},
        {"num-failures", 1, NULL, 'r'},
        {"failure-frequency", 1, NULL, 'q'},
        {"num-orphans", 1, NULL, 'o'},
        {"extra-internal-nodes", 1, NULL, 'e'},
        {0, 0, 0, 0}
    };
    int option_index = 0;
    int c;

    bool arg_error=false;
    while (true && !arg_error) {
        c = getopt_long (iargc, iargv, "Mmd:j:b:s:f:l:t:p:r:o:q:e:", long_options, &option_index);

		if (c == -1)
            break;

        switch (c) {
        case 'M':
            obenchmark_type = "macro";
            break;
        case 'm':
            obenchmark_type = "micro";
            break;
        case 'd':
            oduration = atoi( optarg );
            break;
        case 'b':
            obackend_exe = optarg;
            break;
        case 's':
            oso_file = optarg;
            break;
        case 'f':
            ohosts = optarg;
            ohosts_format  = "hostfile";
            break;
        case 'l':
            ohosts = optarg;
            ohosts_format  = "slurmlist";
            break;
        case 't':
            ohosts = optarg;
            ohosts_format  = "topology_file";
            break;
        case 'p':
            oprocs_per_node = atoi( optarg );
            break;
        case 'j':
            ojob_id = atoi( optarg );
            break;
        case 'r':
            onfailures = atoi( optarg );
            break;
        case 'q':
            ofailure_frequency = atoi( optarg );
            break;
        case 'o':
            onorphans = atoi( optarg );
            break;
        case 'e':
            oextra_internal_procs = atoi( optarg );
            break;
        default:
            arg_error=true;
            break;
        }
    }

    if( oso_file.empty() ) {
        fprintf( stderr, "Must specify <so_file>\n" );
        arg_error=true;
    }
    if( obackend_exe.empty() ) {
        fprintf( stderr, "Must specify <backend_exe>\n" );
        arg_error=true;
    }
    if( ohosts.empty() ) {
        fprintf( stderr, "Must specify <hostfile|hostlist|topology_file>\n" );
        arg_error=true;
    }

    if( obenchmark_type == "unknown" ) {
        fprintf( stderr, "Must specify <benchmarktype>\n" );
        arg_error=true;
    }

    if ( arg_error ) {
        fprintf(stderr, "Usage: %s [OPTIONS]\n"
                "Options are:\n"
                "\t-o,--num-orphans <norphans>\n"
                "\t-d,--duration <nsecs>\n"
                "\t-f,--hostfile <hostfile>\n"
                "\t-l,--hostlist <hostlist>\n"
                "\t-b,--backend-exe <backend_exe>\n"
                "\t-s,--so-file <so_file>\n"
                "\t-p,--procs-per-node <procs_per_node>\n"
                "\t-i,--inject-failure\n"
                "\t-e,--extra-internal-nodes\n",
                iargv[0] );
        exit(-1);
    }
}
