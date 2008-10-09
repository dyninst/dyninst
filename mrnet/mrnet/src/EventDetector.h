#if !defined( __event_detector_h )
#define __event_detector_h 1

#include "PeerNode.h"

namespace MRN {

class EventDetector{
 public:
    static bool start( Network *inetwork );
    static bool stop( void );

 private:
    static long _thread_id;

    static void * main( void * iarg );
    static int init_NewChildFDConnection( Network * inetwork,
                                          PeerNodePtr iparent_node );
    static int recover_FromChildFailure( Network *inetwork, Rank ifailed_rank );
    static int recover_FromParentFailure( Network *inetwork );
};

};

#endif /* __event_detector_h */
