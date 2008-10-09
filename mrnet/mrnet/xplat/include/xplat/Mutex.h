/****************************************************************************
 * Copyright © 2003-2007 Dorian C. Arnold, Philip C. Roth, Barton P. Miller *
 *                  Detailed MRNet usage rights in "LICENSE" file.          *
 ****************************************************************************/

// $Id: Mutex.h,v 1.5 2008/10/09 19:54:07 mjbrim Exp $
#ifndef XPLAT_MUTEX_H
#define XPLAT_MUTEX_H

namespace XPlat
{

class Mutex
{
public:
    class Data
    {
    public:
        virtual ~Data( void ) { }

        virtual int Lock( void ) = 0;
        virtual int Unlock( void ) = 0;
    };

private:
    Data* data;

public:
    Mutex( void );
    virtual ~Mutex( void );

    virtual int Lock( void );
    virtual int Unlock( void );
};

} // namespace XPlat

#endif // XPLAT_MUTEX_H
