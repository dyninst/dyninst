/***********************************************************************
 * Copyright © 2003-2004 Dorian C. Arnold, Philip C. Roth, Barton P. Miller *
 *                  Detailed MRNet usage rights in "LICENSE" file.     *
 **********************************************************************/

// $Id: Thread-pthread.C,v 1.4 2004/03/23 01:12:24 eli Exp $
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


void
Thread::Exit( void* val )
{
    pthread_exit( val );
}

} // namespace XPlat

