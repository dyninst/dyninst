#if !defined(__filter_h)
#define __filter_h 1

#include <string>
#include <list>
#include <map>
#include <vector>
#include "mrnet/src/Message.h"
#include "mrnet/src/pthread_sync.h"
#include "mrnet/src/FilterDefinitions.h"

namespace MRN
{

class Filter {

 protected:
    unsigned short filter_id;
    void * local_storage; //TODO: deallocate when filter is destructed

 public:
    static std::map < unsigned int, void *>FilterFuncById;
    static std::map < unsigned int, std::string > FilterFmtById;
    static int load_FilterFunc( const char *so_file, const char *func,
                                bool transformation_filter = true,
                                unsigned short in_fid = 0 );
    static unsigned short get_NextFilterFuncId( void );

    Filter( unsigned short _filter_id );
    virtual ~ Filter(  );
    virtual int push_packets( std::vector < Packet >&packets_in,
                              std::vector < Packet >&packets_out ) = 0;
};

class TransFilter:public Filter {
 private:
    std::string fmt_str;
    void ( *trans_filter ) ( std::vector < Packet >&packets_in,
                             std::vector < Packet >&packets_in,
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
                            std::list < RemoteNode * >&, void ** );
    std::list < RemoteNode * >downstream_nodes;
    pthread_sync fsync;
    
 public:
    SyncFilter( unsigned short _filter_id, std::list < RemoteNode * >& );
    virtual ~ SyncFilter(  );
    virtual int push_packets( std::vector < Packet >&packets_in,
                              std::vector < Packet >&packets_out );
};

} // namespace MRN

#endif  /* __filter_h */
