#include <math.h>

#include "ThroughputExperiment.h"

using namespace std;

ThroughputExperiment::ThroughputExperiment( )
    : _start_timestamp( 0 ), _stop_timestamp( 0 )
{
}

void ThroughputExperiment::start()
{
    struct timeval tv;
    while(gettimeofday(&tv, NULL) == -1);
    _start_timestamp = (double(tv.tv_sec)) + ((double)tv.tv_usec) / 1000000.0;
}

void ThroughputExperiment::stop()
{
    struct timeval tv;
    while(gettimeofday(&tv, NULL) == -1);
    _stop_timestamp = (double(tv.tv_sec)) + ((double)tv.tv_usec) / 1000000.0;
}

void ThroughputExperiment::insert_DataPoint( uint32_t insamples )
{
    struct timeval tv;
    while(gettimeofday(&tv, NULL) == -1);

    DataPoint * dp = new DataPoint;
    dp->_timestamp = (double(tv.tv_sec)) + (((double)tv.tv_usec) / 1000000.0);
    dp->_nsamples = insamples;

    //initialize start time if start() not called explicitly
    if( fabs(_start_timestamp) < .000001 ) {
        _start_timestamp = dp->_timestamp;
    }

    _data_points.insert( dp );
}

//thruput for last iperiod msecs. 0 => thruput across all samples
double ThroughputExperiment::get_ThroughPut( uint32_t iperiod )
{
    uint32_t nsamples=0;
    double start_time, stop_time;

    double thruput;
    DataPoint * newest_dp;

    if( _data_points.empty() ) {
        return 0;
    }

    struct timeval tv;
    while(gettimeofday(&tv, NULL) == -1);
    double cur_time = (double(tv.tv_sec)) + ((double)tv.tv_usec) / 1000000.0;

    if( fabs(_stop_timestamp) < .0000001 ) {
        stop_time = cur_time;
    }
    else{
        stop_time = _stop_timestamp;
    }

    newest_dp = *(_data_points.begin());

    if( iperiod == 0 ) {
        //thruput across all samples
        start_time = _start_timestamp;
    }
    else {
        //datapoints should be sorted from greatest timestamp to least
        //iterate until we find a timestamp collected more than "iperiod msecs" ago
        set< DataPoint *, lt_datapoint >::const_iterator iter;

        DataPoint * oldest_valid_dp=NULL;
        for( iter=_data_points.begin(); iter!=_data_points.end(); iter++ ) {
            if( cur_time - (*iter)->_timestamp > iperiod/1000 ) {
                break;
            }
            oldest_valid_dp=*iter;
        }

        if( oldest_valid_dp == NULL ) {
            //no datapoints collected over time period
            start_time = stop_time;
        }
        else {
            start_time = oldest_valid_dp->_timestamp;
            nsamples = newest_dp->_nsamples - oldest_valid_dp->_nsamples;
        }
    }

    thruput = (stop_time - start_time) / nsamples;

    return thruput;
}

bool ThroughputExperiment::dump_Statistics( const char * ifilename )
{
    FILE * f = fopen( ifilename, "a" );
    if( f == NULL ) {
        fprintf( stderr, "fopen(\"%s\") failed.\n", ifilename );
        perror( "fopen()" );
        return false;
    }

    set< DataPoint *, lt_datapoint >::const_iterator iter;

    fprintf( f, "%7.7s %8.8s %7.7s\n", "TIME(S)", "ACCUM.", "THRUPUT" );
    for( iter=_data_points.begin(); iter!=_data_points.end(); iter++ ) {
        DataPoint * cur_data_point = *iter;

        double duration = cur_data_point->_timestamp - _start_timestamp;
        double thruput = cur_data_point->_nsamples/duration;

        fprintf( f, "%7.3lf %8u %7.3lf\n", duration, cur_data_point->_nsamples,
                 thruput );
    } 

    fclose(f);
    return true;
}
