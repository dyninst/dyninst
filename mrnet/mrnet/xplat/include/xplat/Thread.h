/****************************************************************************
 * Copyright © 2003-2007 Dorian C. Arnold, Philip C. Roth, Barton P. Miller *
 *                  Detailed MRNet usage rights in "LICENSE" file.          *
 ****************************************************************************/

// $Id: Thread.h,v 1.5 2008/10/09 19:53:53 mjbrim Exp $
#ifndef XPLAT_THREAD_H
#define XPLAT_THREAD_H

namespace XPlat
{

class Thread
{
public:
    typedef void* (*Func)( void* );
    typedef long Id;

    static int Create( Func func, void* data, Id* id );
    static Id GetId( void );
    static int Join( Id joinWith, void** exitValue );
    static int Cancel( Id id );
    static void Exit( void* val );
};

} // namespace XPlat

#endif // XPLAT_THREAD_H
