/****************************************************************************
 * Copyright © 2003-2007 Dorian C. Arnold, Philip C. Roth, Barton P. Miller *
 *                  Detailed MRNet usage rights in "LICENSE" file.          *
 ****************************************************************************/

#if !defined( paradynfilterdefinitions_h )
#define paradynfilterdefinitions_h 1

#include <vector>
#include "mrnet/ParadynFilterIds.h"
#include "mrnet/Packet.h"

namespace MRN
{

extern const char * TFILTER_SAVE_LOCAL_CLOCK_SKEW_UPSTREAM_FORMATSTR;
void save_LocalClockSkewUpstream( const std::vector < PacketPtr >&,
                                  std::vector < PacketPtr >&,
                                  void**, PacketPtr&, bool& );

extern const char * TFILTER_SAVE_LOCAL_CLOCK_SKEW_DOWNSTREAM_FORMATSTR;
void save_LocalClockSkewDownstream( const std::vector < PacketPtr >&,
                                    std::vector < PacketPtr >&,
                                    void**, PacketPtr&, bool& );

extern const char * TFILTER_GET_CLOCK_SKEW_FORMATSTR;
void get_ClockSkew( const std::vector < PacketPtr >&,
                    std::vector < PacketPtr >&,
                    void**, PacketPtr&, bool& );

extern const char * TFILTER_PD_UINT_EQ_CLASS_FORMATSTR;
void tfilter_PDUIntEqClass( const std::vector < PacketPtr >&,
                           std::vector < PacketPtr >&,
                           void**, PacketPtr&, bool& );
}

#endif /* paradynfilterdefinitions_h */
