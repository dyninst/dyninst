/***********************************************************************
 * Copyright © 2003-2004 Dorian C. Arnold, Philip C. Roth, Barton P. Miller *
 *                  Detailed MRNet usage rights in "LICENSE" file.     *
 **********************************************************************/

// $Id: Once.h,v 1.2 2004/03/23 01:12:22 eli Exp $
#ifndef XPLAT_ONCE_H
#define XPLAT_ONCE_H

namespace XPlat
{

class Once
{
public:
    class Data
    {
    public:
        virtual ~Data( void ) { }
        virtual int DoOnce( void (*func)( void ) ) = NULL;
    };

private:
    Data* data;
    
public:
    Once( void );
    virtual ~Once( void )                       { delete data; }

    virtual int DoOnce( void (*func)( void ) )  { return data->DoOnce( func ); }
};

} // namespace XPlat

#endif // XPLAT_ONCE_H
