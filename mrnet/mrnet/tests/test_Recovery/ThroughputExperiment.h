#if !defined( __ThroughputExperiment_h )
#define __ThroughputExperiment_h 1

#include <set>
#include <stdio.h>
#include <sys/time.h>

#include "mrnet/Types.h"

class ThroughputExperiment
{
 public:

    ThroughputExperiment( );
    void start();
    void stop();
    void insert_DataPoint( uint32_t insamples );

    //thruput for last iperiod msecs. 0 => thruput across all samples
    double get_ThroughPut( uint32_t iperiod=0 );
    bool dump_Statistics( const char * ifilename );

 private:
    class DataPoint{
    public:
        double _timestamp;
        uint32_t _nsamples;
    };

    struct lt_datapoint {
        bool operator()(const DataPoint* d1, const DataPoint* d2) const
        {
            return d1->_timestamp < d2->_timestamp;
        }
    };

    std::set< DataPoint *, lt_datapoint > _data_points;
    double _start_timestamp;
    double _stop_timestamp;

};

#endif /* __ThroughputExperiment_h */
