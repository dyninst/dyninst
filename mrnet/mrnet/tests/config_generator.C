#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <vector>

int main(int argc, char **argv)
{
    char * machine_file, *outfile, *befile;
    unsigned int num_backends, fanout, i, j, tmp_int;
    char tmp_str[256], tmp_str2[256], *num_proc_str;
    FILE *f;
    unsigned int cur_parent=0, next_orphan=0, num_nodes=0, depth=0, pow;
    std::vector<char *> hosts;

    if(argc != 6){
        fprintf(stderr,
            "usage: %s <infile> <outfile> <befile> <num_backends> <fan-out>\n",
                argv[0] );
        exit(-1);
    }

    machine_file = argv[1];
    outfile = argv[2];
    befile = argv[3];
    num_backends = atoi(argv[4]);
    fanout = atoi(argv[5]);

    if( num_backends <= 0 ||
        fanout <= 0 ){
        fprintf(stderr, "num_backends: %d and fanout:%d must be > 0\n",
                num_backends, fanout);
        exit(-1);
    }

    tmp_int = num_backends;
    depth=0;
    do{
        if( tmp_int % fanout != 0 ){
            fprintf(stderr, "num_backends: %d must be a power of fanout:%d\n",
                    num_backends, fanout);
            exit(-1);
        }
        tmp_int /= fanout;
        depth++;
    } while (tmp_int != 1);

    pow=1;
    for(i=0; i<=depth; i++){
        num_nodes += pow;
        pow*=fanout;
    }
    f = fopen(machine_file, "r");
    if( !f ){
        perror("fopen()");
        exit(-1);
    }

    char * cur_host;
    unsigned int num_procs;
    while( fscanf(f, "%s", tmp_str) != EOF ){
        //fprintf(stderr, "processing %s\n");
        cur_host = tmp_str;
        num_procs = 1;

        if( (num_proc_str = strstr(tmp_str, ":")) != NULL ){
            *(num_proc_str) = '\0'; //null terminate host name
            num_procs = atoi(num_proc_str+1);
        }
        //fprintf(stderr, "host: %s, num_procs: %d\n", cur_host, num_procs);
        for(i=0; i<num_procs; i++){
            //fprintf(stderr, "inserting %s:%d\n", cur_host, i);
            sprintf(tmp_str2, "%s:%d", cur_host, i);
            hosts.push_back( strdup(tmp_str2) ); //place in host vector
        }
    }
    fclose(f);

    if( hosts.size() < num_nodes ){
        fprintf(stderr, "not enough nodes in %s: %d of %d\n",
                machine_file, (int)hosts.size(), num_nodes);
        exit(-1);
    }
    
    f = fopen(outfile, "w");
    if( !f ){
        perror("fopen()");
        exit(-1);
    }

    next_orphan = 1;
    cur_parent = 0;
    fprintf(stderr, "%d backends, %d fanout, %d nodes\n", num_backends,
            fanout, num_nodes);
    for(i=1; i<num_nodes; ){
        fprintf(f, "%s => ", hosts[cur_parent++]);

        for(j=0; j<fanout; j++){
            fprintf(f, "%s ", hosts[next_orphan++]);
            i++;
        }
        fprintf(f, ";\n");
    }
    fclose(f);

    f = fopen(befile, "w");
    if( !f )
    {
        perror("fopen()");
        exit(-1);
    }

    for(i=cur_parent; i<num_nodes; i++ )
    {
        fprintf( f, "%s\n", hosts[i] );
    }
    fclose(f);

    exit(0);
}
