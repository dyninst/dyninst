/***********************************************************************
 * Copyright © 2003 Dorian C. Arnold, Philip C. Roth, Barton P. Miller *
 *                  Detailed MRNet usage rights in "LICENSE" file.     *
 **********************************************************************/

// $Id: Tokenizer.h,v 1.1 2003/11/14 19:36:04 pcroth Exp $
#ifndef XPLAT_TOKENIZER_H
#define XPLAT_TOKENIZER_H

#include <string>

namespace XPlat
{

class Tokenizer
{
private:
    const std::string& s;
    std::string::size_type curPos;

public:
    Tokenizer( const std::string& _s )
      : s( _s )
    {
        Reset();
    }

    void Reset( void )
    {
        curPos = 0;
    }

    std::string::size_type
    GetNextToken( std::string::size_type& len, const char* delim = " \t" )
    {
        std::string::size_type ret = curPos;

        if( curPos != std::string::npos )
        {
            // we're not yet at the end of the string
            // skip any whitespace starting at curPos
            curPos = s.find_first_not_of( delim, curPos );

            // curPos now points at start of next token, if there is one
            ret = curPos;

            // check if there actually is another token
            if( curPos != std::string::npos )
            {
                // there is at least one more token
                std::string::size_type curEnd = s.find_first_of( delim, curPos );
                if( curEnd == std::string::npos )
                {
                    // this is definitely the last token
                    curEnd = s.length();
                }
                len = curEnd - curPos;

                // advance curPos past the current token
                curPos += len;
                if( curPos == s.length() )
                {
                    curPos = std::string::npos;
                }
            }
        }

        if( ret == std::string::npos )
        {
            len = 0;
        }
        return ret;
    }
};

} // namespace XPlat

#endif // XPLAT_TOKENIZER_H
