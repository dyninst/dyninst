/***********************************************************************
 * Copyright © 2003 Dorian C. Arnold, Philip C. Roth, Barton P. Miller *
 *                  Detailed MRNet usage rights in "LICENSE" file.     *
 **********************************************************************/

// $Id: TLSKey-pthread.h,v 1.1 2003/11/14 19:27:04 pcroth Exp $
#ifndef PTHR_PTHREADTLSKEY_H
#define PTHR_PTHREADTLSKEY_H

#include <pthread.h>
#include "xplat/TLSKey.h"

namespace XPlat
{

class PthreadTLSKeyData : public TLSKey::Data
{
private:
    pthread_key_t key;
    bool initialized;

public:
    PthreadTLSKeyData( void );
    virtual ~PthreadTLSKeyData( void );

    virtual void* Get( void ) const
    {
        return pthread_getspecific( key );
    }

    virtual int Set( void* data )
    {
        return pthread_setspecific( key, data );
    }
};

} // namespace XPlat

#endif // XPLAT_PTHREADTLSKEY_H
