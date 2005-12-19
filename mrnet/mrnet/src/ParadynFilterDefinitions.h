#if !defined( paradynfilterdefinitions_h )
#define paradynfilterdefinitions_h 1

#include <vector>
#include "mrnet/ParadynFilterIds.h"

namespace MRN
{
class Packet; 

extern const char * TFILTER_SAVE_LOCAL_CLOCK_SKEW_UPSTREAM_FORMATSTR;
void save_LocalClockSkewUpstream( const std::vector < Packet >&,
                                  std::vector < Packet >&,
                                  void** );

extern const char * TFILTER_SAVE_LOCAL_CLOCK_SKEW_DOWNSTREAM_FORMATSTR;
void save_LocalClockSkewDownstream( const std::vector < Packet >&,
                                    std::vector < Packet >&,
                                    void** );

extern const char * TFILTER_GET_CLOCK_SKEW_FORMATSTR;
void get_ClockSkew( const std::vector < Packet >&,
                    std::vector < Packet >&,
                    void** );

extern const char * TFILTER_PD_UINT_EQ_CLASS_FORMATSTR;
void tfilter_PDUIntEqClass( const std::vector < Packet >&,
                           std::vector < Packet >&,
                           void** );
}

#endif /* paradynfilterdefinitions_h */
