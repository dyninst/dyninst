#include <map>
#include <string>

#include "mrnet/src/FilterDefinitions.h"
#include "mrnet/src/utils.h"
#include "mrnet/src/DataElement.h"
#include "mrnet/src/Packet.h"

namespace MRN
{

unsigned short TFILTER_NULL=0;
const char * TFILTER_NULL_FORMATSTR="";  // "" => "Don't check fmt string"

unsigned short TFILTER_SUM=0;
const char * TFILTER_SUM_FORMATSTR="";  // "" => "Don't check fmt string"

unsigned short TFILTER_AVG=0;
const char * TFILTER_AVG_FORMATSTR="";  // "" => "Don't check fmt string"

unsigned short TFILTER_MIN=0;
const char * TFILTER_MIN_FORMATSTR="";  // "" => "Don't check fmt string"

unsigned short TFILTER_MAX=0;
const char * TFILTER_MAX_FORMATSTR="";  // "" => "Don't check fmt string"

unsigned short TFILTER_ARRAY_CONCAT=0;
const char * TFILTER_ARRAY_CONCAT_FORMATSTR=""; // "" => "Don't check fmt string"

unsigned short TFILTER_INT_EQ_CLASS=0;
const char * TFILTER_INT_EQ_CLASS_FORMATSTR="%aud %aud %aud";

unsigned short SFILTER_WAITFORALL=0;
unsigned short SFILTER_DONTWAIT=0;
unsigned short SFILTER_TIMEOUT=0;

static inline void max(void *in1, void *in2, void* out, DataType type);
static inline void min(void *in1, void *in2, void* out, DataType type);
static inline void div(void *in1, int in2, void* out, DataType type);
static inline void sum(void *in1, void *in2, void* out, DataType type);

/*=====================================================*
 *    Default Transformation Filter Definitions        *
 *=====================================================*/
void tfilter_IntSum( std::vector < Packet >&packets_in,
                   std::vector < Packet >&packets_out,
                   void ** /* client data */ )
{
    int sum = 0;
    
    for( unsigned int i = 0; i < packets_in.size( ); i++ ) {
        Packet cur_packet = packets_in[i];
        sum += cur_packet[0].get_int32_t( );
    }
    
    Packet new_packet( packets_in[0].get_Tag( ),
                       packets_in[0].get_StreamId( ),
                       "%d", sum );
    packets_out.push_back( new_packet );
}

void tfilter_Sum( std::vector < Packet >&packets_in,
	       std::vector < Packet >&packets_out,
	       void ** /* client data */ )
{
    char result[8]; //ptr to 8 bytes
    std::string format_string;
    DataType type=UNKNOWN_T;

    memset(result, 0, 8); //zeroing the result buf
    for( unsigned int i = 0; i < packets_in.size( ); i++ ) {
        Packet cur_packet = packets_in[i];

        assert( strcmp(cur_packet.get_FormatString(),
                       packets_in[0].get_FormatString()) == 0 );

        if( i == 0 ){ //1st time thru
            //hack to get pass "%" in arg to fmt2type()
            type = Fmt2Type( (cur_packet.get_FormatString()+1) );
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
                fprintf(stderr, "packet_type: %d (%s)\n", type,
                        cur_packet.get_FormatString() );
                return;
            }
        }
        sum( result, &(cur_packet[0].val.p), result, type );
    }
    
    if( type == FLOAT_T ){
        //TODO: not sure why float needs special casing, but it does
        float tmp = *((float*)result);
        Packet new_packet( packets_in[0].get_StreamId( ),
                           packets_in[0].get_Tag( ),
                           format_string.c_str(), tmp );
        packets_out.push_back( new_packet );
    }
    else{
        Packet new_packet( packets_in[0].get_StreamId( ),
                           packets_in[0].get_Tag( ),
                           format_string.c_str(), result );
        packets_out.push_back( new_packet );
    }
}

void tfilter_Max( std::vector < Packet >&packets_in,
	       std::vector < Packet >&packets_out,
	       void ** /* client data */ )
{
    char result[8]; //ptr to 8 bytes
    std::string format_string;
    DataType type=UNKNOWN_T;

    memset(result, 0, 8); //zeroing the result buf
    for( unsigned int i = 0; i < packets_in.size( ); i++ ) {
        Packet cur_packet = packets_in[i];
        assert( strcmp(cur_packet.get_FormatString(),
                       packets_in[0].get_FormatString()) == 0 );
	if( i == 0 ){ //1st time thru
	    type = Fmt2Type( cur_packet.get_FormatString( ) );
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
	      return;
	    }
	}
	max( result, &(cur_packet[0].val.p), result, type );
    }
    
    Packet new_packet( packets_in[0].get_StreamId( ),
                       packets_in[0].get_Tag( ),
                       format_string.c_str(), result );
    packets_out.push_back( new_packet );
}

