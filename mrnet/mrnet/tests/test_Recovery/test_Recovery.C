#include "test_Recovery.h"

#include <stdio.h>

using namespace std;

int dump_EqClass( set<uint32_t> ieq_class, const char * ifilename )
{
    FILE * fp = fopen( ifilename, "w" );

    if( !fp ) {
        perror( "fopen()" );
        return -1;
    }

    set<uint32_t>::const_iterator iter;
    for( iter=ieq_class.begin(); iter!=ieq_class.end(); iter++ ) {
        fprintf( fp, "%u\n", *iter );
    }
    fclose( fp );

    return 0;
}
