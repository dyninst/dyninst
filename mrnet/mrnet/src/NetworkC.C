#include <iostream>
#include "mrnet/h/MRNetC.h"
#include "mrnet/h/MRNet.h"
#include "mrnet/src/StreamImpl.h"
#include <assert.h>

int
MRN_new_Network(const char * _filename,
                                const char * _commnode,
                                const char * _application)
{
    return Network::new_Network( _filename, _commnode, _application );
}

int
MRN_new_NetworkNoBE( const char* cfgFileName,
                                const char* commNodeExe,
                                const char* leafInfoFile )
{
    return  Network::new_NetworkNoBE( cfgFileName, commNodeExe, leafInfoFile );
}



int
MRN_connect_Backends( void )
{
    return Network::connect_Backends();
}





void
MRN_delete_Network()
{
    Network::delete_Network();
}



int
MRN_init_Backend(const char *_hostname, const char *_port,
                             const char *_phostname,
                             const char *_pport, const char *_pid)
{
    return Network::init_Backend( _hostname, _port,
                                    _phostname,
                                    _pport, _pid );
}

void
MRN_error_str(const char *s)
{
    MRN_error_str(s);
}


void*
MRN_get_BroadcastCommunicator( void )
{
    return (void*)Communicator::get_BroadcastCommunicator();
}


void* 
MRN_Stream_new_Stream( void* comm, int fid )
{
    return (void*)Stream::new_Stream( (Communicator*)comm, fid );
}



int
MRN_Stream_recv_any( int* tag, void** buf, void** stream )
{
    return Stream::recv( tag, buf, (Stream**)stream );
}




int
MRN_Stream_flush( void* stream )
{
    return ((Stream*)stream)->flush();
}


int
MRN_Stream_recv( void* stream, int* tag, void** buf )
{
    return ((Stream*)stream)->recv( tag, buf );
}



int
MRN_Stream_send( void* stream, int tag, const char* fmt, ... )
{
    va_list arg_list;

    va_start(arg_list, fmt);
    int ret = ((StreamImpl*)stream)->send_aux( tag, fmt, arg_list );
    va_end(arg_list);

    return ret;
}


int
MRN_Stream_unpack( char* buf, const char* fmt, ... )
{
    va_list arg_list;

    va_start(arg_list, fmt);
    int ret = StreamImpl::unpack( buf, fmt, arg_list );
    va_end(arg_list);

    return ret;
}