void tfilter_Min( std::vector < Packet >&packets_in,
	       std::vector < Packet >&packets_out,
	       void ** /* client data */ )
{
    char result[8]; //ptr to 8 bytes
    std::string format_string;
    DataType type=UNKNOWN_T;

    memset(result, 0, 8); //zeroing the result buf
    for( unsigned int i = 0; i < packets_in.size( ); i++ ) {
        Packet cur_packet = packets_in[i];
        assert( strcmp(cur_packet.get_FormatString(),
                       packets_in[0].get_FormatString()) == 0 );
	if( i == 0 ){ //1st time thru
	    type = Fmt2Type( cur_packet.get_FormatString( ) );
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
	      return;
	    }
	}
	min( result, &(cur_packet[0].val.p), result, type );
    }
    
    Packet new_packet( packets_in[0].get_StreamId( ),
                       packets_in[0].get_Tag( ),
                       format_string.c_str(), result );
    packets_out.push_back( new_packet );
}

void tfilter_Avg( std::vector < Packet >&packets_in,
	       std::vector < Packet >&packets_out,
	       void ** /* client data */ )
{
    char result[8]; //ptr to 8 bytes
    int num_results=0;
    std::string format_string;
    DataType type=UNKNOWN_T;

    memset(result, 0, 8); //zeroing the result buf
    for( unsigned int i = 0; i < packets_in.size( ); i++ ) {
        Packet cur_packet = packets_in[i];
        assert( strcmp(cur_packet.get_FormatString(),
                       packets_in[0].get_FormatString()) == 0 );
	if( i == 0 ){ //1st time thru
	    type = Fmt2Type( cur_packet.get_FormatString( ) );
	    switch(type){
	    case CHAR_T:
		format_string="%c %d";
		break;
	    case UCHAR_T:
	      format_string="%uc %d";
	      break;
	    case INT16_T:
	      format_string="%hd %d";
	      break;
	    case UINT16_T:
	      format_string="%uhd %d";
	      break;
	    case INT32_T:
	      format_string="%d %d";
	      break;
	    case UINT32_T:
	      format_string="%ud %d";
	      break;
	    case INT64_T:
	      format_string="%ld %d";
	      break;
	    case UINT64_T:
	      format_string="%uld %d";
	      break;
	    case FLOAT_T:
	      format_string="%f %d";
	      break;
	    case DOUBLE_T:
	      format_string="%lf %d";
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
	      return;
	    }
	}
	sum( result, &(cur_packet[0].val.p), result, type );
	num_results += cur_packet[1].val.d;
    }
    
    div( result, num_results, result, type );
    Packet new_packet( packets_in[0].get_StreamId( ),
                       packets_in[0].get_Tag( ),
                       format_string.c_str(), result, num_results );
    packets_out.push_back( new_packet );
}



void tfilter_ArrayConcat( std::vector < Packet >&packets_in,
			    std::vector < Packet >&packets_out,
			    void ** /* client data */ )
{
    int result_array_size=0;
    char * result_array=NULL;
    int data_size=0;
    std::string format_string;
 
    for( unsigned int i = 0; i < packets_in.size( ); i++ ) {
        Packet cur_packet = packets_in[i];
        assert( strcmp(cur_packet.get_FormatString(),
                       packets_in[0].get_FormatString()) == 0 );
	if( i == 0 ){ //1st time thru
	    DataType type = Fmt2Type( cur_packet.get_FormatString( ) );
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
	      return;
	    }
	}
	result_array_size += cur_packet[0].array_len;
    }

    result_array = (char *) malloc( result_array_size * data_size );

    int pos = 0;
    for( unsigned int i = 0; i < packets_in.size(); i++ ) {
        Packet cur_packet = packets_in[i];
        memcpy( result_array + pos,
		cur_packet[0].val.p,
		cur_packet[0].array_len * data_size );
	pos += ( cur_packet[0].array_len * data_size );
    }
    
    Packet new_packet( packets_in[0].get_StreamId( ),
                       packets_in[0].get_Tag( ),
                       format_string.c_str(), result_array, result_array_size );
    packets_out.push_back( new_packet );
}

