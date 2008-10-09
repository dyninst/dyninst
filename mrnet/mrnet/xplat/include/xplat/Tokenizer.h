/****************************************************************************
 * Copyright © 2003-2007 Dorian C. Arnold, Philip C. Roth, Barton P. Miller *
 *                  Detailed MRNet usage rights in "LICENSE" file.          *
 ****************************************************************************/

// $Id: Tokenizer.h,v 1.4 2008/10/09 19:54:14 mjbrim Exp $
#ifndef XPLAT_TOKENIZER_H
#define XPLAT_TOKENIZER_H

#include <string>

namespace XPlat
{

class Tokenizer
{
private:
    const std::string &s;
    std::string::size_type curPos;

public:
    Tokenizer( const std::string& _s );

    void Reset( void );

    std::string::size_type
        GetNextToken( std::string::size_type& len, const char* delim = " \t" );

};

} // namespace XPlat

#endif // XPLAT_TOKENIZER_H
