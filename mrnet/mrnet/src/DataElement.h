/****************************************************************************
 * Copyright © 2003-2007 Dorian C. Arnold, Philip C. Roth, Barton P. Miller *
 *                  Detailed MRNet usage rights in "LICENSE" file.          *
 ****************************************************************************/

#if !defined(dataelement_h)
#define dataelement_h 1

#include "mrnet/Types.h"

namespace MRN {

typedef union{
    char c;
    unsigned char uc;
    int16_t hd;
    uint16_t uhd;
    int32_t d;
    uint32_t ud;
    int64_t ld;
    uint64_t uld;
    float f;
    double lf;
    void * p; // May need to be allocated by pdr routine
} DataValue;

typedef enum {
    UNKNOWN_T, CHAR_T, UCHAR_T,
    CHAR_ARRAY_T, UCHAR_ARRAY_T,
    STRING_T, STRING_ARRAY_T,
    INT16_T, UINT16_T,
    INT16_ARRAY_T, UINT16_ARRAY_T,
    INT32_T, UINT32_T,
    INT32_ARRAY_T, UINT32_ARRAY_T,
    INT64_T, UINT64_T,
    INT64_ARRAY_T, UINT64_ARRAY_T,
    FLOAT_T, DOUBLE_T,
    FLOAT_ARRAY_T, DOUBLE_ARRAY_T
} DataType;

class DataElement{
    friend class Packet;
 public:
    DataValue val;
    DataElement( void );
    ~DataElement();

    void set_DestroyData( bool d );
    DataType get_Type( void ) const;

    char get_char( void ) const;
    void set_char( char c );

    unsigned char get_uchar( void ) const;
    void set_uchar( unsigned char uc );

    int16_t get_int16_t( void ) const;
    void set_int16_t( int16_t hd );

    uint16_t get_uint16_t( void ) const;
    void set_uint16_t( uint16_t uhd );

    int32_t get_int32_t( void ) const;
    void set_int32_t( int32_t d );

    uint32_t get_uint32_t( void ) const;
    void set_uint32_t( uint32_t ud );

    int64_t get_int64_t( void ) const;
    void set_int64_t( int64_t ld );

    uint64_t get_uint64_t( void ) const;
    void set_uint64_t( uint64_t uld );

    float get_float( void ) const;
    void set_float( float f );

    double get_double( void ) const;
    void set_double( double lf );

    const char * get_string( void ) const;
    void set_string( const char *p );

    const void * get_array( DataType *t, uint32_t *len ) const;
    void set_array( const void *p, DataType t, uint32_t len );

 private:
    DataType type;
    uint32_t array_len;
    bool destroy_data;
};

DataType Fmt2Type(const char * cur_fmt);

} /* namespace MRN */

#endif /* dataelement_h */