void tfilter_IntEqClass( std::vector < Packet >&packets_in,
		      std::vector < Packet >&packets_out,
		      void ** /* client data */ )
{
    DataType type;
    unsigned int array_len0, array_len1, array_len2;
    std::map < unsigned int, std::vector < unsigned int > >classes;

    // find equivalence classes across our input 
    for( unsigned int i = 0; i < packets_in.size(); i++ ) {
        Packet cur_packet = packets_in[i];
        unsigned int *vals = ( unsigned int * )
	  ( cur_packet[0].get_array(&type, &array_len0) );
	unsigned int *memcnts = ( unsigned int * )
	  ( cur_packet[1].get_array(&type, &array_len1) );
	unsigned int *mems = ( unsigned int * )
	  ( cur_packet[2].get_array(&type, &array_len2) );

	assert( array_len0 == array_len1 );
	unsigned int curClassMemIdx = 0;
	for( unsigned int j = 0; j < array_len0; j++ ) {
	    mrn_printf( 3, MCFL, stderr,
			"\tclass %d: val = %u, nMems = %u, mems = ", j,
			vals[j], memcnts[j] );

	    // update the members for the current class
	    for( unsigned int k = 0; k < memcnts[j]; k++ ) {
	        mrn_printf( 3, MCFL, stderr, "%d ",
			    mems[curClassMemIdx + k] );
		classes[vals[j]].push_back( mems[curClassMemIdx + k] );
	    }
	    mrn_printf( 3, MCFL, stderr, "\n" );
	    curClassMemIdx += memcnts[j];
	}
    }

    // build data structures for the output 
    unsigned int *values = new unsigned int[classes.size(  )];
    unsigned int *memcnts = new unsigned int[classes.size(  )];
    unsigned int nMems = 0;
    unsigned int curIdx = 0;
    for( std::map < unsigned int,
	   std::vector < unsigned int > >::iterator iter = classes.begin(  );
	 iter != classes.end(  ); iter++ ) {
        values[curIdx] = iter->first;
	memcnts[curIdx] = ( iter->second ).size(  );
	nMems += memcnts[curIdx];
	curIdx++;
    }
    unsigned int *mems = new unsigned int[nMems];
    unsigned int curMemIdx = 0;
    unsigned int curClassIdx = 0;
    for( std::map < unsigned int,
	   std::vector < unsigned int > >::iterator iter = classes.begin(  );
	 iter != classes.end(  ); iter++ ) {
        for( unsigned int j = 0; j < memcnts[curClassIdx]; j++ ) {
	  mems[curMemIdx] = ( iter->second )[j];
	  curMemIdx++;
	}
	curClassIdx++;
    }
    
    Packet new_packet( packets_in[0].get_StreamId( ),
		       packets_in[0].get_Tag( ),
		       "%aud %aud %aud",
		       values, classes.size( ),
		       memcnts, classes.size( ),
		       mems, nMems );
    packets_out.push_back( new_packet );

    mrn_printf( 3, MCFL, stderr, "tfilter_IntEqClass: returning\n" );
    unsigned int curMem = 0;
    for( unsigned int i = 0; i < classes.size(  ); i++ ) {
        mrn_printf( 3, MCFL, stderr,
		    "\tclass %d: val = %u, nMems = %u, mems = ", i,
		    ( ( unsigned int * )( packets_out[0][0].val.p) )[i],
		    ( ( unsigned int * )( packets_out[0][1].val.p) )[i] );
	for( unsigned int j = 0;
	     j < ( ( unsigned int * )( packets_out[0][1].val.p ) )[i];
	     j++ ) {
	    mrn_printf( 3, MCFL, stderr, "%d ",
			( ( unsigned int * )( packets_out[0][2].val.
					      p ) )[curMem] );
	    curMem++;
	}
	mrn_printf( 3, MCFL, stderr, "\n" );
    }
}

/*============================================*
 *    Default SyncFilter Definitions        *
 *============================================*/

