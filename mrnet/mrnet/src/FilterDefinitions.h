/***********************************************************************
 * Copyright © 2003-2004 Dorian C. Arnold, Philip C. Roth, Barton P. Miller *
 *                  Detailed MRNet usage rights in "LICENSE" file.     *
 **********************************************************************/

#if !defined( filterdefinitions_h )
#define filterdefinitions_h 1

#include <list>
#include <vector>
#include "mrnet/FilterIds.h"


namespace MRN
{

class Packet;
class RemoteNode;


// transformation filters
extern const char * TFILTER_NULL_FORMATSTR;

extern const char * TFILTER_SUM_FORMATSTR;
void tfilter_Sum( const std::vector < Packet >&, std::vector < Packet >&, void** );

extern const char * TFILTER_AVG_FORMATSTR;
void tfilter_Avg( const std::vector < Packet >&, std::vector < Packet >&, void** );

extern const char * TFILTER_MIN_FORMATSTR;
void tfilter_Min( const std::vector < Packet >&, std::vector < Packet >&, void** );

extern const char * TFILTER_MAX_FORMATSTR;
void tfilter_Max( const std::vector < Packet >&, std::vector < Packet >&, void** );

extern const char * TFILTER_ARRAY_CONCAT_FORMATSTR;
void tfilter_ArrayConcat(const std::vector < Packet >&,
                         std::vector < Packet >&,
                         void**);

extern const char * TFILTER_INT_EQ_CLASS_FORMATSTR;
void tfilter_IntEqClass( const std::vector < Packet >&,
                         std::vector < Packet >&,
                         void** );


// synchronization filters
void sfilter_WaitForAll( std::vector < Packet >&, std::vector < Packet >&,
                         const std::list < RemoteNode * >&, void ** );

void sfilter_TimeOut( std::vector < Packet >&, std::vector < Packet >&,
                      const std::list < RemoteNode * >&, void ** );

} // namespace MRN

#endif  /* filterdefinitions_h */
