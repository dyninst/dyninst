/****************************************************************************
 * Copyright © 2003-2007 Dorian C. Arnold, Philip C. Roth, Barton P. Miller *
 *                  Detailed MRNet usage rights in "LICENSE" file.          *
 ****************************************************************************/

// $Id: Thread-pthread.C,v 1.6 2008/10/09 19:53:54 mjbrim Exp $
#include <pthread.h>
#include "xplat/Thread.h"


namespace XPlat
{

int
Thread::Create( Func func, void* data, Id* id )
{
    return pthread_create( (pthread_t*)id, NULL, func, data );
}


Thread::Id
Thread::GetId( void )
{
    return (Thread::Id) pthread_self();
}


int
Thread::Join( Thread::Id joinWith, void** exitValue )
{
    pthread_t jwith = (pthread_t) joinWith;
    return pthread_join( jwith, exitValue );
}

int
Thread::Cancel( Thread::Id iid )
{
    pthread_t id = (pthread_t) iid;
    return pthread_cancel( id );
}

void
Thread::Exit( void* val )
{
    pthread_exit( val );
}

} // namespace XPlat