void sfilter_WaitForAll( std::vector < Packet >&packets_in,
                         std::vector < Packet >&packets_out,
                         std::list < RemoteNode * >&downstream_nodes,
                         void **local_storage )
{
    std::map < RemoteNode *, std::list < Packet >*>*PacketListByNode;
    
    mrn_printf( 3, MCFL, stderr, "In sync_WaitForAll()\n" );
    if( *local_storage == NULL ) {
        PacketListByNode =
            new std::map < RemoteNode *, std::list < Packet >*>;
        *local_storage = PacketListByNode;

        std::list < RemoteNode * >::iterator iter;
        mrn_printf( 3, MCFL, stderr,
                    "Creating Map of %d downstream_nodes\n",
                    downstream_nodes.size(  ) );
        for( iter = downstream_nodes.begin(  );
             iter != downstream_nodes.end(  ); iter++ ) {
            ( *PacketListByNode )[( *iter )] = new std::list < Packet >;
        }
    }
    else {
        PacketListByNode =
            ( std::map < RemoteNode *, std::list < Packet >*>* )
            * local_storage;
    }

    //place all incoming packets in appropriate list
    mrn_printf( 3, MCFL, stderr, "Placing %d incoming packets\n",
                packets_in.size(  ) );
    for( unsigned int i=0; i<packets_in.size(); i++ ){
        ( ( *PacketListByNode )[ packets_in[i].get_InletNode() ] )->
            push_back(  packets_in[i] );
    }
    packets_in.clear(  );

    //check to see if all lists have at least one packet, "a wave"
    mrn_printf( 3, MCFL, stderr,
                "Checking if all downstream_nodes are ready ..." );
    std::map < RemoteNode *, std::list < Packet >*>::iterator iter2;
    for( iter2 = PacketListByNode->begin(  );
         iter2 != PacketListByNode->end(  ); iter2++ ) {
        if( ( ( *iter2 ).second )->size(  ) == 0 ) {
            //all lists not ready!
            mrn_printf( 3, 0, 0, stderr, "no!\n" );
            return;
        }
    }
    mrn_printf( 3, 0, 0, stderr, "yes!\n" );

    mrn_printf( 3, MCFL, stderr, "Placing outgoing packets\n" );
    //if we get here, all lists ready. push front of all lists onto "packets_out"
    for( iter2 = PacketListByNode->begin(  );
         iter2 != PacketListByNode->end(  ); iter2++ ) {
        packets_out.push_back( ( ( *iter2 ).second )->front(  ) );
        ( ( *iter2 ).second )->pop_front(  );
    }
    mrn_printf( 3, MCFL, stderr, "Returning %d outgoing packets\n",
                packets_out.size(  ) );
}

void sfilter_TimeOut( std::vector < Packet >&packets_in,
                      std::vector < Packet >&packets_out,
                      std::list < RemoteNode * >&, void **local_storage )
{
}

static inline void sum(void *in1, void *in2, void* out, DataType type)
{
  switch (type){
  case CHAR_T:
    *( (char*) out ) = *((char*)in1) + *((char*)in2);
    break;
  case UCHAR_T:
    *( (uchar_t*) out ) = *((uchar_t*)in1) + *((uchar_t*)in2);
    break;
  case INT16_T:
    *( (int16_t*) out ) = *((int16_t*)in1) + *((int16_t*)in2);
    break;
  case UINT16_T:
    *( (uint16_t*) out ) = *((uint16_t*)in1) + *((uint16_t*)in2);
    break;
  case INT32_T:
    *( (int32_t*) out ) = *((int32_t*)in1) + *((int32_t*)in2);
    break;
  case UINT32_T:
    *( (uint32_t*) out ) = *((uint32_t*)in1) + *((uint32_t*)in2);
    break;
  case INT64_T:
    *( (int64_t*) out ) = *((int64_t*)in1) + *((int64_t*)in2);
    break;
  case UINT64_T:
    *( (uint64_t*) out ) = *((uint64_t*)in1) + *((uint64_t*)in2);
    break;
  case FLOAT_T:
    *( (float*) out ) = *((float*)in1) + *((float*)in2);
    break;
  case DOUBLE_T:
    *( (double*) out ) = *((double*)in1) + *((double*)in2);
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
    assert(0);
  }
}

static inline void div(void *in1, int in2, void* out, DataType type)
{
  switch (type){
  case CHAR_T:
    *( (char*) out ) = *((char*)in1) / in2;
    break;
  case UCHAR_T:
    *( (uchar_t*) out ) = *((uchar_t*)in1) / in2;
    break;
  case INT16_T:
    *( (int16_t*) out ) = *((int16_t*)in1) / in2;
    break;
  case UINT16_T:
    *( (uint16_t*) out ) = *((uint16_t*)in1) / in2;
    break;
  case INT32_T:
    *( (int32_t*) out ) = *((int32_t*)in1) / in2;
    break;
  case UINT32_T:
    *( (uint32_t*) out ) = *((uint32_t*)in1) / in2;
    break;
  case INT64_T:
    *( (int64_t*) out ) = *((int64_t*)in1) / (int64_t)in2;
    break;
  case UINT64_T:
    *( (uint64_t*) out ) = *((uint64_t*)in1) / (int64_t)in2;
    break;
  case FLOAT_T:
    *( (float*) out ) = *((float*)in1) / (float)in2;
    break;
  case DOUBLE_T:
    *( (double*) out ) = *((double*)in1) / (double)in2;
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
    assert(0);
  }
}

