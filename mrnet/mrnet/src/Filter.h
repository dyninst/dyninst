/***********************************************************************
 * Copyright © 2003 Dorian C. Arnold, Philip C. Roth, Barton P. Miller *
 *                  Detailed MRNet usage rights in "LICENSE" file.     *
 **********************************************************************/

#if !defined(__filter_h)
#define __filter_h 1

#include <string>
#include <list>
#include <map>
#include <vector>
#include "mrnet/src/Message.h"
#include "mrnet/src/FilterDefinitions.h"
#include "xplat/Mutex.h"

namespace MRN
{

class Filter: public Error {

 protected:
    unsigned short filter_id;
    void * local_storage; //TODO: deallocate when filter is destructed

 public:
    static void initialize_static_stuff( );
    static void free_static_stuff( );
    static std::map < unsigned short, void *>FilterFuncById;
    static std::map < unsigned short, std::string > FilterFmtById;
    static int load_FilterFunc( const char *so_file, const char *func,
                                bool transformation_filter = true,
                                unsigned short in_fid = 0 );
    static unsigned short register_Filter( void *, const char * );

    Filter( unsigned short _filter_id );
    virtual ~ Filter(  );
    virtual int push_packets( std::vector < Packet >&packets_in,
                              std::vector < Packet >&packets_out ) = 0;
};

class TransFilter:public Filter {
 private:
    std::string fmt_str;
    void ( *trans_filter ) ( const std::vector < Packet >&packets_in,
                             std::vector < Packet >&packets_out,
                             void ** );
 public:
    TransFilter( unsigned short _fid );
    virtual ~ TransFilter(  );
    virtual int push_packets( std::vector < Packet >&packets_in,
                              std::vector < Packet >&packets_out );
};

class RemoteNode;
class SyncFilter:public Filter {
 private:
    std::map < RemoteNode *, std::vector < Packet >*>PacketsByNode;
    void ( *sync_filter ) ( std::vector < Packet >&,
                            std::vector < Packet >&,
                            const std::list < RemoteNode * >&, void ** );
    const std::list < RemoteNode * >downstream_nodes;
    XPlat::Mutex fsync;
    
 public:
    SyncFilter( unsigned short _filter_id,
                const std::list < RemoteNode * >& );
    virtual ~ SyncFilter(  );
    virtual int push_packets( std::vector < Packet >&packets_in,
                              std::vector < Packet >&packets_out );
};

inline unsigned short Filter::register_Filter( void *func, const char * fmt )
{
    static unsigned short next_filter_id=0; 

    next_filter_id++;
    FilterFuncById[next_filter_id] = func;
    FilterFmtById[next_filter_id] = fmt;

    return next_filter_id;
}

inline void Filter::initialize_static_stuff( )
{
    TFILTER_NULL = register_Filter( (void*)NULL, TFILTER_NULL_FORMATSTR);

    TFILTER_SUM = register_Filter( (void*)tfilter_Sum, TFILTER_SUM_FORMATSTR);

    TFILTER_AVG = register_Filter( (void*)tfilter_Avg, TFILTER_AVG_FORMATSTR);

    TFILTER_MAX = register_Filter( (void*)tfilter_Max, TFILTER_MAX_FORMATSTR);

    TFILTER_MIN = register_Filter( (void*)tfilter_Min, TFILTER_MIN_FORMATSTR);

    TFILTER_ARRAY_CONCAT = register_Filter( (void*)tfilter_ArrayConcat,
                                            TFILTER_ARRAY_CONCAT_FORMATSTR);

    TFILTER_INT_EQ_CLASS = register_Filter( (void*)tfilter_IntEqClass,
                                            TFILTER_INT_EQ_CLASS_FORMATSTR);

    SFILTER_DONTWAIT = register_Filter( (void*)NULL, "");

    SFILTER_WAITFORALL = register_Filter( (void*)sfilter_WaitForAll, "");

    SFILTER_TIMEOUT = register_Filter( (void*)sfilter_TimeOut, "");
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
