/***********************************************************************
 * Copyright © 2003 Dorian C. Arnold, Philip C. Roth, Barton P. Miller *
 *                  Detailed MRNet usage rights in "LICENSE" file.     *
 **********************************************************************/

#ifndef MRNETWORKC_H
#define MRNETWORKC_H

#define AGGR_INT_SUM_ID 2000
#define AGGR_FLOAT_AVG_ID 2001
#define AGGR_FLOAT_MAX_ID 2006
#define AGGR_CHARARRAY_CONCAT_ID 2007
#define AGGR_INT_EQ_CLASS_ID 2008


// TEMPORARY C interfact to MRNet.
// needed because of incompatibilities between xlC and gcc compilers on AIX,
// and bug in gcc stdc++ libraries.
extern "C"
{

// class MC_Network methods
int MRN_new_Network(const char * _filename, 
                            const char * _commnode,
                            const char * _backend);
int MRN_new_NetworkNoBE(const char * _filename,
                            const char * _commnode,
                            const char* leafInfoFile );
int MRN_connect_Backends( void );
void MRN_delete_Network();
int MRN_init_Backend(const char *hostname, const char *port,
                          const char *phostname, const char *pport,
                          const char *pid);
void MRN_error_str(const char *);


void* MRN_get_BroadcastCommunicator( void );

void* MRN_Stream_new_Stream( void* comm, int fid );
int MRN_Stream_recv_any( int* tag, void** buf, void** stream );
int MRN_Stream_unpack( char* buf, const char* fmt, ... );

int MRN_Stream_send( void* stream, int tag, const char* format_str, ... );
int MRN_Stream_flush( void* stream );
int MRN_Stream_recv( void* stream, int* tag, void** buf );

}   // end extern "C"

#endif // MRNETWORKC_H
