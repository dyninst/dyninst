/***********************************************************************
 * Copyright © 2003-2004 Dorian C. Arnold, Philip C. Roth, Barton P. Miller *
 *                  Detailed MRNet usage rights in "LICENSE" file.     *
 **********************************************************************/

// $Id: TLSKey-pthread.C,v 1.3 2005/03/21 18:58:32 darnold Exp $
#include <assert.h>
#include "TLSKey-pthread.h"

namespace XPlat
{

TLSKey::TLSKey( void )
  : data( new PthreadTLSKeyData )
{
    // nothing else to do
}


PthreadTLSKeyData::PthreadTLSKeyData( void )
  : initialized( false )
{
    int cret = pthread_key_create( &key, NULL );
    if( cret == 0 )
    {
        initialized = true;
    }
}


PthreadTLSKeyData::~PthreadTLSKeyData( void )
{
    if( initialized )
    {
        pthread_key_delete( key );
    }
}

} // namespace XPlat

