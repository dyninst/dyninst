/***********************************************************************
 * Copyright © 2003 Dorian C. Arnold, Philip C. Roth, Barton P. Miller *
 *                  Detailed MRNet usage rights in "LICENSE" file.     *
 **********************************************************************/

// $Id: Mutex.h,v 1.1 2003/11/14 19:36:04 pcroth Exp $
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
