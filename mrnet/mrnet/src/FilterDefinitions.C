/****************************************************************************
 * Copyright © 2003-2007 Dorian C. Arnold, Philip C. Roth, Barton P. Miller *
 *                  Detailed MRNet usage rights in "LICENSE" file.          *
 ****************************************************************************/

#include <map>
#include <set>
#include <vector>
#include <string>

#include "mrnet/MRNet.h"

#include "FilterDefinitions.h"
#include "utils.h"
#include "DataElement.h"
#include "PeerNode.h"

using namespace std;

namespace MRN
{

FilterId TFILTER_NULL=0;
const char * TFILTER_NULL_FORMATSTR="";  // "" => "Don't check fmt string"

FilterId TFILTER_SUM=0;
const char * TFILTER_SUM_FORMATSTR="";  // "" => "Don't check fmt string"

FilterId TFILTER_AVG=0;
const char * TFILTER_AVG_FORMATSTR="";  // "" => "Don't check fmt string"

FilterId TFILTER_MIN=0;
const char * TFILTER_MIN_FORMATSTR="";  // "" => "Don't check fmt string"

FilterId TFILTER_MAX=0;
const char * TFILTER_MAX_FORMATSTR="";  // "" => "Don't check fmt string"

FilterId TFILTER_ARRAY_CONCAT=0;
const char * TFILTER_ARRAY_CONCAT_FORMATSTR=""; // "" => "Don't check fmt string"

FilterId TFILTER_INT_EQ_CLASS=0;
const char * TFILTER_INT_EQ_CLASS_FORMATSTR="%aud %aud %aud";

FilterId SFILTER_WAITFORALL=0;
FilterId SFILTER_DONTWAIT=0;
FilterId SFILTER_TIMEOUT=0;

static inline void mrn_max(const void *in1, const void *in2, void* out, DataType type);
static inline void mrn_min(const void *in1, const void *in2, void* out, DataType type);
static inline void div(const void *in1, int in2, void* out, DataType type);
static inline void mult(const void *in1, int in2, void* out, DataType type);
static inline void sum(const void *in1, const void *in2, void* out, DataType type);

/*=====================================================*
 *    Default Transformation Filter Definitions        *
 *=====================================================*/
void tfilter_IntSum( const vector< PacketPtr >& ipackets,
                     vector< PacketPtr >& opackets,
                     vector< PacketPtr >& /* opackets_reverse */,
                     void ** /* client data */, PacketPtr& )
{
    int sum = 0;
    
    for( unsigned int i = 0; i < ipackets.size( ); i++ ) {
        PacketPtr cur_packet( ipackets[i] );
        sum += (*cur_packet)[0]->get_int32_t( );
    }
    
    PacketPtr new_packet( new Packet( ipackets[0]->get_StreamId( ),
                                      ipackets[0]->get_Tag( ),
                                      "%d", sum ) );
    opackets.push_back( new_packet );
}

void tfilter_Sum( const vector< PacketPtr >& ipackets,
                  vector< PacketPtr >& opackets,
                  vector< PacketPtr >& /* opackets_reverse */,
                  void ** /* client data */, PacketPtr& )
{
    char result[8]; //ptr to 8 bytes
    string format_string;
    DataType type=UNKNOWN_T;

    memset(result, 0, 8); //zeroing the result buf
    for( unsigned int i = 0; i < ipackets.size( ); i++ ) {
        PacketPtr cur_packet( ipackets[i] );

        assert( strcmp(cur_packet->get_FormatString(),
                       ipackets[0]->get_FormatString()) == 0 );

        if( i == 0 ){ //1st time thru
            //hack to get pass "%" in arg to fmt2type()
            type = Fmt2Type( (cur_packet->get_FormatString()+1) );
            switch(type){
            case CHAR_T:
                format_string="%c";
                break;
            case UCHAR_T:
                format_string="%uc";
                break;
            case INT16_T:
                format_string="%hd";
                break;
            case UINT16_T:
                format_string="%uhd";
                break;
            case INT32_T:
                format_string="%d";
                break;
            case UINT32_T:
                format_string="%ud";
                break;
            case INT64_T:
                format_string="%ld";
                break;
            case UINT64_T:
                format_string="%uld";
                break;
            case FLOAT_T:
                format_string="%f";
                break;
            case DOUBLE_T:
                format_string="%lf";
                break;
            case CHAR_ARRAY_T:
            case UCHAR_ARRAY_T:
            case INT16_ARRAY_T:
            case UINT16_ARRAY_T:
            case INT32_ARRAY_T:
            case UINT32_ARRAY_T:
            case INT64_ARRAY_T:
            case UINT64_ARRAY_T:
            case FLOAT_ARRAY_T:
            case DOUBLE_ARRAY_T:
            case STRING_T:
            case UNKNOWN_T:
            default:
                fprintf(stderr, "ERROR: tfilter_Sum() - invalid packet type: %d (%s)\n", type,
                        cur_packet->get_FormatString() );
                return;
            }
        }
        sum( result, &((*cur_packet)[0]->val.p), result, type );
    }

    if( type == CHAR_T ){
        char tmp = *((char *)result);
        PacketPtr new_packet( new Packet( ipackets[0]->get_StreamId( ),
                                          ipackets[0]->get_Tag( ),
                                          format_string.c_str(), tmp ) );
        opackets.push_back( new_packet );
    }
    else if( type == UCHAR_T ){
        uchar_t tmp = *((uchar_t *)result);
        PacketPtr new_packet( new Packet( ipackets[0]->get_StreamId( ),
                                          ipackets[0]->get_Tag( ),
                                          format_string.c_str(), tmp ) );
        opackets.push_back( new_packet );
    }
    else if( type == FLOAT_T ){
        float tmp = *((float*)result);
        PacketPtr new_packet( new Packet( ipackets[0]->get_StreamId( ),
                                          ipackets[0]->get_Tag( ),
                                          format_string.c_str(), tmp ) );
        opackets.push_back( new_packet );
    }
    else if( type == DOUBLE_T ){
        double tmp = *((double*)result);
        PacketPtr new_packet( new Packet( ipackets[0]->get_StreamId( ),
                                          ipackets[0]->get_Tag( ),
                                          format_string.c_str(), tmp ));
        opackets.push_back( new_packet );
    }
    else if( type == INT16_T ){
        int16_t tmp = *((int16_t*)result);
        PacketPtr new_packet( new Packet( ipackets[0]->get_StreamId( ),
                                          ipackets[0]->get_Tag( ),
                                          format_string.c_str(), tmp ) );
        opackets.push_back( new_packet );
    }
    else if( type == UINT16_T ){
        uint16_t tmp = *((uint16_t*)result);
        PacketPtr new_packet( new Packet( ipackets[0]->get_StreamId( ),
                                          ipackets[0]->get_Tag( ),
                                          format_string.c_str(), tmp ) );
        opackets.push_back( new_packet );
    }
    else if( type == INT32_T ){
        int32_t tmp = *((int32_t*)result);
        PacketPtr new_packet( new Packet( ipackets[0]->get_StreamId( ),
                                          ipackets[0]->get_Tag( ),
                                          format_string.c_str(), tmp ) );
        opackets.push_back( new_packet );
    }
    else if( type == UINT32_T ){
        uint32_t tmp = *((uint32_t*)result);
        PacketPtr new_packet( new Packet( ipackets[0]->get_StreamId( ),
                                          ipackets[0]->get_Tag( ),
                                          format_string.c_str(), tmp ) );
        opackets.push_back( new_packet );
    }
    else if( type == INT64_T ){
        int64_t tmp = *((int64_t*)result);
        PacketPtr new_packet( new Packet( ipackets[0]->get_StreamId( ),
                                          ipackets[0]->get_Tag( ),
                                          format_string.c_str(), tmp ) );
        opackets.push_back( new_packet );
    }
    else if( type == UINT64_T ){
        uint64_t tmp = *((uint64_t*)result);
        PacketPtr new_packet( new Packet( ipackets[0]->get_StreamId( ),
                                          ipackets[0]->get_Tag( ),
                                          format_string.c_str(), tmp ) );
        opackets.push_back( new_packet );
    }
    else{
        assert(0);
    }
}

void tfilter_Max( const vector< PacketPtr >& ipackets,
                  vector< PacketPtr >& opackets,
                  vector< PacketPtr >& /* opackets_reverse */,
                  void ** /* client data */, PacketPtr& )
{
    char result[8]; //ptr to 8 bytes
    string format_string;
    DataType type=UNKNOWN_T;
    
    for( unsigned int i = 0; i < ipackets.size( ); i++ ) {
        PacketPtr cur_packet( ipackets[i] );
        assert( strcmp(cur_packet->get_FormatString(),
                       ipackets[0]->get_FormatString()) == 0 );
        if( i == 0 ){ //1st time thru
            //+ 1 "hack" to get past "%" in arg to fmt2type()	
            type = Fmt2Type( cur_packet->get_FormatString( ) +1);
            switch(type){
            case CHAR_T:
                format_string="%c";
                break;
            case UCHAR_T:
                format_string="%uc";
                break;
            case INT16_T:
                format_string="%hd";
                break;
            case UINT16_T:
                format_string="%uhd";
                break;
            case INT32_T:
                format_string="%d";
                break;
            case UINT32_T:
                format_string="%ud";
                break;
            case INT64_T:
                format_string="%ld";
                break;
            case UINT64_T:
                format_string="%uld";
                break;
            case FLOAT_T:
                format_string="%f";
                break;
            case DOUBLE_T:
                format_string="%lf";
                break;
            case CHAR_ARRAY_T:
            case UCHAR_ARRAY_T:
            case INT16_ARRAY_T:
            case UINT16_ARRAY_T:
            case INT32_ARRAY_T:
            case UINT32_ARRAY_T:
            case INT64_ARRAY_T:
            case UINT64_ARRAY_T:
            case FLOAT_ARRAY_T:
            case DOUBLE_ARRAY_T:
            case STRING_T:
            case UNKNOWN_T:
            default:
                fprintf(stderr, "ERROR: tfilter_Max() - invalid packet type: %d (%s)\n", type,
                        cur_packet->get_FormatString() );
                return;
            }
            mrn_max( &((*cur_packet)[0]->val.p), &((*cur_packet)[0]->val.p), result, type );
        }
        else{
            mrn_max( result, &((*cur_packet)[0]->val.p), result, type );
        }
    }

    if( type == CHAR_T ){
        char tmp = *((char *)result);
        PacketPtr new_packet( new Packet( ipackets[0]->get_StreamId( ),
                                          ipackets[0]->get_Tag( ),
                                          format_string.c_str(), tmp ) );
        opackets.push_back( new_packet );
    }
    else if( type == UCHAR_T ){
        uchar_t tmp = *((uchar_t *)result);
        PacketPtr new_packet( new Packet( ipackets[0]->get_StreamId( ),
                                          ipackets[0]->get_Tag( ),
                                          format_string.c_str(), tmp ) );
        opackets.push_back( new_packet );
    }
    else if( type == FLOAT_T ){
        float tmp = *((float*)result);
        PacketPtr new_packet( new Packet( ipackets[0]->get_StreamId( ),
                                          ipackets[0]->get_Tag( ),
                                          format_string.c_str(), tmp ) );
        opackets.push_back( new_packet );
    }
    else if( type == DOUBLE_T ){
        double tmp = *((double*)result);
        PacketPtr new_packet( new Packet( ipackets[0]->get_StreamId( ),
                                          ipackets[0]->get_Tag( ),
                                          format_string.c_str(), tmp ) );
        opackets.push_back( new_packet );
    }
    else if( type == INT16_T ){
        int16_t tmp = *((int16_t*)result);
        PacketPtr new_packet( new Packet( ipackets[0]->get_StreamId( ),
                                          ipackets[0]->get_Tag( ),
                                          format_string.c_str(), tmp ) );
        opackets.push_back( new_packet );
    }
    else if( type == UINT16_T ){
        uint16_t tmp = *((uint16_t*)result);
        PacketPtr new_packet( new Packet( ipackets[0]->get_StreamId( ),
                                          ipackets[0]->get_Tag( ),
                                          format_string.c_str(), tmp ) );
        opackets.push_back( new_packet );
    }
    else if( type == INT32_T ){
        int32_t tmp = *((int32_t*)result);
        PacketPtr new_packet( new Packet( ipackets[0]->get_StreamId( ),
                                          ipackets[0]->get_Tag( ),
                                          format_string.c_str(), tmp ) );
        opackets.push_back( new_packet );
    }
    else if( type == UINT32_T ){
        uint32_t tmp = *((uint32_t*)result);
        PacketPtr new_packet( new Packet( ipackets[0]->get_StreamId( ),
                                          ipackets[0]->get_Tag( ),
                                          format_string.c_str(), tmp ) );
        opackets.push_back( new_packet );
    }
    else if( type == INT64_T ){
        int64_t tmp = *((int64_t*)result);
        PacketPtr new_packet( new Packet( ipackets[0]->get_StreamId( ),
                                          ipackets[0]->get_Tag( ),
                                          format_string.c_str(), tmp ) );
        opackets.push_back( new_packet );
    }
    else if( type == UINT64_T ){
        uint64_t tmp = *((uint64_t*)result);
        PacketPtr new_packet( new Packet( ipackets[0]->get_StreamId( ),
                                          ipackets[0]->get_Tag( ),
                                          format_string.c_str(), tmp ) );
        opackets.push_back( new_packet );
    }
    else{
        assert(0);
    }
}

void tfilter_Min( const vector< PacketPtr >& ipackets,
                  vector< PacketPtr >& opackets,
                  vector< PacketPtr >& /* opackets_reverse */,
                  void ** /* client data */, PacketPtr& )
{
    char result[8]; //ptr to 8 bytes
    string format_string;
    DataType type=UNKNOWN_T;

    for( unsigned int i = 0; i < ipackets.size( ); i++ ) {
        PacketPtr cur_packet( ipackets[i] );
        assert( strcmp(cur_packet->get_FormatString(),
                       ipackets[0]->get_FormatString()) == 0 );

        if( i == 0 ){ //1st time thru
            //+ 1 "hack" to get past "%" in arg to fmt2type()	
            type = Fmt2Type( cur_packet->get_FormatString( ) +1);
            switch(type){
            case CHAR_T:
                format_string="%c";
                break;
            case UCHAR_T:
                format_string="%uc";
                break;
            case INT16_T:
                format_string="%hd";
                break;
            case UINT16_T:
                format_string="%uhd";
                break;
            case INT32_T:
                format_string="%d";
                break;
            case UINT32_T:
                format_string="%ud";
                break;
            case INT64_T:
                format_string="%ld";
                break;
            case UINT64_T:
                format_string="%uld";
                break;
            case FLOAT_T:
                format_string="%f";
                break;
            case DOUBLE_T:
                format_string="%lf";
                break;
            case CHAR_ARRAY_T:
            case UCHAR_ARRAY_T:
            case INT16_ARRAY_T:
            case UINT16_ARRAY_T:
            case INT32_ARRAY_T:
            case UINT32_ARRAY_T:
            case INT64_ARRAY_T:
            case UINT64_ARRAY_T:
            case FLOAT_ARRAY_T:
            case DOUBLE_ARRAY_T:
            case STRING_T:
            case UNKNOWN_T:
            default:
                fprintf(stderr, "ERROR: tfilter_Min() - invalid packet type: %d (%s)\n", type,
                        cur_packet->get_FormatString() );
                return;
            }
            mrn_min( &((*cur_packet)[0]->val.p), &((*cur_packet)[0]->val.p), result, type );
        }
        else{
            mrn_min( result, &((*cur_packet)[0]->val.p), result, type );
        }
    }
    
    if( type == CHAR_T ){
        char tmp = *((char *)result);
        PacketPtr new_packet( new Packet( ipackets[0]->get_StreamId( ),
                                          ipackets[0]->get_Tag( ),
                                          format_string.c_str(), tmp ) );
        opackets.push_back( new_packet );
    }
    else if( type == UCHAR_T ){
        uchar_t tmp = *((uchar_t *)result);
        PacketPtr new_packet( new Packet( ipackets[0]->get_StreamId( ),
                                          ipackets[0]->get_Tag( ),
                                          format_string.c_str(), tmp ) );
        opackets.push_back( new_packet );
    }
    else if( type == FLOAT_T ){
        float tmp = *((float*)result);
        PacketPtr new_packet( new Packet( ipackets[0]->get_StreamId( ),
                                          ipackets[0]->get_Tag( ),
                                          format_string.c_str(), tmp ) );
        opackets.push_back( new_packet );
    }
    else if( type == DOUBLE_T ){
        double tmp = *((double*)result);
        PacketPtr new_packet( new Packet( ipackets[0]->get_StreamId( ),
                                          ipackets[0]->get_Tag( ),
                                          format_string.c_str(), tmp ) );
        opackets.push_back( new_packet );
    }
    else if( type == INT16_T ){
        int16_t tmp = *((int16_t*)result);
        PacketPtr new_packet( new Packet( ipackets[0]->get_StreamId( ),
                                          ipackets[0]->get_Tag( ),
                                          format_string.c_str(), tmp ) );
        opackets.push_back( new_packet );
    }
    else if( type == UINT16_T ){
        uint16_t tmp = *((uint16_t*)result);
        PacketPtr new_packet( new Packet( ipackets[0]->get_StreamId( ),
                                          ipackets[0]->get_Tag( ),
                                          format_string.c_str(), tmp ) );
        opackets.push_back( new_packet );
    }
    else if( type == INT32_T ){
        int32_t tmp = *((int32_t*)result);
        PacketPtr new_packet( new Packet( ipackets[0]->get_StreamId( ),
                                          ipackets[0]->get_Tag( ),
                                          format_string.c_str(), tmp ) );
        opackets.push_back( new_packet );
    }
    else if( type == UINT32_T ){
        uint32_t tmp = *((uint32_t*)result);
        PacketPtr new_packet( new Packet( ipackets[0]->get_StreamId( ),
                                          ipackets[0]->get_Tag( ),
                                          format_string.c_str(), tmp ) );
        opackets.push_back( new_packet );
    }
    else if( type == INT64_T ){
        int64_t tmp = *((int64_t*)result);
        PacketPtr new_packet( new Packet( ipackets[0]->get_StreamId( ),
                                          ipackets[0]->get_Tag( ),
                                          format_string.c_str(), tmp ) );
        opackets.push_back( new_packet );
    }
    else if( type == UINT64_T ){
        uint64_t tmp = *((uint64_t*)result);
        PacketPtr new_packet( new Packet( ipackets[0]->get_StreamId( ),
                                          ipackets[0]->get_Tag( ),
                                          format_string.c_str(), tmp ) );
        opackets.push_back( new_packet );
    }
    else{
        assert(0);
    }

}

void tfilter_Avg( const vector < PacketPtr >& ipackets,
                  vector< PacketPtr >& opackets,
                  vector< PacketPtr >& /* opackets_reverse */,
                  void ** /* client data */, PacketPtr& )
{
    char result[8]; //ptr to 8 bytes
    char product[8];
    int num_results=0;
    string format_string;
    DataType type=UNKNOWN_T;

    memset(result, 0, 8); //zeroing the bufs
    memset(product, 0, 8);
		
    for( unsigned int i = 0; i < ipackets.size( ); i++ ) {
        PacketPtr cur_packet( ipackets[i] );
        assert( strcmp(cur_packet->get_FormatString(),
                       ipackets[0]->get_FormatString()) == 0 );

        if( i == 0 ){
            format_string = cur_packet->get_FormatString();

            if( format_string == "%c %d" )
                type = CHAR_T;
            else if( format_string == "%uc %d" )
                type = UCHAR_T;
            else if( format_string == "%hd %d" )
                type = INT16_T;
            else if( format_string == "%uhd %d" )
                type = UINT16_T;
            else if( format_string == "%d %d" )
                type = INT32_T;
            else if( format_string == "%ud %d" )
                type = UINT32_T;
            else if( format_string == "%ld %d" )
                type = INT64_T;
            else if( format_string == "%uld %d" )
                type = UINT64_T;
            else if( format_string == "%f %d" )
                type = FLOAT_T;
            else if( format_string == "%lf %d" )
                type = DOUBLE_T;
            else {
                fprintf(stderr, "ERROR: tfilter_Avg() - invalid packet type: %d (%s)\n", 
                        type, cur_packet->get_FormatString() );
                return;
            }
        }

        mult(&((*cur_packet)[0]->val.p), (*cur_packet)[1]->val.d, product, type);	
        
        sum( result, product, result, type );
	
        num_results += (*cur_packet)[1]->val.d;
        
    }
    
    div( result, num_results, result, type );

    if( type == CHAR_T ){
        char tmp = *((char *)result);
        PacketPtr new_packet( new Packet( ipackets[0]->get_StreamId( ),
                                          ipackets[0]->get_Tag( ),
                                          format_string.c_str(), tmp, num_results ) );
        opackets.push_back( new_packet );
    }
    else if( type == UCHAR_T ){
        uchar_t tmp = *((uchar_t *)result);
        PacketPtr new_packet( new Packet( ipackets[0]->get_StreamId( ),
                                          ipackets[0]->get_Tag( ),
                                          format_string.c_str(), tmp, num_results ) );
        opackets.push_back( new_packet );
    }
    else if( type == FLOAT_T ){
        float tmp = *((float*)result);
        PacketPtr new_packet( new Packet( ipackets[0]->get_StreamId( ),
                                          ipackets[0]->get_Tag( ),
                                          format_string.c_str(), tmp, num_results ) );
        opackets.push_back( new_packet );
    }
    else if( type == DOUBLE_T ){
        double tmp = *((double*)result);
        PacketPtr new_packet( new Packet( ipackets[0]->get_StreamId( ),
                                          ipackets[0]->get_Tag( ),
                                          format_string.c_str(), tmp, num_results ) );
        opackets.push_back( new_packet );
    }
    else if( type == INT16_T ){
        int16_t tmp = *((int16_t*)result);
        PacketPtr new_packet( new Packet( ipackets[0]->get_StreamId( ),
                                          ipackets[0]->get_Tag( ),
                                          format_string.c_str(), tmp, num_results ));
        opackets.push_back( new_packet );
    }
    else if( type == UINT16_T ){
        uint16_t tmp = *((uint16_t*)result);
        PacketPtr new_packet( new Packet( ipackets[0]->get_StreamId( ),
                                          ipackets[0]->get_Tag( ),
                                          format_string.c_str(), tmp, num_results ));
        opackets.push_back( new_packet );
    }
    else if( type == INT32_T ){
        int32_t tmp = *((int32_t*)result);
        PacketPtr new_packet( new Packet( ipackets[0]->get_StreamId( ),
                                          ipackets[0]->get_Tag( ),
                                          format_string.c_str(), tmp, num_results ));
        opackets.push_back( new_packet );
    }
    else if( type == UINT32_T ){
        uint32_t tmp = *((uint32_t*)result);
        PacketPtr new_packet( new Packet( ipackets[0]->get_StreamId( ),
                                          ipackets[0]->get_Tag( ),
                                          format_string.c_str(), tmp, num_results ));
        opackets.push_back( new_packet );
    }
    else if( type == INT64_T ){
        int64_t tmp = *((int64_t*)result);
        PacketPtr new_packet( new Packet( ipackets[0]->get_StreamId( ),
                                          ipackets[0]->get_Tag( ),
                                          format_string.c_str(), tmp, num_results ));
        opackets.push_back( new_packet );
    }
    else if( type == UINT64_T ){
        uint64_t tmp = *((uint64_t*)result);
        PacketPtr new_packet( new Packet( ipackets[0]->get_StreamId( ),
                                          ipackets[0]->get_Tag( ),
                                          format_string.c_str(), tmp, num_results ));
        opackets.push_back( new_packet );
    }
    else{
        PacketPtr new_packet( new Packet( ipackets[0]->get_StreamId( ),
                                          ipackets[0]->get_Tag( ),
                                          format_string.c_str(), result, num_results ));
        opackets.push_back( new_packet );
    }
}



void tfilter_ArrayConcat( const vector< PacketPtr >& ipackets,
                          vector< PacketPtr >& opackets,
                          vector< PacketPtr >& /* opackets_reverse */,
                          void ** /* client data */, PacketPtr& )
{
    unsigned int result_array_size=0, i, j;
    char * result_array=NULL;
    int data_size=0;
    string format_string;
    DataType type=UNKNOWN_T;
 
    vector< void* > iarrays;
    vector< unsigned > iarray_lens;
    void *tmp_arr;
    unsigned tmp_arr_len;
 
    for( i = 0; i < ipackets.size( ); i++ ) {
        PacketPtr cur_packet( ipackets[i] );
        assert( strcmp(cur_packet->get_FormatString(),
                       ipackets[0]->get_FormatString()) == 0 );
        if( i == 0 ){ //1st time thru
            //+ 1 "hack" to get past "%" in arg to fmt2type()	
            type = Fmt2Type( cur_packet->get_FormatString( ) +1);
            switch(type){
            case CHAR_ARRAY_T:
                data_size = sizeof(char);
                format_string="%ac";
                break;
            case UCHAR_ARRAY_T:
                data_size = sizeof(unsigned char);
                format_string="%auc";
                break;
            case INT16_ARRAY_T:
                data_size = sizeof(int16_t);
                format_string="%ahd";
                break;
            case UINT16_ARRAY_T:
                data_size = sizeof(uint16_t);
                format_string="%auhd";
                break;
            case INT32_ARRAY_T:
                data_size = sizeof(int32_t);
                format_string="%ad";
                break;
            case UINT32_ARRAY_T:
                data_size = sizeof(uint32_t);
                format_string="%aud";
                break;
            case INT64_ARRAY_T:
                data_size = sizeof(int64_t);
                format_string="%ald";
                break;
            case UINT64_ARRAY_T:
                data_size = sizeof(uint64_t);
                format_string="%auld";
                break;
            case FLOAT_ARRAY_T:
                data_size = sizeof(float);
                format_string="%af";
                break;
            case DOUBLE_ARRAY_T:
                data_size = sizeof(double);
                format_string="%alf";
                break;
            case STRING_ARRAY_T:
                data_size = sizeof(char*);
                format_string="%as";
                break;
            case CHAR_T:
            case UCHAR_T:
            case INT16_T:
            case UINT16_T:
            case INT32_T:
            case UINT32_T:
            case INT64_T:
            case UINT64_T:
            case FLOAT_T:
            case DOUBLE_T:
            case STRING_T:
            case UNKNOWN_T:
            default:
                fprintf(stderr, "ERROR: tfilter_ArrayConcat() - invalid packet type: %d (%s)\n", 
                        type, cur_packet->get_FormatString() );
                return;
            }
        }
        if( cur_packet->unpack( format_string.c_str(), 
                                &tmp_arr, &tmp_arr_len ) == -1 ) {
            fprintf(stderr, "ERROR: tfilter_ArrayConcat() - unpack(%s) failure\n", 
                    cur_packet->get_FormatString() );
        }
        else {
           iarrays.push_back( tmp_arr );
           iarray_lens.push_back( tmp_arr_len );
           result_array_size += tmp_arr_len;
        }
    }

    result_array = (char *) malloc( result_array_size * data_size );
    
    unsigned pos = 0;
    for( i = 0; i < iarrays.size( ); i++ ) {
        if( type != STRING_ARRAY_T ) {
            memcpy( result_array + pos,
                    iarrays[i],
                    iarray_lens[i] * data_size );
        }
        else {
            char** iarray = (char**) iarrays[i];
            char** oarray = (char**)( result_array + pos );
            for( j = 0; j < iarray_lens[i]; j++ )
                oarray[j] = strdup( iarray[j] );
        }
        pos += ( iarray_lens[i] * data_size );
    }
    fflush(stderr);

    PacketPtr new_packet( new Packet( ipackets[0]->get_StreamId( ),
                                      ipackets[0]->get_Tag( ),
                                      format_string.c_str(), result_array, result_array_size ) );
    // tell MRNet to free result_array
    new_packet->set_DestroyData(true);

    opackets.push_back( new_packet );
}

void tfilter_IntEqClass( const vector< PacketPtr >& ipackets,
                         vector< PacketPtr >& opackets,
                         vector< PacketPtr >& /* opackets_reverse */,
                         void ** /* client data */, PacketPtr& )
{
    DataType type;
    uint32_t array_len0, array_len1, array_len2;
    map< unsigned int, vector < unsigned int > > classes;
    unsigned int i;

    // find equivalence classes across our input 
    for( i = 0; i < ipackets.size(); i++ ) {
        PacketPtr cur_packet( ipackets[i] );
        const unsigned int *vals = ( const unsigned int * )
            ( (*cur_packet)[0]->get_array(&type, &array_len0) );
        const unsigned int *memcnts = ( const unsigned int * )
            ( (*cur_packet)[1]->get_array(&type, &array_len1) );
        const unsigned int *mems = ( const unsigned int * )
            ( (*cur_packet)[2]->get_array(&type, &array_len2) );

	assert( array_len0 == array_len1 );
	unsigned int curClassMemIdx = 0;
	for( unsigned int j = 0; j < array_len0; j++ ) {
	    mrn_dbg( 3, mrn_printf(FLF, stderr,
			"\tclass %d: val = %u, nMems = %u, mems = ", j,
			vals[j], memcnts[j] ));

	    // update the members for the current class
	    for( unsigned int k = 0; k < memcnts[j]; k++ ) {
	        mrn_dbg( 3, mrn_printf(FLF, stderr, "%d ",
			    mems[curClassMemIdx + k] ));
		classes[vals[j]].push_back( mems[curClassMemIdx + k] );
	    }
	    mrn_dbg( 3, mrn_printf(FLF, stderr, "\n" ));
	    curClassMemIdx += memcnts[j];
	}
    }

    // build data structures for the output 
    unsigned int *values = new unsigned int[classes.size(  )];
    unsigned int *memcnts = new unsigned int[classes.size(  )];
    unsigned int nMems = 0;
    unsigned int curIdx = 0;
    for( map< unsigned int,
	      vector< unsigned int > >::iterator iter = classes.begin(  );
	 iter != classes.end(  ); iter++ ) {
        values[curIdx] = iter->first;
	memcnts[curIdx] = ( iter->second ).size(  );
	nMems += memcnts[curIdx];
	curIdx++;
    }
    unsigned int *mems = new unsigned int[nMems];
    unsigned int curMemIdx = 0;
    unsigned int curClassIdx = 0;
    for( map< unsigned int,
              vector< unsigned int > >::iterator citer = classes.begin(  );
            citer != classes.end(  ); citer++ ) {
        for( unsigned int j = 0; j < memcnts[curClassIdx]; j++ ) {
	  mems[curMemIdx] = ( citer->second )[j];
	  curMemIdx++;
	}
	curClassIdx++;
    }
    
    PacketPtr new_packet( new Packet( ipackets[0]->get_StreamId( ),
                                      ipackets[0]->get_Tag( ),
                                      "%aud %aud %aud",
                                      values, classes.size( ),
                                      memcnts, classes.size( ),
                                      mems, nMems ));
    opackets.push_back( new_packet );

    mrn_dbg( 3, mrn_printf(FLF, stderr, "tfilter_IntEqClass: returning\n" ));
    unsigned int curMem = 0;
    for( i = 0; i < classes.size(  ); i++ ) {
        mrn_dbg( 3, mrn_printf(FLF, stderr,
		    "\tclass %d: val = %u, nMems = %u, mems = ", i,
                               ( ( const unsigned int * )( (*opackets[0])[0]->val.p) )[i],
                               ( ( const unsigned int * )( (*opackets[0])[1]->val.p) )[i] ));
	for( unsigned int j = 0;
	     j < ( ( const unsigned int * )( (*opackets[0])[1]->val.p ) )[i];
	     j++ ) {
	    mrn_dbg( 3, mrn_printf(FLF, stderr, "%d ",
                               ( ( const unsigned int * )( (*opackets[0])[2]->val.
					      p ) )[curMem] ));
	    curMem++;
	}
	mrn_dbg( 3, mrn_printf(FLF, stderr, "\n" ));
    }
}

/*==========================================*
 *    Default SyncFilter Definitions        *
 *==========================================*/

typedef struct {
    map < Rank, vector< PacketPtr >* > packets_by_rank;
    set < Rank > ready_peers;
} wfa_state;

void sfilter_WaitForAll( const vector< PacketPtr >& ipackets,
                         vector< PacketPtr >& opackets,
                         vector< PacketPtr >& /* opackets_reverse */,
                         void **local_storage, PacketPtr& )
{
    mrn_dbg_func_begin();
    map < Rank, vector< PacketPtr >* >::iterator map_iter, del_iter;
    wfa_state * state;
    
    int stream_id = ipackets[0]->get_StreamId();
    Stream * stream = network->get_Stream( stream_id );

    //1. Setup/Recover Filter State
    if( *local_storage == NULL ) {
        //Allocate packet buffer map as appropriate
        mrn_dbg( 5, mrn_printf(FLF, stderr, "No previous storage, allocating ...\n"));
        state = new wfa_state;
        *local_storage = state;
    }
    else{
        //Get packet buffer map from filter state
        state = ( wfa_state * ) *local_storage;

        //Check for failed nodes && closed Peers
        map_iter = state->packets_by_rank.begin( );
        while ( map_iter != state->packets_by_rank.end( ) ) {

            Rank rank = (*map_iter).first;
            mrn_dbg( 5, mrn_printf(FLF, stderr, "Node[%d] failed?", rank ));
            if( network->node_Failed( rank ) ) {
                mrn_dbg( 5, mrn_printf(0,0,0, stderr, " Yes\n", rank ));
                mrn_dbg( 5, mrn_printf(FLF, stderr,
                                       "Discarding packets from failed node[%d] ...\n",
                                       rank ));

                del_iter = map_iter;
                map_iter++;

                //clear packet vector
                (*del_iter).second->clear();

                //erase map slot
                state->packets_by_rank.erase( del_iter );
                state->ready_peers.erase( rank );
            }
            else{
                mrn_dbg( 5, mrn_printf(0,0,0, stderr, " No\n", rank ));
                map_iter++;
            }
        }
    }

    //2. Place input packets
    for( unsigned int i=0; i<ipackets.size(); i++ ){
        Rank cur_inlet_rank = ipackets[i]->get_InletNodeRank();
        map_iter = state->packets_by_rank.find( cur_inlet_rank );

        if( network->node_Failed( cur_inlet_rank ) ) {
            //Drop packets from failed node
            continue;
        }

        //Insert packet into map

        //Allocate new slot if necessary
        if( map_iter == state->packets_by_rank.end() ) {
            mrn_dbg( 5, mrn_printf(FLF, stderr,
                                   "Allocating new map slot for node[%d] ...\n",
                                   cur_inlet_rank ));
            state->packets_by_rank[ cur_inlet_rank ] = new vector < PacketPtr >;
        }

        mrn_dbg( 3, mrn_printf(FLF, stderr, "Placing packet[%d] from node[%d]\n",
                               i, cur_inlet_rank ));
        state->packets_by_rank[ cur_inlet_rank ]->push_back( ipackets[i] );
        state->ready_peers.insert( cur_inlet_rank );
    }

    set< Rank > peers = stream->get_ChildPeers( );
    set< Rank > closed_peers = stream->get_ClosedPeers( );

    mrn_dbg( 3, mrn_printf(FLF, stderr, "slots: %d ready:%d peers:%d closed:%d\n",
                           state->packets_by_rank.size(), state->ready_peers.size(),
                           peers.size(), closed_peers.size() ) );

    if( closed_peers.empty() && state->ready_peers.size() < peers.size() ) {
        //no closed peers and not all peers ready, so sync condition not met
        return;
    }

    set< Rank > unready_peers;
    set_difference( peers.begin(), peers.end(),
                    state->ready_peers.begin(), state->ready_peers.end(),
                    inserter( unready_peers, unready_peers.begin() ) );
    if( unready_peers != closed_peers ) {
        //some peers not ready and haven't been closed, sync condition not met
        return;
    }

    //If we get here, SYNC CONDITION MET!
    if( state->ready_peers.size() + closed_peers.size() < peers.size() ) {
        return;
    }

    //check for a complete wave
    mrn_dbg( 3, mrn_printf(FLF, stderr, "All child nodes ready?" ));

    //3. All nodes ready! Place output packets
    for( map_iter = state->packets_by_rank.begin( );
         map_iter != state->packets_by_rank.end( );
         map_iter++ ) {

        if( (*map_iter).second->empty() ) {
            //list should only be empty if peer closed stream
            mrn_dbg( 3, mrn_printf(FLF, stderr, "Node[%d]'s slot is empty\n",
                                   (*map_iter).first ) );
            continue;
        }

        mrn_dbg( 3, mrn_printf(FLF, stderr, "Popping packet from Node[%d]\n",
                               (*map_iter).first ) );
        //push head of list onto output vector
        opackets.push_back( (*map_iter).second->front() );
        (*map_iter).second->erase( (*map_iter).second->begin() );
        
        //if list now empty, remove slot from ready list
        if( (*map_iter).second->empty() ) {
            mrn_dbg( 3, mrn_printf(FLF, stderr, "Removing Node[%d] from ready list\n",
                                   (*map_iter).first ) );
            state->ready_peers.erase( (*map_iter).first );
        }
    }
    mrn_dbg( 3, mrn_printf(FLF, stderr, "Returning %d packets\n", opackets.size(  ) ));
}

void sfilter_TimeOut( const vector< PacketPtr > &,
                      vector< PacketPtr > &,
                      vector< PacketPtr > &,
                      void **, PacketPtr& )
{
}

static inline void sum(const void *in1, const void *in2, void* out, DataType type)
{
  switch (type){
  case CHAR_T:
    *( (char*) out ) = *((const char*)in1) + *((const char*)in2);
    break;
  case UCHAR_T:
    *( (uchar_t*) out ) = *((const uchar_t*)in1) + *((const uchar_t*)in2);
    break;
  case INT16_T:
    *( (int16_t*) out ) = *((const int16_t*)in1) + *((const int16_t*)in2);
    break;
  case UINT16_T:
    *( (uint16_t*) out ) = *((const uint16_t*)in1) + *((const uint16_t*)in2);
    break;
  case INT32_T:
    *( (int32_t*) out ) = *((const int32_t*)in1) + *((const int32_t*)in2);
    break;
  case UINT32_T:
    *( (uint32_t*) out ) = *((const uint32_t*)in1) + *((const uint32_t*)in2);
    break;
  case INT64_T:
    *( (int64_t*) out ) = *((const int64_t*)in1) + *((const int64_t*)in2);
    break;
  case UINT64_T:
    *( (uint64_t*) out ) = *((const uint64_t*)in1) + *((const uint64_t*)in2);
    break;
  case FLOAT_T:
    *( (float*) out ) = *((const float*)in1) + *((const float*)in2);
    break;
  case DOUBLE_T:
    *( (double*) out ) = *((const double*)in1) + *((const double*)in2);
    break;
  case CHAR_ARRAY_T:
  case UCHAR_ARRAY_T:
  case INT16_ARRAY_T:
  case UINT16_ARRAY_T:
  case INT32_ARRAY_T:
  case UINT32_ARRAY_T:
  case INT64_ARRAY_T:
  case UINT64_ARRAY_T:
  case FLOAT_ARRAY_T:
  case DOUBLE_ARRAY_T:
  case STRING_T:
  case STRING_ARRAY_T:
  case UNKNOWN_T:
    assert(0);
  }
}

static inline void mult(const void *in1, int in2, void* out, DataType type)
{
  switch (type){
  case CHAR_T:
    *( (char*) out ) = *((const char*)in1) * in2;
    break;
  case UCHAR_T:
    *( (uchar_t*) out ) = *((const uchar_t*)in1) * in2;
    break;
  case INT16_T:
    *( (int16_t*) out ) = *((const int16_t*)in1) * in2;
    break;
  case UINT16_T:
    *( (uint16_t*) out ) = *((const uint16_t*)in1) * in2;
    break;
  case INT32_T:
    *( (int32_t*) out ) = *((const int32_t*)in1) * in2;	
    break;
  case UINT32_T:
    *( (uint32_t*) out ) = *((const uint32_t*)in1) * in2;
    break;
  case INT64_T:
    *( (int64_t*) out ) = *((const int64_t*)in1) * (int64_t)in2;
    break;
  case UINT64_T:
    *( (uint64_t*) out ) = *((const uint64_t*)in1) * (int64_t)in2;
    break;
  case FLOAT_T:
    *( (float*) out ) = *((const float*)in1) * (float)in2;
    break;
  case DOUBLE_T:
    *( (double*) out ) = *((const double*)in1) * (double)in2;
    break;
  case CHAR_ARRAY_T:
  case UCHAR_ARRAY_T:
  case INT16_ARRAY_T:
  case UINT16_ARRAY_T:
  case INT32_ARRAY_T:
  case UINT32_ARRAY_T:
  case INT64_ARRAY_T:
  case UINT64_ARRAY_T:
  case FLOAT_ARRAY_T:
  case DOUBLE_ARRAY_T:
  case STRING_T:
  case STRING_ARRAY_T:
  case UNKNOWN_T:
    assert(0);
  }
}

static inline void div(const void *in1, int in2, void* out, DataType type)
{
  switch (type){
  case CHAR_T:
    *( (char*) out ) = *((const char*)in1) / in2;
    break;
  case UCHAR_T:
    *( (uchar_t*) out ) = *((const uchar_t*)in1) / in2;
    break;
  case INT16_T:
    *( (int16_t*) out ) = *((const int16_t*)in1) / in2;
    break;
  case UINT16_T:
    *( (uint16_t*) out ) = *((const uint16_t*)in1) / in2;
    break;
  case INT32_T:
    *( (int32_t*) out ) = *((const int32_t*)in1) / in2;
    break;
  case UINT32_T:
    *( (uint32_t*) out ) = *((const uint32_t*)in1) / in2;
    break;
  case INT64_T:
    *( (int64_t*) out ) = *((const int64_t*)in1) / (int64_t)in2;
    break;
  case UINT64_T:
    *( (uint64_t*) out ) = *((const uint64_t*)in1) / (int64_t)in2;
    break;
  case FLOAT_T:
    *( (float*) out ) = *((const float*)in1) / (float)in2;
    break;
  case DOUBLE_T:
    *( (double*) out ) = *((const double*)in1) / (double)in2;
    break;
  case CHAR_ARRAY_T:
  case UCHAR_ARRAY_T:
  case INT16_ARRAY_T:
  case UINT16_ARRAY_T:
  case INT32_ARRAY_T:
  case UINT32_ARRAY_T:
  case INT64_ARRAY_T:
  case UINT64_ARRAY_T:
  case FLOAT_ARRAY_T:
  case DOUBLE_ARRAY_T:
  case STRING_T:
  case STRING_ARRAY_T:
  case UNKNOWN_T:
    assert(0);
  }
}

static inline void mrn_min(const void *in1, const void *in2, void* out, DataType type)
{
    switch (type){
    case CHAR_T:
        *( (char*) out ) = ( ( *((const char*)in1) < *((const char*)in2) ) ?
                             *((const char*)in1) : *((const char*)in2) );
      break;
    case UCHAR_T:
        *( (uchar_t*) out ) = ( ( *((const uchar_t*)in1) < *((const uchar_t*)in2) ) ?
                                *((const uchar_t*)in1) : *((const uchar_t*)in2) );
      break;
    case INT16_T:
        *( (int16_t*) out ) = ( ( *((const int16_t*)in1) < *((const int16_t*)in2) ) ?
                                *((const int16_t*)in1) : *((const int16_t*)in2) );
        break;
    case UINT16_T:
        *( (uint16_t*) out ) = ( ( *((const uint16_t*)in1) < *((const uint16_t*)in2) ) ?
                                 *((const uint16_t*)in1) : *((const uint16_t*)in2) );
      break;
    case INT32_T:
        *( (int32_t*) out ) = ( ( *((const int32_t*)in1) < *((const int32_t*)in2) ) ?
                                *((const int32_t*)in1) : *((const int32_t*)in2) );
      break;
    case UINT32_T:
        *( (uint32_t*) out ) = ( ( *((const uint32_t*)in1) < *((const uint32_t*)in2) ) ?
                                 *((const uint32_t*)in1) : *((const uint32_t*)in2) );
      break;
    case INT64_T:
        *( (int64_t*) out ) = ( ( *((const int64_t*)in1) < *((const int64_t*)in2) ) ?
                                *((const int64_t*)in1) : *((const int64_t*)in2) );
      break;
    case UINT64_T:
        *( (uint64_t*) out ) = ( ( *((const uint64_t*)in1) < *((const uint64_t*)in2) ) ?
                                 *((const uint64_t*)in1) : *((const uint64_t*)in2) );
      break;
    case FLOAT_T:
        *( (float*) out ) = ( ( *((const float*)in1) < *((const float*)in2) ) ?
                              *((const float*)in1) : *((const float*)in2) );
      break;
    case DOUBLE_T:
        *( (double*) out ) = ( ( *((const double*)in1) < *((const double*)in2) ) ?
                               *((const double*)in1) : *((const double*)in2) );
      break;
    case CHAR_ARRAY_T:
    case UCHAR_ARRAY_T:
    case INT16_ARRAY_T:
    case UINT16_ARRAY_T:
    case INT32_ARRAY_T:
    case UINT32_ARRAY_T:
    case INT64_ARRAY_T:
    case UINT64_ARRAY_T:
    case FLOAT_ARRAY_T:
    case DOUBLE_ARRAY_T:
    case STRING_T:
    case STRING_ARRAY_T:
    case UNKNOWN_T:
        assert(0);
    }
}

static inline void mrn_max(const void *in1, const void *in2, void* out, DataType type )
{
    switch (type){
    case CHAR_T:
        *( (char*) out ) = ( ( *((const char*)in1) > *((const char*)in2) ) ?
                             *((const char*)in1) : *((const char*)in2) );
        break;
    case UCHAR_T:
        *( (uchar_t*) out ) = ( ( *((const uchar_t*)in1) > *((const uchar_t*)in2) ) ?
                                *((const uchar_t*)in1) : *((const uchar_t*)in2) );
        break;
    case INT16_T:
        *( (int16_t*) out ) = ( ( *((const int16_t*)in1) > *((const int16_t*)in2) ) ?
                                *((const int16_t*)in1) : *((const int16_t*)in2) );
        break;
    case UINT16_T:
        *( (uint16_t*) out ) = ( ( *((const uint16_t*)in1) > *((const uint16_t*)in2) ) ?
                                 *((const uint16_t*)in1) : *((const uint16_t*)in2) );
        break;
    case INT32_T:
        *( (int32_t*) out ) = ( ( *((const int32_t*)in1) > *((const int32_t*)in2) ) ?
                                *((const int32_t*)in1) : *((const int32_t*)in2) );
        break;
    case UINT32_T:
        *( (uint32_t*) out ) = ( ( *((const uint32_t*)in1) > *((const uint32_t*)in2) ) ?
                                 *((const uint32_t*)in1) : *((const uint32_t*)in2) );
        break;
    case INT64_T:
        *( (int64_t*) out ) = ( ( *((const int64_t*)in1) > *((const int64_t*)in2) ) ?
                                *((const int64_t*)in1) : *((const int64_t*)in2) );
        break;
    case UINT64_T:
        *( (uint64_t*) out ) = ( ( *((const uint64_t*)in1) > *((const uint64_t*)in2) ) ?
                                 *((const uint64_t*)in1) : *((const uint64_t*)in2) );
        break;
    case FLOAT_T:
        *( (float*) out ) = ( ( *((const float*)in1) > *((const float*)in2) ) ?
                              *((const float*)in1) : *((const float*)in2) );
        break;
    case DOUBLE_T:
        *( (double*) out ) = ( ( *((const double*)in1) > *((const double*)in2) ) ?
                               *((const double*)in1) : *((const double*)in2) );
        break;
    case CHAR_ARRAY_T:
    case UCHAR_ARRAY_T:
    case INT16_ARRAY_T:
    case UINT16_ARRAY_T:
    case INT32_ARRAY_T:
    case UINT32_ARRAY_T:
    case INT64_ARRAY_T:
    case UINT64_ARRAY_T:
    case FLOAT_ARRAY_T:
    case DOUBLE_ARRAY_T:
    case STRING_T:
    case STRING_ARRAY_T:
    case UNKNOWN_T:
        assert(0);
    }
}



} /* namespace MRN */
