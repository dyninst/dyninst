/***********************************************************************
 * Copyright © 2003 Dorian C. Arnold, Philip C. Roth, Barton P. Miller *
 *                  Detailed MRNet usage rights in "LICENSE" file.     *
 **********************************************************************/

// $Id: Thread.h,v 1.1 2003/11/14 19:36:04 pcroth Exp $
#ifndef XPLAT_THREAD_H
#define XPLAT_THREAD_H

namespace XPlat
{

class Thread
{
public:
    typedef void* (*Func)( void* );
    typedef int Id;

    static int Create( Func func, void* data, Id* id );
    static Id GetId( void );
    static int Join( Id joinWith, void** exitValue );
    static void Exit( void* val );
};

} // namespace XPlat

#endif // XPLAT_THREAD_H