static inline void min(void *in1, void *in2, void* out, DataType type)
{
  switch (type){
  case CHAR_T:
    *( (char*) out ) = ( ( *((char*)in1) < *((char*)in2) ) ?
			 *((char*)in1) : *((char*)in2) );
    break;
  case UCHAR_T:
    *( (uchar_t*) out ) = ( ( *((uchar_t*)in1) < *((uchar_t*)in2) ) ?
			    *((uchar_t*)in1) : *((uchar_t*)in2) );
    break;
  case INT16_T:
    *( (int16_t*) out ) = ( ( *((int16_t*)in1) < *((int16_t*)in2) ) ?
			    *((int16_t*)in1) : *((int16_t*)in2) );
    break;
  case UINT16_T:
    *( (uint16_t*) out ) = ( ( *((uint16_t*)in1) < *((uint16_t*)in2) ) ?
			     *((uint16_t*)in1) : *((uint16_t*)in2) );
    break;
  case INT32_T:
    *( (int32_t*) out ) = ( ( *((int32_t*)in1) < *((int32_t*)in2) ) ?
			    *((int32_t*)in1) : *((int32_t*)in2) );
    break;
  case UINT32_T:
    *( (uint32_t*) out ) = ( ( *((uint32_t*)in1) < *((uint32_t*)in2) ) ?
			     *((uint32_t*)in1) : *((uint32_t*)in2) );
    break;
  case INT64_T:
    *( (int64_t*) out ) = ( ( *((int64_t*)in1) < *((int64_t*)in2) ) ?
			    *((int64_t*)in1) : *((int64_t*)in2) );
    break;
  case UINT64_T:
    *( (uint64_t*) out ) = ( ( *((uint64_t*)in1) < *((uint64_t*)in2) ) ?
			     *((uint64_t*)in1) : *((uint64_t*)in2) );
    break;
  case FLOAT_T:
    *( (float*) out ) = ( ( *((float*)in1) < *((float*)in2) ) ?
			  *((float*)in1) : *((float*)in2) );
    break;
  case DOUBLE_T:
    *( (double*) out ) = ( ( *((double*)in1) < *((double*)in2) ) ?
			   *((double*)in1) : *((double*)in2) );
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
    assert(0);
  }
}

static inline void max(void *in1, void *in2, void* out, DataType type)
{
  switch (type){
  case CHAR_T:
    *( (char*) out ) = ( ( *((char*)in1) > *((char*)in2) ) ?
			 *((char*)in1) : *((char*)in2) );
    break;
  case UCHAR_T:
    *( (uchar_t*) out ) = ( ( *((uchar_t*)in1) > *((uchar_t*)in2) ) ?
			    *((uchar_t*)in1) : *((uchar_t*)in2) );
    break;
  case INT16_T:
    *( (int16_t*) out ) = ( ( *((int16_t*)in1) > *((int16_t*)in2) ) ?
			    *((int16_t*)in1) : *((int16_t*)in2) );
    break;
  case UINT16_T:
    *( (uint16_t*) out ) = ( ( *((uint16_t*)in1) > *((uint16_t*)in2) ) ?
			     *((uint16_t*)in1) : *((uint16_t*)in2) );
    break;
  case INT32_T:
    *( (int32_t*) out ) = ( ( *((int32_t*)in1) > *((int32_t*)in2) ) ?
			    *((int32_t*)in1) : *((int32_t*)in2) );
    break;
  case UINT32_T:
    *( (uint32_t*) out ) = ( ( *((uint32_t*)in1) > *((uint32_t*)in2) ) ?
			     *((uint32_t*)in1) : *((uint32_t*)in2) );
    break;
  case INT64_T:
    *( (int64_t*) out ) = ( ( *((int64_t*)in1) > *((int64_t*)in2) ) ?
			    *((int64_t*)in1) : *((int64_t*)in2) );
    break;
  case UINT64_T:
    *( (uint64_t*) out ) = ( ( *((uint64_t*)in1) > *((uint64_t*)in2) ) ?
			     *((uint64_t*)in1) : *((uint64_t*)in2) );
    break;
  case FLOAT_T:
    *( (float*) out ) = ( ( *((float*)in1) > *((float*)in2) ) ?
			  *((float*)in1) : *((float*)in2) );
    break;
  case DOUBLE_T:
    *( (double*) out ) = ( ( *((double*)in1) > *((double*)in2) ) ?
			   *((double*)in1) : *((double*)in2) );
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
    assert(0);
  }
}

} /* namespace MRN */
