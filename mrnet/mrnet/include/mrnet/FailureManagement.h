/****************************************************************************
 * Copyright © 2003-2008 Dorian C. Arnold, Philip C. Roth, Barton P. Miller *
 *                  Detailed MRNet usage rights in "LICENSE" file.          *
 ****************************************************************************/

#if !defined (__failure_management_h)
#define __failure_management_h 1

#define FAILURE_REPORTING_PORT 45678

#include "mrnet/MRNet.h"
#include "utils.h"

#include <map>

namespace MRN {

int start_FailureManager( Network * inetwork );
int stop_FailureManager( void );
void set_NumFailures( unsigned int infailures );
void set_FailureFrequency( unsigned int ifailure_frequency );
int waitFor_FailureManager( void );
int inject_Failure( NetworkTopology::Node * );

class FailureEvent{
 public:
    class RecoveryReport{
    public:
        unsigned int _orphan_rank;
        unsigned int _failed_parent_rank;
        double _lat_new_parent;
        double _lat_connection;
        double _lat_filter_state;
        double _lat_cleanup;
        double _lat_overall;
        RecoveryReport( unsigned int iorphan_rank,
                        unsigned int ifailed_parent_rank,
                        double ilat_new_parent,
                        double ilat_connection,
                        double ilat_filter_state,
                        double ilat_cleanup,
                        double ilat_overall )
            : _orphan_rank( iorphan_rank ),
            _failed_parent_rank( ifailed_parent_rank ),
            _lat_new_parent( ilat_new_parent ),
            _lat_connection( ilat_connection ),
            _lat_filter_state( ilat_filter_state ),
            _lat_cleanup( ilat_cleanup ),
            _lat_overall( ilat_overall ) {}
    };
    
    static std::map< Rank, FailureEvent * > FailureEventMap;
    Rank _failed_rank;
    Timer _timer;
    std::vector< RecoveryReport * > _recovery_reports;

    FailureEvent( Rank irank );
    void dump_FailureRecoveryStatistics( FILE * ifp );
    static int dump_FailureRecoveryStatistics( const char * ifilename );
};

} /* namespace MRN */

#endif /* __failure_management_h */

