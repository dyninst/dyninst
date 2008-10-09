/****************************************************************************
 * Copyright © 2003-2008 Dorian C. Arnold, Philip C. Roth, Barton P. Miller *
 *                  Detailed MRNet usage rights in "LICENSE" file.          *
 ****************************************************************************/

#if !defined(__filter_h)
#define __filter_h 1

#include <string>
#include <vector>
#include <map>
#include <vector>
#include <set>
#include "Error.h"
#include "Message.h"
#include "FilterDefinitions.h"
#include "xplat/Mutex.h"

#include "ParadynFilterDefinitions.h"
#include "PeerNode.h"

namespace MRN
{

class Filter: public Error {
    friend class FilterCounter;

 public:
    Filter( unsigned short iid );
    ~Filter( void );
    int push_Packets( std::vector < PacketPtr > &ipackets,
                      std::vector < PacketPtr > &opackets, 
                      std::vector < PacketPtr > &opackets_reverse
                    );
    PacketPtr get_FilterState( int istream_id );
    void set_FilterParams( PacketPtr iparams );

    static int load_FilterFunc( const char *so_file, const char *func );
    static unsigned short register_Filter( void (*ifilter_func)( ),
                                           void (*istate_func)( ),
                                           const char * ifmt );

 private:
    static void initialize_static_stuff( );
    static void free_static_stuff( );

    static std::map < unsigned short, void(*)() > FilterFuncs;
    static std::map < unsigned short, void(*)() > GetStateFuncs;
    static std::map < unsigned short, std::string > FilterFmts;

    unsigned short _id;
    void * _filter_state;  //TODO: deallocate when filter is destructed
    XPlat::Mutex _mutex;
    PacketPtr _params;
    std::string _fmt_str;
    void ( *_filter_func )( const std::vector < PacketPtr >&,
                            std::vector < PacketPtr >&, 
                            std::vector < PacketPtr >&, 
                            void **, PacketPtr& );
    PacketPtr ( *_get_state_func )( void ** ifilter_state, int istream_id );
};

inline void Filter::initialize_static_stuff( )
{
    TFILTER_NULL = register_Filter( NULL, NULL, TFILTER_NULL_FORMATSTR);

    TFILTER_SUM = register_Filter( (void(*)())tfilter_Sum, NULL, TFILTER_SUM_FORMATSTR);

    TFILTER_AVG = register_Filter( (void(*)())tfilter_Avg, NULL, TFILTER_AVG_FORMATSTR);

    TFILTER_MAX = register_Filter( (void(*)())tfilter_Max, NULL, TFILTER_MAX_FORMATSTR);

    TFILTER_MIN = register_Filter( (void(*)())tfilter_Min, NULL, TFILTER_MIN_FORMATSTR);

    TFILTER_ARRAY_CONCAT = register_Filter( (void(*)())tfilter_ArrayConcat, NULL,
                                            TFILTER_ARRAY_CONCAT_FORMATSTR);

    TFILTER_INT_EQ_CLASS = register_Filter( (void(*)())tfilter_IntEqClass, NULL,
                                            TFILTER_INT_EQ_CLASS_FORMATSTR);

    TFILTER_SAVE_LOCAL_CLOCK_SKEW_UPSTREAM =
        register_Filter( (void(*)())save_LocalClockSkewUpstream, NULL,
                         TFILTER_SAVE_LOCAL_CLOCK_SKEW_UPSTREAM_FORMATSTR );

    TFILTER_SAVE_LOCAL_CLOCK_SKEW_DOWNSTREAM =
        register_Filter( (void(*)())save_LocalClockSkewDownstream, NULL,
                         TFILTER_SAVE_LOCAL_CLOCK_SKEW_DOWNSTREAM_FORMATSTR );

    TFILTER_GET_CLOCK_SKEW =
        register_Filter( (void(*)())get_ClockSkew, NULL,
                         TFILTER_GET_CLOCK_SKEW_FORMATSTR );

    TFILTER_PD_UINT_EQ_CLASS = register_Filter( (void(*)())tfilter_PDUIntEqClass,
                                                NULL,
                                                TFILTER_PD_UINT_EQ_CLASS_FORMATSTR);

    SFILTER_DONTWAIT = register_Filter( (void(*)())NULL, NULL, "");

    SFILTER_WAITFORALL = register_Filter( (void(*)())sfilter_WaitForAll, NULL, "");

    SFILTER_TIMEOUT = register_Filter( (void(*)())sfilter_TimeOut, NULL, "");
}

inline void Filter::free_static_stuff( )
{
}

class FilterCounter {
 private:
   static int count;
  
 public:
   FilterCounter() {
      if (count++ == 0)
         Filter::initialize_static_stuff();
   }
  ~FilterCounter() {
      if (--count == 0)
         Filter::free_static_stuff();
   }
};

} // namespace MRN
#endif  /* __filter_h */
