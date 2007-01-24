/****************************************************************************
 * Copyright © 2003-2007 Dorian C. Arnold, Philip C. Roth, Barton P. Miller *
 *                  Detailed MRNet usage rights in "LICENSE" file.          *
 ****************************************************************************/

// $Id: Once.h,v 1.4 2007/01/24 19:33:47 darnold Exp $
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
        virtual int DoOnce( void (*func)( void ) ) = 0;
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
