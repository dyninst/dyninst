/***********************************************************************
 * Copyright © 2003-2004 Dorian C. Arnold, Philip C. Roth, Barton P. Miller *
 *                  Detailed MRNet usage rights in "LICENSE" file.     *
 **********************************************************************/

// $Id: Mutex.h,v 1.2 2004/03/23 01:12:22 eli Exp $
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

        virtual int Lock( void ) = NULL;
        virtual int Unlock( void ) = NULL;
    };

private:
    Data* data;

public:
    Mutex( void );
    virtual ~Mutex( void )          { delete data; }

    virtual int Lock( void )        { return data->Lock(); }
    virtual int Unlock( void )      { return data->Unlock(); }
};

} // namespace XPlat

#endif // XPLAT_MUTEX_H
