#if !defined( filterdefinitions_h )
#define filterdefinitions_h 1

#include "mrnet/src/Packet.h"
#include <list>

namespace MRN
{

#define AGGR_NULL 0
#define AGGR_NULL_FORMATSTR ""

#define AGGR_INT_SUM_ID 2000
#define AGGR_INT_SUM_FORMATSTR "%d"
void aggr_Int_Sum( std::vector < Packet >&,
                   std::vector < Packet >&,
                   void** );

#define AGGR_FLOAT_AVG_ID 2001
#define AGGR_FLOAT_AVG_FORMATSTR "%f"
void aggr_Float_Avg( std::vector < Packet >&,
                     std::vector < Packet >&,
                     void** );

#define AGGR_FLOAT_MAX_ID 2006
#define AGGR_FLOAT_MAX_FORMATSTR "%lf"
void aggr_Float_Max( std::vector < Packet >&,
                     std::vector < Packet >&,
                     void** );

#define AGGR_CHARARRAY_CONCAT_ID 2007
#define AGGR_CHARARRAY_CONCAT_FORMATSTR "%ac"
void aggr_CharArray_Concat( std::vector < Packet >&,
                            std::vector < Packet >&,
                            void** );

#define AGGR_INT_EQ_CLASS_ID    2008
#define AGGR_INT_EQ_CLASS_FORMATSTR "%aud %aud %aud"
void aggr_IntEqClass( std::vector < Packet >&,
                      std::vector < Packet >&,
                      void** );

#define SYNC_WAITFORALL 2003
void sync_WaitForAll( std::vector < Packet >&, std::vector < Packet >&,
                      std::list < RemoteNode * >&, void ** );

#define SYNC_DONTWAIT 2004

#define SYNC_TIMEOUT 2005
void sync_TimeOut( std::vector < Packet >&, std::vector < Packet >&,
                   std::list < RemoteNode * >&, void ** );

} // namespace MRN

#endif  /* filterdefinitions_h */
