/***********************************************************************
 * Copyright © 2003-2004 Dorian C. Arnold, Philip C. Roth, Barton P. Miller *
 *                  Detailed MRNet usage rights in "LICENSE" file.     *
 **********************************************************************/

// $Id: Thread.h,v 1.2 2004/03/23 01:12:22 eli Exp $
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
