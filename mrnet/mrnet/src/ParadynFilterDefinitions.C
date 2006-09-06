/***********************************************************************
 * Copyright © 2003-2004 Dorian C. Arnold, Philip C. Roth, Barton P. Miller *
 *                  Detailed MRNet usage rights in "LICENSE" file.     *
 **********************************************************************/

#include <map>
#include <string>

#if !defined(os_windows)
#include <sys/time.h>
#endif

#include <math.h>


#include "FilterDefinitions.h"
#include "utils.h"
#include "DataElement.h"
#include "Packet.h"

#include "ParadynFilterDefinitions.h"

#define MAXDOUBLE   1.701411834604692293e+308


double
TimeVal2Double( const timeval& tv )
{
    return ((double)tv.tv_sec) + (((double)tv.tv_usec)/1000000U);
}

double savedLocalParSendTimestamp = 0.0;  // time parent sent message
double savedLocalRecvTimestamp = 0.0;     // time we received from parent
double * localSkews = NULL;

namespace MRN
{

// Downstream filter used to save time skew data on downward flow
// "%lf" => parents_send_time
const char * TFILTER_SAVE_LOCAL_CLOCK_SKEW_DOWNSTREAM_FORMATSTR="%d %lf";
FilterId TFILTER_SAVE_LOCAL_CLOCK_SKEW_DOWNSTREAM=0;
	void getTime(struct timeval *tmval)
	{
#if defined(os_windows)
		SYSTEMTIME systime;
		FILETIME ftime;
		GetSystemTime(&systime);
		SystemTimeToFileTime(&systime, &ftime);
		unsigned val = ftime.dwLowDateTime;
#else
    gettimeofday(tmval, NULL);
#endif
	}
void save_LocalClockSkewDownstream( const std::vector<Packet>& in_pkts,
                                    std::vector<Packet>& out_pkts,
                                    void** /* cd */ )
{
    //DataType atype;
    //unsigned int alen;
    struct timeval recvTimeVal, sendTimeVal;
    double sendTime;

    getTime(&recvTimeVal);

    // sample "receive" time as early as possible for incoming message
		savedLocalRecvTimestamp = TimeVal2Double( recvTimeVal );

    // save the time our parent sent the downstream message,
    unsigned int id = in_pkts[0][0]->get_uint32_t();

    savedLocalParSendTimestamp = in_pkts[0][1]->get_double( );

    // sample "send" time as late as possible for outgoing message(s)
    getTime(&sendTimeVal);

    sendTime = TimeVal2Double( sendTimeVal );

    out_pkts.push_back( Packet( in_pkts[0].get_StreamId(),
                                     in_pkts[0].get_Tag(),
                                     "%ud %lf",  id, sendTime ) );
}

// Upstream filter used to save time skew data on upward flow and propagate
// upwards.
// "%ald" => parents_send_time, local_recv_time, local_send_time
const char * TFILTER_SAVE_LOCAL_CLOCK_SKEW_UPSTREAM_FORMATSTR="%ud %ud %alf";
FilterId TFILTER_SAVE_LOCAL_CLOCK_SKEW_UPSTREAM=0;

void save_LocalClockSkewUpstream( const std::vector<Packet>& in_pkts,
                                  std::vector<Packet>& out_pkts,
                                  void** /* cd */ )
{
    struct timeval recvTimeVal, sendTimeVal;
    double recvTime, sendTime;

    // sample "receive" time
    getTime(&recvTimeVal);

    recvTime = TimeVal2Double( recvTimeVal );

    unsigned int id = (in_pkts[0])[0]->get_uint32_t();

    // ensure we have a place to save our skews
    if( localSkews == NULL ) {
        localSkews = new double[ in_pkts.size() ];
        for( unsigned int i = 0; i < in_pkts.size(); i++ ) {
            localSkews[i] = MAXDOUBLE;
        }
    }

    for( unsigned int i = 0; i < in_pkts.size(); i++ ) {
        // compute the skew for the current child
        // Note: assumes that the order in which we are
        // presented the inputs does not change from call to call
        // TODO remove this assumption - map them by their input node.
        //
        // skew = our_recv_time - (child_send_time + msg_latency),
        // 
        //   where the actual message latency is approximated by 
        //
        // msg_latency = (our_recv_time - our_send_time - 
        //                  (child_send_time - child_recv_time)) / 2.
        //
        DataType atype;
        unsigned int alen;
		
        const double * inTimestamps = (const double *)(in_pkts[i][2]->get_array( &atype, (uint32_t *) &alen ));
        assert( inTimestamps != NULL );
        assert( atype == DOUBLE_ARRAY_T );
        assert( alen == 3 );

        const double & ourSendTime = inTimestamps[0];
        const double & curChildRecvTime = inTimestamps[1];
        const double & curChildSendTime = inTimestamps[2];
        double curMsgLat = 
            (recvTime - ourSendTime - (curChildSendTime - curChildRecvTime)) / 2.0;
				double temp = recvTime - (curChildSendTime + curMsgLat);
				if(fabs(localSkews[i])> fabs(temp))
					localSkews[i] = temp;
  
		}

    // we send to our parent the time we received its message and the
    // time we sent this message.

    // indicate where our send timestamp should go
    double timestamps[3];
    timestamps[0] = savedLocalParSendTimestamp;
    timestamps[1] = savedLocalRecvTimestamp;
     
    // sample "send" time as late as possible for outgoing message(s)
    getTime(&sendTimeVal);
    sendTime = TimeVal2Double( sendTimeVal );
    timestamps[2] = sendTime;

    out_pkts.push_back( Packet( in_pkts[0].get_StreamId(),
                                     in_pkts[0].get_Tag(),
                                     "%ud %ud %alf", id, 3, timestamps, 3 ) );
}

const char * TFILTER_GET_CLOCK_SKEW_FORMATSTR="%ud %ud %alf";
FilterId TFILTER_GET_CLOCK_SKEW=0;

void
get_ClockSkew( const std::vector<Packet>& in_pkts,
                std::vector<Packet>& out_pkts,
                void** /* cd */ )
{
    assert( localSkews != NULL );
    unsigned int id = in_pkts[0][0]->get_uint32_t();

    // our input is the cumulative skew to each backend
    // reachable from our child processes
    std::vector<unsigned int> itemCounts;
    std::vector<const unsigned int*> daemonIdArrays;
    std::vector<const double*> skewArrays;
    unsigned int nReachableBackends = 0;
    for( std::vector<Packet>::const_iterator iter = in_pkts.begin();
            iter != in_pkts.end();
            iter++ )
    {
        DataType atype;
        unsigned int alen;

        // extract skew array from current packet
        const double* curSkews = (const double*)((*iter)[2]->get_array( &atype, (uint32_t *) &alen ));
        assert( curSkews != NULL );
        assert( atype == DOUBLE_ARRAY_T );

        skewArrays.push_back( curSkews );

        itemCounts.push_back( alen );
        nReachableBackends += alen;
    }


    // compute the cumulative skew to each daemon reachable by our process
    //unsigned int* backendIds = new unsigned int[nReachableBackends];
    double* cumulativeSkews = new double[nReachableBackends];
    unsigned int curIdx = 0;
    for( unsigned int i = 0; i < in_pkts.size(); i++ )
    {
        unsigned int curLen = itemCounts[i];

        // copy cumulative skews from current input
        memcpy( &(cumulativeSkews[curIdx]),
                    skewArrays[i],
                    curLen * sizeof(double) );

        // adjust cumulative skews based on the observed local skew

        for( unsigned int j = 0; j < curLen; j++ )
        {
            cumulativeSkews[curIdx+j] += localSkews[i];
        }

        curIdx += curLen;
    }


    // deliver the cumulative skews upstream
    out_pkts.push_back( Packet( in_pkts[0].get_StreamId(),
                                     in_pkts[0].get_Tag(),
                                     "%ud %ud %alf", id, nReachableBackends,
                                     cumulativeSkews, nReachableBackends ));
}

FilterId TFILTER_PD_UINT_EQ_CLASS=0;
const char * TFILTER_PD_UINT_EQ_CLASS_FORMATSTR="%ud %ud %ad %ad";

//format str: "%ud %ud %ad %ad "
//  %ud => unsigned int daemon_id (igen_spec)
//  %ud => size of parallel array (pdvector of objects in igen)
//  %ad => array of classes (1 for each object in parallel array)
//  %ad => array of class reps (1 for each object in parallel array)

void tfilter_PDUIntEqClass( const std::vector < Packet >&packets_in,
                           std::vector < Packet >&packets_out,
                           void ** /* client data */ )
{
    DataType type=UNKNOWN_T;
    const uint32_t *class_array, *class_rep_array;
    uint32_t class_array_len, class_rep_array_len;
    uint32_t cur_class, cur_class_rep;

    uint32_t sdm_id=packets_in[0][0]->get_uint32_t(); //daemon id (igen wants)

    //mrn_printf( 1, MCFL, stderr, "In EqClassFilter(%d packets in) constructor\n", packets_in.size() );

    std::map < uint32_t, uint32_t >classes;

    // find equivalence classes across our input 

    //for each packet
    for( unsigned int i = 0; i < packets_in.size(); i++ ) {
        Packet cur_packet = packets_in[i];
        /*mrn_printf( 1, MCFL, stderr,
                    "Packet[%d]: tag=%d,stream=%d,fmt=%s,num_elems=%d\n",
                    i,
                    cur_packet.get_Tag(),
                    cur_packet.get_StreamId(),
                    cur_packet.get_FormatString(),
                    cur_packet.get_NumDataElements() );
	*/
        class_array = (uint32_t *)cur_packet[2]->get_array(&type, &class_array_len);
        //mrn_printf( 1, MCFL, stderr, "class_array(%p): len: %d\n",
        //            class_array, class_array_len );
        class_rep_array = (uint32_t *)cur_packet[3]->get_array(&type, &class_rep_array_len);
	// mrn_printf( 1, MCFL, stderr, "class_rep_array(%p): len: %d\n",
        //            class_rep_array, class_rep_array_len );

        assert( class_array_len == class_rep_array_len );

        //for each EqClass in the packet
        for( unsigned int j = 0; j < class_array_len; j++ ){
            cur_class = class_array[j];
            cur_class_rep = class_rep_array[j];
	    //   mrn_printf( 1, MCFL, stderr, "\teqclass: %d, class_rep: %d\n",
            //            cur_class, cur_class_rep );

            if( classes.find( cur_class ) == classes.end() ){
                //class is not yet in map, add!
                classes[ cur_class ] = cur_class_rep;
            }
        }
    }
    //mrn_printf( 1, MCFL, stderr, "Done processing inputs\n");

    // build data structures for the output 
    uint32_t *new_class_array = new uint32_t[ classes.size( ) ];
    uint32_t new_class_array_len = classes.size( );
    uint32_t *new_class_rep_array = new uint32_t[ classes.size( ) ];
    uint32_t new_class_rep_array_len = classes.size( );

    std::map < uint32_t, uint32_t >::iterator iter;
    uint32_t j=0;
    for( iter = classes.begin();
         iter != classes.end( );
         iter++, j++ ) {
      //mrn_printf( 1, MCFL, stderr, "Adding class:%d, rep: %d to output array\n", iter->first, iter->second );
        new_class_array[j] = iter->first;
        new_class_rep_array[j] = iter->second;
    }

    Packet new_packet( packets_in[0].get_StreamId( ),
                       packets_in[0].get_Tag( ),
                       "%ud %ud %ad %ad", sdm_id, new_class_array_len, 
                       new_class_array, new_class_array_len,
                       new_class_rep_array, new_class_rep_array_len );

    packets_out.push_back( new_packet );
    //mrn_printf( 1, MCFL, stderr, "Done processing output\n");
}

} /* namespace MRN */
