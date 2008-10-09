#include "NetworkImpl.h"
#include "test_NetworkGraph.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

namespace MRN
{
extern int mrndebug;
extern const char* mrnBufPtr;
extern unsigned int mrnBufRemaining;
extern int mrn_flex_debug;
int mrnparse();
};

extern FILE *mrnin;

static int parse_config_file( const char* cfg, bool mem_buf=false );
static int dot2jpg( const char * basename );

int main( int argc, char ** argv )
{
    //if( argc != 2 ){
        //fprintf( stderr, "Usage: %s <topology_file>\n", argv[0]);
        //exit( -1 );
    //}

    //char * config_file = argv[1];
    //if( parse_config_file( topology, true ) != 0 ){
    if( parse_config_file( "a.top", false ) != 0 ){
        fprintf( stderr, "parse_config_file() failed\n" );
        exit( -1 );
    }

    if( MRN::NetworkImpl::parsed_graph->validate() == false ){
        fprintf( stderr, "parsed graph invalid\n" );
        exit( -1 );
    }

    //MRN::NetworkImpl::parsed_graph->print_Graph( stdout );
    MRN::NetworkImpl::parsed_graph->print_DOTGraph( "a1.dot" );
    dot2jpg( "a1" );

    printf( "Removing node 25 ... \n");
    MRN::NetworkImpl::parsed_graph->remove_NodeByRank( 25 );
    MRN::NetworkImpl::parsed_graph->print_DOTGraph( "a2.dot" );
    dot2jpg( "a2" );

    std::vector< MRN::NetworkNode * > orphans =
        MRN::NetworkImpl::parsed_graph->get_OrphanedNodes();

    unsigned int new_parent_rank=24;
    for( unsigned int i=0; i<orphans.size(); i++ ){
        printf( "Setting %d's parent to %d\n", orphans[i]->get_Rank(),
                new_parent_rank );
        MRN::NetworkImpl::parsed_graph->set_Parent( orphans[i]->get_Rank(),
                                                    new_parent_rank );
    }

    MRN::NetworkImpl::parsed_graph->print_DOTGraph( "a3.dot" );
    dot2jpg( "a3" );


}

int parse_config_file( const char* config, bool using_mem_buf )
{
    int status;
    //MRN::mrndebug=1;
    //MRN::mrn_flex_debug=1;

    if( using_mem_buf ) {
        // set up to parse config from a buffer in memory
        MRN::mrnBufPtr = config;
        MRN::mrnBufRemaining = strlen( config );
    }
    else{
        mrnin = fopen( config, "r" );
        if( mrnin == NULL ) {
            perror( "fopen()" );
            return -1;
        }
    }

    status = MRN::mrnparse( );

    if( status != 0 ) {
        fprintf( stderr, "mrnparse() failed: %s: Parse Error\n", config );
        return -1;
    }

    return 0;
}

//convert .dot to .jpeg
int dot2jpg( const char * basename )
{
    char command[ 1024 ];

    snprintf( command, sizeof(command),
              "dot -o %s.jpg -T jpeg %s.dot", basename, basename );
    printf( "%s ... ", command );

    if( system( command ) != 0 ){
        printf( "failed!\n" );
        perror( "system()" );
        return -1;
    }
    printf( "success!\n" );

    return 0;
}
